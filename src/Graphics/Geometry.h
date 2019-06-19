#ifndef INCLUDED_GEOMETRY
#define INCLUDED_GEOMETRY

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec3 targetPosition0;
	glm::vec3 targetNormal0;
	glm::vec3 targetTangent0;
	glm::vec3 targetPosition1;
	glm::vec3 targetNormal1;
	glm::vec3 targetTangent1;
	Vertex() {}
	Vertex(glm::vec3 position, glm::vec3 color = glm::vec3(0.5f), glm::vec3 normal = glm::vec3(0), glm::vec2 texCoord = glm::vec2(0)) :
		position(position),
		color(color),
		normal(normal),
		texCoord(texCoord)
	{

	}
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

struct Sphere
{
	glm::vec3 position;
	float radius;
};

struct Triangle
{
	unsigned int v0, v1, v2;
	Triangle(unsigned int v0, unsigned int v1, unsigned int v2) :
		v0(v0), v1(v1), v2(v2)
	{}
};

struct TriangleSurface
{
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;

	void addVertex(Vertex& v)
	{
		vertices.push_back(v);
	}
	void addTriangle(Triangle& t)
	{
		triangles.push_back(t);
	}
};

#endif // INCLUDED_GEOMETRY