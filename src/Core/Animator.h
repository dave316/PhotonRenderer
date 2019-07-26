#ifndef INCLUDED_ANIMATOR
#define INCLUDED_ANIMATOR

#pragma once

#include "Component.h"
#include "Transform.h"

#include <Graphics/Animation.h>
#include <Graphics/NodeAnimation.h>
#include <Graphics/MorphAnimation.h>



class Animator : public Component
{
private:
	std::vector<Animation::Ptr> animations;
	std::vector<NodeAnimation::Ptr> nodeAnims;
	std::vector<MorphAnimation::Ptr> morphAnims;
	unsigned int currentAnimation = 0;
	
public:
	Animator() {}
	void setSkin(BoneNode& node);
	void addAnimation(Animation::Ptr animation);
	void addNodeAnim(NodeAnimation::Ptr animation);
	void addMorphAnim(MorphAnimation::Ptr animation);
	void update(float dt);
	void switchAnimation(unsigned int index);
	void transform(Transform::Ptr transform);
	bool hasMorphAnim();
	bool hasRiggedAnim();
	glm::vec2 getWeights();
	std::vector<glm::mat4> getBoneTransform();

	typedef std::shared_ptr<Animator> Ptr;
	static Ptr create()
	{
		return std::make_shared<Animator>();
	}

};

#endif // INCLUDED_ANIMATOR