#include "Renderer.h"

#include <GL/GLBuffer.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/Primitives.h>

//#include <IO/ModelLoader.h>
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
	std::string name = "SpecGlossVsMetalRough";
	std::cout << "loading model " << name << std::endl;
	std::string fn = name + "/glTF/" + name + ".gltf";

	IO::GLTFImporter importer;
	auto root = importer.importModel(path + "/" + fn);
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
		auto vsCode = loadTxtFile(shaderPath + "/Default.vert");
		auto fsCode = loadTxtFile(shaderPath + "/Default.frag");

		GL::VertexShader vs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		defaultProgram.attachShader(vs);
		defaultProgram.attachShader(fs); 
		if (!defaultProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << defaultProgram.getErrorLog() << std::endl;
		}
		defaultProgram.loadUniforms();

		defaultProgram.setUniform("material.baseColorFactor", glm::vec4(1.0f));
		defaultProgram.setUniform("material.alphaCutOff", 0.0f);
		defaultProgram.setUniform("material.baseColorTex", 0);
		defaultProgram.setUniform("material.pbrTex", 1);
		defaultProgram.setUniform("material.normalTex", 2);
		defaultProgram.setUniform("material.occlusionTex", 3);
		defaultProgram.setUniform("material.emissiveTex", 4);

		defaultProgram.setUniform("irradianceMap", 5);
		defaultProgram.setUniform("specularMap", 6);
		defaultProgram.setUniform("brdfLUT", 7);
		//program.setUniform("w0", 0.0f);
		//program.setUniform("w1", 0.0f);
	}

	{
		auto vsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.vert");
		auto gsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.geom");
		auto fsCode = loadTxtFile(shaderPath + "/PanoToCubeMap.frag");

		GL::VertexShader vs;
		GL::GeometryShader gs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!gs.compile(gsCode.c_str()))
		{
			std::cout << "error compiling geometry shader" << std::endl;
			std::cout << gs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		pano2cmProgram.attachShader(vs);
		pano2cmProgram.attachShader(gs);
		pano2cmProgram.attachShader(fs);
		if (!pano2cmProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << pano2cmProgram.getErrorLog() << std::endl;
		}
		pano2cmProgram.loadUniforms();

		pano2cmProgram.setUniform("envMap", 0);
	}

	{
		auto vsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.vert");
		auto gsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.geom");
		auto fsCode = loadTxtFile(shaderPath + "/IBLDiffuseIrradiance.frag");

		GL::VertexShader vs;
		GL::GeometryShader gs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!gs.compile(gsCode.c_str()))
		{
			std::cout << "error compiling geometry shader" << std::endl;
			std::cout << gs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		irradianceProgram.attachShader(vs);
		irradianceProgram.attachShader(gs);
		irradianceProgram.attachShader(fs);
		if (!irradianceProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << irradianceProgram.getErrorLog() << std::endl;
		}
		irradianceProgram.loadUniforms();
	}

	{
		auto vsCode = loadTxtFile(shaderPath + "/IBLSpecular.vert");
		auto gsCode = loadTxtFile(shaderPath + "/IBLSpecular.geom");
		auto fsCode = loadTxtFile(shaderPath + "/IBLSpecular.frag");

		GL::VertexShader vs;
		GL::GeometryShader gs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!gs.compile(gsCode.c_str()))
		{
			std::cout << "error compiling geometry shader" << std::endl;
			std::cout << gs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		specularProgram.attachShader(vs);
		specularProgram.attachShader(gs);
		specularProgram.attachShader(fs);
		if (!specularProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << specularProgram.getErrorLog() << std::endl;
		}
		specularProgram.loadUniforms();
	}

	{
		auto vsCode = loadTxtFile(shaderPath + "/IBLIntegrateBRDF.vert");
		auto fsCode = loadTxtFile(shaderPath + "/IBLIntegrateBRDF.frag");

		GL::VertexShader vs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		integrateBRDFProgram.attachShader(vs);
		integrateBRDFProgram.attachShader(fs);
		if (!integrateBRDFProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << integrateBRDFProgram.getErrorLog() << std::endl;
		}
		integrateBRDFProgram.loadUniforms();
	}

	{
		auto vsCode = loadTxtFile(shaderPath + "/Skybox.vert");
		auto fsCode = loadTxtFile(shaderPath + "/Skybox.frag");

		GL::VertexShader vs;
		GL::FragmentShader fs;

		if (!vs.compile(vsCode.c_str()))
		{
			std::cout << "error compiling vertex shader" << std::endl;
			std::cout << vs.getErrorLog() << std::endl;
		}

		if (!fs.compile(fsCode.c_str()))
		{
			std::cout << "error compiling fragment shader" << std::endl;
			std::cout << fs.getErrorLog() << std::endl;
		}

		skyboxProgram.attachShader(vs);
		skyboxProgram.attachShader(fs);
		if (!skyboxProgram.link())
		{
			std::cout << "error linking shader program" << std::endl;
			std::cout << skyboxProgram.getErrorLog() << std::endl;
		}
		skyboxProgram.loadUniforms();

		skyboxProgram.setUniform("envMap", 0);
		//program.setUniform("w0", 0.0f);
		//program.setUniform("w1", 0.0f);
	}
}

