#include "Animator.h"

void Animator::addAnimation(Animation::Ptr animation)
{
	animations.push_back(animation);
}

void Animator::update(float dt)
{
	if (currentAnimation < animations.size())
		animations[currentAnimation]->update(dt);
}

void Animator::switchAnimation(unsigned int index)
{
	if (index < animations.size())
		currentAnimation = index;
}

glm::mat4 Animator::getAnimationTransform()
{
	return animations[currentAnimation]->getTransform();
}