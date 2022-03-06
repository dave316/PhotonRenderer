#include "Renderer.h"

#include <Graphics/Framebuffer.h>
#include <Graphics/MeshPrimitives.h>
#include <Graphics/Shader.h>

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
	initLUTs();

	//std::string assetPath = "../../../../assets";
	std::string assetPath = "../../../../assets";
	lutSheenE = IO::loadTexture16(assetPath + "/lut_sheen_E.png", false);	

	cameraUBO.bindBase(0);

	initFBOs();
	//initFonts();

	return true;
}

void Renderer::initLUTs()
{
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

	unitCube = MeshPrimitives::createCube(glm::vec3(0), 1.0f);
	screenQuad = MeshPrimitives::createQuad(glm::vec3(0.0f), 2.0f);

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
	refractionTex = Texture2D::create(w, h, GL::RGBA8);
	refractionTex->generateMipmaps();
	refractionTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	refractionFBO = Framebuffer::create(w, h);
	refractionFBO->addRenderTexture(GL::COLOR0, refractionTex);
	refractionFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);

	screenTex = Texture2D::create(w, h, GL::RGBA8);
	screenFBO = Framebuffer::create(w, h);
	screenFBO->addRenderTexture(GL::COLOR0, screenTex);
	screenFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);
}

void Renderer::initShader()
{
	std::string shaderPath = "../../../../src/Shaders";
	auto shaderList = IO::loadShadersFromPath(shaderPath);
	for (auto s : shaderList)
		shaders.insert(std::pair(s->getName(), s));
	defaultShader = shaders["Default"];
	defaultShader->setUniform("screenTex", 14);
	defaultShader->setUniform("irradianceMap", 15);
	defaultShader->setUniform("specularMapGGX", 16);
	defaultShader->setUniform("specularMapCharlie", 17);
	defaultShader->setUniform("sheenLUTE", 18);
	defaultShader->setUniform("ggxLUT", 19);
	defaultShader->setUniform("charlieLUT", 20);
	defaultShader->setUniform("useIBL", useIBL);

	// TODO: fix shadow maps! Reduce texture unit usage, too many active...

	std::vector<int> units;
	for (int i = 10; i < 15; i++)
		units.push_back(i);
	defaultShader->setUniform("shadowMaps[0]", units);
	//defaultShader->setUniform("numLights", (int)lights.size());

	skyboxShader = shaders["Skybox"];
	skyboxShader->setUniform("envMap", 0);

	textShader = shaders["Text"];
	textShader->setUniform("P", glm::ortho(0.0f, (float)width, 0.0f, (float)height));
	textShader->setUniform("atlas", 0);

	unlitShader = shaders["Unlit"];
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("tex", 0);
}

void Renderer::initEnv(Scene::Ptr scene)
{
	scene->initEnvMaps(shaders);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Renderer::initLights(Scene::Ptr scene)
{
	scene->initLights(defaultShader);
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
		//std::string lightStr = "Lights: " + std::to_string(lights.size());

		glm::vec3 textColor(0.2f, 0.5f, 0.9f);
		int fontSize = 20;

		Text2D::Ptr text1(new Text2D(font, resStr, textColor, glm::vec2(20.0f, 80.0f), fontSize));
		Text2D::Ptr text2(new Text2D(font, nameStr, textColor, glm::vec2(20.0f, 60.0f), fontSize));
		//Text2D::Ptr text3(new Text2D(font, lightStr, textColor, glm::vec2(20.0f, 40.0f), fontSize));
		Text2D::Ptr text4(new Text2D(font, "Font: Arial, Size " + std::to_string(fontSize), textColor, glm::vec2(20.0f), fontSize));
		texts.push_back(text1);
		texts.push_back(text2);
		//texts.push_back(text3);
		texts.push_back(text4);
	}
	fonts.push_back(font);

	FT_Done_FreeType(ft);
}

void Renderer::resize(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;

	initFBOs();
}

