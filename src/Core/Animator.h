#ifndef INCLUDED_ANIMATOR
#define INCLUDED_ANIMATOR

#pragma once

#include "Component.h"
#include "Transform.h"
#include "Light.h"

#include <Graphics/Animation.h>
#include <Graphics/Skin.h>

class Animator : public Component
{
private:
	std::vector<Entity::Ptr> nodes;
	std::vector<Camera::Ptr> cameras;
	std::vector<Light::Ptr> lights;
	std::vector<Material::Ptr> materials;
	std::vector<Animation::Ptr> animations;
	bool playing = false;
	bool playAllAnimations = false;
	unsigned int currentAnimation = 0;
	unsigned int numBones = 0;

public:
	Animator(bool playAllAnimations) : playAllAnimations(playAllAnimations) {}
	void setNodes(std::vector<Entity::Ptr>& nodes);
	void setCameras(std::vector<Camera::Ptr>& cameras);
	void setLights(std::vector<Light::Ptr>& lights);
	void setMaterials(std::vector<Material::Ptr>& materials);
	void addAnimation(Animation::Ptr animation);
	void update(float dt);
	void play();
	void stop();
	void switchAnimation(unsigned int index);
	int numAnimations();
	bool isFinished();
	void clear();
	void printInfo();
	std::vector<float> getWeights();
	std::vector<Entity::Ptr> getNodes();
	std::vector<Material::Ptr> getMaterials();
	std::vector<std::string> getAnimationNames();
	typedef std::shared_ptr<Animator> Ptr;
	static Ptr create(bool playAllAnimations)
	{
		return std::make_shared<Animator>(playAllAnimations);
	}

};

#endif // INCLUDED_ANIMATOR