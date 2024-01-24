#ifndef INCLUDED_COLLIDER
#define INCLUDED_COLLIDER

#pragma once

#include "Component.h"

#include <Math/Shapes.h>

class Collider : public Component
{
private:

public:
	virtual bool rayTest(Ray& ray, glm::vec3& hitpoint) = 0;
	typedef std::shared_ptr<Collider> Ptr;
};

class BoxCollider : public Collider
{
private:
	Box boundingBox;
public:
	BoxCollider(Box& bbox);
	bool rayTest(Ray& ray, glm::vec3& hitpoint);
	Box getAABB();
};

class SphereCollider : public Collider
{
private:
	Sphere sphere;
public:
	SphereCollider(Sphere& sphere);
	bool rayTest(Ray& ray, glm::vec3& hitpoint);
};


#endif // INCLUDED_COLLIDER