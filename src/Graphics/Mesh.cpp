#include "Mesh.h"

#include <iostream>

Mesh::Mesh(const std::string& name, TriangleSurface& surface, unsigned int materialIndex) :
	name(name), materialIndex(materialIndex)
{
	updatGeometry(surface);
	//std::vector<GLuint> indices;
	//for (auto& tri : surface.triangles)
	//{
	//	indices.push_back(tri.v0);
	//	indices.push_back(tri.v1);
	//	indices.push_back(tri.v2);
	//}

	//vertexBuffer.upload(surface.vertices);
	//indexBuffer.upload(indices);

	//vao.addAttrib(0, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, position));
	//vao.addAttrib(1, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, color));
	//vao.addAttrib(2, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	//vao.addAttrib(3, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	//vao.addAttrib(4, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	//vao.addAttrib(5, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
	//vao.addAttrib(6, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));
	////vao.addAttrib(5, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
	////vao.addAttrib(6, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition0));
	////vao.addAttrib(7, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetNormal0));
	////vao.addAttrib(8, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetTangent0));
	////vao.addAttrib(9, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetPosition1));
	////vao.addAttrib(10, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetNormal1));
	////vao.addAttrib(11, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, targetTangent1));

	//vao.bind();
	//indexBuffer.bind();
	//vao.unbind();
}

Mesh::~Mesh()
{
	//std::cout << "mesh " << name << " destroyed" << std::endl;
}

void Mesh::updatGeometry(TriangleSurface& surface)
{
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
	vao.addAttrib(3, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	vao.addAttrib(4, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	vao.addAttrib(5, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
	vao.addAttrib(6, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

	vao.bind();
	indexBuffer.bind();
	vao.unbind();
}

void Mesh::draw()
{
	vao.bind();
	if (indexBuffer.size() > 0)
		glDrawElements(GL_TRIANGLES, indexBuffer.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(GL_TRIANGLES, 0, vertexBuffer.size());
	vao.unbind();
}

void Mesh::drawPoints()
{
	vao.bind();
	glDrawArrays(GL_POINTS, 0, vertexBuffer.size());
	vao.unbind();
}