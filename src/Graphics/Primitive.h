#ifndef INCLUDED_PRIMITVE
#define INCLUDED_PRIMITVE

#pragma once

#include <GL/GLBuffer.h>
#include <GL/GLVertexArray.h>

#include "Material.h"
#include "Geometry.h"

#include <memory>
#include <string>
#include <map>

class Primitive
{
private:
	std::string name;
	GL::VertexBuffer<Vertex> vertexBuffer;
	GL::IndexBuffer<GLuint> indexBuffer;
	GL::VertexArray vao;
	GLenum topology;

	AABB boundingBox;
	TriangleSurface surface;
	Texture2DArray::Ptr morphTex = nullptr;

	Material::Ptr material;
	std::map<int, Material::Ptr> variants;
	bool computeFlatNormals = false;

	Primitive(const Primitive&) = delete;
	Primitive& operator=(const Primitive&) = delete;
public:

	Primitive(const std::string& name, TriangleSurface& surface, GLenum topology, Material::Ptr material);
	~Primitive();
	void flipWindingOrder();
	void updatGeometry(TriangleSurface& surface);
	void setMaterial(Material::Ptr material);
	void setMorphTarget(Texture2DArray::Ptr morphTex);
	void addVariant(int index, Material::Ptr material);
	void setFlatNormals(bool flatNormals);
	void setBoundingBox(glm::vec3& minPoint, glm::vec3& maxPoint);
	void switchVariant(int index);
	void draw();
	bool getFlatNormals();
	int numVertices();
	int numTriangles();
	AABB getBoundingBox();
	Material::Ptr getMaterial();
	std::string getName()
	{
		return name;
	}
	typedef std::shared_ptr<Primitive> Ptr;
	static Ptr create(const std::string& name, TriangleSurface& surface, GLenum topology, Material::Ptr material)
	{
		return std::make_shared<Primitive>(name, surface, topology, material);
	}
};

#endif // INCLUDED_PRIMITVE