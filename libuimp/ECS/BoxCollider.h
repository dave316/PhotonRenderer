#ifndef INCLUDED_UNITYBOXCOLLIDER
#define INCLUDED_UNITYBOXCOLLIDER

#pragma once

#include "Component.h"

namespace Unity
{
	class BoxCollider : public Component
	{
	public:
		BoxCollider();
		~BoxCollider();

		typedef std::shared_ptr<BoxCollider> Ptr;
		static Ptr create()
		{
			return std::make_shared<BoxCollider>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYBOXCOLLIDER