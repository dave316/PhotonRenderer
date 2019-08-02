#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Shader.h>

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Transform.h>

#include <IO/ModelImporter.h>

class Renderer
{
	Shader::Ptr defaultShader;
	Shader::Ptr pano2cmShader;
	Shader::Ptr skyboxShader;
	Shader::Ptr irradianceShader;
	Shader::Ptr specularShader;
	Shader::Ptr integrateBRDFShader;

	Mesh::Ptr unitCube;
	TextureCubeMap::Ptr cubeMap;
	TextureCubeMap::Ptr irradianceMap;
	TextureCubeMap::Ptr specularMap;
	Texture2D::Ptr brdfLUT;
	
	std::vector<Entity::Ptr> rootEntitis;
	std::vector<Entity::Ptr> entities;
	unsigned int modelIndex = 0;

public:
	Renderer(unsigned int width, unsigned int height);
	bool init();
	void initShader();
	void initEnvMaps();
	void loadGLTFModels(std::string path);
	void nextModel();
	void updateAnimations(float dt);
	void updateCamera(Camera& camera);
	void render();
};

#endif // INCLUDED_RENDERER