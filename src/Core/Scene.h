#ifndef INCLUDED_SCENE
#define INCLUDED_SCENE

#pragma once

#include <Graphics/Framebuffer.h>
#include <Graphics/Light.h>

#include <Core/Entity.h>

#include <IO/GLTFImporter.h>

class Scene
{
private:
	std::string name;
	std::map<std::string, Entity::Ptr> rootEntities;
	std::map<std::string, Light::Ptr> lights;
	GL::UniformBuffer<Light::UniformData> lightUBO;
	std::vector<Framebuffer::Ptr> shadowFBOs;
	std::vector<std::vector<glm::mat4>> views;
	std::vector<IO::GLTFCamera> cameras;
	std::vector<std::string> variants;
	std::string currentModel;

	Mesh::Ptr screenQuad;
	Mesh::Ptr unitCube;
	TextureCubeMap::Ptr cubeMap;
	TextureCubeMap::Ptr irradianceMap;
	TextureCubeMap::Ptr specularMapGGX;
	TextureCubeMap::Ptr specularMapCharlie;
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
public:
	Scene(const std::string& name);
	~Scene();

	void initEnvMaps(std::map<std::string, Shader::Ptr>& shaders);
	void initLights(Shader::Ptr defaultShader);
	void loadModel(std::string name, std::string path);
	void updateAnimations(float dt);
	void updateAnimationState(float dt);
	void playAnimations();
	void stopAnimations();
	void switchAnimations(int index);
	void switchVariant(int idx);
	void nextMaterial();
	void useIBL();
	void useSkybox();
	void clear();
	IO::GLTFCamera getCamera(int idx);
	std::map<std::string, Entity::Ptr>& getEntities();
	std::vector<Framebuffer::Ptr>& getShadwoFBOs();
	std::vector<std::vector<glm::mat4>>& getViews();
	std::vector<std::string> getCameraNames();
	std::vector<std::string> getVariantNames();

	typedef std::shared_ptr<Scene> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Scene>(name);
	}
};

#endif // INCLUDED_SCENE