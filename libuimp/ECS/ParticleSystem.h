#ifndef INCLUDED_UNITYPARTICLESYSTEM
#define INCLUDED_UNITYPARTICLESYSTEM

#pragma once

#include "Component.h"

namespace Unity
{
	class ParticleSystem : public Component
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		typedef std::shared_ptr<ParticleSystem> Ptr;
		static Ptr create()
		{
			return std::make_shared<ParticleSystem>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYPARTICLESYSTEM