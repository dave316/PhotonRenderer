#include "Renderer.h"

#include <Graphics/Framebuffer.h>
#include <Graphics/Primitives.h>
#include <Graphics/Shader.h>

#include <IO/GLTFImporter.h>
#include <IO/ImageLoader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <filesystem>

namespace json = rapidjson;

std::string loadTxtFile(const std::string& fileName)
{
	std::ifstream file(fileName);
	std::stringstream ss;

	if (file.is_open())
	{
		ss << file.rdbuf();
	}
	else
	{
		std::cout << "could not open file " << fileName << std::endl;
	}

	return ss.str();
}

Renderer::Renderer(unsigned int width, unsigned int height)
{
}

bool Renderer::init()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, 1920, 1080);

	initShader();
	initEnvMaps();

	std::string assetPath = "../assets";
	std::string path = assetPath + "/glTF-Sample-Models/2.0";
	std::string name = "EnvironmentTest";
	std::cout << "loading model " << name << std::endl;
	std::string fn = name + "/glTF/" + name + ".gltf";

	IO::GLTFImporter importer;
	//auto root = importer.importModel(path + "/" + fn);
	auto root = importer.importModel(assetPath + "/Adam/adamHead.gltf");
	//root->getComponent<Transform>()->setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0)));
	//root->getComponent<Transform>()->setScale(glm::vec3(0.01f));
	rootEntitis.push_back(root);
	entities = importer.getEntities();
	importer.clear();

	//loadGLTFModels(path);

	//root->getComponent<Transform>()->update(glm::mat4(1.0f));

	//IO::ModelImporter importer;
	//auto rootEntity = importer.importModel(path + "/" + fn);
	//rootEntitis.push_back(rootEntity);
	//entities = importer.getEntities();
	//importer.clear();
	 
	return true;
}

void Renderer::initShader()
{
	std::string shaderPath = "../src/Shaders";
	{
		defaultShader = Shader::create("Default");
		auto vsCode = loadTxtFile(shaderPath + "/Default.vert");
		auto fsCode = loadTxtFile(shaderPath + "/Default.frag");
		defaultShader->compile<GL::VertexShader>(vsCode);
		defaultShader->compile<GL::FragmentShader>(fsCode);
		defaultShader->link();

		defaultShader->setUniform("material.baseColorFactor", glm::vec4(1.0f));
		defaultShader->setUniform("material.alphaCutOff", 0.0f);
		defaultShader->setUniform("material.baseColorTex", 0);
		defaultShader->setUniform("material.pbrTex", 1);
		defaultShader->setUniform("material.normalTex", 2);
		defaultShader->setUniform("material.occlusionTex", 3);
		defaultShader->setUniform("material.emissiveTex", 4);

		defaultShader->setUniform("irradianceMap", 5);
		defaultShader->setUniform("specularMap", 6);
		defaultShader->setUniform("brdfLUT", 7);
	}

	{
		pano2cmShader = Shader::create("PanoToCubeMap");
		auto vsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.vert");
		auto gsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.geom");
		auto fsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.frag");
		pano2cmShader->compile<GL::VertexShader>(vsCode);
		pano2cmShader->compile<GL::GeometryShader>(gsCode);
		pano2cmShader->compile<GL::FragmentShader>(fsCode);
		pano2cmShader->link();

		pano2cmShader->setUniform("envMap", 0);
	}

	{
		irradianceShader = Shader::create("IBLDiffuseIrradiance");
		auto vsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.vert");
		auto gsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.geom");
		auto fsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.frag");
		irradianceShader->compile<GL::VertexShader>(vsCode);
		irradianceShader->compile<GL::GeometryShader>(gsCode);
		irradianceShader->compile<GL::FragmentShader>(fsCode);
		irradianceShader->link();
	}

	{
		specularShader = Shader::create("IBLSpecular");
		auto vsCode = loadTxtFile(shaderPath + "/IBLSpecular.vert");
		auto gsCode = loadTxtFile(shaderPath + "/IBLSpecular.geom");
		auto fsCode = loadTxtFile(shaderPath + "/IBLSpecular.frag");
		specularShader->compile<GL::VertexShader>(vsCode);
		specularShader->compile<GL::GeometryShader>(gsCode);
		specularShader->compile<GL::FragmentShader>(fsCode);
		specularShader->link();
	}

	{
		integrateBRDFShader = Shader::create("IBLIntegrateBRDF");
		auto vsCode = loadTxtFile(shaderPath + "/IBLIntegrateBRDF.vert");
		auto fsCode = loadTxtFile(shaderPath + "/IBLIntegrateBRDF.frag");
		integrateBRDFShader->compile<GL::VertexShader>(vsCode);
		integrateBRDFShader->compile<GL::FragmentShader>(fsCode);
		integrateBRDFShader->link();
	}

	{
		skyboxShader = Shader::create("Skybox");
		auto vsCode = loadTxtFile(shaderPath + "/Skybox.vert");
		auto fsCode = loadTxtFile(shaderPath + "/Skybox.frag");
		skyboxShader->compile<GL::VertexShader>(vsCode);
		skyboxShader->compile<GL::FragmentShader>(fsCode);
		skyboxShader->link();

		skyboxShader->setUniform("envMap", 0);
	}
}

