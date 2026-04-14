#ifndef INCLUDED_UNITYCHARACTERCONTROLLER
#define INCLUDED_UNITYCHARACTERCONTROLLER

#pragma once

#include "Component.h"

namespace Unity
{
	class CharacterController : public Component
	{
	public:
		CharacterController();
		~CharacterController();

		typedef std::shared_ptr<CharacterController> Ptr;
		static Ptr create()
		{
			return std::make_shared<CharacterController>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYCHARACTERCONTROLLER