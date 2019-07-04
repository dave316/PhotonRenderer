#include "Transform.h"
#include <iostream>

Transform::Transform(Entity* entity, glm::mat4 transform) :
	entity(entity),
	position(0.0f),
	scale(1.0f),
	transform(transform),
	localTransform(transform)
{}

Transform::~Transform()
{
	//std::cout << "Transform: destroyed" << std::endl;
}

void Transform::update(glm::mat4 parentTransform)
{
	//glm::mat4 T = glm::translate(glm::mat4(), position);
	//glm::mat4 R = glm::mat4_cast(rotation);
	//glm::mat4 S = glm::scale(glm::mat4(), scale);
	//transform = parentTransform * (S * R * T);
	transform = parentTransform * transform;
	for (auto c : children)
		c->update(transform);
}

void Transform::localTransformation(glm::mat4 T)
{
	transform = localTransform * T;
}

void Transform::reset()
{
	transform = localTransform;
}
 
void Transform::setRotation(glm::quat q)
{
	rotation = q;
	update(glm::mat4());
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

