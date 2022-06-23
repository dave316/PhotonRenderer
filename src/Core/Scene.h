#ifndef INCLUDED_SCENE
#define INCLUDED_SCENE

#pragma once

#include <Graphics/EnvironmentMap.h>
#include <Graphics/Framebuffer.h>

#include <Core/Camera.h>
#include <Core/Light.h>
#include <Core/Entity.h>

struct CameraInfo
{
	glm::mat4 P;
	glm::mat4 V;
	glm::vec3 pos;
	CameraInfo() {}
	CameraInfo(glm::mat4 P, glm::mat4 V, glm::vec3 pos) :
		P(P), V(V), pos(pos)
	{}
};

class Scene
{
private:
	std::string name;
	std::map<std::string, Entity::Ptr> rootEntities;
	std::map<int, Entity::Ptr> allEntities;
	GL::UniformBuffer<Light::UniformData> lightUBO;
	std::vector<Framebuffer::Ptr> shadowFBOs;
	std::vector<std::vector<glm::mat4>> views;
	std::map<std::string, CameraInfo> cameras;
	std::map<std::string, int> modelInfo;
	std::map<std::string, int> renderInfo;
	std::string currentModel;

	std::vector<Primitive::Ptr> boundingBoxes;
	std::vector<glm::mat4> boxMat;
	Entity::Ptr selectedModel;

	Primitive::Ptr screenQuad;
	Primitive::Ptr unitCube;

	EnvironmentMap::Ptr skybox;
	EnvironmentMap::Ptr irradianceMap;
	EnvironmentMap::Ptr specularMapGGX;
	EnvironmentMap::Ptr specularMapCharlie;

	EnvironmentMap::Ptr reflectionProbe = nullptr;
	EnvironmentMap::Ptr irradianceProbe;
	EnvironmentMap::Ptr specularProbe;

	bool useTransmission = false;
	bool usePerceptualQuantization = false;
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
public:
	Scene(const std::string& name);
	~Scene();

	void initEnvMaps(std::string fn, std::map<std::string, Shader::Ptr>& shaders);
	void initLightProbe(EnvironmentMap::Ptr lightProbe, std::map<std::string, Shader::Ptr>& shaders);
	void initLights(std::map<std::string, Shader::Ptr>& shaders);
	void initShadowMaps();
	bool loadModelASSIMP(std::string name, std::string path);
	bool loadModelGLTF(std::string name, std::string path);
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
	void unselect();
	bool hasTransmission();
	int numLights();
	std::map<std::string, int> getModelInfo();
	std::map<std::string, int> getRenderInfo();
	CameraInfo getCameraInfo(std::string name);
	std::map<std::string, Entity::Ptr>& getEntities();
	std::vector<Framebuffer::Ptr>& getShadwoFBOs();
	std::vector<std::vector<glm::mat4>>& getViews();
	std::vector<std::string> getCameraNames();
	std::vector<std::string> getVariantNames();
	std::vector<std::string> getAnimations();
	AABB getBoundingBox();
	Entity::Ptr getRootNode(std::string name);
	Entity::Ptr getCurrentModel();
	Entity::Ptr getNode(int id);
	Entity::Ptr selectModelRaycast(glm::vec3 start, glm::vec3 end);
	// Returns all hit entities sorted by distance to camera
	std::vector<Entity::Ptr> selectModelsRaycast(glm::vec3 start, glm::vec3 end); 
	std::map<std::string, Entity::Ptr> getRootEntities();
	std::map<std::string, std::vector<Entity::Ptr>> getOpaqueEntities();
	std::map<std::string, std::vector<Entity::Ptr>> getTransparentEntities();

	typedef std::shared_ptr<Scene> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Scene>(name);
	}
};

#endif // INCLUDED_SCENE