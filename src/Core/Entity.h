#ifndef INCLUDED_ENTITY
#define INCLUDED_ENTITY

#pragma once

#include "Component.h"
#include "Transform.h"

#include <map>
#include <string>
#include <typeindex>
#include <iostream>

class Entity
{
private:
	std::string name;
	std::map<std::type_index, Component::Ptr> components;

public:
	Entity(const std::string& name) : name(name)
	{
		auto t = Transform::Ptr(new Transform(this));
		addComponent(t);
	}

	~Entity()
	{
		std::cout << "Entity " << name << " destroyed" << std::endl;
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
		Transform::Ptr transform = getComponent<Transform>();
		for (int i = 0; i < transform->getNumChildren(); i++)
		{
			auto childTransform = transform->getChild(i);
			auto e = childTransform->getEntity();
			
			auto component = e->getComponent<T>();
			if(component)
				allComponents.push_back(e->getComponent<T>());

			std::vector<std::shared_ptr<T>> childComponents = e->getComponentsInChildren<T>();
			for (auto childComp : childComponents)
				allComponents.push_back(childComp);
		}
		return allComponents;
	}

	template<typename T>
	std::vector<Entity*> getChildrenWithComponent()
	{
		std::vector<Entity*> entities;
		Transform::Ptr transform = getComponent<Transform>();
		for (int i = 0; i < transform->getNumChildren(); i++)
		{
			auto childTransform = transform->getChild(i);
			auto e = childTransform->getEntity();

			auto component = e->getComponent<T>();
			if (component)
				entities.push_back(e);

			auto chiledEntities = e->getChildrenWithComponent<T>();
			for (auto child : chiledEntities)
				entities.push_back(child);
		}
		return entities;
	}

	std::string getName() const
	{
		return name;
	}

	typedef std::shared_ptr<Entity> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Entity>(name);
	}
};


#endif // INCLUDED_ENTITY