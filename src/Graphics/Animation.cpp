#include "Animation.h"

#include <glm/gtc/matrix_inverse.hpp>

unsigned int Channel::findPosition(float currentTime)
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

unsigned int Channel::findRotation(float currentTime)
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

unsigned int Channel::findScaling(float currentTime)
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

Animation::Animation() :
	name(""),
	duration(0.0f),
	nodeIndex(0),
	numBones(0)
{

}

Animation::Animation(const std::string& name, float duration, unsigned int nodeIndex, unsigned int numBones) :
	name(name),
	duration(duration),
	nodeIndex(nodeIndex),
	numBones(numBones)
{

}

void Animation::setBoneTree(BoneNode& root)
{
	rootNode = root;
}

void Animation::addChannel(int index, Channel& channel)
{
	channels.insert(std::make_pair(index, channel));
}

glm::mat4 Animation::calcInterpPosition(int index)
{
	if (channels.find(index) == channels.end())
	{
		std::cout << "channel " << index << " not found" << std::endl;
		return glm::mat4(1.0f);
	}

	glm::vec3 result(0.0f);
	Channel& channel = channels[index];
	if (channel.positions.size() == 1)
	{
		result = channel.positions[0].second;
	}
	else if(channel.positions.size() > 1)
	{
		unsigned int positionIndex = channel.findPosition(currentTime);
		unsigned int nextPositionIndex = positionIndex + 1;
		float deltaTime = channel.positions[nextPositionIndex].first - channel.positions[positionIndex].first;
		float factor = (currentTime - channel.positions[positionIndex].first) / deltaTime;
		glm::vec3 start = channel.positions[positionIndex].second;
		glm::vec3 end = channel.positions[nextPositionIndex].second;
		result = glm::mix(start, end, factor);
	}
	//pos = result;
	return glm::translate(glm::mat4(1.0f), result);
}

glm::mat4 Animation::calcInterpRotation(int index)
{
	if (channels.find(index) == channels.end())
	{
		std::cout << "channel " << index << " not found" << std::endl;
		return glm::mat4(1.0f);
	}

	glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
	Channel& channel = channels[index];
	if (channel.rotations.size() == 1)
	{
		result = channel.rotations[0].second;
	}
	else if(channel.rotations.size() > 1)
	{
		unsigned int rotationIndex = channel.findRotation(currentTime);
		unsigned int nextRotationIndex = rotationIndex + 1;
		float deltaTime = channel.rotations[nextRotationIndex].first - channel.rotations[rotationIndex].first;
		float factor = (currentTime - channel.rotations[rotationIndex].first) / deltaTime;
		const glm::quat& start = channel.rotations[rotationIndex].second;
		const glm::quat& end = channel.rotations[nextRotationIndex].second;
		result = glm::slerp(start, end, factor);
	}
	//rot = result;
	return glm::mat4_cast(result);
}

glm::mat4 Animation::calcInterpScaling(int index)
{
	if (channels.find(index) == channels.end())
	{
		std::cout << "channel " << index << " not found" << std::endl;
		return glm::mat4(1.0f);
	}

	glm::vec3 result(1.0f);
	Channel& channel = channels[index];
	if (channel.scales.size() == 1)
	{
		result = channel.scales[0].second;
	}
	else if(channel.scales.size() > 1)
	{
		unsigned int scaleIndex = channel.findScaling(currentTime);
		unsigned int nextScaleIndex = scaleIndex + 1;
		float deltaTime = channel.scales[nextScaleIndex].first - channel.scales[scaleIndex].first;
		float factor = (currentTime - channel.scales[scaleIndex].first) / deltaTime;
		glm::vec3 start = channel.scales[scaleIndex].second;
		glm::vec3 end = channel.scales[nextScaleIndex].second;
		result = glm::mix(start, end, factor);
	}
	//scale = result;
	return glm::scale(glm::mat4(1.0f), result);
}

void Animation::readBoneTree(BoneNode& node, glm::mat4 parentTransform)
{
	//std::cout << "joint: " << node.boneIndex << std::endl;

	glm::mat4 translation = calcInterpPosition(node.boneIndex);
	glm::mat4 rotation = calcInterpRotation(node.boneIndex);
	glm::mat4 scale = calcInterpScaling(node.boneIndex);
	glm::mat4 localTransform = translation * rotation * scale;
	glm::mat4 globalTransform = parentTransform * localTransform;
	glm::mat4 boneTransform = globalTransform * node.boneTransform;
	glm::mat3 normalTransform = glm::inverseTranspose(glm::mat3(boneTransform));
	//boneTransforms.push_back(boneTransform);
	boneTransforms[node.jointIndex] = boneTransform;
	normalTransforms[node.jointIndex] = normalTransform;

	for (auto& c : node.children)
		readBoneTree(c, globalTransform);
}

void Animation::update(float time)
{
	currentTime = fmodf(time, duration);

	//boneTransforms.clear();
	boneTransforms.resize(numBones);
	normalTransforms.resize(numBones);
	readBoneTree(rootNode, glm::mat4(1.0f));
}

std::vector<glm::mat4> Animation::getBoneTransform()
{
	return boneTransforms;
}

std::vector<glm::mat3> Animation::getNormalTransform()
{
	return normalTransforms;
}

unsigned int Animation::getNodeIndex()
{
	return nodeIndex;
}