void Renderer::updateShadows(Scene::Ptr scene)
{
	auto depthShader = shaders["DepthCubemap"];
	auto shadowFBOs = scene->getShadwoFBOs();
	auto views = scene->getViews();

	// should be put in light class together with the updating
	for (int i = 0; i < views.size(); i++)
	{
		shadowFBOs[i]->begin();
		glCullFace(GL_FRONT);
		depthShader->use();
		depthShader->setUniform("lightIndex", i);
		depthShader->setUniform("VP[0]", views[i]);
		renderScene(scene, depthShader, true);
		shadowFBOs[i]->end();
		glCullFace(GL_BACK);
	}
}

void Renderer::updateCamera(Camera& camera)
{
	Camera::UniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos)
{
	Camera::UniformData cameraData;
	cameraData.VP = P * V;
	cameraData.V = V;
	cameraData.P = P;
	cameraData.position = glm::vec4(pos, 0.0f);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::renderScene(Scene::Ptr scene, Shader::Ptr shader, bool transmission)
{
	auto& rootEntitis = scene->getEntities();

	for (auto [name, e] : rootEntitis)
	{
		auto models = e->getChildrenWithComponent<Renderable>();
		auto animator = e->getComponent<Animator>();
		std::vector<Entity::Ptr> renderEntities;
		for (auto m : models)
			if(!m->getComponent<Renderable>()->useBlending() && !m->getComponent<Renderable>()->isTransmissive())
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
				if(!weights.empty())
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
				skin.computeJoints(nodes); // TODO: this should be done on animation update, not each frame...
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

void Renderer::renderToScreen(Scene::Ptr scene)
{
	if (useIBL)
	{
		//irradianceMap->use(15);
		//specularMapGGX->use(16);
		//specularMapCharlie->use(17);
		scene->useIBL();
		lutSheenE->use(18);
		ggxLUT->use(19);
		charlieLUT->use(20);
	}
	scene->useIBL();

	//for (int i = 0; i < shadowFBOs.size(); i++)
	//	shadowFBOs[i]->useTexture(GL::DEPTH, 10 + i);

	// offscreen pass for transmission
	refractionFBO->begin();
	if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", false);
		//cubeMap->use(0);
		scene->useSkybox();
		unitCube->draw();
		glCullFace(GL_BACK);
	}

	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", false);
	renderScene(scene, defaultShader, false);
	refractionFBO->end();
	refractionTex->generateMipmaps();
	refractionTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	refractionTex->use(14);

	// main render pass
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", true);
		//cubeMap->use(0);
		scene->useSkybox();
		unitCube->draw();
		glCullFace(GL_BACK);
	}
	
	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", true);
	renderScene(scene, defaultShader, true);
}

Texture2D::Ptr Renderer::renderToTexture(Scene::Ptr scene)
{
	if (useIBL)
	{
		//irradianceMap->use(15);
		//specularMapGGX->use(16);
		//specularMapCharlie->use(17);
		scene->useIBL();
		lutSheenE->use(18);
		ggxLUT->use(19);
		charlieLUT->use(20);
	}
	scene->useIBL();

	//for (int i = 0; i < shadowFBOs.size(); i++)
	//	shadowFBOs[i]->useTexture(GL::DEPTH, 10 + i);

	// offscreen pass for transmission
	refractionFBO->begin();
	if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", false);
		//cubeMap->use(0);
		scene->useSkybox();
		unitCube->draw();
		glCullFace(GL_BACK);
	}

	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", false);
	renderScene(scene, defaultShader, false);
	refractionFBO->end();
	refractionTex->generateMipmaps();
	refractionTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

	// main render pass
	screenFBO->begin();
	if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		skyboxShader->setUniform("useGammaEncoding", true);
		//cubeMap->use(0);
		scene->useSkybox();
		unitCube->draw();
		glCullFace(GL_BACK);
	}

	refractionTex->use(14);
	defaultShader->use();
	defaultShader->setUniform("useGammaEncoding", true);
	renderScene(scene, defaultShader, true);
	scene->renderBoxes(unlitShader);
	screenFBO->end();

	//glViewport(0, 0, width, height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//screenTex->use(0);
	//unlitShader->use();
	//screenQuad->draw();

	return screenTex;
}

void Renderer::renderText()
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader->use();
	for (auto& textMesh : texts)
		textMesh->draw(textShader);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}
