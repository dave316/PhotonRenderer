#include "Animation.h"
#include <iostream>

Animation::Animation() :
	name(""),
	currentTransform(1.0f),
	currentTime(0.0f),
	ticksPerSecond(0.0f),
	duration(0.0f)
{

}

Animation::Animation(const std::string& name, float ticksPerSecond, float duration) :
	name(name),
	currentTransform(1.0f),
	currentTime(0.0f),
	ticksPerSecond(ticksPerSecond),
	duration(duration)
{

}

void Animation::setPositions(std::vector<std::pair<float, glm::vec3>>& positions)
{
	this->positions = positions;
}

void Animation::setRotations(std::vector<std::pair<float, glm::quat>>& rotations)
{
	this->rotations = rotations;
}

void Animation::setScales(std::vector<std::pair<float, glm::vec3>>& scales)
{
	this->scales = scales;
}

unsigned int Animation::findPosition(float currentTime)
{
	for (int i = 0; i < positions.size() - 1; i++)
	{
		if (currentTime < positions[i + 1].first)
		{
			return i;
		}
	}
	return 0;
}

unsigned int Animation::findRotation(float currentTime)
{
	for (int i = 0; i < rotations.size() - 1; i++)
	{
		if (currentTime < rotations[i + 1].first)
		{
			return i;
		}
	}
	return 0;
}

unsigned int Animation::findScaling(float currentTime)
{
	for (int i = 0; i < scales.size() - 1; i++)
	{
		if (currentTime < scales[i + 1].first)
		{
			return i;
		}
	}
	return 0;
}

glm::mat4 Animation::calcInterpPosition()
{
	glm::vec3 result;
	if (positions.size() == 1)
	{
		result = positions[0].second;
	}
	else
	{
		unsigned int positionIndex = findPosition(currentTime);
		unsigned int nextPositionIndex = positionIndex + 1;
		float deltaTime = positions[nextPositionIndex].first - positions[positionIndex].first;
		float factor = (currentTime - positions[positionIndex].first) / deltaTime;
		glm::vec3 start = positions[positionIndex].second;
		glm::vec3 end = positions[nextPositionIndex].second;
		result = glm::mix(start, end, factor);
	}
	return glm::translate(glm::mat4(1.0f), result);
}

glm::mat4 Animation::calcInterpRotation()
{
	glm::quat result;
	if (rotations.size() == 1)
	{
		result = rotations[0].second;
	}
	else
	{
		unsigned int rotationIndex = findRotation(currentTime);
		unsigned int nextRotationIndex = rotationIndex + 1;
		float deltaTime = rotations[nextRotationIndex].first - rotations[rotationIndex].first;
		float factor = (currentTime - rotations[rotationIndex].first) / deltaTime;
		const glm::quat& start = rotations[rotationIndex].second;
		const glm::quat& end = rotations[nextRotationIndex].second;
		result = glm::slerp(start, end, factor);
	}
	return glm::mat4_cast(result);
}

glm::mat4 Animation::calcInterpScaling()
{
	glm::vec3 result;
	if (scales.size() == 1)
	{
		result = scales[0].second;
	}
	else
	{
		unsigned int scaleIndex = findScaling(currentTime);
		unsigned int nextScaleIndex = scaleIndex + 1;
		float deltaTime = scales[nextScaleIndex].first - scales[scaleIndex].first;
		float factor = (currentTime - scales[scaleIndex].first) / deltaTime;
		glm::vec3 start = scales[scaleIndex].second;
		glm::vec3 end = scales[nextScaleIndex].second;
		result = glm::mix(start, end, factor);
	}
	return glm::scale(glm::mat4(1.0f), result);
}

void Animation::update(float time)
{
	currentTime = fmodf(time, duration);
	glm::mat4 translation = calcInterpPosition();
	glm::mat4 rotation = calcInterpRotation();
	glm::mat4 scale = calcInterpScaling();
	currentTransform = translation * rotation * scale;
}

std::string Animation::getName()
{
	return name;
}

glm::mat4 Animation::getTransform()
{
	return currentTransform;
}