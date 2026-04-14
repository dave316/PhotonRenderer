#ifndef INCLUDED_ANIMATOR
#define INCLUDED_ANIMATOR

#pragma once

#include "Component.h"

#include <Graphics/Animation.h>

namespace pr
{
	class Animator : public Component
	{
	public:
		Animator(bool playAllAnimations);
		void setNodes(std::vector<pr::Entity::Ptr>& nodes);
		void setComponents(std::map<uint32, pr::Component::Ptr> components);
		void addAnimation(pr::Animation::Ptr animation);
		void update(float dt);
		void switchAnimation(uint32 index);
		uint32 getNumAnimations();
		std::vector<pr::Entity::Ptr> getNodes();
		std::vector<float> getWeights();
		std::vector<std::string> getAnimationNames();

		typedef std::shared_ptr<Animator> Ptr;
		static Ptr create(bool playAllAnimations)
		{
			return std::make_shared<Animator>(playAllAnimations);
		}

	private:
		std::map<uint32, pr::Component::Ptr> components;
		std::vector<pr::Entity::Ptr> nodes;
		std::vector<pr::Animation::Ptr> animations;
		uint32 currentAnimation = 0;
		bool playAllAnimations = false;
	};
}

#endif // INCLUDED_ANIMATOR