#include "Renderer.h"

#include <Core/Animator.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/MeshPrimitives.h>
#include <Graphics/Shader.h>

#include <IO/AssimpImporter.h>
#include <IO/ShaderLoader.h>
#include <IO/Image/ImageDecoder.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace json = rapidjson;

void extern debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		std::cout << std::hex;
		std::cout << "OpenGL error: type=" << type << " severity: " << severity << " message: " << message << std::endl;
		std::cout << std::dec;
	}
}

Renderer::Renderer(unsigned int width, unsigned int height) :
	width(width), height(height)
{
}

Renderer::~Renderer()
{
}

bool Renderer::init()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(0.8f, 0.77f, 0.54f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glViewport(0, 0, width, height);
	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(debugCallback, 0);

	unitCube = MeshPrimitives::createCube(glm::vec3(0), 1.0f);
	screenQuad = MeshPrimitives::createQuad(glm::vec3(0.0f), 2.0f);

	initShader();
	initLUTs();

	//std::string assetPath = "../../../../assets";
	std::string assetPath = "../../../../assets";
	//lutSheenE = IO::loadTexture16(assetPath + "/lut_sheen_E.png", false);	

	cameraUBO.bindBase(0);

	initFBOs();
	//initFonts();

	return true;
}

void Renderer::initLUTs()
{
	//if (fs::exists("brdf_lut.ktx2"))
	//{
	//	brdfLUT = IO::loadTextureKTX("brdf_lut.ktx2");
	//	brdfLUT->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
	//}
	//else
	{
		auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

		int size = 512;
		auto brdfFBO = Framebuffer::create(size, size);
		brdfLUT = Texture2D::create(size, size, GL::RGBA16F);
		brdfLUT->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
		brdfFBO->addRenderTexture(GL::COLOR0, brdfLUT);
		brdfFBO->begin();
		integrateBRDFShader->use();
		screenQuad->draw();
		brdfFBO->end();

		//IO::saveTextureKTX("brdf_lut.ktx2", brdfLUT);
	}
}

void Renderer::initFBOs()
{
	int w = width;
	int h = height;
	//refractionTex = Texture2D::create(w, h, GL::RGBA8);
	//refractionTex->generateMipmaps();
	//refractionTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	//refractionFBO = Framebuffer::create(w, h);
	//refractionFBO->addRenderTexture(GL::COLOR0, refractionTex);
	//refractionFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);

	screenTex = Texture2D::create(w, h, GL::RGBA8);
	refrTex = Texture2D::create(w, h, GL::RGBA8);
	refrTex->generateMipmaps();
	refrTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

	screenFBO = Framebuffer::create(w, h);
	screenFBO->addRenderTexture(GL::COLOR0, screenTex);
	screenFBO->addRenderTexture(GL::COLOR1, refrTex);
	screenFBO->addRenderBuffer(GL::DEPTH_STENCIL, GL::D24_S8);

	postTex = Texture2D::create(w, h, GL::RGBA8);
	postFBO = Framebuffer::create(w, h);
	postFBO->addRenderTexture(GL::COLOR0, postTex);

	outlineFBO = Framebuffer::create(w, h);
	outlineFBO->addRenderTexture(GL::COLOR0, GL::RGBA8);
	outlineFBO->addRenderTexture(GL::COLOR1, GL::RGBA8);
	outlineFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);
}

void Renderer::initShader()
{
	std::string shaderPath = "../../../../src/Shaders";
	auto shaderList = IO::loadShadersFromPath(shaderPath);
	for (auto s : shaderList)
		shaders.insert(std::pair(s->getName(), s));

	for (auto [name, shader] : shaders)
	{
		if (name.substr(0, 7).compare("Default") == 0)
		{
			shader->setUniform("screenTex", 14);
			shader->setUniform("irradianceMap", 15);
			shader->setUniform("specularMapGGX", 16);
			shader->setUniform("specularMapCharlie", 17);
			shader->setUniform("brdfLUT", 18);
			shader->setUniform("useIBL", useIBL);

			// TODO: fix shadow maps! Reduce texture unit usage, too many active...

			std::vector<int> units;
			for (int i = 8; i <= 12; i++)
				units.push_back(i);
			shader->setUniform("shadowMaps[0]", units);
			shader->setUniform("shadowMap", 19);
			shader->setUniform("morphTargets", 20);
		}
	}

	auto depthShader = shaders["Depth"];
	depthShader->setUniform("morphTargets", 20);

	auto depthCMShader = shaders["DepthCubemap"];
	depthCMShader->setUniform("morphTargets", 20);

	skyboxShader = shaders["Skybox"];
	skyboxShader->setUniform("envMap", 0);

	auto skyboxCMShader = shaders["SkyboxCubemap"];
	skyboxCMShader->setUniform("envMap", 0);

	textShader = shaders["Text"];
	textShader->setUniform("P", glm::ortho(0.0f, (float)width, 0.0f, (float)height));
	textShader->setUniform("atlas", 0);

	unlitShader = shaders["Unlit"];
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("morphTargets", 20);
	unlitShader->setUniform("useGammaEncoding", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("tex", 0);
	
	outlineShader = shaders["Outline"];
	outlineShader->setUniform("inputTexture", 0);
}

void Renderer::initEnv(std::string fn, Scene::Ptr scene)
{
	glDisable(GL_CULL_FACE);

	scene->initEnvMaps(fn, shaders);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Renderer::initLightProbe(EnvironmentMap::Ptr lightProbe, Scene::Ptr scene)
{
	glDisable(GL_CULL_FACE);

	scene->initLightProbe(lightProbe, shaders);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Renderer::initLights(Scene::Ptr scene)
{
	scene->initLights(shaders);
}

//void Renderer::initFonts()
//{
//	// TODO: better error handling...
//
//	FT_Library ft;
//	FT_Init_FreeType(&ft);
//
//	std::vector<std::string> fileNames;
//	fileNames.push_back("../../../../assets/fonts/arial.ttf");
//	fileNames.push_back("../../../../assets/fonts/arialbd.ttf");
//	fileNames.push_back("../../../../assets/fonts/ariali.ttf");
//	fileNames.push_back("../../../../assets/fonts/arialbi.ttf");
//	auto font = Font::Ptr(new Font(ft, fileNames, 11, 60));
//	if (font->isLoaded())
//	{
//		std::string resStr = "Resolution: " + std::to_string(width) + "x" + std::to_string(height);
//		std::string nameStr = "Animation index: 0";
//		//std::string lightStr = "Lights: " + std::to_string(lights.size());
//
//		glm::vec3 textColor(0.2f, 0.5f, 0.9f);
//		int fontSize = 20;
//
//		Text2D::Ptr text1(new Text2D(font, resStr, textColor, glm::vec2(20.0f, 80.0f), fontSize));
//		Text2D::Ptr text2(new Text2D(font, nameStr, textColor, glm::vec2(20.0f, 60.0f), fontSize));
//		//Text2D::Ptr text3(new Text2D(font, lightStr, textColor, glm::vec2(20.0f, 40.0f), fontSize));
//		Text2D::Ptr text4(new Text2D(font, "Font: Arial, Size " + std::to_string(fontSize), textColor, glm::vec2(20.0f), fontSize));
//		texts.push_back(text1);
//		texts.push_back(text2);
//		//texts.push_back(text3);
//		texts.push_back(text4);
//	}
//	fonts.push_back(font);
//
//	FT_Done_FreeType(ft);
//}

void Renderer::resize(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;

	initFBOs();
}

void Renderer::updateShadows(Scene::Ptr scene)
{
	auto depthShader = shaders["Depth"];
	auto depthCMShader = shaders["DepthCubemap"];
	auto shadowFBOs = scene->getShadwoFBOs();
	auto views = scene->getViews();

	// should be put in light class together with the updating
	for (int i = 0; i < views.size(); i++)
	{
		if (views[i].size() == 1)
		{
			// TODO: single depth shader
			shadowFBOs[i]->begin();
			glCullFace(GL_FRONT);
			depthShader->use();
			depthShader->setUniform("VP", views[i][0]);
			renderScene(scene, depthShader, false);
			shadowFBOs[i]->end();
			glCullFace(GL_BACK);

			for (auto [name, shader] : shaders)
			{
				shader->setUniform("VP", views[i][0]);
			}
		}
		else
		{
			shadowFBOs[i]->begin();
			glCullFace(GL_FRONT);
			depthCMShader->use();
			depthCMShader->setUniform("lightIndex", i);
			depthCMShader->setUniform("VP[0]", views[i]);
			renderScene(scene, depthCMShader, false);
			shadowFBOs[i]->end();
			glCullFace(GL_BACK);
		}
	}
}

void Renderer::setIBL(bool useIBL)
{
	this->useIBL = useIBL;
	for (auto [_, shader] : shaders)
		shader->setUniform("useIBL", useIBL);
}

void Renderer::setLights(int numLights)
{
	for (auto& [_, s] : shaders)
		s->setUniform("numLights", numLights);
}

void Renderer::setDebugChannel(int channel)
{
	for (auto& [_, s] : shaders)
		s->setUniform("debugChannel", channel);
}

//void Renderer::updateSHEnv(IO::SphericalLightingProbe shProbe)
//{
//	for (auto [_, shader] : shaders)
//		shader->setUniform("SHEnv[0]", shProbe.coeffs);
//}

void Renderer::updateCamera(FPSCamera& camera)
{
	FPSCamera::UniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos)
{
	FPSCamera::UniformData cameraData;
	cameraData.VP = P * V;
	cameraData.V = V;
	cameraData.P = P;
	cameraData.position = glm::vec4(pos, 0.0f);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::renderScene(Scene::Ptr scene, Shader::Ptr defShader, bool transmission)
{
	if (transmission)
	{
		auto transparent = scene->getTransparentEntities();

		for (auto [name, models] : transparent)
		{
			Shader::Ptr shader;
			if (defShader)
				shader = defShader;
			else
				shader = shaders[name];

			shader->use();
			for (auto m : models)
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();
				Entity::Ptr p = m;
				while (p->getParent() != nullptr)
					p = p->getParent();

				auto animator = p->getComponent<Animator>();

				if (r->useMorphTargets())
				{
					std::vector<float> weights;
					if (animator)
						weights = animator->getWeights();
					else
						weights = r->getWeights();
					shader->setUniform("numMorphTargets", (int)weights.size());
					if (!weights.empty())
						shader->setUniform("morphWeights[0]", weights);
				}
				else
				{
					shader->setUniform("numMorphTargets", 0);
				}

				if (r->isSkinnedMesh())
				{
					auto nodes = animator->getNodes();
					auto skin = r->getSkin();
					skin->computeJoints(nodes); // TODO: this should be done on animation update, not each frame...
					auto boneTransforms = skin->getBoneTransform();
					auto normalTransforms = skin->getNormalTransform();
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
	else
	{
		auto opaque = scene->getOpaqueEntities();
		for (auto [name, models] : opaque)
		{
			Shader::Ptr shader;
			if (defShader)
				shader = defShader;
			else
				shader = shaders[name];

			shader->use();
			for (auto m : models)
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();
				Entity::Ptr p = m;
				while (p->getParent() != nullptr)
					p = p->getParent();

				auto animator = p->getComponent<Animator>();

				if (r->useMorphTargets())
				{
					std::vector<float> weights;
					if (animator)
						weights = animator->getWeights();
					else
						weights = r->getWeights();
					shader->setUniform("numMorphTargets", (int)weights.size());
					if (!weights.empty())
						shader->setUniform("morphWeights[0]", weights);
				}
				else
				{
					shader->setUniform("numMorphTargets", 0);
				}

				if (r->isSkinnedMesh())
				{
					auto nodes = animator->getNodes();
					auto skin = r->getSkin();
					skin->computeJoints(nodes); // TODO: this should be done on animation update, not each frame...
					auto boneTransforms = skin->getBoneTransform();
					auto normalTransforms = skin->getNormalTransform();
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
}

void Renderer::renderOutline(Entity::Ptr entity)
{
	glStencilMask(0x00);

	glm::vec3 color = glm::vec3(1.0f, 0.533f, 0.0f);
	
	outlineFBO->begin();
	unlitShader->use();
	unlitShader->setUniform("useTex", false);
	unlitShader->setUniform("orthoProjection", false);
	unlitShader->setUniform("skipEmptyFragments", false);
	unlitShader->setUniform("solidColor", glm::pow(color, glm::vec3(2.2)));

	Entity::Ptr p = entity;
	while (p->getParent() != nullptr)
		p = p->getParent();

	auto animator = p->getComponent<Animator>();
	auto models = entity->getChildrenWithComponent<Renderable>();
	for (auto m : models)
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
			unlitShader->setUniform("numMorphTargets", (int)weights.size());
			if (!weights.empty())
				unlitShader->setUniform("morphWeights[0]", weights);
		}
		else
		{
			unlitShader->setUniform("numMorphTargets", 0);
		}

		if (r->isSkinnedMesh())
		{
			auto nodes = animator->getNodes();
			auto skin = r->getSkin();
			skin->computeJoints(nodes); // TODO: this should be done on animation update, not each frame...
			auto boneTransforms = skin->getBoneTransform();
			auto normalTransforms = skin->getNormalTransform();
			unlitShader->setUniform("hasAnimations", true);
			unlitShader->setUniform("bones[0]", boneTransforms);
			unlitShader->setUniform("normals[0]", normalTransforms);
		}
		else
		{
			unlitShader->setUniform("hasAnimations", false);
		}

		t->setUniforms(unlitShader);
		r->render(unlitShader);
	}
	outlineFBO->end();

	screenFBO->bindDraw();

	// Draw selected model into stencil buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glDepthMask(0x00);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	for (auto m : models)
	{
		auto r = m->getComponent<Renderable>();
		auto t = m->getComponent<Transform>();
		t->setUniforms(unlitShader);
		r->render(unlitShader);
	}

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDepthMask(0xFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_DEPTH_TEST);

	outlineShader->use();
	outlineFBO->useTexture(GL::COLOR0, 0);
	screenQuad->draw();

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::renderToScreen(Scene::Ptr scene)
{
	auto screenTex = renderForward(scene);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screenTex->use(0);
	unlitShader->use();
	screenQuad->draw();
}

Texture2D::Ptr Renderer::renderForward(Scene::Ptr scene)
{
	if (useIBL)
	{
		scene->useIBL();
		brdfLUT->use(18);
	}
	scene->useIBL();

	glStencilMask(0xFF);
	screenFBO->begin();
	glStencilMask(0x00);
	if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		scene->useSkybox();
		unitCube->draw();
		glCullFace(GL_BACK);
	}

	renderScene(scene, nullptr, false);

	if (scene->hasTransmission())
	{
		// This is quite expensive... find a better way to do this!
		refrTex->generateMipmaps();
		refrTex->use(14);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	renderScene(scene, nullptr, true);

	screenFBO->end();

	return screenTex;
}

Texture2D::Ptr Renderer::renderToTexture(Scene::Ptr scene)
{
	auto mainTex = renderForward(scene);

	screenFBO->bind();
	auto selectedModel = scene->getCurrentModel();
	if (selectedModel != nullptr)
		renderOutline(selectedModel);
	screenFBO->end();

	postFBO->begin();
	screenTex->use(0);
	unlitShader->setUniform("M", glm::mat4(1.0f));
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("useGammaEncoding", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("numMorphTargets", 0);
	unlitShader->setUniform("hasAnimations", false);
	unlitShader->use();
	screenQuad->draw();
	postFBO->end();

	return postTex;
}

EnvironmentMap::Ptr Renderer::renderToCubemap(Scene::Ptr scene)
{
	if (useIBL)
	{
		scene->useIBL();
		brdfLUT->use(18);
	}
	scene->useIBL();

	int faceSize = 1024;
	auto lightProbeEnv = EnvironmentMap::create(faceSize);
	auto cm = lightProbeEnv->getCubeMap();
	cm->generateMipmaps();

	auto lightProbeFBO = Framebuffer::create(faceSize, faceSize);
	lightProbeFBO->addRenderTexture(GL::COLOR0, cm);
	lightProbeFBO->addRenderTexture(GL::DEPTH, GL::DEPTH24, true);

	glm::vec3 pos = glm::vec3(0, 5, 0);
	lightProbeEnv->setViews(pos);

	glm::vec3 skyPos = glm::vec3(0);
	glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
	std::vector<glm::mat4> VP;
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(skyPos, skyPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	updateCamera(glm::mat4(1.0f), glm::mat4(1.0f), pos);

	lightProbeFBO->begin();

	auto skyboxCMShader = shaders["SkyboxCubemap"];
	
	glDepthMask(0x00);
	glCullFace(GL_FRONT);
	skyboxCMShader->use();
	skyboxCMShader->setUniform("VP[0]", VP);
	scene->useSkybox();
	unitCube->draw();
	glDepthMask(0xFF);
	glCullFace(GL_BACK);

	auto defaultCMShader = shaders["DefaultCubemap"];
	defaultCMShader->setUniform("VP[0]", lightProbeEnv->views);
	defaultCMShader->use();
	renderScene(scene, defaultCMShader, false);

	lightProbeFBO->end();

	cm->generateMipmaps();
	cm->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	
	return lightProbeEnv;
}

//void Renderer::renderText()
//{
//	glDisable(GL_DEPTH_TEST);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	textShader->use();
//	for (auto& textMesh : texts)
//		textMesh->draw(textShader);
//	glDisable(GL_BLEND);
//	glEnable(GL_DEPTH_TEST);
//}
