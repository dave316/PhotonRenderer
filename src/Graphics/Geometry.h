#ifndef INCLUDED_GEOMETRY
#define INCLUDED_GEOMETRY

#define MORPH_TARGETS

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 texCoord0;
	glm::vec2 texCoord1;
	glm::vec4 tangent;
	glm::vec4 boneIDs;
	glm::vec4 boneWeights;
#ifdef MORPH_TARGETS
	glm::vec3 targetPosition0;
	glm::vec3 targetNormal0;
	glm::vec3 targetTangent0;
	glm::vec3 targetPosition1;
	glm::vec3 targetNormal1;
	glm::vec3 targetTangent1;
#endif

	Vertex(glm::vec3 position = glm::vec3(0), 
		glm::vec4 color = glm::vec4(1.0f), 
		glm::vec3 normal = glm::vec3(0), 
		glm::vec2 texCoord0 = glm::vec2(0),
		glm::vec2 texCoord1 = glm::vec2(0),
		glm::vec4 tangent = glm::vec4(0),
		glm::vec4 boneIDs = glm::vec4(0),
		glm::vec4 boneWeights = glm::vec4(0)
	):

		position(position),
		color(color),
		normal(normal),
		texCoord0(texCoord0),
		texCoord1(texCoord1),
		tangent(tangent),
		boneIDs(boneIDs),
		boneWeights(boneWeights)
	{
#ifdef MORPH_TARGETS
		targetPosition0 = glm::vec3(0);
		targetNormal0 = glm::vec3(0);
		targetTangent0 = glm::vec3(0);
		targetPosition1 = glm::vec3(0);
		targetNormal1 = glm::vec3(0);
		targetTangent1 = glm::vec3(0);
#endif
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

struct TriangleIndices
{
	unsigned int v0, v1, v2;
	TriangleIndices(unsigned int v0, unsigned int v1, unsigned int v2) :
		v0(v0), v1(v1), v2(v2)
	{}
};

struct TriangleSurface
{
	std::vector<Vertex> vertices;
	std::vector<TriangleIndices> triangles;

	void addVertex(Vertex& v)
	{
		vertices.push_back(v);
	}
	void addTriangle(TriangleIndices& t)
	{
		triangles.push_back(t);
	}

	void calcSmoothNormals()
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

	void calcFlatNormals()
	{
		for (int i = 0; i < vertices.size(); i += 3)
		{
			Vertex& v0 = vertices[i];
			Vertex& v1 = vertices[i + 1];
			Vertex& v2 = vertices[i + 2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			v0.normal = normal;
			v1.normal = normal;
			v2.normal = normal;
		}
	}

	void calcTangentSpace()
	{
		// TODO: calc tanget space on correct uv set
		for (auto& t : triangles)
		{
			Vertex& v0 = vertices[t.v0];
			Vertex& v1 = vertices[t.v1];
			Vertex& v2 = vertices[t.v2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;
			glm::vec2 uv0 = v0.texCoord0;
			glm::vec2 uv1 = v1.texCoord0;
			glm::vec2 uv2 = v2.texCoord0;

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

	void flipWindingOrder()
	{
		for (auto& t : triangles)
			std::swap(t.v0, t.v2);
	}
};

struct Triangle
{
	glm::vec3 v0, v1, v2;
	glm::vec3 n0, n1, n2;
	glm::vec3 plane;
	unsigned int triID;
};

class AABB
{
private:
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

public:
	AABB();
	AABB(glm::vec3& minPoint, glm::vec3& maxPoint);

	glm::vec3 getMinPoint() const;
	glm::vec3 getMaxPoint() const;
	void expand(const glm::vec3& point);
	void expand(const Triangle& tri);
	void expand(const AABB& box);
	float radius();
	glm::vec3 getCenter();
	glm::vec3 getSize();
	std::vector<glm::vec3> getPoints();
};

#endif // INCLUDED_GEOMETRY