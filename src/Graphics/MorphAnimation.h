#ifndef INCLUDED_MORPHANIMATION
#define INCLUDED_MORPHANIMATION

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

class MorphAnimation
{
	std::string name;
	std::vector<std::pair<float, std::pair<float, float>>> weights;
	glm::vec2 currentWeight;

	float currentTime;
	float ticksPerSecond;
	float duration;
	unsigned int nodeIndex;

	MorphAnimation(const MorphAnimation&) = delete;
	MorphAnimation& operator=(const MorphAnimation&) = delete;
public:
	MorphAnimation();
	MorphAnimation(const std::string& name, float ticksPerSecond, float duration, unsigned int nodeIndex, std::vector<std::pair<float, std::pair<float, float>>> weights);

	unsigned int findWeights(float currentTime);
	glm::vec2 calcInterpWeight();

	void update(float time);
	glm::vec2 getWeights();

	unsigned int getNodeIndex();

	typedef std::shared_ptr<MorphAnimation> Ptr;
	static Ptr create()
	{
		return std::make_shared<MorphAnimation>();
	}
};

#endif // INCLUDED_MORPHANIMATION