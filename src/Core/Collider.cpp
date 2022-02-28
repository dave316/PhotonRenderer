#include "Collider.h"

#include <Physics/Intersection.h>

BoxCollider::BoxCollider(AABB& bbox)
{
	this->boundingBox = bbox;
}

bool BoxCollider::rayTest(Ray& ray, glm::vec3& hitpoint)
{
	return Intersection::rayBoxIntersection(ray, boundingBox, hitpoint);
}

SphereCollider::SphereCollider(Sphere& sphere)
{
	this->sphere = sphere;
}

bool SphereCollider::rayTest(Ray& ray, glm::vec3& hitpoint)
{
	return Intersection::raySphereIntersection(ray, sphere, hitpoint);
}