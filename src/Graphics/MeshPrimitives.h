#ifndef INCLUDED_MESHPRIMITIVES
#define INCLUDED_MESHPRIMITIVES

#pragma once

#include "Primitive.h"

namespace MeshPrimitives
{
	void addFace(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
	void addLine(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

	Primitive::Ptr createIcosphere(glm::vec3 position, float radius, unsigned int subdivisions);
	Primitive::Ptr createUVSphere(glm::vec3 position, float radius, unsigned int rings, unsigned int sectors);
	Primitive::Ptr createCube(glm::vec3 position, float edgeLength);
	Primitive::Ptr createBox(glm::vec3 position, glm::vec3 size);
	Primitive::Ptr createLineBox(glm::vec3 position, glm::vec3 size);
	Primitive::Ptr createQuad(glm::vec3 position, float edgeLength);
}

#endif // INCLUDED_MESHPRIMITIVES