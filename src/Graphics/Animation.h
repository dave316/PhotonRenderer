#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <Core/Entity.h>
#include <Graphics/Animation.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

struct Channel
{
	std::vector<std::pair<float, glm::vec3>> positions;
	std::vector<std::pair<float, glm::quat>> rotations;
	std::vector<std::pair<float, glm::vec3>> scales;

	int findPosition(float currentTime);
	int findRotation(float currentTime);
	int findScaling(float currentTime);
	unsigned int nodeIndex;
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

	float currentTime;
	float duration;
	bool finished = false;

	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
public:
	Animation();
	Animation(const std::string& name, float duration);
	~Animation();

	glm::vec3 calcInterpPosition(int index);
	glm::quat calcInterpRotation(int index);
	glm::vec3 calcInterpScaling(int index);

	void addChannel(int index, Channel& channel);
	void update(float dt, std::vector<Entity::Ptr>& nodes);
	void reset();
	bool isFinished();
	void print();
	typedef std::shared_ptr<Animation> Ptr;
	static Ptr create(const std::string& name, float duration)
	{
		return std::make_shared<Animation>(name, duration);
	}
};

#endif // INCLUDED_ANIMATION