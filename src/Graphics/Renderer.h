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
#include <Core/Scene.h>

#include <IO/GLTFImporter.h>

class Renderer
{
	Shader::Ptr defaultShader;
	Shader::Ptr skyboxShader;
	Shader::Ptr textShader;

	std::vector<Font::Ptr> fonts;
	std::vector<Text2D::Ptr> texts; // add component for text objects/renderer
	
	std::map<std::string, Shader::Ptr> shaders;
	GL::UniformBuffer<Camera::UniformData> cameraUBO;

	Framebuffer::Ptr screenFBO;
	Texture2D::Ptr screenTex;
	Mesh::Ptr screenQuad;
	Mesh::Ptr unitCube;
	Texture2D::Ptr ggxLUT;
	Texture2D::Ptr charlieLUT;
	Texture2D::Ptr lutSheenE; // TODO: generate based on https://dassaultsystemes-technology.github.io/EnterprisePBRShadingModel/spec-2021x.md.html#components/sheen
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
	void initEnv(Scene::Ptr scene);
	void initLights(Scene::Ptr scene);
	void initFonts();

	void updateCamera(Camera& camera);
	void updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos);
	void updateShadows(Scene::Ptr scene);
	void renderScene(Scene::Ptr scene, Shader::Ptr shader, bool transmission);
	void render(Scene::Ptr scene);
	void renderText();
};

#endif // INCLUDED_RENDERER