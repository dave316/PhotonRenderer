#ifndef INCLUDED_INTERSECTION
#define INCLUDED_INTERSECTION

#pragma once

#include <Graphics/Geometry.h>

namespace Intersection
{
	inline static bool rayBoxIntersection(Ray& ray, const Box& box, glm::vec3& hitPoint)
	{
		glm::vec3 bMin = (box.getMinPoint() - ray.origin) / ray.direction;
		glm::vec3 bMax = (box.getMaxPoint() - ray.origin) / ray.direction;
		glm::vec3 mMin = glm::min(bMin, bMax);
		glm::vec3 mMax = glm::max(bMin, bMax);
		float tmin = glm::max(glm::max(mMin.x, mMin.y), mMin.z);
		float tmax = glm::min(glm::min(mMax.x, mMax.y), mMax.z);
		if (tmax < 0 || tmin > tmax)
			return false;

		hitPoint = ray.origin + tmin * ray.direction;
		return true;
	}

	inline static bool raySphereIntersection(Ray& ray, const Sphere& sphere, glm::vec3 &hitpoint)
	{
		glm::vec3 v = ray.origin - sphere.position;
		float a = glm::dot(v, ray.direction);
		float b = glm::dot(v, v) - sphere.radius * sphere.radius;
		if (a > 0.0f && b > 0.0f)
			return false;

		float d = a * a - b;
		if (d < 0.0f)
			return false;

		float t = -a - glm::sqrt(d);
		if (t < 0.0f)
			t = 0.0f;

		hitpoint = ray.origin + t * ray.direction;
		return true;
	}
}

#endif // INCLUDED_COLLISION