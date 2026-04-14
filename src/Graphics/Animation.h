#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <Platform/Types.h>
#include <glm/glm.hpp>
#include <iostream>
#include <Core/Entity.h>

#include "Material.h"
namespace pr
{
	enum class Interpolation
	{
		STEP,
		LINEAR,
		CUBIC
	};

	enum class AnimAttribute
	{
		TRANSFORM_POSITION = 0x1,
		TRANSFORM_ROTATION = 0x2,
		TRANSFORM_SCALE = 0x3,
		TRANSFORM_WEIGHTS = 0x4,
		TRANSFORM_MASK = 0xF,

		CAMERA_ASPECT = 0x10,
		CAMERA_YFOV = 0x20,
		CAMERA_ZFAR = 0x30,
		CAMERA_ZNEAR = 0x40,
		CAMERA_MASK = 0xF0,

		LIGHT_COLOR = 0x100,
		LIGHT_INTENSITY = 0x200,
		LIGHT_RANGE = 0x300,
		LIGHT_INNERANGLE = 0x400,
		LIGHT_OUTERANGLE = 0x500,
		LIGHT_MASK = 0xF00,

		MATERIAL_BASECOLOR = 0x01000,
		MATERIAL_ROUGHNESS = 0x02000,
		MATERIAL_METALLIC = 0x03000,
		MATERIAL_EMISSIVE_FACTOR = 0x04000,
		MATERIAL_EMISSIVE_STRENGTH = 0x05000,
		MATERIAL_ALPHA_CUTOFF = 0x06000,
		MATERIAL_NORMAL_SCALE = 0x07000,
		MATERIAL_OCCLUSION_STRENGTH = 0x08000,
		MATERIAL_IOR = 0x09000,

		MATERIAL_SHEEN_COLOR_FACTOR = 0x0A000,
		MATERIAL_SHEEN_ROUGHNESS_FACTOR = 0x0B000,
		MATERIAL_CLEARCOAT_FACTOR = 0x0C000,
		MATERIAL_CLEARCOAT_ROUGHNESS_FACTOR = 0x0D000,
		MATERIAL_TRANSMISSION_FACTOR = 0x0E000,
		MATERIAL_THICKNESS_FACTOR = 0x0F000,
		MATERIAL_ATTENUATION_COLOR = 0x10000,
		MATERIAL_ATTENUATION_DISTANCE = 0x11000,
		MATERIAL_SPECULAR_FACTOR = 0x12000,
		MATERIAL_SPECULAR_COLOR_FACTOR = 0x13000,
		MATERIAL_IRIDESCENCE_FACTOR = 0x14000,
		MATERIAL_IRIDESCENCE_IOR = 0x15000,
		MATERIAL_IRIDESCENCE_THICKNESS_MINIMUM = 0x16000,
		MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM = 0x17000,
		MATERIAL_ANISOTROPY_STRENGTH = 0x18000,
		MATERIAL_ANISOTROPY_ROTATION = 0x19000,
		MATERIAL_DIFFUSE_TRANSMISSION_FACTOR = 0x1A000,
		MATERIAL_DIFFUSE_TRANSMISSION_COLOR_FACTOR = 0x1B000,
		MATERIAL_DISPERSION = 0x1C000,

		MATERIAL_BASECOLOR_TEXTURE = 0x1D000,
		MATERIAL_METALROUGH_TEXTURE = 0x1E000,
		MATERIAL_NORMAL_TEXTURE = 0x1F000,
		MATERIAL_OCCLUSION_TEXTURE = 0x20000,
		MATERIAL_EMISSIVE_TEXTURE = 0x21000,
		MATERIAL_SHEEN_COLOR_TEXTURE = 0x22000,
		MATERIAL_SHEEN_ROUGHNESS_TEXTURE = 0x23000,
		MATERIAL_CLEARCOAT_TEXTURE = 0x24000,
		MATERIAL_CLEARCOAT_ROUGHNESS_TEXTURE = 0x25000,
		MATERIAL_CLEARCOAT_NORMAL_TEXTURE = 0x26000,
		MATERIAL_TRANSMISSION_TEXTURE = 0x27000,
		MATERIAL_THICKNESS_TEXTURE = 0x28000,
		MATERIAL_SPECULAR_TEXTURE = 0x29000,
		MATERIAL_SPECULAR_COLOR_TEXTURE = 0x2A000,
		MATERIAL_IRIDESCENCE_TEXTURE = 0x2B000,
		MATERIAL_IRIDESCENCE_THICKNESS_TEXTURE = 0x2C000,
		MATERIAL_ANISOTROPY_TEXTURE = 0x2D000,
		MATERIAL_DIFFUSE_TRANSMISSION_TEXTURE = 0x2E000,
		MATERIAL_DIFFUSE_TRANSMISSION_COLOR_TEXTURE = 0x2F000,

		MATERIAL_MASK = 0xFF000,

		TEXTURE_OFFSET = 0x100000,
		TEXTURE_ROTATION = 0x200000,
		TEXTURE_SCALE = 0x300000,
		TEXTURE_MASK = 0xF00000
	};


	inline constexpr int operator& (AnimAttribute a, AnimAttribute b)
	{
		return (static_cast<int>(a) & static_cast<int>(b));
	}

	inline constexpr int operator| (AnimAttribute a, AnimAttribute b)
	{
		return (static_cast<int>(a) | static_cast<int>(b));
	}

