#include "Primitive.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
unsigned int Primitive::globalIDCount = 0;
Primitive::Primitive(const std::string& name, TriangleSurface& surface, GLenum topology) : 
	name(name), topology(topology)
{
	updatGeometry(surface);

	id = globalIDCount;
	globalIDCount++;
}

Primitive::~Primitive()
{
	//std::cout << "Primitive " << name << " destroyed" << std::endl;
}

void Primitive::flipWindingOrder()
{
	surface.flipWindingOrder();
	updatGeometry(surface);
}

void Primitive::updatGeometry(TriangleSurface& surface)
{
	this->surface = surface;

	std::vector<GLuint> indices;
	for (auto& tri : surface.triangles)
	{
		indices.push_back(tri.v0);
		indices.push_back(tri.v1);
		indices.push_back(tri.v2);
	}

	vertexBuffer.upload(surface.vertices);
	indexBuffer.upload(indices);

	vao.addAttrib(0, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, position));
	vao.addAttrib(1, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, color));
	vao.addAttrib(2, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	vao.addAttrib(3, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord0));
	vao.addAttrib(4, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord1));
	vao.addAttrib(5, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	vao.addAttrib(6, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
	vao.addAttrib(7, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

	vao.bind();
	indexBuffer.bind();
	vao.unbind();
}

void Primitive::preTransform(const glm::mat4& T)
{
	glm::mat3 N = glm::mat3(glm::inverseTranspose(T));

	boundingBox = Box();
	for (auto& v : surface.vertices)
	{
		v.position = glm::vec3(T * glm::vec4(v.position, 1.0f));
		v.normal = N * v.normal;
		boundingBox.expand(v.position);
	}

	updatGeometry(surface);
}

void Primitive::setMorphTarget(Texture2DArray::Ptr morphTex)
{
	this->morphTex = morphTex;
}

void Primitive::setFlatNormals(bool flatNormals)
{
	this->computeFlatNormals = flatNormals;
}

void Primitive::setBoundingBox(glm::vec3& minPoint, glm::vec3& maxPoint)
{
	boundingBox = Box(minPoint, maxPoint);
}

void Primitive::setBoundingBox(Box& boundingBox)
{
	this->boundingBox = boundingBox;
}

void Primitive::setInstances(unsigned int num)
{
	this->isInstanced = true;
	this->numInstances = num;
}

void Primitive::draw()
{
	numDrawCalls++;

	if(morphTex)
		morphTex->use(20);
	vao.bind();

	if (isInstanced)
		glDrawElementsInstanced(topology, indexBuffer.size(), GL_UNSIGNED_INT, 0, numInstances);
	else
	{
		if (indexBuffer.size() > 0)
			glDrawElements(topology, indexBuffer.size(), GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(topology, 0, vertexBuffer.size());
	}

	vao.unbind();
}

void Primitive::reset()
{
	numDrawCalls = 0;
}

bool Primitive::getFlatNormals()
{
	return computeFlatNormals;
}

bool Primitive::isUsingInstancing()
{
	return isInstanced;
}

int Primitive::getNumDrawCalls()
{
	return numDrawCalls;
}

int Primitive::numVertices()
{
	return surface.vertices.size();
}

int Primitive::numTriangles()
{
	return surface.triangles.size();
}

Box Primitive::getBoundingBox()
{
	return boundingBox;
}

TriangleSurface Primitive::getSurface()
{
	return surface;
}

GLuint Primitive::getVaoID()
{
	return vao;
}

unsigned int Primitive::getID()
{
	return id;
}