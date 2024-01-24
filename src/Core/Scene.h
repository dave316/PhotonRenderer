#ifndef INCLUDED_SCENE
#define INCLUDED_SCENE

#pragma once

#include <Core/Animator.h>
#include <Core/Camera.h>
#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Volume.h>
#include <Core/Collider.h>

#include <Graphics/FPSCamera.h>

#include <IO/Unity/LightData.h>

struct ReflectionProbe
{
	glm::vec4 position;
	glm::vec4 boxMin;
	glm::vec3 boxMax;
	int index;
};

struct SHLightProbes
{
	std::vector<IO::Unity::Tetrahedron> tetrahedras;
	std::vector<IO::Unity::SH9> coeffs;
	std::vector<glm::vec3> positions;
};

struct Skybox
{
	Texture2D::Ptr texture = nullptr;
	float exposure = 1.0f;
	float rotation = 0.0f;
	glm::vec4 color = glm::vec4(1);
	glm::vec3 tint = glm::vec4(1);
};

class Scene
{
private:
	std::string name;
	std::map<std::string, Entity::Ptr> rootEntities;
	std::map<int, Entity::Ptr> allEntities;
	std::map<int, Component::Ptr> allComponents;
	Entity::Ptr selectedModel;
	std::string currentModel;

	GL::ShaderStorageBuffer<IBLUniformData> iblUBO;

	Skybox skybox;
	Texture2DArray::Ptr lightMaps = nullptr;
	Texture2DArray::Ptr directionMaps = nullptr;
	Texture2DArray::Ptr iesProfiles = nullptr;
	SHLightProbes lightProbes;
	
	int numLights = 0;
	bool useTransmission = false;

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
public:
	Scene(const std::string& name);
	~Scene();

	void addRootEntity(std::string name, Entity::Ptr entity);
	void addRootEntity(Entity::Ptr entity);
	void addLightMaps(Texture2DArray::Ptr lightmaps);
	void addDirectionMaps(Texture2DArray::Ptr dirMaps);
	void setIESProfile(Texture2DArray::Ptr iesProfiles);
	void setSkybox(Skybox& skybox);
	void setLightProbes(SHLightProbes& lightProbes);
	void removeRootEntity(std::string name);
	bool hasTransmission();
	void checkWindingOrder();
	void updateAnimations(float dt);
	void updateAnimationState(float dt);
	void playAnimations();
	void stopAnimations();
	void switchAnimations(int index);
	void switchVariant(int index);
	void select(Entity::Ptr e);
	void unselect();
	int getNumLights();
	void clear();
	std::vector<std::string> getAnimations();
	std::map<std::string, Entity::Ptr> getRootEntities();
	std::map<std::string, std::vector<Renderable::Ptr>> batchOpaqueInstances();
	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> getOpaqueEntitiesCullFrustrum(FPSCamera& camera);
	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> getOpaqueEntities();
	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> getTransparentEntities();
	std::vector<Entity::Ptr> selectModelsRaycast(glm::vec3 start, glm::vec3 end);
	Entity::Ptr getNode(int id);
	Entity::Ptr getCurrentModel();
	PostProcessParameters getCurrentProfile(glm::vec3 cameraPosition);
	Texture2DArray::Ptr getLightMaps();
	Texture2DArray::Ptr getDirectionMaps();
	Texture2DArray::Ptr getIESProfiles();
	SHLightProbes& getLightProbes();
	Skybox& getSkybox();
	Box getBoundingBox();
	typedef std::shared_ptr<Scene> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Scene>(name);
	}
};

#endif // INCLUDED_SCENE