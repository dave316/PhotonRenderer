#include "FogVolume.h"
#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>
namespace pr
{
	FogVolume::FogVolume()
	{

	}
	
	FogVolume::~FogVolume()
	{

	}

	void FogVolume::writeUniformData(FogVolumeUniformData& data, Transform::Ptr transform)
	{
		float sigmaS = glm::max(glm::max(scattering.r, scattering.g), scattering.b);
		float extinction = absorbtion + sigmaS;

		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
			data.worldToLocal = glm::transpose(glm::inverse(transform->getTransform()));
		else
			data.worldToLocal = glm::inverse(transform->getTransform());
		data.scatteringExtinction = glm::vec4(scattering, extinction);
		data.emissive = glm::vec4(emissive, 0.0f);
		data.phase = phase;
		data.densityTexIndex = -1;
	}

	void FogVolume::setDensityPerlinNoise(float factor)
	{
		int s = 32;
		uint32 size = s * s * s;
		float* buffer = new float[size];
		for (int i = 0; i < s; i++)
		{
			for (int j = 0; j < s; j++)
			{
				for (int k = 0; k < s; k++)
				{
					int c = 1;
					float x = static_cast<float>(i) / 8.0f;
					float y = static_cast<float>(j) / 8.0f;
					float z = static_cast<float>(k) / 8.0f;

					int o;
					float frequency = 1.0f;
					float amplitude = 1.0f;
					float sum = 0.0f;

					for (o = 0; o < 8; o++) {
						float r = stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 4, 4, 4, (unsigned char)o) * amplitude;
						sum += (float)fabs(r);
						frequency *= 2.0f;
						amplitude *= 0.5f;
					}
					buffer[k * s * s * c + j * s * c + i * c + 0] = factor * sum;
				}
			}
		}

		density = pr::Texture3D::create(s, s, s, GPU::Format::R32F);
		density->upload((uint8*)buffer, size * sizeof(float));
		density->setAddressMode(GPU::AddressMode::Repeat);
		density->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
		density->setLayout();
	}

	void FogVolume::setDensitySphere(float factor)
	{
		int s = 32;
		uint32 size = s * s * s;
		float* buffer = new float[size];
		for (int i = 0; i < s; i++)
		{
			for (int j = 0; j < s; j++)
			{
				for (int k = 0; k < s; k++)
				{
					int c = 1;
					float sx = static_cast<float>(i) / 31.0f;
					float sy = static_cast<float>(j) / 31.0f;
					float sz = static_cast<float>(k) / 31.0f;
					glm::vec3 pos = glm::vec3(sx, sy, sz) * 2.0f - 1.0f;
					float len = glm::length(pos);
					buffer[k * s * s * c + j * s * c + i * c + 0] = factor * glm::clamp(1.0f - len, 0.0f, 1.0f);
				}
			}
		}

		density = pr::Texture3D::create(s, s, s, GPU::Format::R32F);
		density->upload((uint8*)buffer, size * sizeof(float));
		density->setAddressMode(GPU::AddressMode::Repeat);
		density->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
		density->setLayout();
	}

	void FogVolume::setEmissive(glm::vec3 emissive)
	{
		this->emissive = emissive;
	}

	void FogVolume::setScattering(glm::vec3 scattering)
	{
		this->scattering = scattering;
	}

	void FogVolume::setAbsorbtion(float absorbtion)
	{
		this->absorbtion = absorbtion;
	}

	void FogVolume::setPhase(float phase)
	{
		this->phase = phase;
	}

	Texture3D::Ptr FogVolume::getDensityTex()
	{
		return density;
	}
}
