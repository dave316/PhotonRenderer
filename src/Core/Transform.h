#ifndef INCLUDED_TRANSFORM
#define INCLUDED_TRANSFORM

#pragma once

#include "Component.h"

#include <Graphics/Shader.h>
#include <Math/Shapes.h>

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
	glm::vec3 scale;
	glm::mat4 transform;
	glm::mat3 normalTransform;
	
	Box boundingBox;

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
	void setBounds(Box& aabb);
	Box getBounds();

	glm::vec3 getLocalPosition();
	glm::quat getLocalRotation();
	glm::vec3 getLocalScale();
	glm::mat4 getLocalTransform();
	glm::vec3 getPosition();
	glm::quat getRotation();
	glm::vec3 getScale();
	glm::mat4 getTransform();
	glm::mat3 getNormalMatrix();
};

#endif // INCLUDED_TRANSFORM