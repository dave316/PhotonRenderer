#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Skin.h>
#include <Graphics/Shader.h>

struct Primitive
{
	Mesh::Ptr mesh;
	Material::Ptr material;
};

class Renderable : public Component
{
	std::vector<Primitive> primitives;
	std::string name;
	Skin skin;
	bool skinnedMesh = false;
		
public:
	Renderable() {}
	~Renderable();
	void addMesh(std::string name, Mesh::Ptr mesh, Material::Ptr material);
	void render(Shader::Ptr shader);
	void print();
	void flipWindingOrder();
	void setSkin(Skin& skin);
	bool isSkinnedMesh();
	bool useBlending();
	std::string getName();
	std::vector<Vertex> getVertices();
	Skin getSkin();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE