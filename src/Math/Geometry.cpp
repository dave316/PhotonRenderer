#include "Geometry.h"

Ray::Ray(glm::vec3 origin, glm::vec3 direction) :
	origin(origin), direction(direction)
{

}

glm::vec3 Ray::march(float t)
{
	return origin + t * direction;
}

AABB::AABB()
{
	float maxVal = std::numeric_limits<float>::max();
	float minVal = -maxVal;

	minPoint = glm::vec3(maxVal);
	maxPoint = glm::vec3(minVal);
}

AABB::AABB(glm::vec3& minPoint, glm::vec3& maxPoint) :
	minPoint(minPoint),
	maxPoint(maxPoint)
{

}

glm::vec3 AABB::getMinPoint() const
{
	return minPoint;
}

glm::vec3 AABB::getMaxPoint() const
{
	return maxPoint;
}

void AABB::expand(const glm::vec3& point)
{
	maxPoint = glm::max(maxPoint, point);
	minPoint = glm::min(minPoint, point);
}

void AABB::expand(const Triangle& tri)
{
	expand(tri.v0);
	expand(tri.v1);
	expand(tri.v2);
}

void AABB::expand(const AABB& box)
{
	expand(box.getMinPoint());
	expand(box.getMaxPoint());
}

float AABB::radius()
{
	glm::vec3 diff = maxPoint - minPoint;
	return glm::sqrt(glm::dot(diff, diff) * 0.25f);
}

float AABB::volume()
{
	glm::vec3 size = getSize();
	return size.x * size.y * size.z;
}

bool AABB::isInside(const glm::vec3& point)
{
	return(point.x > minPoint.x &&
		point.y > minPoint.y &&
		point.z > minPoint.z &&
		point.x < maxPoint.x &&
		point.y < maxPoint.y &&
		point.z < maxPoint.z);
}

glm::vec3 AABB::getCenter()
{
	return (maxPoint + minPoint) / 2.0f;
}

glm::vec3 AABB::getSize()
{
	return maxPoint - minPoint;
}

//Sphere::Sphere() :
//	position(0),
//	radius(1)
//{
//}
//
//Sphere::Sphere(glm::vec3& position, float radius)
//{
//	this->position = position;
//	this->radius = radius;
//}

Sphere::Sphere(glm::vec3 position, float radius) :
	position(position), radius(radius)
{}

float Sphere::area()
{
	return 0;
}

float Sphere::volume()
{
	return 0;
}