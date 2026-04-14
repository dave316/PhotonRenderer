#ifndef INCLUDED_UNITYCAMERA
#define INCLUDED_UNITYCAMERA

#pragma once

#include "Component.h"

namespace Unity
{
	class Camera : public Component
	{
	public:
		Camera();
		~Camera();

		typedef std::shared_ptr<Camera> Ptr;
		static Ptr create()
		{
			return std::make_shared<Camera>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYCAMERA