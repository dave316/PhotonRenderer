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
	std::vector<float> morphWeights;
	std::string name;
	Skin skin;
	bool skinnedMesh = false;
	bool morphTagets = false;
		
public:
	Renderable() {}
	~Renderable();
	void addMesh(std::string name, Mesh::Ptr mesh, Material::Ptr material);
	void render(Shader::Ptr shader);
	void print();
	void flipWindingOrder();
	void setSkin(Skin& skin);
	void setMorphWeights(std::vector<float>& weights);
	bool isSkinnedMesh();
	bool useMorphTargets();
	bool useBlending();
	bool isTransmissive();
	std::string getName();
	std::vector<float> getWeights();
	std::vector<Vertex> getVertices();
	void computeJoints(std::vector<Entity::Ptr>& nodes);
	Skin getSkin();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE