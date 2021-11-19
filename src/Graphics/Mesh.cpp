#include "Mesh.h"

#include <iostream>

Mesh::Mesh(const std::string& name, TriangleSurface& surface, GLenum topology, unsigned int materialIndex) :
	name(name), topology(topology), materialIndex(materialIndex)
{
	updatGeometry(surface);
}

Mesh::~Mesh()
{
	//std::cout << "mesh " << name << " destroyed" << std::endl;
}

void Mesh::flipWindingOrder()
{
	surface.flipWindingOrder();
	updatGeometry(surface);
}

void Mesh::updatGeometry(TriangleSurface& surface)
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
#ifdef MORPH_TARGETS_2
	vao.addAttrib(8, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition0));
	vao.addAttrib(9, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetNormal0));
	vao.addAttrib(10, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetTangent0));
	vao.addAttrib(11, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition1));
	vao.addAttrib(12, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetNormal1));
	vao.addAttrib(13, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetTangent1));
#endif

#ifdef MORPH_TARGETS_8
	vao.addAttrib(8, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition0));
	vao.addAttrib(9, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition1));
	vao.addAttrib(10, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition2));
	vao.addAttrib(11, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition3));
	vao.addAttrib(12, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition4));
	vao.addAttrib(13, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition5));
	vao.addAttrib(14, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition6));
	vao.addAttrib(15, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition7));
#endif

	vao.bind();
	indexBuffer.bind();
	vao.unbind();
}

void Mesh::draw()
{
	vao.bind();
	if (indexBuffer.size() > 0)
		glDrawElements(topology, indexBuffer.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(topology, 0, vertexBuffer.size());
	vao.unbind();
}

void Mesh::drawPoints()
{
	vao.bind();
	glDrawArrays(GL_POINTS, 0, vertexBuffer.size());
	vao.unbind();
}

std::vector<Vertex> Mesh::getVertices()
{
	return surface.vertices;
}