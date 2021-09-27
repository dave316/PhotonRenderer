#include "Animator.h"

void Animator::setNodes(std::vector<Entity::Ptr>& nodes)
{
	this->nodes = nodes;
}

void Animator::addAnimation(Animation::Ptr animation)
{
	animations.push_back(animation);
}

void Animator::update(float dt)
{
	if (playing)
	{
		if (playAllAnimations)
		{
			for (auto a : animations)
				a->update(dt, nodes);
		}
		else
		{
			auto anim = animations[currentAnimation];
			anim->update(dt, nodes);
		}
	}
}

void Animator::play()
{
	if (playAllAnimations)
	{
		for (auto a : animations)
			a->reset();
	}
	else
	{
		animations[currentAnimation]->reset();
	}
	
	playing = true;
}

void Animator::stop()
{
	if (playAllAnimations)
	{
		for (auto a : animations)
			a->reset();
	}
	else
	{
		animations[currentAnimation]->reset();
	}
	
	playing = false;
}

bool Animator::isFinished()
{
	bool finished = true;
	if (playAllAnimations)
	{		
		for (auto a : animations)
			finished &= a->isFinished();
	}
	else
	{
		finished &= animations[currentAnimation]->isFinished();
	}

	return finished;
}

void Animator::switchAnimation(unsigned int index)
{
	if (index < animations.size())
		currentAnimation = index;
}

int Animator::numAnimations()
{
	return animations.size();
}

void Animator::clear()
{
	nodes.clear();
}

void Animator::printInfo()
{
	for (int i = 0; i < animations.size(); i++)
		animations[i]->print();
}

std::vector<float> Animator::getWeights()
{
	return animations[currentAnimation]->getWeights();
}

std::vector<Entity::Ptr> Animator::getNodes()
{
	return nodes;
}
