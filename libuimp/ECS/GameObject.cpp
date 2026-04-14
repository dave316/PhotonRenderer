#include "GameObject.h"
#include <iostream>

unsigned int Unity::GameObject::globalIDCount = 0;
namespace Unity
{
	GameObject::GameObject(std::string name) :
		name(name)
	{
		id = globalIDCount;
		globalIDCount++;

		//std::cout << "creating GameObject " << std::to_string(id) << std::endl;
	}

	GameObject::~GameObject()
	{
		//std::cout << "destroying GameObject " << std::to_string(id) << std::endl;
	}

	std::string GameObject::getName()
	{
		return name;
	}

	void GameObject::setActive(bool active)
	{
		this->active = active;
	}

	bool GameObject::isActive()
	{
		return active;
	}
}