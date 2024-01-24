#include "Transform.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>

Transform::Transform() :
	localPosition(0.0f),
	localRotation(1.0f, 0.0f, 0.0f, 0.0f),
	localScale(1.0f),
	localTransform(1.0f),
	position(0.0f),
	rotation(1.0f, 0.0f, 0.0f, 0.0f),
	scale(1.0f),
	transform(1.0f),
	normalTransform(1.0f)
{}

Transform::~Transform()
{
	//std::cout << "Transform: destroyed" << std::endl;
}

void Transform::setLocalPosition(glm::vec3 p)
{
	this->localPosition = p;
	updateLocalTransform();
}

void Transform::setLocalRotation(glm::quat q)
{
	this->localRotation = q;
	updateLocalTransform();
}

void Transform::setLocalScale(glm::vec3 s)
{
	this->localScale = s;
	updateLocalTransform();
}

void Transform::setLocalTransform(glm::mat4 M)
{
	this->localTransform = M;

	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(M, localScale, localRotation, localPosition, skew, persp);
	updateLocalTransform();
}

void Transform::updateLocalTransform()
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), localPosition);
	glm::mat4 R = glm::mat4_cast(localRotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), localScale);
	localTransform = T * R * S;
}

void Transform::update(glm::mat4 parentTransform)
{
	transform = parentTransform * localTransform;
	normalTransform = glm::inverseTranspose(glm::mat3(transform));

	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(transform, scale, rotation, position, skew, persp);
}

void Transform::setUniforms(Shader::Ptr shader)
{
	shader->setUniform("M", transform);
	shader->setUniform("N", normalTransform);
}

void Transform::translate(glm::vec3 t)
{
	localPosition += t;
	updateLocalTransform();
}

void Transform::rotate(float angle, glm::vec3 axis)
{
	localRotation *= glm::angleAxis(glm::radians(angle), axis);
	updateLocalTransform();
}

void Transform::setBounds(Box& aabb)
{
	boundingBox = aabb;
}

Box Transform::getBounds()
{
	return boundingBox;
}

glm::mat4 Transform::getTransform()
{
	return transform;
}

glm::mat3 Transform::getNormalMatrix()
{
	return normalTransform;
}

glm::vec3 Transform::getLocalPosition()
{
	return localPosition;
}

glm::quat Transform::getLocalRotation()
{
	return localRotation;
}

glm::vec3 Transform::getLocalScale()
{
	return localScale;
}

glm::mat4 Transform::getLocalTransform()
{
	return localTransform;
}

glm::vec3 Transform::getPosition()
{
	return position;
}

glm::quat Transform::getRotation()
{
	return rotation;
}

glm::vec3 Transform::getScale()
{
	return scale;
}