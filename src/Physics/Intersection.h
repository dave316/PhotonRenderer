#ifndef INCLUDED_INTERSECTION
#define INCLUDED_INTERSECTION

#pragma once

#include <Graphics/Geometry.h>

namespace Intersection
{
	inline static bool rayBoxIntersection(Ray& ray, const AABB& box, glm::vec3& hitPoint)
	{
		glm::vec3 bMin = (box.getMinPoint() - ray.origin) * ray.dirInv;
		glm::vec3 bMax = (box.getMaxPoint() - ray.origin) * ray.dirInv;
		glm::vec3 mMin = glm::min(bMin, bMax);
		glm::vec3 mMax = glm::max(bMin, bMax);
		float tmin = glm::max(glm::max(mMin.x, mMin.y), mMin.z);
		float tmax = glm::min(glm::min(mMax.x, mMax.y), mMax.z);
		if (tmax < 0 || tmin > tmax)
			return false;

		hitPoint = ray.origin + tmin * ray.direction;
		return true;
	}
}

#endif // INCLUDED_COLLISION