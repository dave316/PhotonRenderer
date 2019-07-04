#ifndef INCLUDED_TRANSFORM
#define INCLUDED_TRANSFORM

#pragma once

#include "Component.h"

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
	glm::mat4 localTransform;
	glm::mat4 transform;
	
	Transform::Ptr parent;
	std::vector<Transform::Ptr> children;

public:
	Transform(Entity* entity, glm::mat4 transform);
	~Transform();
	void setRotation(glm::quat q);
	void update(glm::mat4 parentTransform);
	void localTransformation(glm::mat4 T);
	void reset();
	void addChild(Transform::Ptr child);
	int getNumChildren();
	Transform::Ptr getChild(int index);
	Entity* getEntity();
	operator glm::mat4() const
	{
		return transform;
	}
};

#endif // INCLUDED_TRANSFORM