void Renderer::initEnvMaps()
{
	unitCube = Primitives::createCube(glm::vec3(0), 1.0f);

	//std::string assetPath = "../assets";
	std::string assetPath = "C:/Users/dave316/Seafile/Assets/EnvMaps";
	auto pano = IO::loadTextureHDR(assetPath + "/Serpentine_Valley/Serpentine_Valley_3k.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/Newport_Loft/Newport_Loft_Ref.hdr");

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
		cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR);

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
	specularShader->use();

	{
		int size = 256;
		specularMap = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMap->generateMipmaps();
		specularMap->setFilter(GL::LINEAR_MIPMAP_LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 8;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMap, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	glEnable(GL_DEPTH_TEST);

	auto screenQuad = Primitives::createQuad(glm::vec3(0.0f), 2.0f);

	{
		int size = 512;
		brdfLUT = Texture2D::create(size, size, GL::RG16F);
		brdfLUT->setWrap(GL::CLAMP_TO_EDGE);

		integrateBRDFShader->use();
		auto brdfFBO = Framebuffer::create(size, size);
		brdfFBO->addRenderTexture(GL::COLOR0, brdfLUT);
		brdfFBO->begin();
		screenQuad->draw();
		brdfFBO->end();
	}

	glViewport(0, 0, 1920, 1080);
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
				//if (name.compare("Sponza") == 0)
				{
					std::cout << "loading model " << name << "...";
					std::string fn = name + "/glTF/" + name + ".gltf";
					auto rootEntity = importer.importModel(path + "/" + fn);
					if (rootEntity != nullptr)
					{
						rootEntitis.push_back(rootEntity);
						auto children = importer.getEntities();
						entities.insert(entities.end(), children.begin(), children.end());
					}
					importer.clear();
					std::cout << "done!" << std::endl;
				}
			}
		}
	}
}

void Renderer::updateAnimations(float dt)
{
	//std::cout << "update animation" << std::endl;
	// apply transformations from animations
	auto rootEntity = rootEntitis[modelIndex];
	auto animators = rootEntity->getChildrenWithComponent<Animator>();
 	for (auto e : animators)
	{
		auto a = e->getComponent<Animator>();
		auto t = e->getComponent<Transform>();

		a->update(dt);

		//defaultProgram.setUniform("hasAnimations", false);

		if (a->hasRiggedAnim())
		{
			auto boneTransforms = a->getBoneTransform();
			defaultShader->setUniform("bones[0]", boneTransforms);
			defaultShader->setUniform("hasAnimations", true);
		}
		else if (a->hasMorphAnim())
		{
			glm::vec2 weights = a->getWeights();
			defaultShader->setUniform("w0", weights.x);
			defaultShader->setUniform("w1", weights.y);
		}
		else
			a->transform(t);
	}

	// propagate the transformations trough the scene hirarchy
	rootEntity->getComponent<Transform>()->update(glm::mat4(1.0f));
}

void Renderer::updateCamera(Camera& camera)
{
	Camera::UniformData cameraData;
	camera.writeUniformData(cameraData);

	defaultShader->setUniform("VP", cameraData.VP);
	defaultShader->setUniform("cameraPos", glm::vec3(cameraData.position));

	skyboxShader->setUniform("V", cameraData.V);
	skyboxShader->setUniform("P", cameraData.P);
}

void Renderer::nextModel()
{
	modelIndex = (++modelIndex) % (unsigned int)rootEntitis.size();
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	defaultShader->use();

	irradianceMap->use(5);
	specularMap->use(6);
	brdfLUT->use(7);

	auto e = rootEntitis[modelIndex];
	{
		// TODO: do depth/tansparent sorting
		auto models = e->getChildrenWithComponent<Renderable>();
		std::vector<Entity*> transparentEntities;
		std::vector<Entity*> opaqueEntities;
		for (auto m : models)
		{
			auto r = m->getComponent<Renderable>();
			if (r->useBlending())
				transparentEntities.push_back(m);
			else
				opaqueEntities.push_back(m);
		}
		for (auto m : opaqueEntities)
		{
			auto r = m->getComponent<Renderable>();
			auto t = m->getComponent<Transform>();

			t->setUniforms(defaultShader);
			r->render(defaultShader);
		}
		for (auto m : transparentEntities)
		{
			auto r = m->getComponent<Renderable>();
			auto t = m->getComponent<Transform>();

			t->setUniforms(defaultShader);
			r->render(defaultShader);
		}
	}

	skyboxShader->use();
	cubeMap->use(0);
	unitCube->draw();
}
