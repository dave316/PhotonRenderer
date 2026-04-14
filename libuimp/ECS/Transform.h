#ifndef INCLUDED_UNITYTRANSFORM
#define INCLUDED_UNITYTRANSFORM

#pragma once

#include "Component.h"
#include "GameObject.h"
#include <Types/DataTypes.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Unity
{
	class Transform : public Component
	{
	public:
		typedef std::shared_ptr<Transform> Ptr;

		Transform();
		~Transform();

		void addChild(Transform::Ptr child);
		void setParent(Transform::Ptr parent);
		void setGameObject(GameObject::Ptr gameObject);
		void setLocalPosition(glm::vec3 p);
		void setLocalRotation(glm::quat q);
		void setLocalScale(glm::vec3 s);
		void setLocalTransform(glm::mat4 M);
		void updateLocalTransform();
		void clearGameObject();
				
		glm::vec3 getLocalPosition();
		glm::quat getLocalRotation();
		glm::vec3 getLocalScale();
		glm::mat4 getLocalTransform();

		GameObject::Ptr getGameObject();
		Transform::Ptr getChild(uint32 index);
		uint32 numChildren();

		template<typename T>
		std::vector<std::shared_ptr<T>> getComponentsInChildren(bool onlyActive = false)
		{
			std::vector<std::shared_ptr<T>> allComponents;
			if (onlyActive && !gameObject->isActive())
				return allComponents;
			auto rootComponent = gameObject->getComponent<T>();
			if (rootComponent)
				allComponents.push_back(rootComponent);
			for (auto c : children)
			{
				auto childComponents = c->getComponentsInChildren<T>(onlyActive);
				for (auto childComp : childComponents)
					allComponents.push_back(childComp);
			}
			return allComponents;
		}

		template<typename T>
		std::vector<std::shared_ptr<GameObject>> getChildrenWithComponent(bool onlyActive = false)
		{
			std::vector<std::shared_ptr<GameObject>> entities;
			if (onlyActive && !gameObject->isActive())
				return entities;
			if (gameObject->getComponent<T>())
				entities.push_back(gameObject);
			for (auto c : children)
			{
				auto chiledEntities = c->getChildrenWithComponent<T>(onlyActive);
				for (auto child : chiledEntities)
					entities.push_back(child);
			}
			return entities;
		}

		static Ptr create()
		{
			return std::make_shared<Transform>();
		}
	private:
		glm::vec3 localPosition;
		glm::quat localRotation;
		glm::vec3 localScale;
		glm::mat4 localTransform;

		std::vector<Transform::Ptr> children;
		Transform::Ptr parent;
		GameObject::Ptr gameObject;
	};
}

#endif // INCLUDED_UNITYTRANSFORM