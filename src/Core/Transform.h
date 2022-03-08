#ifndef INCLUDED_TRANSFORM
#define INCLUDED_TRANSFORM

#pragma once

#include "Component.h"

#include <Graphics/Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform : public Component
{
public:
	typedef std::shared_ptr<Transform> Ptr;

private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::mat4 transform;
	glm::vec3 localPosition;
	glm::quat localRotation;
	glm::vec3 localScale;
	glm::mat4 localTransform;
	glm::mat3 normalTransform;

public:
	Transform();
	~Transform();
	void setPosition(glm::vec3 p);
	void setRotation(glm::quat q);
	void setScale(glm::vec3 s);
	void setTransform(glm::mat4 M);
	void setLocalPostion(glm::vec3 p);
	void setLocalRotation(glm::quat q);
	void setLocalScale(glm::vec3 s);
	void setLocalTransform(glm::mat4 M);
	void update(glm::mat4 parentTransform);
	void setUniforms(Shader::Ptr shader);
	void translate(glm::vec3 t);
	void rotate(float angle, glm::vec3 axis);
	void calcNormalMatrix();
	glm::mat4 getLocalTransform();
	glm::mat4 getTransform();
	glm::vec3 getPosition();
	glm::quat getRotation();
};

#endif // INCLUDED_TRANSFORM