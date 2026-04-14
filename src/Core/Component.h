#ifndef INCLUDED_COMPONENT
#define INCLUDED_COMPONENT

#pragma once

#include <memory>

namespace pr
{
	class Component
	{
	public:
		Component();
		virtual ~Component() = 0;
		unsigned int getID();
		typedef std::shared_ptr<Component> Ptr;
	private:
		unsigned int id;
		static unsigned int globalIDCount;
	};
}

#endif // INCLUDED_COMPONENT