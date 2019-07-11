#include "Primitives.h"

namespace Primitives
{
	glm::vec3 calcNormal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
	{
		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;
		glm::vec3 n = glm::normalize(glm::cross(e1, e2));

		return n;
	}

	void addFace(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
	{
		uint32_t baseIndex = (uint32_t)surface.vertices.size();

		Vertex v[4];
		v[0].position = v0;
		v[1].position = v1;
		v[2].position = v2;
		v[3].position = v3;

		glm::vec3 n = calcNormal(v0, v1, v2);
		v[0].normal = n;
		v[1].normal = n;
		v[2].normal = n;
		v[3].normal = n;

		v[0].texCoord = glm::vec2(0.0f, 0.0f);
		v[1].texCoord = glm::vec2(1.0f, 0.0f);
		v[2].texCoord = glm::vec2(1.0f, 1.0f);
		v[3].texCoord = glm::vec2(0.0f, 1.0f);

		surface.addVertex(v[0]);
		surface.addVertex(v[1]);
		surface.addVertex(v[2]);
		surface.addVertex(v[3]);

		Triangle t0(baseIndex, baseIndex + 1, baseIndex + 2);
		Triangle t1(baseIndex, baseIndex + 2, baseIndex + 3);

		surface.addTriangle(t0);
		surface.addTriangle(t1);
	}

	Mesh::Ptr createCube(glm::vec3 position, float edgeLength)
	{
		float s = edgeLength / 2.0f;

		glm::vec3 v1 = glm::vec3(-s, -s, s) + position;
		glm::vec3 v2 = glm::vec3(s, -s, s) + position;
		glm::vec3 v3 = glm::vec3(-s, s, s) + position;
		glm::vec3 v4 = glm::vec3(s, s, s) + position;
		glm::vec3 v5 = glm::vec3(-s, -s, -s) + position;
		glm::vec3 v6 = glm::vec3(s, -s, -s) + position;
		glm::vec3 v7 = glm::vec3(-s, s, -s) + position;
		glm::vec3 v8 = glm::vec3(s, s, -s) + position;

		TriangleSurface surface;
		addFace(surface, v1, v2, v4, v3);
		addFace(surface, v6, v5, v7, v8);
		addFace(surface, v5, v1, v3, v7);
		addFace(surface, v2, v6, v8, v4);
		addFace(surface, v5, v6, v2, v1);
		addFace(surface, v3, v4, v8, v7);

		Mesh::Ptr mesh = Mesh::create("cube", surface, 0);

		return mesh;
	}

	Mesh::Ptr createQuad(glm::vec3 position, float edgeLength)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		float s = edgeLength / 2.0f;

		glm::vec3 v1 = glm::vec3(-s, -s, 0.0f) + position;
		glm::vec3 v2 = glm::vec3(s, -s, 0.0f) + position;
		glm::vec3 v3 = glm::vec3(s, s, 0.0f) + position;
		glm::vec3 v4 = glm::vec3(-s, s, 0.0f) + position;

		TriangleSurface surface;
		addFace(surface, v1, v2, v3, v4);

		Mesh::Ptr mesh = Mesh::create("quad", surface, 0);

		return mesh;
	}
}