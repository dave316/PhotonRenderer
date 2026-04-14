#ifndef INCLUDED_UNITYCANVAS
#define INCLUDED_UNITYCANVAS

#pragma once

#include "Component.h"

namespace Unity
{
	class Canvas : public Component
	{
	public:
		Canvas();
		~Canvas();

		typedef std::shared_ptr<Canvas> Ptr;
		static Ptr create()
		{
			return std::make_shared<Canvas>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYCANVAS