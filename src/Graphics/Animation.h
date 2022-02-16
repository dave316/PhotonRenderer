#ifndef INCLUDED_ANIMATION
#define INCLUDED_ANIMATION

#pragma once

#include <Core/Entity.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

enum Interpolation
{
	STEP,
	LINEAR,
	CUBIC
};

struct Channel
{
	std::vector<std::pair<float, std::vector<glm::vec3>>> positions;
	std::vector<std::pair<float, std::vector<glm::quat>>> rotations;
	std::vector<std::pair<float, std::vector<glm::vec3>>> scales;
	std::vector<std::pair<float, std::vector<float>>> weights;

	int findPosition(float currentTime);
	int findRotation(float currentTime);
	int findScaling(float currentTime);
	int findWeight(float currentTime);

	Interpolation interpolation = Interpolation::LINEAR;
};

class Animation
{
private:
	std::string name;
	std::map<int, Channel> channels;
	std::vector<float> currentWeights; // TODO: should be put in renderable

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
	std::vector<float> calcInterpWeight(int index);
	std::vector<float> getWeights();

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