#ifndef INCLUDED_SCENE
#define INCLUDED_SCENE

#pragma once

#include <Graphics/Framebuffer.h>
#include <Core/Light.h>

#include <Core/Entity.h>

#include <IO/GLTFImporter.h>

class Scene
{
private:
	std::string name;
	std::map<std::string, Entity::Ptr> rootEntities;
	//std::map<std::string, Entity::Ptr> allEntities;
	std::map<int, Entity::Ptr> allEntities;
	GL::UniformBuffer<Light::UniformData> lightUBO;
	std::vector<Framebuffer::Ptr> shadowFBOs;
	std::vector<std::vector<glm::mat4>> views;
	std::vector<IO::GLTFCamera> cameras;
	std::vector<std::string> variants;
	std::string currentModel;

	std::vector<Mesh::Ptr> boundingBoxes;
	std::vector<glm::mat4> boxMat;

	Mesh::Ptr screenQuad;
	Mesh::Ptr unitCube;
	TextureCubeMap::Ptr cubeMap;
	TextureCubeMap::Ptr irradianceMap;
	TextureCubeMap::Ptr specularMapGGX;
	TextureCubeMap::Ptr specularMapCharlie;

	bool useTransmission = false;
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
public:
	Scene(const std::string& name);
	~Scene();

	void initEnvMaps(std::map<std::string, Shader::Ptr>& shaders);
	void initLights(Shader::Ptr defaultShader);
	void initShadowMaps();
	void loadModel(std::string name, std::string path);
	void addEntity(std::string name, Entity::Ptr entity);
	void addLight(std::string name);
	void removeRootEntity(std::string name);
	void updateAnimations(float dt);
	void updateAnimationState(float dt);
	void playAnimations();
	void stopAnimations();
	void switchAnimations(int index);
	void switchVariant(int idx);
	void nextMaterial();
	void renderBoxes(Shader::Ptr shader);
	void useIBL();
	void useSkybox();
	void clear();
	void updateBoxes();
	void selectBox(Entity::Ptr e);
	bool hasTransmission();
	IO::GLTFCamera getCamera(int idx);
	std::map<std::string, Entity::Ptr>& getEntities();
	std::vector<Framebuffer::Ptr>& getShadwoFBOs();
	std::vector<std::vector<glm::mat4>>& getViews();
	std::vector<std::string> getCameraNames();
	std::vector<std::string> getVariantNames();
	AABB getBoundingBox();
	Entity::Ptr getCurrentModel();
	Entity::Ptr getNode(int id);
	Entity::Ptr selectModelRaycast(glm::vec3 start, glm::vec3 end);

	typedef std::shared_ptr<Scene> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Scene>(name);
	}
};

#endif // INCLUDED_SCENE