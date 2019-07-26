#include "Animator.h"

void Animator::setSkin(BoneNode& node)
{
	for (auto anim : animations)
		anim->setBoneTree(node);
}

void Animator::addAnimation(Animation::Ptr animation)
{
	animations.push_back(animation);
}

void Animator::addNodeAnim(NodeAnimation::Ptr animation)
{
	nodeAnims.push_back(animation);
}

void Animator::addMorphAnim(MorphAnimation::Ptr animation)
{
	morphAnims.push_back(animation);
}

void Animator::update(float dt)
{
	if (currentAnimation < animations.size())
		animations[currentAnimation]->update(dt);

	if (currentAnimation < nodeAnims.size())
		nodeAnims[currentAnimation]->update(dt);

	if (currentAnimation < morphAnims.size())
		morphAnims[currentAnimation]->update(dt);
}

void Animator::switchAnimation(unsigned int index)
{
	// TODO: implement this proberly...
	if (index < animations.size())
		currentAnimation = index;
}

void Animator::transform(Transform::Ptr transform)
{
	if (currentAnimation < nodeAnims.size())
	{
		auto anim = nodeAnims[currentAnimation];
		if (anim->hasPositions())
			transform->setPosition(anim->getPos());
		if (anim->hasRotations())
			transform->setRotation(anim->getRot());
		if (anim->hasScale())
			transform->setScale(anim->getScale());
	}
}

bool Animator::hasMorphAnim()
{
	return (!morphAnims.empty());
}

bool Animator::hasRiggedAnim()
{
	return (!animations.empty());
}

glm::vec2 Animator::getWeights()
{
	glm::vec2 weights(0.0);
	if (currentAnimation < morphAnims.size())
		weights = morphAnims[currentAnimation]->getWeights();
	return weights;
}

std::vector<glm::mat4> Animator::getBoneTransform()
{
	std::vector<glm::mat4> boneTransforms;
	if (currentAnimation < animations.size())
		boneTransforms = animations[currentAnimation]->getBoneTransform();
	return boneTransforms;
}