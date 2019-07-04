#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

class Animation
{
	std::string name;
	std::vector<std::pair<float, glm::vec3>> positions;
	std::vector<std::pair<float, glm::quat>> rotations;
	std::vector<std::pair<float, glm::vec3>> scales;

	glm::vec3 pos = glm::vec3(0.0f);
	glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	float currentTime;
	float ticksPerSecond;
	float startTime;
	float endTime;
	float duration;

	unsigned int nodeIndex;

	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
public:
	Animation();
	Animation(const std::string& name, float ticksPerSecond, float startTime, float endTime, float duration, unsigned int nodeIndex);

	void setPositions(std::vector<std::pair<float, glm::vec3>>& positions);
	void setRotations(std::vector<std::pair<float, glm::quat>>& positions);
	void setScales(std::vector<std::pair<float, glm::vec3>>& positions);

	unsigned int findPosition(float currentTime);
	unsigned int findRotation(float currentTime);
	unsigned int findScaling(float currentTime);

	glm::mat4 calcInterpPosition();
	glm::mat4 calcInterpRotation();
	glm::mat4 calcInterpScaling();

	void update(float time);
	bool hasPositions();
	bool hasRotations();
	bool hasScale();
	glm::vec3 getPos();
	glm::quat getRot();
	glm::vec3 getScale();
	std::string getName();
	unsigned int getNodeIndex();

	typedef std::shared_ptr<Animation> Ptr;
	static Ptr create()
	{
		return std::make_shared<Animation>();
	}
};

#endif // INCLUDED_MATERIAL