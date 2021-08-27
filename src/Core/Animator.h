#ifndef INCLUDED_ANIMATOR
#define INCLUDED_ANIMATOR

#pragma once

#include "Component.h"
#include "Transform.h"

#include <Graphics/Animation.h>
#include <Graphics/Skin.h>



class Animator : public Component
{
private:
	std::vector<Entity::Ptr> nodes;
	std::vector<Animation::Ptr> animations;
	bool playing = false;
	unsigned int currentAnimation = 0;
	unsigned int numBones = 0;

public:
	Animator() {}
	void setNodes(std::vector<Entity::Ptr>& nodes);
	void addAnimation(Animation::Ptr animation);
	void update(float dt);
	void play();
	void stop();
	void switchAnimation(unsigned int index);
	int numAnimations();
	bool isFinished();
	void clear();
	void printInfo();
	std::vector<Entity::Ptr> getNodes();
	typedef std::shared_ptr<Animator> Ptr;
	static Ptr create()
	{
		return std::make_shared<Animator>();
	}

};

#endif // INCLUDED_ANIMATOR