#ifndef INCLUDED_MESH
#define INCLUDED_MESH

#pragma once

#include "Primitive.h"
#include "Material.h"

struct SubMesh
{
	Primitive::Ptr primitive;
	Material::Ptr material;
	std::vector<Material::Ptr> variants;
};

class Mesh
{
private:
	std::string name;
	std::vector<SubMesh> subMeshes;
	std::vector<std::string> variants;
	std::vector<float> morphWeights;
	Box boundingBox;
	int numVertices = 0;
	int numTrianlges = 0;
			
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
public:
	static int numDrawCalls;
	Mesh(const std::string& name);
	~Mesh();
	void addSubMesh(SubMesh subMesh);
	void addVariant(std::string name);
	void switchVariant(int index);
	void setMorphWeights(std::vector<float>& weights);
	void draw(Shader::Ptr shader, bool useShader = false);
	void flipWindingOrder();
	bool useMorphTargets();
	bool useBlending();
	bool isTransmissive();
	int getNumVertices();
	int getNumTriangles();
	int getNumPrimitives();
	void setMaterial(unsigned int index, Material::Ptr material);
	void clear();
	Box getBoundingBox();
	std::vector<SubMesh>& getSubMeshes();
	std::vector<float> getWeights();
	std::vector<std::string> getVariants();
	std::string getName()
	{
		return name;
	}
	typedef std::shared_ptr<Mesh> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Mesh>(name);
	}
};

#endif // INCLUDED_MESH