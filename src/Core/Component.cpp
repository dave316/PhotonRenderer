#include "Component.h"

unsigned int pr::Component::globalIDCount = 0;

namespace pr
{
	Component::Component()
	{
		id = globalIDCount;
		globalIDCount++;
	}
	Component::~Component()
	{}

	unsigned int Component::getID()
	{
		return id;
	}
}