void Renderer::initEnvMaps()
{
	unitCube = Primitives::createCube(glm::vec3(0), 1.0f);

	std::string assetPath = "../assets";
	//std::string assetPath = "C:/Users/dave316/Seafile/Assets/EnvMaps";
	//auto pano = IO::loadTextureHDR(assetPath + "/Brooklyn_Bridge_Planks/Brooklyn_Bridge_Planks_2k.hdr");
	auto pano = IO::loadTextureHDR(assetPath + "/Newport_Loft/Newport_Loft_Ref.hdr");

	glm::vec3 position = glm::vec3(0);
	std::vector<glm::mat4> VP;

	glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	pano2cmProgram.setUniform("M", glm::mat4(1.0f));
	pano2cmProgram.setUniform("VP[0]", VP);
	pano2cmProgram.setUniform("panorama", 0);
	pano2cmProgram.use();

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

	irradianceProgram.setUniform("VP[0]", VP);
	irradianceProgram.setUniform("environmentMap", 0);
	irradianceProgram.use();

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

	specularProgram.setUniform("VP[0]", VP);
	specularProgram.setUniform("environmentMap", 0);
	specularProgram.use();

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
			specularProgram.setUniform("roughness", roughness);

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

		integrateBRDFProgram.use();
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
	// apply transformations from animations
	auto rootEntity = rootEntitis[modelIndex];
	auto animators = rootEntity->getChildrenWithComponent<Animator>();
	for (auto e : animators)
	{
		auto a = e->getComponent<Animator>();
		auto t = e->getComponent<Transform>();

		a->update(dt);

		if (a->hasMorphAnim())
		{
			glm::vec2 weights = a->getWeights();
			defaultProgram.setUniform("w0", weights.x);
			defaultProgram.setUniform("w1", weights.y);
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

	defaultProgram.setUniform("VP", cameraData.VP);
	defaultProgram.setUniform("cameraPos", glm::vec3(cameraData.position));

	skyboxProgram.setUniform("V", cameraData.V);
	skyboxProgram.setUniform("P", cameraData.P);
}

void Renderer::nextModel()
{
	modelIndex = (++modelIndex) % (unsigned int)rootEntitis.size();
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	defaultProgram.use();

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

			t->setUniforms(defaultProgram);
			r->render(defaultProgram);
		}
		for (auto m : transparentEntities)
		{
			auto r = m->getComponent<Renderable>();
			auto t = m->getComponent<Transform>();

			t->setUniforms(defaultProgram);
			r->render(defaultProgram);
		}
	}

	skyboxProgram.use();
	cubeMap->use(0);
	unitCube->draw();
}
