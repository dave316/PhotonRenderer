#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Skin.h>
#include <Graphics/Shader.h>

struct RenderPrimitive
{
	Mesh::Ptr mesh;
	std::vector<Material::Ptr> materials;
	unsigned int materialIndex = 0;
	bool computeFlatNormals = false;

	Material::Ptr getMaterial()
	{
		if (materialIndex < materials.size())
			return materials[materialIndex];
	}

	void switchMaterial(unsigned int materialIndex)
	{
		if (materialIndex < materials.size())
			this->materialIndex = materialIndex;
	}
};

class Renderable : public Component
{
	std::vector<RenderPrimitive> primitives;
	std::vector<float> morphWeights;
	std::string name;
	Skin skin;
	bool skinnedMesh = false;
	bool morphTagets = false;
		
public:
	Renderable() {}
	~Renderable();
	void addPrimitive(RenderPrimitive& primitive);
	void render(Shader::Ptr shaders);
	void switchMaterial(unsigned int materialIndex);
	void print();
	void flipWindingOrder();
	void setSkin(Skin& skin);
	void setMorphWeights(std::vector<float>& weights);
	bool isSkinnedMesh();
	bool useMorphTargets();
	bool useBlending();
	bool isTransmissive();
	std::string getShader();
	std::string getName();
	std::vector<float> getWeights();
	std::vector<Vertex> getVertices();
	void computeJoints(std::vector<Entity::Ptr>& nodes);
	Skin getSkin();
	AABB getBoundingBox();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE