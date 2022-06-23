#ifndef INCLUDED_TRANSFORM
#define INCLUDED_TRANSFORM

#pragma once

#include "Component.h"

#include <Graphics/Shader.h>
#include <Graphics/Geometry.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform : public Component
{
public:
	typedef std::shared_ptr<Transform> Ptr;

private:
	// local transformations
	glm::vec3 localPosition;
	glm::quat localRotation;
	glm::vec3 localScale;
	glm::mat4 localTransform;

	// world transformations
	glm::vec3 position;
	glm::quat rotation;
	glm::mat4 transform;
	glm::mat3 normalTransform;
	
	AABB boundingBox;

public:
	Transform();
	~Transform();
	void setLocalPosition(glm::vec3 p);
	void setLocalRotation(glm::quat q);
	void setLocalScale(glm::vec3 s);
	void setLocalTransform(glm::mat4 M);
	void updateLocalTransform();
	void update(glm::mat4 parentTransform);
	void setUniforms(Shader::Ptr shader);
	void translate(glm::vec3 t);
	void rotate(float angle, glm::vec3 axis);
	void setBounds(AABB& aabb);
	AABB getBounds();

	glm::vec3 getLocalPosition();
	glm::quat getLocalRotation();
	glm::vec3 getLocalScale();
	glm::mat4 getLocalTransform();
	glm::vec3 getPosition();
	glm::quat getRotation();
	glm::mat4 getTransform();
};

#endif // INCLUDED_TRANSFORM