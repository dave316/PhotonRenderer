#ifndef INCLUDED_FRUSTRUM
#define INCLUDED_FRUSTRUM

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Primitive.h>

namespace Math
{
	struct Plane
	{
		glm::vec3 normal;
		float distance;

		Plane(glm::vec3 p = glm::vec3(0), glm::vec3 n = glm::vec3(0, 0, 1));
		float getSignedDistance(glm::vec3 p);
		bool isInside(Box& box);
	};

	struct Frustrum
	{
		Plane nearP;
		Plane farP;
		Plane right;
		Plane left;
		Plane top;
		Plane bottom;

		Frustrum(FPSCamera& camera);
		bool isInside(Box& bbox, glm::mat4 localToWorld);
	};

	// TODO: integrate in Frustrum class
	std::vector<glm::vec4> getFrustrumCorners(glm::mat4& VP);
	glm::mat4 getLightSpaceMatrix(UserCamera& camera, glm::vec3& lightDir, float zNear, float zFar);
	std::vector<glm::mat4> getLightSpaceMatrices(UserCamera& camera, glm::vec3 lightDir);
	std::vector<glm::mat4> creatCMViews(glm::vec3 pos, float zNear, float zFar);
}

#endif // INCLUDED_FRUSTRUM