#ifndef INCLUDED_UNITYAUDIOLISTENER
#define INCLUDED_UNITYAUDIOLISTENER

#pragma once

#include "Component.h"

namespace Unity
{
	class AudioListener : public Component
	{
	public:
		AudioListener();
		~AudioListener();

		typedef std::shared_ptr<AudioListener> Ptr;
		static Ptr create()
		{
			return std::make_shared<AudioListener>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYAUDIOLISTENER