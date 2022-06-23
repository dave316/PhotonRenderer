#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Skin.h>
#include <Graphics/Shader.h>

//struct RenderPrimitive
//{
//	Mesh::Ptr mesh;
//	std::vector<Material::Ptr> materials;
//	unsigned int materialIndex = 0;
//	bool computeFlatNormals = false;
//
//	Material::Ptr getMaterial()
//	{
//		if (materialIndex < materials.size())
//			return materials[materialIndex];
//	}
//
//	void switchMaterial(unsigned int materialIndex)
//	{
//		if (materialIndex < materials.size())
//			this->materialIndex = materialIndex;
//	}
//};

class Renderable : public Component
{
	Mesh::Ptr mesh;
	Skin::Ptr skin;
	bool skinnedMesh = false;
	bool morphTagets = false;
		
public:
	Renderable() {}
	~Renderable();
	void setMesh(Mesh::Ptr mesh);
	void render(Shader::Ptr shaders);
	void switchMaterial(int materialIndex);
	void setSkin(Skin::Ptr skin);
	bool isSkinnedMesh();
	bool useMorphTargets();
	bool useBlending();
	bool isTransmissive();
	std::string getShader();
	std::vector<float> getWeights();
	void computeJoints(std::vector<Entity::Ptr>& nodes);
	Mesh::Ptr getMesh();
	Skin::Ptr getSkin();
	AABB getBoundingBox();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE