#ifndef INCLUDED_FOGVOLUME
#define INCLUDED_FOGVOLUME

#pragma once

#include "Component.h"
#include "Transform.h"
#include <Graphics/Texture.h>

namespace pr
{
	struct FogVolumeUniformData
	{
		glm::mat4 worldToLocal;
		glm::vec4 scatteringExtinction;
		glm::vec4 emissive;
		float phase;
		int densityTexIndex;
		uint32 padding0;
		uint32 padding1;
	};

	class FogVolume : public Component
	{
	public:
		FogVolume();
		~FogVolume();

		void writeUniformData(FogVolumeUniformData& data, Transform::Ptr transform);
		void setDensityPerlinNoise(float factor);
		void setDensitySphere(float factor);
		void setEmissive(glm::vec3 emissive);
		void setScattering(glm::vec3 scattering);
		void setAbsorbtion(float absorbtion);
		void setPhase(float phase);
		Texture3D::Ptr getDensityTex();
		glm::vec3 getEmissive() { return emissive; }
		glm::vec3 getScattering() { return scattering; }
		float getAbsorbtion() { return absorbtion; }
		float getPhase() { return phase; }

		typedef std::shared_ptr<FogVolume> Ptr;
		static Ptr create()
		{
			return std::make_shared<FogVolume>();
		}
	private:
		Texture3D::Ptr density;
		glm::vec3 emissive = glm::vec3(0);
		glm::vec3 scattering = glm::vec3(1);
		float absorbtion = 0.0f;
		float phase = 0.0f;
		bool useDensityTex = false;
	};
}

#endif // INCLUDED_FOGVOLUME