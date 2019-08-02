#ifndef INCLUDED_TRANSFORM
#define INCLUDED_TRANSFORM

#pragma once

#include "Component.h"

#include <Graphics/Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

class Entity;
class Transform : public Component
{
public:
	typedef std::shared_ptr<Transform> Ptr;

private:
	Entity* entity;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::mat4 transform;
	glm::mat3 normalTransform;
	
	Transform::Ptr parent;
	std::vector<Transform::Ptr> children;

public:
	Transform(Entity* entity);
	~Transform();
	void setPosition(glm::vec3 p);
	void setRotation(glm::quat q);
	void setScale(glm::vec3 s);
	void setTransform(glm::mat4 T);
	void update(glm::mat4 parentTransform);
	void updateTransform(glm::mat4 parentTransform);
	void addChild(Transform::Ptr child);
	int getNumChildren();
	Transform::Ptr getChild(int index);
	Entity* getEntity();

	void setUniforms(Shader::Ptr shader);


	//operator glm::mat4() const
	//{
	//	return transform;
	//}
};

#endif // INCLUDED_TRANSFORM