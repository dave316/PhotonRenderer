#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

//#include <Graphics/Animation.h>
//#include <Graphics/MorphAnimation.h>
#include <Graphics/Material.h>
#include <Graphics/Mesh.h>

struct Primitive
{
	Mesh::Ptr mesh;
	Material::Ptr material;
};

// TODO: move animation stuff to its own component
class Renderable : public Component
{
	//MorphAnimation::Ptr morphAnim = nullptr;
	//Animation::Ptr animation = nullptr;
	//Material::Ptr material;
	//Mesh::Ptr mesh;
	std::vector<Primitive> primitives;
		
public:
	Renderable() {}
	~Renderable();
	//Renderable(Mesh::Ptr mesh, Material::Ptr material);
	void addMesh(Mesh::Ptr mesh, Material::Ptr material);
	//void setAnimation(Animation::Ptr anim);
	//void setMorphAnim(MorphAnimation::Ptr anim);
	//void update(float dt);
	void render(GL::Program& program);
	void print();
	bool useBlending();
	//bool hasAnimations();
	//bool hasMorphAnim();
	//glm::vec2 getWeights();
	//glm::mat4 getTransform();
	typedef std::shared_ptr<Renderable> Ptr;
	//static Ptr create(Mesh::Ptr mesh, Material::Ptr material)
	//{
	//	return std::make_shared<Renderable>(mesh, material);
	//}
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE