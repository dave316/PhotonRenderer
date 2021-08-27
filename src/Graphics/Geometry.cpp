#include "Geometry.h"

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

glm::vec3 AABB::getCenter()
{
	return (maxPoint + minPoint) / 2.0f;
}

glm::vec3 AABB::getSize()
{
	return (maxPoint - minPoint);
}

std::vector<glm::vec3> AABB::getPoints()
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