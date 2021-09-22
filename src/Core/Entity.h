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
	std::string name;
	std::map<std::type_index, Component::Ptr> components;
	std::vector<std::shared_ptr<Entity>> children;

public:
	Entity(const std::string& name) : name(name)
	{
		auto t = Transform::Ptr(new Transform());
		addComponent(t);
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

	std::shared_ptr<Entity> getChild(int index)
	{
		return children[index];
	}

	typedef std::shared_ptr<Entity> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Entity>(name);
	}
};


#endif // INCLUDED_ENTITY