#include "Transform.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>

Transform::Transform() :
	position(0.0f),
	rotation(1.0f, 0.0f, 0.0f, 0.0f),
	scale(1.0f),
	transform(1.0f),
	localPosition(0.0f),
	localRotation(1.0f, 0.0f, 0.0f, 0.0f),
	localScale(1.0f),
	localTransform(1.0f),
	normalTransform(1.0f)
{}

Transform::~Transform()
{
	//std::cout << "Transform: destroyed" << std::endl;
}

void Transform::setPosition(glm::vec3 p)
{
	this->position = p;
}

void Transform::setRotation(glm::quat q)
{
	this->rotation = q;
}

void Transform::setScale(glm::vec3 s)
{
	this->scale = s;
}

void Transform::setTransform(glm::mat4 M)
{
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(M, scale, rotation, position, skew, persp);
}

void Transform::setLocalPostion(glm::vec3 p)
{
	localPosition = p;
}

void Transform::setLocalRotation(glm::quat q)
{
	localRotation = q;
}

void Transform::setLocalScale(glm::vec3 s)
{
	localScale = s;
}

void Transform::setLocalTransform(glm::mat4 M)
{
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(M, localScale, localRotation, localPosition, skew, persp);
}

void Transform::update(glm::mat4 parentTransform)
{
	// TODO: update only when needed (ie. when transform has been changed)
	//		 otherwise this will get very slow...

	glm::mat4 localT = glm::translate(glm::mat4(1.0f), localPosition);
	glm::mat4 localR = glm::mat4_cast(localRotation);
	glm::mat4 localS = glm::scale(glm::mat4(1.0f), localScale);
	localTransform = localT * localR * localS;

	glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 R = glm::mat4_cast(rotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
	transform = parentTransform * (T * R * S) * localTransform;
	normalTransform = glm::inverseTranspose(glm::mat3(transform));
}

void Transform::setUniforms(Shader::Ptr shader)
{
	shader->setUniform("M", transform);
	shader->setUniform("N", normalTransform);
}

void Transform::translate(glm::vec3 t)
{
	position += t;
}

void Transform::rotate(float angle, glm::vec3 axis)
{
	rotation *= glm::angleAxis(glm::radians(angle), axis);
}

glm::mat4 Transform::getLocalTransform()
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 R = glm::mat4_cast(rotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
	return (T * R * S);
}

glm::mat4 Transform::getTransform()
{
	return transform;
}

glm::vec3 Transform::getPosition()
{
	return position;
}

glm::quat Transform::getRotation()
{
	return rotation;
}

void Transform::calcNormalMatrix()
{
	normalTransform = glm::inverseTranspose(glm::mat3(transform));
}