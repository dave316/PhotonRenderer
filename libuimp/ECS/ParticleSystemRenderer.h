#ifndef INCLUDED_UNITYPARTICLESYSTEMRENDERER
#define INCLUDED_UNITYPARTICLESYSTEMRENDERER

#pragma once

#include "Component.h"

namespace Unity
{
	class ParticleSystemRenderer : public Component
	{
	public:
		ParticleSystemRenderer();
		~ParticleSystemRenderer();

		typedef std::shared_ptr<ParticleSystemRenderer> Ptr;
		static Ptr create()
		{
			return std::make_shared<ParticleSystemRenderer>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYPARTICLESYSTEMRENDERER