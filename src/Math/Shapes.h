#ifndef INCLUDED_SHAPES
#define INCLUDED_SHAPES

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 origin, glm::vec3 direction);
	glm::vec3 march(float t);
};

struct Line
{
	glm::vec3 v0, v1;

	Line(glm::vec3 v0, glm::vec3 v1);
	float length();
};

struct Triangle
{
	glm::vec3 v0, v1, v2;

	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
	float area();
};

struct Sphere
{
	glm::vec3 position;
	float radius;

	Sphere(glm::vec3 position = glm::vec3(0), float radius = 1.0f);
	float area();
	float volume();
};

struct Cylinder
{
	glm::vec3 position;
	float radius;
	float height;

	Cylinder(glm::vec3 position, float radius, float height);
	float area();
	float volume();
};

struct Cube
{
private:
	glm::vec3 position;
	float edgeLength;

	Cube(glm::vec3 position, float edgeLength = 1.0f);
	float area();
	float volume();
};

class Box
{
private:
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

public:
	Box();
	Box(glm::vec3& minPoint, glm::vec3& maxPoint);

	glm::vec3 getMinPoint() const;
	glm::vec3 getMaxPoint() const;
	void expand(const glm::vec3& point);
	void expand(const Triangle& tri);
	void expand(const Box& box);
	float radius();
	float volume();
	bool isInside(const glm::vec3& point);
	glm::vec3 getCenter();
	glm::vec3 getSize();
	std::vector<glm::vec3> getPoints();
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	Plane(glm::vec3 p = glm::vec3(0), glm::vec3 n = glm::vec3(0,0,1));
	float getSignedDistance(glm::vec3 p);
	bool isInside(Box& box);
};

#endif // INCLUDED_SHAPES