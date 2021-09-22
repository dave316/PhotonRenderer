#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Text.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Light.h>
#include <Graphics/Shader.h>

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Transform.h>

class Renderer
{
	Shader::Ptr defaultShader;
	Shader::Ptr skyboxShader;
	Shader::Ptr textShader;

	std::vector<Font::Ptr> fonts;
	std::vector<Text2D::Ptr> texts; // add component for text objects/renderer
	std::map<std::string, Light::Ptr> lights;
	std::map<std::string, Shader::Ptr> shaders;
	std::vector<Framebuffer::Ptr> shadowFBOs;
	std::vector<std::vector<glm::mat4>> views;

	GL::UniformBuffer<Camera::UniformData> cameraUBO;
	GL::UniformBuffer<Light::UniformData> lightUBO;

	Framebuffer::Ptr screenFBO;
	Texture2D::Ptr screenTex;
	Mesh::Ptr screenQuad;
	Mesh::Ptr unitCube;
	TextureCubeMap::Ptr cubeMap;
	TextureCubeMap::Ptr irradianceMap;
	TextureCubeMap::Ptr specularMap;
	Texture2D::Ptr brdfLUT;
	Texture2D::Ptr charlieLUT;
	Texture2D::Ptr lutSheenE; // TODO: generate based on https://dassaultsystemes-technology.github.io/EnterprisePBRShadingModel/spec-2021x.md.html#components/sheen
	
	std::map<std::string, Entity::Ptr> rootEntitis;
	unsigned int width;
	unsigned int height;

public:
	Renderer(unsigned int width, unsigned int height);
	~Renderer();
	bool init();
	void initEnvMaps();
	void initFBOs();
	void initLights();
	void initShader();
	void initFonts();
	void loadGLTFModels(std::string path);
	void loadAssimpModels(std::string path);
	void loadModel(std::string name, std::string path);
	void nextMaterial();
	void updateAnimations(float dt);
	void updateAnimationState(float dt);
	void updateCamera(Camera& camera);
	void updateShadows();
	void renderScene(Shader::Ptr shader, bool transmission);
	void render();
	void renderText();
};

#endif // INCLUDED_RENDERER