#ifndef INCLUDED_UNITYAUDIOSOURCE
#define INCLUDED_UNITYAUDIOSOURCE

#pragma once

#include "Component.h"

namespace Unity
{
	class AudioSource : public Component
	{
	public:
		AudioSource();
		~AudioSource();

		typedef std::shared_ptr<AudioSource> Ptr;
		static Ptr create()
		{
			return std::make_shared<AudioSource>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYAUDIOSOURCE