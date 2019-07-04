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

void Animator::transform(Transform::Ptr transform)
{
	auto anim = animations[currentAnimation];
	if (anim->hasPositions())
		transform->setPosition(anim->getPos());
	if (anim->hasRotations())
		transform->setRotation(anim->getRot());
	if (anim->hasScale())
		transform->setScale(anim->getScale());
}