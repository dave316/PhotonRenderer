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

#include <IO/GLTFImporter.h>

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
	TextureCubeMap::Ptr specularMapGGX;
	TextureCubeMap::Ptr specularMapCharlie;
	Texture2D::Ptr ggxLUT;
	Texture2D::Ptr charlieLUT;
	Texture2D::Ptr lutSheenE; // TODO: generate based on https://dassaultsystemes-technology.github.io/EnterprisePBRShadingModel/spec-2021x.md.html#components/sheen
	bool useIBL = true;
	bool useSkybox = false;

	std::vector<std::string> variants; // TODO: store variants in mesh or renderable
	std::vector<IO::GLTFCamera> cameras;
	std::map<std::string, Entity::Ptr> rootEntitis;
	std::string currentModel;
	unsigned int width;
	unsigned int height;

public:
	Renderer(unsigned int width, unsigned int height);
	~Renderer();
	bool init();
	void initKTXTest();
	void initSceneMaterials();
	void initSceneAnisotropy();
	void initEnvMaps();
	void initFBOs();
	void initLights();
	void initShader();
	void initFonts();
	void loadGLTFModels(std::string path);
	void loadAssimpModels(std::string path);
	void loadModel(std::string name, std::string path);
	void nextCamera();
	void nextMaterial();
	void switchCamera(int idx);
	void switchVariant(int idx);
	void updateAnimations(float dt);
	void updateAnimationState(float dt);
	void updateCamera(Camera& camera);
	void updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos);
	void updateShadows();
	void playAnimations();
	void stopAnimations();
	void switchAnimations(int index);
	void renderScene(Shader::Ptr shader, bool transmission);
	void render();
	void renderText();
	void clear();
	std::vector<std::string> getCameraNames();
	std::vector<std::string> getVariantNames();
};

#endif // INCLUDED_RENDERER