#include "Mesh.h"
#include <glm/gtc/matrix_inverse.hpp>
namespace Unity
{
	Mesh::Mesh(const std::string& name) :
		name(name)
	{

	}

	Mesh::~Mesh()
	{

	}

	void Mesh::addPosition(glm::vec3 position)
	{
		vertices.push_back(position);
	}

	void Mesh::addNormal(glm::vec3 normal)
	{
		normals.push_back(normal);
	}

	void Mesh::addTangent(glm::vec4 tangent)
	{
		tangents.push_back(tangent);
	}

	void Mesh::addUV(glm::vec2 uv)
	{
		uv1.push_back(uv);
	}

	void Mesh::addUV2(glm::vec2 uv)
	{
		uv2.push_back(uv);
	}

	void Mesh::addColor(glm::vec4 color)
	{
		colors.push_back(color);
	}

	void Mesh::addTriangle(uint32 i0, uint32 i1, uint32 i2)
	{
		triangles.push_back(i0);
		triangles.push_back(i1);
		triangles.push_back(i2);
	}

	void Mesh::preTransform(const glm::mat4& T)
	{
		glm::mat3 N = glm::mat3(glm::inverseTranspose(T));
		for (uint32 i = 0; i < vertices.size(); i++)
		{
			vertices[i] = glm::vec3(T * glm::vec4(vertices[i], 1.0f));
			normals[i] = N * normals[i];
		}
	}

	std::string Mesh::getName()
	{
		return name;
	}
}