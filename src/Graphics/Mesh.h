#ifndef INCLUDED_MESH
#define INCLUDED_MESH

#pragma once

#include "Primitive.h"
#include "Material.h"

class Mesh
{
private:
	std::string name;
	std::vector<Primitive::Ptr> primitives;
	std::vector<std::string> variants;
	std::vector<float> morphWeights;
	AABB boundingBox;
	int numVertices = 0;
	int numTrianlges = 0;

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
public:

	Mesh(const std::string& name);
	~Mesh();
	void addPrimitive(Primitive::Ptr primitive);
	void addVariant(std::string name);
	void switchVariant(int index);
	void setMorphWeights(std::vector<float>& weights);
	void draw(Shader::Ptr shader);
	void flipWindingOrder();
	bool useMorphTargets();
	bool useBlending();
	bool isTransmissive();
	int getNumVertices();
	int getNumTriangles();
	int getNumPrimitives();
	AABB getBoundingBox();
	std::vector<Primitive::Ptr> getPrimitives();
	std::vector<float> getWeights();
	std::vector<std::string> getVariants();
	std::string getShader();
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