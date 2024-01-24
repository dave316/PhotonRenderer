#include "Collider.h"

#include <Math/Intersection.h>

BoxCollider::BoxCollider(Box& bbox)
{
	this->boundingBox = bbox;
}

bool BoxCollider::rayTest(Ray& ray, glm::vec3& hitpoint)
{
	return Intersection::rayBoxIntersection(ray, boundingBox, hitpoint);
}

Box BoxCollider::getAABB()
{
	return boundingBox;
}

SphereCollider::SphereCollider(Sphere& sphere)
{
	this->sphere = sphere;
}

bool SphereCollider::rayTest(Ray& ray, glm::vec3& hitpoint)
{
	return Intersection::raySphereIntersection(ray, sphere, hitpoint);
}