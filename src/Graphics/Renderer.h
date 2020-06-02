#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Shader.h>

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Transform.h>

#include <IO/ModelImporter.h>

struct Triangle
{
	glm::vec3 v0, v1, v2;
	glm::vec3 n0, n1, n2;
	glm::vec3 plane;
	unsigned int triID;
};

class AABB
{
private:
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

public:
	AABB();
	AABB(glm::vec3& minPoint, glm::vec3& maxPoint);

	glm::vec3 getMinPoint() const;
	glm::vec3 getMaxPoint() const;
	void expand(const glm::vec3& point);
	void expand(const Triangle& tri);
	void expand(const AABB& box);
	float radius();
	glm::vec3 getCenter();
	glm::vec3 getSize();
	std::vector<glm::vec3> getPoints();
};

class Renderer
{
	Shader::Ptr defaultShader;
	Shader::Ptr skyboxShader;
	std::map<std::string, Shader::Ptr> shaders;

	GL::UniformBuffer<Camera::UniformData> cameraUBO;
	
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
	void loadModel(std::string path);
	void nextModel();
	void updateAnimations(float dt);
	void updateCamera(Camera& camera);
	void render();
};

#endif // INCLUDED_RENDERER