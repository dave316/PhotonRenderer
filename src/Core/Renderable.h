#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Shader.h>

struct Primitive
{
	Mesh::Ptr mesh;
	Material::Ptr material;
};

class Renderable : public Component
{
	std::vector<Primitive> primitives;
		
public:
	Renderable() {}
	~Renderable();
	void addMesh(Mesh::Ptr mesh, Material::Ptr material);
	void render(Shader::Ptr shader);
	void print();
	bool useBlending();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE