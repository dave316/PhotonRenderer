#ifndef INCLUDED_GLTFANIMATION
#define INCLUDED_GLTFANIMATION

#pragma once

typedef unsigned int uint;

#include "BinaryData.h"

#include <Graphics/Animation.h>
#include <Graphics/Skin.h>
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace json = rapidjson;

namespace IO
{
	namespace glTF
	{
		struct AnimationChannel
		{
			uint32_t sampler = 0;
			uint32_t targetNode = 0;
			std::string targetPath;
			std::string targetPointer;
		};

		struct AnimationSampler
		{
			uint32_t input = 0;
			uint32_t output = 0;
			Interpolation interpolation = Interpolation::LINEAR;
		};

		Skin::Ptr loadSkin(const json::Value& skinNode, BinaryData& data);
		Animation::Ptr loadAnimation(const json::Value& animNode, BinaryData& data);
		IChannel::Ptr loadPointer(BinaryData& data, AnimationSampler& sampler, AnimationChannel& channel);

		template<typename Type>
		IChannel::Ptr loadChannel(BinaryData& data, AnimationSampler& sampler, AnimAttribute attribute, unsigned int targetIndex)
		{
			std::vector<float> times;
			std::vector<Type> values;
			data.loadData(sampler.input, times);
			data.loadData(sampler.output, values);

			auto channel = Channel<Type>::create(sampler.interpolation, attribute, targetIndex);
			int numValues = values.size() / times.size();
			for (int i = 0; i < times.size(); i++)
			{
				std::vector<Type> allValues;
				for (int j = 0; j < numValues; j++)
				{
					int index = i * numValues + j;
					allValues.push_back(values[index]);
				}
				channel->addValue(times[i], allValues);
			}
			return channel;
		}
	}
}

#endif // INCLUDED_GLTFANIMATION