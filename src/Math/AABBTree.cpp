#include "AABBTree.h"

#include <algorithm>

AABBNode::AABBNode(AABBNode* parent) :
	parent(parent)
{
	leftChild = nullptr;
	rightChild = nullptr;
	tri = nullptr;
}

AABBNode::~AABBNode()
{
	delete leftChild;
	delete rightChild;
	delete tri;
}

bool AABBNode::compareX(Triangle a, Triangle b)
{
	return a.plane.x < b.plane.x;
}

bool AABBNode::compareY(Triangle a, Triangle b)
{
	return a.plane.y < b.plane.y;
}

bool AABBNode::compareZ(Triangle a, Triangle b)
{
	return a.plane.z < b.plane.z;
}

void AABBNode::addTriangles(TriList& triangles)
{
	if (triangles.empty())
		return;

	if (triangles.size() == 1)
	{
		boundingBox.expand(triangles[0].v0);
		boundingBox.expand(triangles[0].v1);
		boundingBox.expand(triangles[0].v2);
		tri = new Triangle(triangles[0]);

		return;
	}

	std::vector<Triangle> splits[6];

	float boundX = splitEntries(compareX, triangles, splits[0], splits[1]);
	float boundY = splitEntries(compareY, triangles, splits[2], splits[3]);
	float boundZ = splitEntries(compareZ, triangles, splits[4], splits[5]);

	leftChild = new AABBNode(this);
	rightChild = new AABBNode(this);

	if (boundX <= boundY && boundX <= boundZ)
	{
		leftChild->addTriangles(splits[0]);
		rightChild->addTriangles(splits[1]);
	}
	else if (boundY <= boundX && boundY <= boundZ)
	{
		leftChild->addTriangles(splits[2]);
		rightChild->addTriangles(splits[3]);
	}
	else //(if (boundZ <= boundX && boundZ <= boundY)
	{
		assert(boundZ <= boundX && boundZ <= boundY);
		leftChild->addTriangles(splits[4]);
		rightChild->addTriangles(splits[5]);
	}

	boundingBox.expand(leftChild->getBoundingBox());
	boundingBox.expand(rightChild->getBoundingBox());
}

float AABBNode::splitEntries(bool(*compare)(Triangle, Triangle), TriList& entries, TriList& splitA, TriList& splitB)
{
	size_t numEntries = entries.size();
	splitA.reserve(numEntries / 2);
	splitB.reserve(numEntries / 2);

	std::sort(entries.begin(), entries.end(), compare);

	AABB boundA;
	AABB boundB;

	for (size_t i = 0; i < numEntries / 2; i++)
	{
		Triangle& tri = entries[i];
		boundA.expand(tri.v0);
		boundA.expand(tri.v1);
		boundA.expand(tri.v2);
		splitA.push_back(tri);
	}

	for (size_t i = numEntries / 2; i < numEntries; i++)
	{
		Triangle& tri = entries[i];
		boundB.expand(tri.v0);
		boundB.expand(tri.v1);
		boundB.expand(tri.v2);
		splitB.push_back(tri);
	}

	return boundA.radius() + boundB.radius();
}

glm::vec3 getInterpNormal(Triangle* tri, glm::vec2& uv)
{
	glm::vec3& n0 = tri->n0;
	glm::vec3& n1 = tri->n1;
	glm::vec3& n2 = tri->n2;
	return n0 + (n1 - n0) * uv.x + (n2 - n0) * uv.y;
}

bool AABBNode::raycast(Ray& ray, glm::vec3& hitPoint, glm::vec2& uv, unsigned int& triID)
{
	if (tri)
	{
		if (Intersections::rayTriIntersection(ray, *tri, hitPoint, uv))
		{
			triID = tri->triID;
			return true;
		}
		return false;
	}

	if (!leftChild || !rightChild)
		return false;

	const AABB& boundA = leftChild->getBoundingBox();
	const AABB& boundB = rightChild->getBoundingBox();

	glm::vec3 closestA;
	glm::vec3 closestB;

	if (!Intersections::rayBoxIntersection(ray, boundA, closestA))
	{
		return rightChild->raycast(ray, hitPoint, uv, triID);
	}
	if (!Intersections::rayBoxIntersection(ray, boundB, closestB))
	{
		return leftChild->raycast(ray, hitPoint, uv, triID);
	}

	glm::vec3 diffA = closestA - ray.origin;
	glm::vec3 diffB = closestB - ray.origin;
	float distA = glm::dot(diffA, diffA);
	float distB = glm::dot(diffB, diffB);

	glm::vec3 hitPointA;
	glm::vec3 hitPointB;
	glm::vec2 uvA;
	glm::vec2 uvB;
	unsigned int idA = 0;
	unsigned int idB = 0;

	if (distA < distB)
	{
		if (leftChild->raycast(ray, hitPointA, uvA, idA))
		{
			glm::vec3 diffHitA = hitPointA - ray.origin;
			float distHitA = glm::dot(diffHitA, diffHitA);
			if (distHitA < distB)
			{
				hitPoint = hitPointA;
				uv = uvA;
				triID = idA;
				return true;
			}

			if (!rightChild->raycast(ray, hitPointB, uvB, idB))
			{
				hitPoint = hitPointA;
				uv = uvA;
				triID = idA;
				return true;
			}

			glm::vec3 diffHitB = hitPointB - ray.origin;
			float distHitB = glm::dot(diffHitB, diffHitB);
			if (distHitA < distHitB)
			{
				hitPoint = hitPointA;
				uv = uvA;
				triID = idA;
				return true;
			}

			hitPoint = hitPointB;
			uv = uvB;
			triID = idB;
			return true;
		}
		else
		{
			return rightChild->raycast(ray, hitPoint, uv, triID);
		}
	}
	else
	{
		if (rightChild->raycast(ray, hitPointB, uvB, idB))
		{
			glm::vec3 diffHitB = hitPointB - ray.origin;
			float distHitB = glm::dot(diffHitB, diffHitB);
			if (distHitB < distA)
			{
				hitPoint = hitPointB;
				uv = uvB;
				triID = idB;
				return true;
			}

			if (!leftChild->raycast(ray, hitPointA, uvA, idA))
			{
				hitPoint = hitPointB;
				uv = uvB;
				triID = idB;
				return true;
			}

			glm::vec3 diffHitA = hitPointA - ray.origin;
			float distHitA = glm::dot(diffHitA, diffHitA);
			if (distHitB < distHitA)
			{
				hitPoint = hitPointB;
				uv = uvB;
				triID = idB;
				return true;
			}

			hitPoint = hitPointA;
			uv = uvA;
			triID = idA;
			return true;
		}
		else
		{
			return leftChild->raycast(ray, hitPoint, uv, triID);
		}
	}
}

const AABB& AABBNode::getBoundingBox()
{
	return boundingBox;
}