	class IChannel
	{
	public:
		IChannel(AnimAttribute animAttr, uint32 targetIndex) : 
			animAttr(animAttr),
			targetIndex(targetIndex)
		{

		}
		virtual ~IChannel() = 0
		{

		}
		uint32 getTargetIndex()
		{
			return targetIndex;
		}
		AnimAttribute getAttributeType()
		{
			return animAttr;
		}
		float getMinTime()
		{
			return minTime;
		}

		float getMaxTime()
		{
			return maxTime;
		}
		typedef std::shared_ptr<IChannel> Ptr;
	protected:
		AnimAttribute animAttr;
		uint32 targetIndex = 0;
		float maxTime = 0.0f;
		float minTime = std::numeric_limits<float>::max();
	};

	template<typename Type>
	class Channel : public IChannel
	{
	public:
		Channel(AnimAttribute animAttr, Interpolation interpolation, uint32 targetIndex) :
			IChannel(animAttr, targetIndex),
			interpolation(interpolation)
		{

		}

		void addValue(float time, std::vector<Type> values)
		{
			maxTime = glm::max(maxTime, time);
			minTime = glm::max(minTime, time);

			times.push_back(time);
			this->values.push_back(values);
		}

		Type mix(Type a, Type b, float f)
		{
			return glm::mix(a, b, f);
		}

		Type interpolate(float time)
		{
			if (time < times[0] || time >= times[times.size() - 1])
				return values[0][0];

			// get the first index in time that is before the current time
			int index = 0;
			for (int i = 0; i < times.size() - 1; i++)
			{
				if (time < times[i + 1])
				{
					index = i;
					break;
				}
			}
			
			int nextIndex = index + 1;
			float deltaTime = times[nextIndex] - times[index];
			float factor = (time - times[index]) / deltaTime;
			Type start = values[index][0];
			Type end = values[nextIndex][0];
			Type result = Type();
			switch (interpolation)
			{
				case Interpolation::STEP: // nearest neighbour interpolation
				{
					if (factor < 0.5f)
						result = start;
					else
						result = end;
					break;
				}
				case Interpolation::LINEAR:
				{
					result = mix(start, end, factor);
					break;
				}
				case Interpolation::CUBIC:
				{
					Type a_k1 = values[nextIndex][0];
					Type v_k = values[index][1];
					Type v_k1 = values[nextIndex][1];
					Type b_k = values[index][2];

					float t_k = times[index];
					float t_k1 = times[nextIndex];
					float t_current = time;
					float t = (t_current - t_k) / (t_k1 - t_k);
					Type p_0 = v_k;
					Type m_0 = (t_k1 - t_k) * b_k;
					Type p_1 = v_k1;
					Type m_1 = (t_k1 - t_k) * a_k1;

					float t2 = t * t;
					float t3 = t2 * t;
					Type p = (2 * t3 - 3 * t2 + 1) * p_0 +
						(t3 - 2 * t2 + t) * m_0 +
						(-2 * t3 + 3 * t2) * p_1 +
						(t3 - t2) * m_1;

					result = p;
					break;
				}
			}
			return result;
		}

		std::vector<Type> interpolateElements(float time)
		{
			auto firstValue = values[0];
			if (times.size() == 1)
				return firstValue;
			if (time < times[0] || time >= times[times.size() - 1])
				return firstValue;

			int index = 0;
			for (int i = 0; i < times.size() - 1; i++)
			{
				if (time < times[i + 1])
				{
					index = i;
					break;
				}
			}

			std::vector<Type> result;
			int numValues = static_cast<int>(values[0].size());
			int nextIndex = index + 1;
			float deltaTime = times[nextIndex] - times[index];
			float factor = (time - times[index]) / deltaTime;
			for (int i = 0; i < numValues; i++)
			{
				Type start = values[index][i];
				Type end = values[nextIndex][i];
				Type w = glm::mix(start, end, factor);
				result.push_back(w);
			}
			return result;
		}

		typedef std::shared_ptr<Channel<Type>> Ptr;
		static Ptr create(AnimAttribute animAttr, Interpolation interpolation, unsigned int targetIndex)
		{
			return std::make_shared<Channel<Type>>(animAttr, interpolation, targetIndex);
		}
	private:
		std::vector<float> times;
		std::vector<std::vector<Type>> values;
		Interpolation interpolation = Interpolation::LINEAR;
	};

	glm::quat Channel<glm::quat>::mix(glm::quat a, glm::quat b, float f)
	{
		return glm::slerp(a, b, f);
	}

	class Animation
	{
	public:
		Animation(const std::string& name);
		void addChannel(IChannel::Ptr channel);
		void setDuration(float duration);
		void setMaterialProperty(IChannel::Ptr channel, Material::Ptr material, AnimAttribute attribute);
		void setTextureTransform(IChannel::Ptr iChannel, Material::Ptr material, AnimAttribute attribute, std::string texUniform);
		void update(float dt, std::map<uint32, pr::Component::Ptr> components);
		std::vector<float> getCurrentWeights();
		std::string getName() { return name; }

		template<typename Type>
		void setPropertyValue(IChannel::Ptr iChannel, Material::Ptr material, std::string key)
		{
			auto channel = std::dynamic_pointer_cast<Channel<Type>>(iChannel);
			Type value = channel->interpolate(currentTime);
			material->setProperty(key, value);
		}

		typedef std::shared_ptr<Animation> Ptr;
		static Ptr create(const std::string& name)
		{
			return std::make_shared<Animation>(name);
		}
	private:
		std::string name;
		std::vector<IChannel::Ptr> channels;
		std::vector<float> currentWeights;
		float currentTime = 0.0f;
		float duration = 0.0f;

		Animation(const Animation&) = delete;
		Animation& operator=(const Animation&) = delete;

	};
}

#endif // INCLUDED_SHADER