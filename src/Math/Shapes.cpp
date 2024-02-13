#include "Shapes.h"

Ray::Ray(glm::vec3 origin, glm::vec3 direction) :
	origin(origin),
	direction(direction)
{
}

glm::vec3 Ray::march(float t)
{
	return origin + t * direction;
}

Line::Line(glm::vec3 v0, glm::vec3 v1) :
	v0(v0), v1(v1)
{
}

float Line::length()
{
	return glm::distance(v0, v1);
}

Triangle::Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) :
	v0(v0), v1(v1), v2(v2)
{
}

float Triangle::area()
{
	glm::vec3 e0 = v1 - v0;
	glm::vec3 e1 = v2 - v0;
	return glm::length(glm::cross(e0, e1)) / 2.0;
}

Plane::Plane(glm::vec3 p, glm::vec3 n)
{
	normal = glm::normalize(n);
	distance = glm::dot(normal, p);
}

float Plane::getSignedDistance(glm::vec3 p)
{
	return glm::dot(normal, p) - distance;
}

bool Plane::isInside(Box& bbox)
{
	glm::vec3 maxPoint = bbox.getMaxPoint();
	glm::vec3 center = bbox.getCenter();
	glm::vec3 extents = maxPoint - center;

	float r = extents.x * std::abs(normal.x) +
		extents.y * std::abs(normal.y) +
		extents.z * std::abs(normal.z);

	return -r <= getSignedDistance(center);
}

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

Cylinder::Cylinder(glm::vec3 position, float radius, float height) :
	position(position), radius(radius), height(height)
{
}

float Cylinder::area()
{
	return 0;
}

float Cylinder::volume()
{
	return 0;
}

Cube::Cube(glm::vec3 position, float edgeLength) :
	position(position), edgeLength(edgeLength)
{
}

float Cube::area()
{
	return 0;
}

float Cube::volume()
{
	return 0;
}

Box::Box()
{
	constexpr float maxVal = std::numeric_limits<float>::max();
	constexpr float minVal = -maxVal;

	minPoint = glm::vec3(maxVal);
	maxPoint = glm::vec3(minVal);
}

Box::Box(glm::vec3& minPoint, glm::vec3& maxPoint) :
	minPoint(minPoint),
	maxPoint(maxPoint)
{

}

glm::vec3 Box::getMinPoint() const
{
	return minPoint;
}

glm::vec3 Box::getMaxPoint() const
{
	return maxPoint;
}

void Box::expand(const glm::vec3& point)
{
	maxPoint = glm::max(maxPoint, point);
	minPoint = glm::min(minPoint, point);
}

void Box::expand(const Triangle& tri)
{
	expand(tri.v0);
	expand(tri.v1);
	expand(tri.v2);
}

void Box::expand(const Box& box)
{
	expand(box.getMinPoint());
	expand(box.getMaxPoint());
}

float Box::radius()
{
	glm::vec3 diff = maxPoint - minPoint;
	return glm::sqrt(glm::dot(diff, diff) * 0.25f);
}

float Box::volume()
{
	glm::vec3 size = getSize();
	return size.x * size.y * size.z;
}

bool Box::isInside(const glm::vec3& point)
{
	return(point.x > minPoint.x &&
		point.y > minPoint.y &&
		point.z > minPoint.z &&
		point.x < maxPoint.x&&
		point.y < maxPoint.y&&
		point.z < maxPoint.z);
}

glm::vec3 Box::getCenter()
{
	return (maxPoint + minPoint) / 2.0f;
}

glm::vec3 Box::getSize()
{
	return (maxPoint - minPoint);
}

std::vector<glm::vec3> Box::getPoints()
{
	std::vector<glm::vec3> points;
	points.push_back(minPoint);
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, maxPoint.y, minPoint.z));
	points.push_back(maxPoint);
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, maxPoint.z));
	return points;
}