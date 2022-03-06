#ifndef INCLUDED_ENTITY
#define INCLUDED_ENTITY

#pragma once

#include "Component.h"
#include "Transform.h"

#include <map>
#include <string>
#include <typeindex>
#include <iostream>

class Entity : public std::enable_shared_from_this<Entity>
{
private:
	std::shared_ptr<Entity> parent = nullptr;
	std::string name;
	std::map<std::type_index, Component::Ptr> components;
	std::vector<std::shared_ptr<Entity>> children;

	unsigned int id;

	static unsigned int globalIDCount;

public:
	Entity(const std::string& name, std::shared_ptr<Entity> parent) : name(name), parent(parent)
	{
		auto t = Transform::Ptr(new Transform());
		addComponent(t);
		id = globalIDCount;
		globalIDCount++;
	}

	~Entity()
	{
		//std::cout << "Entity " << name << " destroyed" << std::endl;
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
		if (components.find(typeid(T)) != components.end())
		{
			std::cout << "warning component " << typeid(T).name() << " already exists, and is NOT replaced! (TODO allow to replace it)" << std::endl;
			return;
		}
		components[typeid(T)] = component;
	}

	template<typename T>
	std::shared_ptr<T> getComponent()
	{
		if (components.find(typeid(T)) != components.end())
		{
			return std::dynamic_pointer_cast<T>(components[typeid(T)]);
		}
		else
		{
			//std::cout << "cannot find component (" << typeid(T).name() << ")" << std::endl;
		}
		return nullptr;
	}

	template<typename T>
	std::vector<std::shared_ptr<T>> getComponentsInChildren()
	{
		std::vector<std::shared_ptr<T>> allComponents;
		auto rootComponent = getComponent<T>();
		if(rootComponent)
			allComponents.push_back(rootComponent);
		for (auto c : children)
		{
			auto childComponents = c->getComponentsInChildren<T>();
			for (auto childComp : childComponents)
				allComponents.push_back(childComp);
		}
		return allComponents;
	}

	template<typename T>
	std::vector<std::shared_ptr<Entity>> getChildrenWithComponent()
	{
		std::vector<std::shared_ptr<Entity>> entities;
		if (getComponent<T>())
			entities.push_back(shared_from_this());
		for (auto c : children)
		{
			auto chiledEntities = c->getChildrenWithComponent<T>();
			for (auto child : chiledEntities)
				entities.push_back(child);
		}
		return entities;
	}

	void addChild(std::shared_ptr<Entity> child)
	{
		children.push_back(child);
	}

	void removeChild(std::string name)
	{
		// TODO: this is terrible... find a better way to implement this
		int index = -1;
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i]->getName().compare(name) == 0)
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
			children.erase(children.begin() + index);
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

	std::string getName() const
	{
		return name;
	}

	int numChildren()
	{
		return children.size();
	}

	void setParent(std::shared_ptr<Entity> parent)
	{
		this->parent = parent;
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

	unsigned int getID()
	{
		return id;
	}

	typedef std::shared_ptr<Entity> Ptr;
	static Ptr create(const std::string& name, Ptr parent)
	{
		return std::make_shared<Entity>(name, parent);
	}
};

#endif // INCLUDED_ENTITY