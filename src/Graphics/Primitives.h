#ifndef INCLUDED_PRIMITIVES
#define INCLUDED_PRIMITIVES

#pragma once

#include "Mesh.h"

namespace Primitives
{
	void addFace(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
	void addLine(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

	Mesh::Ptr createSphere(glm::vec3 position, float radius, unsigned int rings, unsigned int sectors);
	Mesh::Ptr createCube(glm::vec3 position, float edgeLength);
	Mesh::Ptr createBox(glm::vec3 position, glm::vec3 size);
	Mesh::Ptr createLineBox(glm::vec3 position, glm::vec3 size);
	Mesh::Ptr createQuad(glm::vec3 position, float edgeLength);
}

#endif // INCLUDED_PRIMITIVES