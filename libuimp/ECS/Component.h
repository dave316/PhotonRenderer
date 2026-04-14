#ifndef INCLUDED_UNITYCOMPONENT
#define INCLUDED_UNITYCOMPONENT

#pragma once

#include <memory>

namespace Unity
{
	class Component
	{
	public:
		Component();
		virtual ~Component() = 0;
		typedef std::shared_ptr<Component> Ptr;
	private:
	};
}

#endif // INCLUDED_UNITYCOMPONENT