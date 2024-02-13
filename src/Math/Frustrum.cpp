#include "Frustrum.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Math
{
	//Plane::Plane()
	//{

	//}

	//Plane::Plane(glm::vec3 p, glm::vec3 n)
	//{
	//	normal = glm::normalize(n);
	//	distance = glm::dot(normal, p);
	//}

	//float Plane::getSignedDistance(glm::vec3 p)
	//{
	//	return glm::dot(normal, p) - distance;
	//}

	//bool Plane::isInside(AABB& bbox)
	//{
	//	glm::vec3 maxPoint = bbox.getMaxPoint();
	//	glm::vec3 center = bbox.getCenter();
	//	glm::vec3 extents = maxPoint - center;

	//	float r = extents.x * std::abs(normal.x) +
	//		extents.y * std::abs(normal.y) +
	//		extents.z * std::abs(normal.z);

	//	return -r <= getSignedDistance(center);
	//}

	Frustrum::Frustrum(FPSCamera& camera)
	{
		glm::vec3 p = camera.getPosition();
		glm::vec3 f = camera.getForward();
		glm::vec3 r = camera.getRight();
		glm::vec3 u = glm::normalize(glm::cross(r, f));
		float zNear = camera.getZNear();
		float zFar = camera.getZFar();
		float aspect = camera.getAspect();
		float fovY = camera.getFov();
		float halfVSide = zFar * tanf(fovY * 0.5f);
		float halfHSide = halfVSide * aspect;
		glm::vec3 frontMultFar = zFar * f;

		near = Plane(p + zNear * f, f);
		far = Plane(p + frontMultFar, -f);
		right = Plane(p, glm::cross(u, frontMultFar + r * halfHSide));
		left = Plane(p, glm::cross(frontMultFar - r * halfHSide, u));
		top = Plane(p, glm::cross(r, frontMultFar - u * halfVSide));
		bottom = Plane(p, glm::cross(frontMultFar + u * halfVSide, r));
	}

	bool Frustrum::isInside(Box& bbox, glm::mat4 localToWorld)
	{
		glm::vec3 maxPoint = bbox.getMaxPoint();
		glm::vec3 center = bbox.getCenter();
		glm::vec3 extents = maxPoint - center;

		glm::vec3 worldCenter = glm::vec3(localToWorld * glm::vec4(center, 1.0f));
		glm::vec3 mr = localToWorld[0] * extents.x;
		glm::vec3 mu = localToWorld[1] * extents.y;
		glm::vec3 mf = (-localToWorld[2]) * extents.z;

		glm::vec3 wr = glm::vec3(1, 0, 0);
		glm::vec3 wu = glm::vec3(0, 1, 0);
		glm::vec3 wf = glm::vec3(0, 0, 1);

		float i = std::abs(glm::dot(wr, mr)) + std::abs(glm::dot(wr, mu)) + std::abs(glm::dot(wr, mf));
		float j = std::abs(glm::dot(wu, mr)) + std::abs(glm::dot(wu, mu)) + std::abs(glm::dot(wu, mf));
		float k = std::abs(glm::dot(wf, mr)) + std::abs(glm::dot(wf, mu)) + std::abs(glm::dot(wf, mf));

		glm::vec3 worldExtents = glm::vec3(i, j, k);
		glm::vec3 minP = worldCenter - worldExtents;
		glm::vec3 maxP = worldCenter + worldExtents;
		Box worldBox(minP, maxP);

		return near.isInside(worldBox) && far.isInside(worldBox) &&
				right.isInside(worldBox) && left.isInside(worldBox) &&
				top.isInside(worldBox) && bottom.isInside(worldBox);
	}

	std::vector<glm::vec4> getFrustrumCorners(glm::mat4& VP)
	{
		auto VP_I = glm::inverse(VP);
		std::vector<glm::vec4> frustumCorners;
		frustumCorners.push_back(glm::vec4(-1, -1, -1, 1));
		frustumCorners.push_back(glm::vec4(-1, -1, 1, 1));
		frustumCorners.push_back(glm::vec4(-1, 1, -1, 1));
		frustumCorners.push_back(glm::vec4(-1, 1, 1, 1));
		frustumCorners.push_back(glm::vec4(1, -1, -1, 1));
		frustumCorners.push_back(glm::vec4(1, -1, 1, 1));
		frustumCorners.push_back(glm::vec4(1, 1, -1, 1));
		frustumCorners.push_back(glm::vec4(1, 1, 1, 1));
		for (auto& fp : frustumCorners)
		{
			auto p = VP_I * fp;
			p /= p.w;
			fp = p;
		}
		return frustumCorners;
	}

	glm::mat4 getLightSpaceMatrix(FPSCamera& camera, glm::vec3& lightDir, float zNear, float zFar)
	{
		auto P = glm::perspective(camera.getFov(), camera.getAspect(), zNear, zFar);
		auto VP = P * camera.getViewMatrix();
		auto corners = getFrustrumCorners(VP);
		auto center = glm::vec3(0);
		for (auto& fp : corners)
			center += glm::vec3(fp);
		center /= corners.size();

		auto lightView = glm::lookAt(center, center + lightDir, glm::vec3(0, 1, 0));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();
		for (auto c : corners)
		{
			auto cView = lightView * c;
			minX = std::min(minX, cView.x);
			maxX = std::max(maxX, cView.x);
			minY = std::min(minY, cView.y);
			maxY = std::max(maxY, cView.y);
			minZ = std::min(minZ, cView.z);
			maxZ = std::max(maxZ, cView.z);
		}

		float zFactor = 20.0f;
		minZ = minZ < 0 ? minZ * zFactor : minZ / zFactor;
		maxZ = maxZ < 0 ? maxZ / zFactor : maxZ * zFactor;

		auto lightProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		return lightProj * lightView;
	}

	std::vector<glm::mat4> getLightSpaceMatrices(FPSCamera& camera, glm::vec3 lightDir)
	{
		float zNear = camera.getZNear();
		float zFar = camera.getZFar();

		std::vector<float> csmLevels =
		{
			zFar / 10.0f,
			zFar / 5.0f,
			zFar / 2.0f,
		};

		std::vector<glm::mat4> lightSpaceMatrices;
		for (int i = 0; i < csmLevels.size() + 1; ++i)
		{
			if (i == 0)
				lightSpaceMatrices.push_back(getLightSpaceMatrix(camera, lightDir, zNear, csmLevels[i]));
			else if (i < csmLevels.size())
				lightSpaceMatrices.push_back(getLightSpaceMatrix(camera, lightDir, csmLevels[i - 1], csmLevels[i]));
			else
				lightSpaceMatrices.push_back(getLightSpaceMatrix(camera, lightDir, csmLevels[i - 1], zFar));
		}
		return lightSpaceMatrices;
	}

	std::vector<glm::mat4> creatCMViews(glm::vec3 pos, float zNear, float zFar)
	{
		glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, zNear, zFar);
		std::vector<glm::mat4> VPs;
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		VPs.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		return VPs;
	}
}
