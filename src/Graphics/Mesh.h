#ifndef INCLUDED_MESH
#define INCLUDED_MESH

#pragma once

#include <GL/GLBuffer.h>
#include <GL/GLVertexArray.h>

#include "Geometry.h"

#include <memory>
#include <string>
#include <map>

class Mesh
{
public:
	enum Attribute
	{
		POSITION,
		COLOR,
		NORMAL,
		TANGENT,
		TEXCOORD,
		JOINTS,
		WEIGHTS
	};
private:

	std::string name;
	GL::VertexBuffer<Vertex> vertexBuffer;
	GL::IndexBuffer<GLuint> indexBuffer;
	GL::VertexArray vao;
	GLenum topology;

	TriangleSurface surface;

	unsigned int materialIndex;

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
public:

	Mesh(const std::string& name, TriangleSurface& surface, GLenum topology, unsigned int index);
	~Mesh();
	void flipWindingOrder();
	void updatGeometry(TriangleSurface& surface);
	unsigned int getMaterialIndex() { return materialIndex; }
	void draw();
	void drawPoints();
	std::vector<Vertex> getVertices();
	std::string getName()
	{
		return name;
	}
	typedef std::shared_ptr<Mesh> Ptr;
	static Ptr create(const std::string& name, TriangleSurface& surface, GLenum topology, unsigned int materialIndex)
	{
		return std::make_shared<Mesh>(name, surface, topology, materialIndex);
	}
};

#endif // INCLUDED_MESH