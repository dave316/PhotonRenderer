#include "Transform.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>

Transform::Transform(Entity* entity) :
	entity(entity),
	position(0.0f),
	rotation(1.0f, 0.0f, 0.0f, 0.0f),
	scaling(1.0f),
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
	this->scaling = s;
}

void Transform::setTransform(glm::mat4 T)
{
	transform = T;
	calcNormalMatrix();
}

void Transform::update(glm::mat4 parentTransform)
{
	//glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
	//glm::mat4 R = glm::mat4_cast(rotation);
	//glm::mat4 S = glm::scale(glm::mat4(1.0f), scaling);
	//transform = parentTransform * (T * R * S);
	localTransform = parentTransform * transform;
	for (auto c : children)
		c->update(localTransform);

	normalTransform = glm::inverseTranspose(glm::mat3(localTransform));
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
	//for (int row = 0; row < 4; row++)
	//{
	//	for (int col = 0; col < 4; col++)
	//	{
	//		std::cout << transform[row][col] << " ";
	//	}
	//	std::cout << std::endl;

	//}
	shader->setUniform("M", localTransform);
	shader->setUniform("N", normalTransform);
}

void Transform::translate(glm::vec3 t)
{
	transform = glm::translate(transform, t);
	//calcNormalMatrix();
}

void Transform::rotate(float angle, glm::vec3 axis)
{
	transform = glm::rotate(transform, glm::radians(angle), axis);
	//calcNormalMatrix();
}

void Transform::scale(glm::vec3 s)
{
	transform = glm::scale(transform, s);
	//calcNormalMatrix();
	//moved = true;
}	

void Transform::setModelMatrix(glm::mat4& M)
{
	transform = M;
	//calcNormalMatrix();
}

glm::mat4 Transform::getTransform()
{
	return transform;
}

void Transform::calcNormalMatrix()
{
	normalTransform = glm::inverseTranspose(glm::mat3(transform));
}