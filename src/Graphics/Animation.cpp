#include "Animation.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

int Channel::findPosition(float currentTime)
{
	if (currentTime < positions[0].first)
		return -1;

	for (int i = 0; i < positions.size() - 1; i++)
	{
		if (currentTime < positions[i + 1].first)
		{
			return i;
		}
	}
	return -1;
}

int Channel::findRotation(float currentTime)
{
	if (currentTime < rotations[0].first)
		return -1;

	for (int i = 0; i < rotations.size() - 1; i++)
	{
		if (currentTime < rotations[i + 1].first)
		{
			return i;
		}
	}

	return -1;
}

int Channel::findScaling(float currentTime)
{
	if (currentTime < scales[0].first)
		return -1;

	for (int i = 0; i < scales.size() - 1; i++)
	{
		if (currentTime < scales[i + 1].first)
		{
			return i;
		}
	}
	return -1;
}

Animation::Animation() :
	name(""),
	currentTime(0.0f),
	duration(0.0f)
{

}

Animation::Animation(const std::string& name, float duration) :
	name(name),
	currentTime(0.0f),
	duration(duration)
{

}

Animation::~Animation()
{
	std::cout << "deleted Animation " << name << std::endl;
}

void Animation::addChannel(int index, Channel& channel)
{
	channels.insert(std::make_pair(index, channel));
}

glm::vec3 Animation::calcInterpPosition(int index)
{
	if (channels.find(index) == channels.end())
		return glm::vec3(0.0f);

	glm::vec3 result(0.0f);
	Channel& channel = channels[index];
	if (channel.positions.size() == 1)
	{
		result = channel.positions[0].second;
	}
	else if (channel.positions.size() > 1)
	{
		int positionIndex = channel.findPosition(currentTime);
		if (positionIndex < 0)
			return channel.positions[0].second;
		int nextPositionIndex = positionIndex + 1;
		float deltaTime = channel.positions[nextPositionIndex].first - channel.positions[positionIndex].first;
		float factor = (currentTime - channel.positions[positionIndex].first) / deltaTime;
		glm::vec3 start = channel.positions[positionIndex].second;
		glm::vec3 end = channel.positions[nextPositionIndex].second;
		result = glm::mix(start, end, factor);
	}
	return result;
}

glm::quat Animation::calcInterpRotation(int index)
{
	if (channels.find(index) == channels.end())
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
	Channel& channel = channels[index];
	if (channel.rotations.size() == 1)
	{
		result = channel.rotations[0].second;
	}
	else if (channel.rotations.size() > 1)
	{
		int rotationIndex = channel.findRotation(currentTime);
		if (rotationIndex < 0)
			return channel.rotations[0].second;
		int nextRotationIndex = rotationIndex + 1;
		float deltaTime = channel.rotations[nextRotationIndex].first - channel.rotations[rotationIndex].first;
		float factor = (currentTime - channel.rotations[rotationIndex].first) / deltaTime;
		const glm::quat& start = channel.rotations[rotationIndex].second;
		const glm::quat& end = channel.rotations[nextRotationIndex].second;
		result = glm::slerp(start, end, factor);
	}
	return glm::mat4_cast(result);
}

glm::vec3 Animation::calcInterpScaling(int index)
{
	if (channels.find(index) == channels.end())
		return glm::vec3(1.0f);

	glm::vec3 result(1.0f);
	Channel& channel = channels[index];
	if (channel.scales.size() == 1)
	{
		result = channel.scales[0].second;
	}
	else if (channel.scales.size() > 1)
	{
		int scaleIndex = channel.findScaling(currentTime);
		if (scaleIndex < 0)
			return channel.scales[0].second;
		int nextScaleIndex = scaleIndex + 1;
		float deltaTime = channel.scales[nextScaleIndex].first - channel.scales[scaleIndex].first;
		float factor = (currentTime - channel.scales[scaleIndex].first) / deltaTime;
		glm::vec3 start = channel.scales[scaleIndex].second;
		glm::vec3 end = channel.scales[nextScaleIndex].second;
		result = glm::mix(start, end, factor);
	}
	return result;
}

void Animation::update(float dt, std::vector<Entity::Ptr>& nodes)
{
	// TODO: update the animation to the last keyframe
	currentTime += dt;
	if (currentTime > duration)
	{
		finished = true;
		return;
	}		

	for (auto&& [nodeIndex, channel] : channels)
	{
		glm::vec3 translation = calcInterpPosition(nodeIndex);
		glm::quat rotation = calcInterpRotation(nodeIndex);
		glm::vec3 scale = calcInterpScaling(nodeIndex);
		auto t = nodes[nodeIndex]->getComponent<Transform>();
		if(!channel.positions.empty())
			t->setPosition(translation);
		if (!channel.rotations.empty())
			t->setRotation(rotation);
		if (!channel.scales.empty())
			t->setScale(scale);
	}
}

void Animation::reset()
{
	currentTime = 0.0f;
	finished = false;
}

bool Animation::isFinished()
{
	return finished;
}

void Animation::print()
{
	std::cout << "duration: " << duration << std::endl;
	//for (auto&& [idx, ch] : channels)
	//	std::cout << "target node index " << idx
	//	<< " pos keys " << ch.positions.size()
	//	<< " rot keys " << ch.rotations.size()
	//	<< " scale keys " << ch.scales.size()
	//	<< std::endl;
}