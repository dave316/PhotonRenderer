#ifndef INCLUDED_UNITYGAMEOBJECT
#define INCLUDED_UNITYGAMEOBJECT

#pragma once

#include "Component.h"
#include <map>
#include <string>
#include <typeindex>
#include <vector>

namespace Unity
{
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject(std::string name);
		~GameObject();

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
				return std::dynamic_pointer_cast<T>(components[typeid(T)]);
			return nullptr;
		}

		std::string getName();
		void setActive(bool active);
		bool isActive();

		typedef std::shared_ptr<GameObject> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<GameObject>(name);
		}
	private:
		unsigned int id;
		static unsigned int globalIDCount;
		bool active = true;
		std::string name = "";
		std::map<std::type_index, Component::Ptr> components;
	};
}

#endif // INCLUDED_UNITYGAMEOBJECT