#include "Animator.h"

namespace pr
{
	Animator::Animator(bool playAllAnimations) : playAllAnimations(playAllAnimations)
	{

	}

	void Animator::setNodes(std::vector<pr::Entity::Ptr>& nodes)
	{
		this->nodes = nodes;
	}

	void Animator::setComponents(std::map<uint32, pr::Component::Ptr> components)
	{
		this->components = components;
	}

	void Animator::addAnimation(pr::Animation::Ptr animation)
	{
		animations.push_back(animation);
	}

	void Animator::update(float dt)
	{
		if (playAllAnimations)
		{
			for (auto a : animations)
				a->update(dt, components);
		}
		else
			animations[currentAnimation]->update(dt, components);
	}

	void Animator::switchAnimation(uint32 index)
	{
		if (index < animations.size())
			currentAnimation = index;
	}

	uint32 Animator::getNumAnimations()
	{
		return static_cast<uint32>(animations.size());
	}

	std::vector<pr::Entity::Ptr> Animator::getNodes()
	{
		return nodes;
	}

	std::vector<float> Animator::getWeights()
	{
		return animations[currentAnimation]->getCurrentWeights();
	}

	std::vector<std::string> Animator::getAnimationNames()
	{
		std::vector<std::string> names;
		for (auto a : animations)
			names.push_back(a->getName());
		return names;
	}
}
