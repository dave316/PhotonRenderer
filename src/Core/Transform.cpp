#include "Transform.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>

Transform::Transform(Entity* entity) :
	entity(entity),
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

void Transform::setTransform(glm::mat4 T)
{
	transform = T;
}

void Transform::update(glm::mat4 parentTransform)
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 R = glm::mat4_cast(rotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
	transform = parentTransform * (T * R * S);
	for (auto c : children)
		c->update(transform);

	normalTransform = glm::inverseTranspose(glm::mat3(transform));
}

void Transform::updateTransform(glm::mat4 parentTransform)
{
	transform = parentTransform * transform;
	for (auto c : children)
		c->updateTransform(transform);
}

void Transform::addChild(Transform::Ptr child)
{
	children.push_back(child);
}

Transform::Ptr Transform::getChild(int index)
{
	if (index >= 0 && index < children.size())
	{
		return children[index];
	}
	return nullptr;
}

int Transform::getNumChildren()
{
	return children.size();
}

Entity* Transform::getEntity()
{
	return entity;
}

void Transform::setUniforms(Shader::Ptr shader)
{
	shader->setUniform("M", transform);
	shader->setUniform("N", normalTransform);
}

