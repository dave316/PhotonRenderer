#include "MorphAnimation.h"
#include <iostream>

MorphAnimation::MorphAnimation() :
	name(""),
	currentWeight(0.0f),
	currentTime(0.0f),
	ticksPerSecond(0.0f),
	duration(0.0f)
{

}

MorphAnimation::MorphAnimation(const std::string& name, float ticksPerSecond, float duration, std::vector<std::pair<float, std::pair<float, float>>> weights) :
	name(name),
	currentWeight(0.0f),
	currentTime(0.0f),
	ticksPerSecond(ticksPerSecond),
	duration(duration),
	weights(weights)
{
}

unsigned int MorphAnimation::findWeights(float currentTime)
{
	for (int i = 0; i < weights.size() - 1; i++)
	{
		if (currentTime < weights[i + 1].first)
		{
			return i;
		}
	}
	return 0;
}

glm::vec2 MorphAnimation::calcInterpWeight()
{
	glm::vec2 result;
	if (weights.size() == 1)
	{
		result.x = weights[0].second.first;
		result.y = weights[0].second.second;
	}
	else
	{
		unsigned int weightIndex = findWeights(currentTime);
		unsigned int nextWeightIndex = weightIndex + 1;
		float deltaTime = weights[nextWeightIndex].first - weights[weightIndex].first;
		float factor = (currentTime - weights[weightIndex].first) / deltaTime;
		float w0_start = weights[weightIndex].second.first;
		float w1_start = weights[weightIndex].second.second;
		float w0_end = weights[nextWeightIndex].second.first;
		float w1_end = weights[nextWeightIndex].second.second;
		result.x = glm::mix(w0_start, w0_end, factor);
		result.y = glm::mix(w1_start, w1_end, factor);
	}
	return result;
}

void MorphAnimation::update(float time)
{
	currentTime = fmodf(time, duration);
	currentWeight = calcInterpWeight();
}

glm::vec2 MorphAnimation::getWeights()
{
	return currentWeight;
}