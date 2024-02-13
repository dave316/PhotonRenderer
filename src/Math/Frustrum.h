#ifndef INCLUDED_FRUSTRUM
#define INCLUDED_FRUSTRUM

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Geometry.h>

#include "Shapes.h"

namespace Math
{
	//struct Plane
	//{
	//	glm::vec3 normal = glm::vec3(0, 1, 0);
	//	float distance = 0;

	//	Plane();
	//	Plane(glm::vec3 p, glm::vec3 n);
	//	float getSignedDistance(glm::vec3 p);
	//	bool isInside(AABB& bbox);
	//};

	struct Frustrum
	{
		Plane near;
		Plane far;
		Plane right;
		Plane left;
		Plane top;
		Plane bottom;

		Frustrum(FPSCamera& camera);
		bool isInside(Box& bbox, glm::mat4 localToWorld);
	};

	// TODO: integrate in Frustrum class
	std::vector<glm::vec4> getFrustrumCorners(glm::mat4& VP);
	glm::mat4 getLightSpaceMatrix(FPSCamera& camera, glm::vec3& lightDir, float zNear, float zFar);
	std::vector<glm::mat4> getLightSpaceMatrices(FPSCamera& camera, glm::vec3 lightDir);
	std::vector<glm::mat4> creatCMViews(glm::vec3 pos, float zNear, float zFar);
}

#endif // INCLUDED_FRUSTRUM