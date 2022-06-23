#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Text.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Shader.h>

#include <Core/Light.h>
#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Transform.h>
#include <Core/Scene.h>

class Renderer
{
	Shader::Ptr defaultShader;
	Shader::Ptr skyboxShader;
	Shader::Ptr textShader;
	Shader::Ptr unlitShader;
	Shader::Ptr outlineShader;
	Shader::Ptr dofShader;

	//std::vector<Font::Ptr> fonts;
	//std::vector<Text2D::Ptr> texts; // add component for text objects/renderer
	
	std::map<std::string, Shader::Ptr> shaders;
	GL::UniformBuffer<FPSCamera::UniformData> cameraUBO;

	Framebuffer::Ptr screenFBO;
	Framebuffer::Ptr postFBO;
	Framebuffer::Ptr outlineFBO;

	Texture2D::Ptr screenTex;
	Texture2D::Ptr refrTex;
	Texture2D::Ptr postTex;

	Primitive::Ptr screenQuad;
	Primitive::Ptr unitCube;

	Texture2D::Ptr brdfLUT;
	bool useIBL = true;
	bool useSkybox = true;

	unsigned int width;
	unsigned int height;

public:
	Renderer(unsigned int width, unsigned int height);
	~Renderer();
	bool init();
	void initLUTs();
	void initFBOs();
	void initShader();
	void initEnv(std::string fn, Scene::Ptr scene);
	void initLightProbe(EnvironmentMap::Ptr lightProbe, Scene::Ptr scene);
	void initLights(Scene::Ptr scene);
	//void initFonts();
	void resize(unsigned int width, unsigned int height);
	void updateCamera(FPSCamera& camera);
	void updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos);
	void updateShadows(Scene::Ptr scene);
	void setIBL(bool useIBL);
	void setLights(int numLights);
	void setDebugChannel(int channel);

	//void updateSHEnv(IO::SphericalLightingProbe shProbe);
	void renderScene(Scene::Ptr scene, Shader::Ptr shader, bool transmission);
	void renderOutline(Entity::Ptr entity);
	void renderToScreen(Scene::Ptr scene);
	Texture2D::Ptr renderForward(Scene::Ptr scene);
	Texture2D::Ptr renderToTexture(Scene::Ptr scene);
	EnvironmentMap::Ptr renderToCubemap(Scene::Ptr scene);
	//void renderText();
};

#endif // INCLUDED_RENDERER