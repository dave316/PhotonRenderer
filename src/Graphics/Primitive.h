#ifndef INCLUDED_PRIMITVE
#define INCLUDED_PRIMITVE

#pragma once

#include <GL/GLBuffer.h>
#include <GL/GLVertexArray.h>

#include "Material.h"
#include "Geometry.h"
#include <Math/Shapes.h>

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

	Box boundingBox;
	TriangleSurface surface;
	Texture2DArray::Ptr morphTex = nullptr;

	bool computeFlatNormals = false;
	unsigned int numDrawCalls = 0;
	unsigned int id;
	static unsigned int globalIDCount;

	bool isInstanced = false;
	unsigned int numInstances = 0;

	Primitive(const Primitive&) = delete;
	Primitive& operator=(const Primitive&) = delete;
public:

	Primitive(const std::string& name, TriangleSurface& surface, GLenum topology);
	~Primitive();
	void flipWindingOrder();
	void updatGeometry(TriangleSurface& surface);
	void preTransform(const glm::mat4& T);
	void setMorphTarget(Texture2DArray::Ptr morphTex);
	void setFlatNormals(bool flatNormals);
	void setBoundingBox(glm::vec3& minPoint, glm::vec3& maxPoint);
	void setBoundingBox(Box& boundingBox);
	void setInstances(unsigned int num);
	void draw();
	void reset();
	bool getFlatNormals();
	bool isUsingInstancing();
	int getNumDrawCalls();
	int numVertices();
	int numTriangles();
	unsigned int getID();
	Box getBoundingBox();
	TriangleSurface getSurface();
	GLuint getVaoID();
	std::string getName()
	{
		return name;
	}
	typedef std::shared_ptr<Primitive> Ptr;
	static Ptr create(const std::string& name, TriangleSurface& surface, GLenum topology)
	{
		return std::make_shared<Primitive>(name, surface, topology);
	}
};

#endif // INCLUDED_PRIMITVE