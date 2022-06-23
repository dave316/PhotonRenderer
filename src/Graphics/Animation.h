#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <Core/Entity.h>
#include <Core/Camera.h>
#include <Core/Light.h>
#include <Graphics/Material.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

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
	MATERIAL_TRANSMISSION_FACTOR = 0x0A000,
	MATERIAL_THICKNESS_FACTOR = 0x0B000,
	MATERIAL_ATTENUATION_COLOR = 0x0C000,
	MATERIAL_ATTENUATION_DISTANCE = 0x0D000,
	MATERIAL_IRIDESCENCE_FACTOR = 0x0E000,
	MATERIAL_IRIDESCENCE_IOR = 0x0F000,
	MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM = 0x10000,

	MATERIAL_BASECOLORTEXTURE = 0x11000,
	MATERIAL_METALROUGHTEXTURE = 0x12000,
	MATERIAL_NORMALTEXTURE = 0x13000,
	MATERIAL_OCCLUSIONTEXTURE = 0x14000,
	MATERIAL_EMISSIVETEXTURE = 0x15000,

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
protected:
	AnimAttribute animAttr;
	unsigned int targetIndex;
	float maxTime = 0.0f;
	float minTime = std::numeric_limits<float>::max();

public:
	IChannel(AnimAttribute animAttr, unsigned int targetIndex) : 
		animAttr(animAttr),
		targetIndex(targetIndex)
	{}
	virtual ~IChannel() = 0
	{

	}

	AnimAttribute getAttributeType()
	{
		return animAttr;
	}

	unsigned int getTargetIndex()
	{
		return targetIndex;
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
};

template<typename Type>
class Channel : public IChannel
{
private:
	std::vector<float> times;
	std::vector<std::vector<Type>> values;
	Interpolation interpolation = Interpolation::LINEAR;

public:
	Channel(Interpolation interpolation, AnimAttribute animAttr, unsigned int targetIndex) :
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

	Type defaultValue()
	{
		return Type();
	}

	Type interpolate(float time)
	{
		if (times.empty() || values.empty())
			return defaultValue();

		Type firstValue = values[0][0];
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

		Type result;
		if (times.size() > 1)
		{
			int nextIndex = index + 1;
			float deltaTime = times[nextIndex] - times[index];
			float factor = (time - times[index]) / deltaTime;
			Type start = values[index][0];
			Type end = values[nextIndex][0];

			switch (interpolation)
			{
			case Interpolation::STEP:
				if (factor < 0.5f)
					result = start;
				else
					result = end;
				break;
			case Interpolation::LINEAR:
				result = mix(start, end, factor);
				break;
			case Interpolation::CUBIC: // TODO: check out of bounds ....
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
		int numValues = values[0].size();
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
	static Ptr create(Interpolation interpolation, AnimAttribute animAttr, unsigned int targetIndex)
	{
		return std::make_shared<Channel<Type>>(interpolation, animAttr, targetIndex);
	}
};

glm::quat Channel<glm::quat>::mix(glm::quat a, glm::quat b, float f)
{
	return glm::slerp(a, b, f);
}

glm::quat Channel<glm::quat>::defaultValue()
{
	return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

class Animation
{
private:
	std::string name;
	std::vector<IChannel::Ptr> channels;
	std::vector<float> currentWeights; // TODO: should be put in renderable

	float currentTime;
	float duration;
	bool finished = false;

	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
public:
	Animation();
	Animation(const std::string& name);
	~Animation();

	std::vector<float> getWeights();

	template<typename Type>
	void setPropertyValue(IChannel::Ptr iChannel, Material::Ptr material, std::string key)
	{
		auto channel = std::dynamic_pointer_cast<Channel<Type>>(iChannel);
		Type value = channel->interpolate(currentTime);
		material->addProperty(key, value);
	}

	// TODO: use template function with component as argument 
	void setCameraProperty(IChannel::Ptr iChannel, Camera::Ptr camera, AnimAttribute attribute);
	void setLightProperty(IChannel::Ptr iChannel, Light::Ptr light, AnimAttribute attribute);
	void setTransformProperty(IChannel::Ptr iChannel, Transform::Ptr transform, AnimAttribute attribute);
	void setMaterialProperty(IChannel::Ptr iChannel, Material::Ptr material, AnimAttribute attribute);
	void setTextureTransform(IChannel::Ptr iChannel, Material::Ptr material, AnimAttribute attribute, std::string texUniform);

	void addChannel(IChannel::Ptr channel);
	void setDuration(float duration);
	void update(float dt, std::vector<Entity::Ptr>& nodes, std::vector<Material::Ptr>& materials, std::vector<Light::Ptr>& lights, std::vector<Camera::Ptr>& cameras);
	void reset();
	bool isFinished();
	void print();
	std::string getName();
	typedef std::shared_ptr<Animation> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Animation>(name);
	}
};

#endif // INCLUDED_ANIMATION