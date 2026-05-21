#ifndef INCLUDED_GEOMETRY
#define INCLUDED_GEOMETRY

#pragma once

#include <glm/glm.hpp>

//struct Ray
//{
//	glm::vec3 origin;
//	glm::vec3 direction;
//	glm::vec3 dirInv;
//};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 origin, glm::vec3 direction);
	glm::vec3 march(float t);
};

struct Triangle
{
	glm::vec3 v0, v1, v2;
	glm::vec3 n0, n1, n2;
	glm::vec3 f0, f1, f2;
	glm::vec3 plane;
	unsigned int triID;
};

struct GridCell
{
	Triangle t0;
	Triangle t1;
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
	float volume();
	bool isInside(const glm::vec3& point);
	glm::vec3 getCenter();
	glm::vec3 getSize();
	std::vector<glm::vec3> getPoints();	
};

//struct Sphere
//{
//	glm::vec3 position;
//	float radius;
//
//	Sphere();
//	Sphere(glm::vec3& position, float radius);
//};

struct Sphere
{
	glm::vec3 position;
	float radius;

	Sphere(glm::vec3 position = glm::vec3(0), float radius = 1.0f);
	float area();
	float volume();
};


#endif // INCLUDED_GEOMETRY