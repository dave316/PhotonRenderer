#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct BoneNode
{
	std::string name;
	std::vector<BoneNode> children;
	glm::mat4 boneTransform;
	glm::mat4 nodeTransform;
	int boneIndex;
	int jointIndex;

	void print()
	{
		std::cout << "boneNode: " << name << " boneIndex: " << boneIndex << ", " << children.size() << " children" << std::endl;
		std::cout << "boneTransform: ";
		const float* bT = (const float*)glm::value_ptr(boneTransform);
		for (int i = 0; i < 16; i++)
			std::cout << bT[i] << " ";
		std::cout << std::endl;
		std::cout << "nodeTransform: ";
		const float* nT = (const float*)glm::value_ptr(nodeTransform);
		for (int i = 0; i < 16; i++)
			std::cout << nT[i] << " ";
		std::cout << std::endl;
		
		for (auto& c : children)
			c.print();
	}
};

struct Channel
{
	std::vector<std::pair<float, glm::vec3>> positions;
	std::vector<std::pair<float, glm::quat>> rotations;
	std::vector<std::pair<float, glm::vec3>> scales;
	
	unsigned int findPosition(float currentTime);
	unsigned int findRotation(float currentTime);
	unsigned int findScaling(float currentTime);
};

class Animation
{
public:
	enum Interpolation
	{
		STEP,
		LINEAR,
		CUBIC
	};
private:
	std::string name;
	std::map<int, Channel> channels;
	std::vector<glm::mat4> boneTransforms;
	std::vector<glm::mat3> normalTransforms;

	BoneNode rootNode;
	float currentTime;
	float duration;
	unsigned int nodeIndex;
	unsigned int numBones;

	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
public:
	Animation();
	Animation(const std::string& name, float duration, unsigned int nodeIndex, unsigned int numBones);

	glm::mat4 calcInterpPosition(int index);
	glm::mat4 calcInterpRotation(int index);
	glm::mat4 calcInterpScaling(int index);

	void setBoneTree(BoneNode& root);
	void addChannel(int index, Channel& channel);
	void readBoneTree(BoneNode& node, glm::mat4 parentTransform);
	void update(float time);

	unsigned int getNodeIndex();
	std::vector<glm::mat4> getBoneTransform();
	std::vector<glm::mat3> getNormalTransform();
	
	typedef std::shared_ptr<Animation> Ptr;
	static Ptr create(const std::string& name, float duration, unsigned int nodeIndex, unsigned int numBones)
	{
		return std::make_shared<Animation>(name, duration, nodeIndex, numBones);
	}
};

#endif // INCLUDED_ANIMATION