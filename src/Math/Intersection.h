#ifndef INCLUDED_COLLISION
#define INCLUDED_COLLISION

#pragma once

#include "Geometry.h"

#include <iostream>

namespace Intersections
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

	inline static bool rayTriIntersection(Ray& ray, Triangle& tri, glm::vec3& hitPoint, glm::vec2& uv)
	{
		glm::vec3 e1 = tri.v1 - tri.v0;
		glm::vec3 e2 = tri.v2 - tri.v0;

		glm::vec3 p = glm::cross(ray.direction, e2);
		float eps = 0.0000001f;
		float det = glm::dot(e1, p);

		if (det > -eps && det < eps)
			return false;

		float det_inv = 1.0f / det;
		glm::vec3 diff = ray.origin - tri.v0;
		float u = glm::dot(diff, p) * det_inv;

		if (u < 0.0 || u > 1.0)
			return false;

		glm::vec3 q = glm::cross(diff, e1);
		float v = glm::dot(ray.direction, q) * det_inv;

		if (v < 0.0 || u + v > 1.0)
			return false;

		float t = glm::dot(e2, q) * (float)det_inv;

		if (t > eps)
		{
			uv.x = u;
			uv.y = v;
			hitPoint = ray.origin + t * ray.direction;
			return true;
		}

		return false;
	}

	inline static bool raySphereIntersection(Ray& ray, Sphere& sphere, float& t)
	{
		glm::vec3 v = ray.origin - sphere.position;
		float a = glm::dot(v, ray.direction);
		float b = glm::dot(v, v) - sphere.radius * sphere.radius;
		if (a > 0.0f && b > 0.0f)
			return false;

		float d = a * a - b;
		if (d < 0.0f)
			return false;

		t = -a - glm::sqrt(d);
		//if (t < 0.0f)
		//	t = 0.0f;
		
		//hitPoint = ray.origin + t * ray.direction;
		//hitNormal = glm::normalize(hitPoint - sphere.position);

		return true;
	}

	inline static bool sphereBoxIntersection(Sphere& sphere, AABB& box, glm::vec3& hitPoint, glm::vec3& hitNormal)
	{
		glm::vec3 minPoint = box.getMinPoint();
		glm::vec3 maxPoint = box.getMaxPoint();
		glm::vec3 center = sphere.position;
		float radius = sphere.radius;
		
		if (center.x < minPoint.x) hitPoint.x = minPoint.x;
		else if (center.x > maxPoint.x) hitPoint.x = maxPoint.x;
		else hitPoint.x = center.x;

		if (center.y < minPoint.y) hitPoint.y = minPoint.y;
		else if (center.y > maxPoint.y) hitPoint.y = maxPoint.y;
		else hitPoint.y = center.y;

		if (center.z < minPoint.z) hitPoint.z = minPoint.z;
		else if (center.z > maxPoint.z) hitPoint.z = maxPoint.z;
		else hitPoint.z = center.z;

		float distance = glm::distance(hitPoint, center);
		if (distance <= radius)
		{
			if (distance > 0)
				hitNormal = glm::normalize(center - hitPoint);
			else
				hitNormal = glm::normalize(center - box.getCenter());
			return true;
		}

		return false;
	}

	inline static bool triContainsPoint(Triangle& tri, glm::vec3 p)
	{
		glm::vec3 e1 = tri.v1 - tri.v0;
		glm::vec3 e2 = tri.v2 - tri.v0;
		glm::vec3 n = glm::normalize(glm::cross(e1, e2));
		if (glm::dot(n, glm::cross(tri.v1 - tri.v0, p - tri.v0)) <= 0)
			return false;
		if (glm::dot(n, glm::cross(tri.v2 - tri.v1, p - tri.v1)) <= 0)
			return false;
		if (glm::dot(n, glm::cross(tri.v0 - tri.v2, p - tri.v2)) <= 0)
			return false;

		return true;
	}

	inline static glm::vec3 closestPointToLine(glm::vec3& p, glm::vec3& a, glm::vec3& b)
	{
		glm::vec3 c = p - a;
		glm::vec3 w = b - a;
		float d = glm::length(w);
		if (glm::abs(d) < 0.0001f)
			return a;
		w /= d;
		float t = glm::dot(w, c);
		if (t < 0.0f)
			return a;
		else if (t > d)
			return b;
		return a + t * w;
	}

	inline static float closestPointToTri(Triangle& t, glm::vec3& p, glm::vec3& result)
	{
		result = closestPointToLine(p, t.v0, t.v1);
		float bestDist = glm::distance(p, result);

		glm::vec3 q = closestPointToLine(p, t.v1, t.v2);
		float d = glm::distance(p, q);
		if (d < bestDist)
		{
			bestDist = d;
			result = q;
		}

		q = closestPointToLine(p, t.v2, t.v0);
		d = glm::distance(p, q);
		if (d < bestDist)
		{
			bestDist = d;
			result = q;
		}

		return bestDist;
	}

	inline static glm::vec3 getSmoothNormal(Triangle& t, glm::vec3& p)
	{
		glm::vec3 v0 = t.v1 - t.v0;
		glm::vec3 v1 = t.v2 - t.v0;
		glm::vec3 v2 = p - t.v0;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;
		return glm::normalize(t.n0 * u + t.n1 * v + t.n2 * w);
	}
}

#endif // INCLUDED_COLLISION