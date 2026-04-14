#ifndef INCLUDED_UNITYMONOBEHAVIOUR
#define INCLUDED_UNITYMONOBEHAVIOUR

#pragma once

#include "Component.h"

namespace Unity
{
	class MonoBehaviour : public Component
	{
	public:
		MonoBehaviour();
		~MonoBehaviour();

		typedef std::shared_ptr<MonoBehaviour> Ptr;
		static Ptr create()
		{
			return std::make_shared<MonoBehaviour>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYMONOBEHAVIOUR