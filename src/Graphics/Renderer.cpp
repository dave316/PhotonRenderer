#include "Renderer.h"

#include <Graphics/Framebuffer.h>
#include <Graphics/Primitives.h>
#include <Graphics/Shader.h>

#include <IO/GLTFImporter.h>
#include <IO/AssimpImporter.h>
#include <IO/ImageLoader.h>
#include <IO/ShaderLoader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace json = rapidjson;

void extern debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		std::cout << std::hex << std::endl;
		std::cout << "OpenGL error: type=" << type << " severity: " << severity << " message: " << message << std::endl;
	}
}

Renderer::Renderer(unsigned int width, unsigned int height) :
	width(width), height(height)
{
}

Renderer::~Renderer()
{
	for (auto&& [name, entity] : rootEntitis)
	{
		auto animators = entity->getComponentsInChildren<Animator>();
		for (auto& a : animators)
			a->clear();
	}
}

bool Renderer::init()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(0.8f, 0.77f, 0.54f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glViewport(0, 0, width, height);
	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(debugCallback, 0);

	initShader();
	initEnvMaps();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	std::string assetPath = "../../../../assets";
	std::string gltfPath = assetPath + "/glTF-Sample-Models/2.0";
	std::string name = "GlamVelvetSofa";
	loadModel(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");
	name = "IridescentDishWithOlives";
	loadModel(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");
	rootEntitis[name]->getComponent<Transform>()->setPosition(glm::vec3(0, 0.5, 2));

	// TODO: generate these with shaders
	lutSheenE = IO::loadTexture16(assetPath + "/lut_sheen_E.png", false);	

	//IO::AssimpImporter assImporter;
	//auto model = assImporter.importModel(assetPath + "/plane.obj");
	////model->getComponent<Transform>()->setPosition(glm::vec3(0,-2.5, 0));
	//model->getComponent<Transform>()->setScale(glm::vec3(20.0f));
	//if (model != nullptr)
	//	rootEntitis.insert(std::make_pair("Aplane", model));
	//assImporter.clear();

	//loadGLTFModels(gltfPath);
	//loadAssimpModels(path);

	for (auto [_, e] : rootEntitis)
	{
		auto animator = e->getComponent<Animator>();
		if (animator)
		{
			//animator->switchAnimation(1);
			animator->play();
		}			
	}

	cameraUBO.bindBase(0);

	initLights();
	initFBOs();
	initFonts();

	return true;
}

void Renderer::initEnvMaps()
{
	// TODO: put env maps in its own class & remove baking from renderer
	auto pano2cmShader = shaders["PanoToCubeMap"];
	auto irradianceShader = shaders["IBLDiffuseIrradiance"];
	auto specularShader = shaders["IBLSpecular"];
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

	unitCube = Primitives::createCube(glm::vec3(0), 1.0f);

	std::string assetPath = "../../../../assets";
	auto pano = IO::loadTextureHDR(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/Newport_Loft/Newport_Loft_Ref.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/directional.hdr");

	if (pano == nullptr)
	{
		std::cout << "no panorama loaded, IBL deactivated" << std::endl;
		useIBL = false;
		return;
	}

	glm::vec3 position = glm::vec3(0);
	std::vector<glm::mat4> VP;

	glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	pano2cmShader->setUniform("M", glm::mat4(1.0f));
	pano2cmShader->setUniform("VP[0]", VP);
	pano2cmShader->setUniform("panorama", 0);
	pano2cmShader->use();

	glDisable(GL_DEPTH_TEST);
	{
		int size = 1024;
		cubeMap = TextureCubeMap::create(size, size, GL::RGB32F);
		cubeMap->generateMipmaps();
		cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto envFBO = Framebuffer::create(size, size);
		envFBO->addRenderTexture(GL::COLOR0, cubeMap);
		envFBO->checkStatus();
		envFBO->begin();
		pano->use(0);
		unitCube->draw();
		envFBO->end();

		cubeMap->generateMipmaps();
	}

	irradianceShader->setUniform("VP[0]", VP);
	irradianceShader->setUniform("environmentMap", 0);
	irradianceShader->use();

	{
		int size = 32;
		irradianceMap = TextureCubeMap::create(size, size, GL::RGB32F);
		auto irrFBO = Framebuffer::create(size, size);
		irrFBO->addRenderTexture(GL::COLOR0, irradianceMap);
		irrFBO->checkStatus();
		irrFBO->begin();
		cubeMap->use(0);
		unitCube->draw();
		irrFBO->end();
	}

	specularShader->setUniform("VP[0]", VP);
	specularShader->setUniform("environmentMap", 0);
	specularShader->setUniform("filterIndex", 0);
	specularShader->use();

	{
		int size = 256;
		specularMapGGX = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMapGGX->generateMipmaps();
		specularMapGGX->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 8;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMapGGX, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	specularShader->setUniform("VP[0]", VP);
	specularShader->setUniform("environmentMap", 0);
	specularShader->setUniform("filterIndex", 1);
	specularShader->use();

	{
		int size = 256;
		specularMapCharlie = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMapCharlie->generateMipmaps();
		specularMapCharlie->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 5;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMapCharlie, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	glEnable(GL_DEPTH_TEST);

	screenQuad = Primitives::createQuad(glm::vec3(0.0f), 2.0f);

	{
		int size = 512;
		ggxLUT = Texture2D::create(size, size, GL::RGB16F);
		ggxLUT->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);

		integrateBRDFShader->use();
		integrateBRDFShader->setUniform("filterIndex", 0);
		auto brdfFBO = Framebuffer::create(size, size);
		brdfFBO->addRenderTexture(GL::COLOR0, ggxLUT);
		brdfFBO->begin();
		screenQuad->draw();
		brdfFBO->end();
	}

	{
		int size = 512;
		charlieLUT = Texture2D::create(size, size, GL::RGB16F);
		charlieLUT->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);

		integrateBRDFShader->use();
		integrateBRDFShader->setUniform("filterIndex", 1);
		auto brdfFBO = Framebuffer::create(size, size);
		brdfFBO->addRenderTexture(GL::COLOR0, charlieLUT);
		brdfFBO->begin();
		screenQuad->draw();
		brdfFBO->end();
	}

	glViewport(0, 0, width, height);
}

void Renderer::initFBOs()
{
	int w = width;
	int h = height;
	screenTex = Texture2D::create(w, h, GL::RGBA8);
	screenTex->generateMipmaps();
	screenTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	screenFBO = Framebuffer::create(w, h);
	screenFBO->addRenderTexture(GL::COLOR0, screenTex);
	screenFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);

	for (auto it : lights)
	{
		auto light = it.second;

		glm::vec3 pos = light->getPosition();
		glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 25.0f);
		std::vector<glm::mat4> VP;
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		views.push_back(VP);

		const unsigned int size = 4096;
		auto shadowMap = TextureCubeMap::create(size, size, GL::DEPTH24);
		shadowMap->setCompareMode();

		auto shadowFBO = Framebuffer::create(size, size);
		shadowFBO->addRenderTexture(GL::DEPTH, shadowMap);
		shadowFBO->checkStatus();
		shadowFBOs.push_back(shadowFBO);
	}
}

void Renderer::initLights()
{
	//std::default_random_engine gen;
	//std::uniform_real_distribution<float> distX(-2.0f, 2.0f);
	//std::uniform_real_distribution<float> distY(2.0f, 2.0f);
	//std::uniform_real_distribution<float> distZ(-2.0f, 2.0f);

	//int numLights = 0;
	//for (int i = 0; i < numLights; i++)
	//{
	//	float x = distX(gen);
	//	float y = distY(gen);
	//	float z = distZ(gen);
	//	//auto light = Light::create(glm::vec3(x, y, z), glm::vec3(0.25f, 0.61f, 1.0f));
	//	auto light = Light::create(glm::vec3(0, 5, 5), glm::vec3(1.0f), 1.0f, 50.0f);
	//	std::string lightName = "light_" + std::to_string(i);
	//	lights.insert(std::make_pair(lightName, light));
	//}

	//auto light = Light::create(LightType::POINT, glm::vec3(0.85f, 0.78f, 0.65f), 10, 10);
	//light->setPostion(glm::vec3(0, 2, 1));
	//lights.insert(std::make_pair("light", light));

	//auto spotLight = Light::create(LightType::SPOT, glm::vec3(0.85f, 0.78f, 0.65f), 100, 100);
	//spotLight->setPostion(glm::vec3(0, 0.1, 0));
	//spotLight->setDirection(glm::vec3(-1, 0, 0));
	//spotLight->setConeAngles(0.0, 0.5);
	//lights.insert(std::make_pair("light", spotLight));

	int i = 0;
	std::vector<Light::UniformData> lightData(lights.size()); 
	for (auto it : lights)
	{
		it.second->writeUniformData(lightData[i]);
		i++;
	}

	lightUBO.upload(lightData, GL_DYNAMIC_DRAW);
	lightUBO.bindBase(1);

	defaultShader->setUniform("numLights", (int)lights.size());
}

void Renderer::initShader()
{
	std::string shaderPath = "../../../../src/Shaders";
	auto shaderList = IO::loadShadersFromPath(shaderPath);
	for (auto s : shaderList)
		shaders.insert(std::pair(s->getName(), s));
	defaultShader = shaders["Default"];
	defaultShader->setUniform("material.baseColorFactor", glm::vec4(1.0f));
	defaultShader->setUniform("material.alphaCutOff", 0.0f);
	defaultShader->setUniform("material.baseColorTex", 0);
	defaultShader->setUniform("material.pbrTex", 1);
	defaultShader->setUniform("material.normalTex", 2);
	defaultShader->setUniform("material.occlusionTex", 3);
	defaultShader->setUniform("material.emissiveTex", 4);
	defaultShader->setUniform("material.sheenColortex", 5);
	defaultShader->setUniform("material.sheenRoughtex", 6);
	defaultShader->setUniform("material.clearCoatTex", 7);
	defaultShader->setUniform("material.clearCoatRoughTex", 8);
	defaultShader->setUniform("material.clearCoatNormalTex", 9);
	defaultShader->setUniform("material.transmissionTex", 10);
	defaultShader->setUniform("material.thicknessTex", 11);
	defaultShader->setUniform("material.specularTex", 12);
	defaultShader->setUniform("material.specularColorTex", 13);
	defaultShader->setUniform("transmissionTex", 14);
	defaultShader->setUniform("irradianceMap", 15);
	defaultShader->setUniform("specularMapGGX", 16);
	defaultShader->setUniform("specularMapCharlie", 17);
	defaultShader->setUniform("sheenLUTE", 18);
	defaultShader->setUniform("ggxLUT", 19);
	defaultShader->setUniform("charlieLUT", 20);

	std::vector<int> units;
	for (int i = 10; i < 15; i++)
		units.push_back(i);
	defaultShader->setUniform("shadowMaps[0]", units);
	defaultShader->setUniform("numLights", (int)lights.size());

	skyboxShader = shaders["Skybox"];
	skyboxShader->setUniform("envMap", 0);

	textShader = shaders["Text"];
	textShader->setUniform("P", glm::ortho(0.0f, (float)width, 0.0f, (float)height));
	textShader->setUniform("atlas", 0);

	auto unlitShader = shaders["Unlit"];
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("tex", 0);
}

void Renderer::initFonts()
{
	// TODO: better error handling...

	FT_Library ft;
	FT_Init_FreeType(&ft);

	std::vector<std::string> fileNames;
	fileNames.push_back("../../../../assets/fonts/arial.ttf");
	fileNames.push_back("../../../../assets/fonts/arialbd.ttf");
	fileNames.push_back("../../../../assets/fonts/ariali.ttf");
	fileNames.push_back("../../../../assets/fonts/arialbi.ttf");
	auto font = Font::Ptr(new Font(ft, fileNames, 11, 60));
	if (font->isLoaded())
	{
		std::string resStr = "Resolution: " + std::to_string(width) + "x" + std::to_string(height);
		std::string nameStr = "Animation index: 0";
		std::string lightStr = "Lights: " + std::to_string(lights.size());

		glm::vec3 textColor(0.2f, 0.5f, 0.9f);
		int fontSize = 20;

		Text2D::Ptr text1(new Text2D(font, resStr, textColor, glm::vec2(20.0f, 80.0f), fontSize));
		Text2D::Ptr text2(new Text2D(font, nameStr, textColor, glm::vec2(20.0f, 60.0f), fontSize));
		Text2D::Ptr text3(new Text2D(font, lightStr, textColor, glm::vec2(20.0f, 40.0f), fontSize));
		Text2D::Ptr text4(new Text2D(font, "Font: Arial, Size " + std::to_string(fontSize), textColor, glm::vec2(20.0f), fontSize));
		texts.push_back(text1);
		texts.push_back(text2);
		texts.push_back(text3);
		texts.push_back(text4);
	}
	fonts.push_back(font);

	FT_Done_FreeType(ft);
}

void Renderer::updateShadows()
{
	auto depthShader = shaders["DepthCubemap"];

	// should be put in light class together with the updating
	for (int i = 0; i < lights.size(); i++)
	{
		shadowFBOs[i]->begin();
		glCullFace(GL_FRONT);
		depthShader->use();
		depthShader->setUniform("lightIndex", i);
		depthShader->setUniform("VP[0]", views[i]);
		renderScene(depthShader, true);
		shadowFBOs[i]->end();
		glCullFace(GL_BACK);
	}
}

void Renderer::loadGLTFModels(std::string path)
{
	std::ifstream file(path + "/model-index.json");
	std::stringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();

	json::Document doc;
	doc.Parse(content.c_str());

	//IO::ModelImporter importer;
	IO::GLTFImporter importer;
	int i = 0;
	std::vector<std::string> filenames;
	for (auto &el : doc.GetArray())
	{
		std::string name(el.FindMember("name")->value.GetString());
		auto it = el.FindMember("variants");
		if (it != el.MemberEnd())
		{
			auto it2 = it->value.FindMember("glTF");
			if (it2 != it->value.MemberEnd())
			{
				if (name.compare("TextureEncodingTest") != 0)
				{
					std::cout << "loading model " << name << "...";
					std::string fn = name + "/glTF/" + name + ".gltf";
					auto rootEntity = importer.importModel(path + "/" + fn);
					if (rootEntity != nullptr)
					{
						auto rootTransform = rootEntity->getComponent<Transform>();

						rootEntity->update(glm::mat4(1.0f));

						AABB aabb;
						auto meshEntities = rootEntity->getChildrenWithComponent<Renderable>();
						for (auto m : meshEntities)
						{
							auto r = m->getComponent<Renderable>();
							auto t = m->getComponent<Transform>();
							glm::mat4 M = t->getTransform();
							auto vertices = r->getVertices();
							for (auto& v : vertices)
							{
								glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0));
								aabb.expand(pos);
							}
						}

						glm::vec3 s = aabb.getSize();
						float scale = 1.0f / glm::max(glm::max(s.x, s.y), s.z);

						float x = i % 10 * 2;
						float y = i / 10 * 2;
						float z = 0;

						rootTransform->setPosition(glm::vec3(x, y, z));
						rootTransform->setScale(glm::vec3(scale));

						rootEntitis.insert(std::make_pair(name, rootEntity));
						//auto children = importer.getEntities();
						//entities.insert(entities.end(), children.begin(), children.end());
					}
					importer.clear();
					std::cout << "done!" << std::endl;
				}
			}
		}

		i++;
		//if (i == 10)
		//	break;
	}
}

void Renderer::loadAssimpModels(std::string path)
{
	std::ifstream file(path + "/model-index.json");
	std::stringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();

	json::Document doc;
	doc.Parse(content.c_str());

	//IO::ModelImporter importer;
	IO::GLTFImporter importer;
	int i = 0;
	std::vector<std::string> filenames;
	for (auto& el : doc.GetArray())
	{
		std::string name(el.FindMember("name")->value.GetString());
		auto it = el.FindMember("variants");
		if (it != el.MemberEnd())
		{
			auto it2 = it->value.FindMember("glTF");
			if (it2 != it->value.MemberEnd())
			{
				//if (name.compare("Sponza") == 0)
				{
					std::cout << "loading model " << name << "...";
					std::string fn = name + "/glTF/" + name + ".gltf";
					auto rootEntity = importer.importModel(path + "/" + fn);
					if (rootEntity != nullptr)
					{
						auto rootTransform = rootEntity->getComponent<Transform>();

						rootEntity->update(glm::mat4(1.0f));

						AABB aabb;
						auto meshEntities = rootEntity->getChildrenWithComponent<Renderable>();
						for (auto m : meshEntities)
						{
							auto r = m->getComponent<Renderable>();
							auto t = m->getComponent<Transform>();
							glm::mat4 M = t->getTransform();
							auto vertices = r->getVertices();
							for (auto& v : vertices)
							{
								glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0));
								aabb.expand(pos);
							}
						}

						glm::vec3 s = aabb.getSize();
						float scale = 1.0f / glm::max(glm::max(s.x, s.y), s.z);

						float x = i % 10 * 2;
						float y = i / 10 * 2;
						float z = 0;

						rootTransform->setPosition(glm::vec3(x, y, z));
						rootTransform->setScale(glm::vec3(scale));

						rootEntitis.insert(std::make_pair(name, rootEntity));
						//auto children = importer.getEntities();
						//entities.insert(entities.end(), children.begin(), children.end());
					}
					importer.clear();
					std::cout << "done!" << std::endl;
				}
			}
		}

		i++;
		//if (i == 10)
		//	break;
	}
}

void Renderer::loadModel(std::string name, std::string path)
{
	IO::GLTFImporter importer;
	auto rootEntity = importer.importModel(path);
	if (rootEntity)
	{
		auto rootTransform = rootEntity->getComponent<Transform>();
		rootEntity->update(glm::mat4(1.0f));
		rootEntitis.insert(std::make_pair(name, rootEntity));
	}

	auto modelLights = importer.getLights();
	for (int i = 0; i < modelLights.size(); i++)
	{
		std::string lightName = name + "_light_" + std::to_string(i);
		lights.insert(std::make_pair(lightName, modelLights[i]));
	}
	importer.clear();

	//AABB aabb;
	//auto meshEntities = rootEntity->getChildrenWithComponent<Renderable>();
	//for (auto m : meshEntities)
	//{
	//	auto r = m->getComponent<Renderable>();
	//	auto t = m->getComponent<Transform>();
	//	glm::mat4 M = t->getTransform();
	//	auto vertices = r->getVertices();
	//	for (auto& v : vertices)
	//	{
	//		glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0));
	//		aabb.expand(pos);
	//	}
	//}

	//glm::vec3 s = aabb.getSize();
	//float scale = 1.0f / glm::max(glm::max(s.x, s.y), s.z);
	//rootTransform->setScale(glm::vec3(scale));
}

