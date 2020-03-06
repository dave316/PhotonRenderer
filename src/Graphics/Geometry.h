#ifndef INCLUDED_GEOMETRY
#define INCLUDED_GEOMETRY

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec4 tangent;
	glm::vec4 boneIDs;
	glm::vec4 boneWeights;
	//glm::vec3 targetPosition0;
	//glm::vec3 targetNormal0;
	//glm::vec3 targetTangent0;
	//glm::vec3 targetPosition1;
	//glm::vec3 targetNormal1;
	//glm::vec3 targetTangent1;
	Vertex(glm::vec3 position = glm::vec3(0), 
		glm::vec4 color = glm::vec4(1.0f), 
		glm::vec3 normal = glm::vec3(0), 
		glm::vec2 texCoord = glm::vec2(0), 
		glm::vec4 tangent = glm::vec4(0),
		glm::vec4 boneIDs = glm::vec4(0),
		glm::vec4 boneWeights = glm::vec4(0)) :

		position(position),
		color(color),
		normal(normal),
		texCoord(texCoord),
		tangent(tangent),
		boneIDs(boneIDs),
		boneWeights(boneWeights)
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

	void calcNormals()
	{
		for (auto& t : triangles)
		{
			Vertex& v0 = vertices[t.v0];
			Vertex& v1 = vertices[t.v1];
			Vertex& v2 = vertices[t.v2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			v0.normal += normal;
			v1.normal += normal;
			v2.normal += normal;
		}

		for (auto& v : vertices)
			v.normal = glm::normalize(v.normal);
	}

	void calcTangentSpace()
	{
		for (auto& t : triangles)
		{
			Vertex& v0 = vertices[t.v0];
			Vertex& v1 = vertices[t.v1];
			Vertex& v2 = vertices[t.v2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;
			glm::vec2 uv0 = v0.texCoord;
			glm::vec2 uv1 = v1.texCoord;
			glm::vec2 uv2 = v2.texCoord;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * e1.x - deltaUV1.y * e2.x);
			tangent.y = f * (deltaUV2.y * e1.y - deltaUV1.y * e2.y);
			tangent.z = f * (deltaUV2.y * e1.z - deltaUV1.y * e2.z);
			tangent = glm::normalize(tangent);

			glm::vec3 bitangent;
			bitangent.x = f * (deltaUV2.x * e1.x - deltaUV1.x * e2.x);
			bitangent.y = f * (deltaUV2.x * e1.y - deltaUV1.x * e2.y);
			bitangent.z = f * (deltaUV2.x * e1.z - deltaUV1.x * e2.z);
			bitangent = glm::normalize(bitangent);

			float handeness = glm::sign(glm::dot(normal, glm::cross(tangent, bitangent)));

			v0.tangent = glm::vec4(glm::vec3(v0.tangent) + tangent, handeness);
			v1.tangent = glm::vec4(glm::vec3(v1.tangent) + tangent, handeness);
			v2.tangent = glm::vec4(glm::vec3(v2.tangent) + tangent, handeness);
		}

		for (auto& v : vertices)
		{
			glm::vec3 t = glm::normalize(glm::vec3(v.tangent));
			float w = v.tangent.w;
			v.tangent = glm::vec4(t, w);
		}			
	}
};

#endif // INCLUDED_GEOMETRY