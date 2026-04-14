#ifndef INCLUDED_UNITYCAPSULECOLLIDER
#define INCLUDED_UNITYCAPSULECOLLIDER

#pragma once

#include "Component.h"

namespace Unity
{
	class CapsuleCollider : public Component
	{
	public:
		CapsuleCollider();
		~CapsuleCollider();

		typedef std::shared_ptr<CapsuleCollider> Ptr;
		static Ptr create()
		{
			return std::make_shared<CapsuleCollider>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYCAPSULECOLLIDER