#ifndef INCLUDED_ENTITY
#define INCLUDED_ENTITY

#pragma once

#include "Component.h"
#include "Transform.h"

#include <map>
#include <string>
#include <typeindex>
#include <vector>
#include <iostream>

namespace pr
{
	class Entity : public std::enable_shared_from_this<Entity>
	{
	private:
		std::string name;
		std::string uri;
		std::shared_ptr<Entity> parent = nullptr;
		std::vector<std::shared_ptr<Entity>> children;
		std::map<std::type_index, Component::Ptr> components;

		bool isPrefab = false;
		bool active = true;
		unsigned int id;

		static unsigned int globalIDCount;

	public:
		Entity(const std::string& name, std::shared_ptr<Entity> parent) : name(name), parent(parent)
		{
			auto t = Transform::Ptr(new Transform());
			addComponent(t);
			id = globalIDCount;
			globalIDCount++;
			//std::cout << "creating entity " << std::to_string(id) << std::endl;
		}

		~Entity()
		{
			//std::cout << "destroying entity " << std::to_string(id) << std::endl;
		}

		template<typename T>
		std::shared_ptr<T> addComponent()
		{
			std::shared_ptr<T> component(new T);
			components[typeid(T)] = component;
			return component;
		}

		template<typename T>
		void addComponent(std::shared_ptr<T> component)
		{
			components[typeid(T)] = component;
		}

		template<typename T>
		std::shared_ptr<T> getComponent()
		{
			if (components.find(typeid(T)) != components.end())
			{
				return std::dynamic_pointer_cast<T>(components[typeid(T)]);
			}
			return nullptr;
		}

		template<typename T>
		std::vector<std::shared_ptr<T>> getComponentsInChildren(bool onlyActive = false)
		{
			std::vector<std::shared_ptr<T>> allComponents;
			if (onlyActive && !active)
				return allComponents;
			auto rootComponent = getComponent<T>();
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
		std::vector<std::shared_ptr<Entity>> getChildrenWithComponent(bool onlyActive = false)
		{
			std::vector<std::shared_ptr<Entity>> entities;
			if (onlyActive && !active)
				return entities;
			if (getComponent<T>())
				entities.push_back(shared_from_this());
			for (auto c : children)
			{
				auto chiledEntities = c->getChildrenWithComponent<T>(onlyActive);
				for (auto child : chiledEntities)
					entities.push_back(child);
			}
			return entities;
		}

		void addChild(std::shared_ptr<Entity> child)
		{
			children.push_back(child);
		}

		void getAllNodes(std::map<int, std::shared_ptr<Entity>>& nodes)
		{
			nodes.insert(std::make_pair(id, shared_from_this()));
			for (auto child : children)
				child->getAllNodes(nodes);
		}

		void update(glm::mat4 parentTransform)
		{
			auto t = getComponent<Transform>();
			t->update(parentTransform);
			auto T = t->getTransform();
			for (auto c : children)
				c->update(T);
		}

		void setName(const std::string& name)
		{
			this->name = name;
		}

		std::string getName() const
		{
			return name;
		}

		void setURI(const std::string& uri)
		{
			this->uri = uri;
		}

		std::string getUri() const
		{
			return uri;
		}

		int numChildren()
		{
			return static_cast<int>(children.size());
		}

		std::shared_ptr<Entity> getParent()
		{
			return parent;
		}

		std::shared_ptr<Entity> getChild(int index)
		{
			return children[index];
		}

		std::shared_ptr<Entity> findByID(unsigned int id)
		{
			if (this->id == id)
				return shared_from_this();

			for (auto c : children)
			{
				auto childEntity = c->findByID(id);
				if (childEntity != nullptr)
					return childEntity;
			}

			return nullptr;
		}

		void print(uint32 depth)
		{
			for (uint32 d = 0; d < depth; d++)
				std::cout << "-";
			std::cout << "node: " << name << ", ID: " << id << std::endl;
			for (auto c : children)
				c->print(depth + 1);
		}

		void clearParent()
		{
			parent = nullptr;
			for (auto c : children)
				c->clearParent();
		}

		unsigned int getID()
		{
			return id;
		}

		// TODO: set subtree aswell...
		void setActive(bool active)
		{
			this->active = active;
		}

		bool isActive()
		{
			return active;
		}

		typedef std::shared_ptr<Entity> Ptr;
		static Ptr create(const std::string& name, Ptr parent)
		{
			return std::make_shared<Entity>(name, parent);
		}
	};
}

#endif // INCLUDED_ENTITY