void Renderer::updateAnimations(float dt)
{
	for (auto [name, rootEntity] : rootEntitis)
	{
		auto animators = rootEntity->getComponentsInChildren<Animator>();
		for (auto a : animators)
			a->update(dt);

		rootEntity->update(glm::mat4(1.0f));
	}
}

void Renderer::updateAnimationState(float dt)
{
	for (auto [name, e] : rootEntitis)
	{
		auto animator = e->getComponent<Animator>();
		if (animator && animator->isFinished())
			animator->play();
	}
}

void Renderer::updateCamera(Camera& camera)
{
	Camera::UniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::nextMaterial()
{
	static unsigned int materialIndex = 0;
	materialIndex = (++materialIndex) % 5;

	auto e = rootEntitis["GlamVelvetSofa"];
	if (e)
	{
		auto renderables = e->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
			r->switchMaterial(materialIndex);
	}
}

void Renderer::renderScene(Shader::Ptr shader, bool transmission)
{
	for (auto [name, e] : rootEntitis)
	{
		auto models = e->getChildrenWithComponent<Renderable>();
		auto animator = e->getComponent<Animator>();
		std::vector<Entity::Ptr> renderEntities;
		for (auto m : models)
			if(!m->getComponent<Renderable>()->useBlending() && 
			   !m->getComponent<Renderable>()->isTransmissive())
				renderEntities.push_back(m);
		if (transmission)
			for (auto m : models)
				if (m->getComponent<Renderable>()->isTransmissive())
					renderEntities.push_back(m);
		for (auto m : models)
			if (m->getComponent<Renderable>()->useBlending())
				renderEntities.push_back(m);

		for (auto m : renderEntities)
		{
			auto r = m->getComponent<Renderable>();
			auto t = m->getComponent<Transform>();

			if (r->useMorphTargets())
			{
				std::vector<float> weights;
				if (animator)
					weights = animator->getWeights();
				else
					weights = r->getWeights();
				shader->setUniform("numMorphTargets", (int)weights.size());
				shader->setUniform("morphWeights[0]", weights);
			}
			else
			{
				shader->setUniform("numMorphTargets", 0);
			}

			if (r->isSkinnedMesh())
			{
				auto nodes = animator->getNodes();
				Skin skin = r->getSkin();
				skin.computeJoints(nodes);
				auto boneTransforms = skin.getBoneTransform();
				auto normalTransforms = skin.getNormalTransform();
				shader->setUniform("hasAnimations", true);
				shader->setUniform("bones[0]", boneTransforms);
				shader->setUniform("normals[0]", normalTransforms);
			}
			else
			{
				shader->setUniform("hasAnimations", false);
			}

			t->setUniforms(shader);
			r->render(shader);
		}
	}
}

void Renderer::render()
{
	if (useIBL)
	{
		irradianceMap->use(15);
		specularMapGGX->use(16);
		specularMapCharlie->use(17);
		lutSheenE->use(18);
		ggxLUT->use(19);
		charlieLUT->use(20);
	}
	
	for (int i = 0; i < shadowFBOs.size(); i++)
		shadowFBOs[i]->useTexture(GL::DEPTH, 10 + i);

	// offscreen pass for transmission
	screenFBO->begin();
	if (useIBL)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", false);
		cubeMap->use(0);
		unitCube->draw();
		glCullFace(GL_BACK);
	}	

	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", false);
	renderScene(defaultShader, false);
	screenFBO->end();
	screenTex->generateMipmaps();
	screenTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

	// main render pass
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screenTex->use(14);
	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", true);
	renderScene(defaultShader, true);

	if (useIBL)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", true);
		cubeMap->use(0);
		unitCube->draw();
		glCullFace(GL_BACK);
	}
}

void Renderer::renderText()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader->use();
	for (auto& textMesh : texts)
		textMesh->draw(textShader);
	glDisable(GL_BLEND);
}
