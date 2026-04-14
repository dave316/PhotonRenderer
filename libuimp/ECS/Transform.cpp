#include "Transform.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace Unity
{
	Transform::Transform() :
		localPosition(0.0f),
		localRotation(1.0f, 0.0f, 0.0f, 0.0f),
		localScale(1.0f),
		localTransform(1.0f)
	{

	}

	Transform::~Transform()
	{

	}

	void Transform::addChild(Transform::Ptr child)
	{
		children.push_back(child);
	}

	void Transform::setParent(Transform::Ptr parent)
	{
		this->parent = parent;
	}

	void Transform::setGameObject(GameObject::Ptr gameObject)
	{
		this->gameObject = gameObject;
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

	void Transform::clearGameObject()
	{
		gameObject = nullptr;
		for (auto c : children)
			c->clearGameObject();
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

	GameObject::Ptr Transform::getGameObject()
	{
		return gameObject;
	}

	Transform::Ptr Transform::getChild(uint32 index)
	{
		if (index < children.size())
			return children[index];
	}

	uint32 Transform::numChildren()
	{
		return children.size();
	}
}