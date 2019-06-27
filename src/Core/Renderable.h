#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Animation.h>
#include <Graphics/MorphAnimation.h>
#include <Graphics/Material.h>
#include <Graphics/Mesh.h>

// TODO: move animation stuff to its own component
class Renderable : public Component
{
	MorphAnimation::Ptr morphAnim = nullptr; // TODO: add logic for Morph Target
	Animation::Ptr animation = nullptr;
	Material::Ptr material;
	Mesh::Ptr mesh;
		
public:
	Renderable() {}
	~Renderable();
	Renderable(Mesh::Ptr mesh, Material::Ptr material);
	void setAnimation(Animation::Ptr anim);
	void setMorphAnim(MorphAnimation::Ptr anim);
	void update(float dt);
	void render(GL::Program& program);
	void print();
	bool hasAnimations();
	bool hasMorphAnim();
	glm::vec2 getWeights();
	glm::mat4 getTransform();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create(Mesh::Ptr mesh, Material::Ptr material)
	{
		return std::make_shared<Renderable>(mesh, material);
	}
};

#endif // INCLUDED_RENDERABLE