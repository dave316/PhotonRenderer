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

int Channel::findWeight(float currentTime)
{
	if (currentTime < weights[0].first)
		return -1;

	for (int i = 0; i < weights.size() - 1; i++)
	{
		if (currentTime < weights[i + 1].first)
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
		result = channel.positions[0].second[0];
	}
	else if (channel.positions.size() > 1)
	{
		int positionIndex = channel.findPosition(currentTime);
		if (positionIndex < 0)
			return channel.positions[0].second[0];
		int nextPositionIndex = positionIndex + 1;
		float deltaTime = channel.positions[nextPositionIndex].first - channel.positions[positionIndex].first;
		float factor = (currentTime - channel.positions[positionIndex].first) / deltaTime;
		glm::vec3 start = channel.positions[positionIndex].second[0];
		glm::vec3 end = channel.positions[nextPositionIndex].second[0];

		switch (channel.interpolation)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::mix(start, end, factor);
			break;
		case Interpolation::CUBIC: // TODO: check out of bounds ....
			glm::vec3 a_k1 = channel.positions[nextPositionIndex].second[0];
			glm::vec3 v_k = channel.positions[positionIndex].second[1];
			glm::vec3 v_k1 = channel.positions[nextPositionIndex].second[1];
			glm::vec3 b_k = channel.positions[positionIndex].second[2];

			float t_k = channel.positions[positionIndex].first;
			float t_k1 = channel.positions[nextPositionIndex].first;
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::vec3 p_0 = v_k;
			glm::vec3 m_0 = (t_k1 - t_k) * b_k;
			glm::vec3 p_1 = v_k1;
			glm::vec3 m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::vec3 p = (2 * t3 - 3 * t2 + 1) * p_0 +
				(t3 - 2 * t2 + t) * m_0 +
				(-2 * t3 + 3 * t2) * p_1 +
				(t3 - t2) * m_1;

			result = p;

			break;
		}
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
		result = channel.rotations[0].second[0];
	}
	else if (channel.rotations.size() > 1)
	{
		int rotationIndex = channel.findRotation(currentTime);
		if (rotationIndex < 0)
			return channel.rotations[0].second[0];
		int nextRotationIndex = rotationIndex + 1;
		float deltaTime = channel.rotations[nextRotationIndex].first - channel.rotations[rotationIndex].first;
		float factor = (currentTime - channel.rotations[rotationIndex].first) / deltaTime;
		const glm::quat& start = channel.rotations[rotationIndex].second[0];
		const glm::quat& end = channel.rotations[nextRotationIndex].second[0];
		
		switch (channel.interpolation)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::slerp(start, end, factor);
			break;
		case Interpolation::CUBIC:
			glm::quat a_k1 = channel.rotations[nextRotationIndex].second[0];
			glm::quat v_k = channel.rotations[rotationIndex].second[1];
			glm::quat v_k1 = channel.rotations[nextRotationIndex].second[1];
			glm::quat b_k = channel.rotations[rotationIndex].second[2];

			float t_k = channel.rotations[rotationIndex].first;
			float t_k1 = channel.rotations[nextRotationIndex].first;
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::quat p_0 = v_k;
			glm::quat m_0 = (t_k1 - t_k) * b_k;
			glm::quat p_1 = v_k1;
			glm::quat m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::quat q = (2 * t3 - 3 * t2 + 1) * p_0 +
				(t3 - 2 * t2 + t) * m_0 +
				(-2 * t3 + 3 * t2) * p_1 +
				(t3 - t2) * m_1;

			result = glm::normalize(q);

			break;
		}
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
		result = channel.scales[0].second[0];
	}
	else if (channel.scales.size() > 1)
	{
		int scaleIndex = channel.findScaling(currentTime);
		if (scaleIndex < 0)
			return channel.scales[0].second[0];
		int nextScaleIndex = scaleIndex + 1;
		float deltaTime = channel.scales[nextScaleIndex].first - channel.scales[scaleIndex].first;
		float factor = (currentTime - channel.scales[scaleIndex].first) / deltaTime;
		glm::vec3 start = channel.scales[scaleIndex].second[0];
		glm::vec3 end = channel.scales[nextScaleIndex].second[0];

		switch (channel.interpolation)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::mix(start, end, factor);
			break;
		case Interpolation::CUBIC:
			glm::vec3 a_k1 = channel.scales[nextScaleIndex].second[0];
			glm::vec3 v_k = channel.scales[scaleIndex].second[1];
			glm::vec3 v_k1 = channel.scales[nextScaleIndex].second[1];
			glm::vec3 b_k = channel.scales[scaleIndex].second[2];

			float t_k = channel.scales[scaleIndex].first;
			float t_k1 = channel.scales[nextScaleIndex].first;
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::vec3 p_0 = v_k;
			glm::vec3 m_0 = (t_k1 - t_k) * b_k;
			glm::vec3 p_1 = v_k1;
			glm::vec3 m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::vec3 p = (2 * t3 - 3 * t2 + 1) * p_0 +
				(t3 - 2 * t2 + t) * m_0 +
				(-2 * t3 + 3 * t2) * p_1 +
				(t3 - t2) * m_1;

			result = p;

			break;
		}
	}
	return result;
}

std::vector<float> Animation::calcInterpWeight(int index)
{
	if (channels.find(index) == channels.end())
		return std::vector<float>();

	std::vector<float> result;
	Channel& channel = channels[index];
	if (channel.weights.size() == 1)
	{
		return channel.weights[0].second;
	}
	else
	{
		int numWeights = channel.weights[0].second.size();
		int weightIndex = channel.findWeight(currentTime);
		if(weightIndex < 0)
			return channel.weights[0].second;
		int nextWeightIndex = weightIndex + 1;
		float deltaTime = channel.weights[nextWeightIndex].first - channel.weights[weightIndex].first;
		float factor = (currentTime - channel.weights[weightIndex].first) / deltaTime;
		for (int i = 0; i < numWeights; i++)
		{
			float start = channel.weights[weightIndex].second[i];
			float end = channel.weights[nextWeightIndex].second[i];
			float w = glm::mix(start, end, factor);
			result.push_back(w);
		}
	}
	return result;
}

std::vector<float> Animation::getWeights()
{
	return currentWeights;
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
		if (!channel.weights.empty())
			currentWeights = calcInterpWeight(nodeIndex);
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