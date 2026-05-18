#ifndef INCLUDED_AABBTREE
#define INCLUDED_AABBTREE

#pragma once

#include "Intersection.h"

#include <vector>

typedef std::vector<Triangle> TriList;

class AABBNode
{
private:
	AABBNode* parent;
	AABBNode* leftChild;
	AABBNode* rightChild;
	AABB boundingBox;
	Triangle* tri;


public:
	AABBNode(AABBNode* parent);
	~AABBNode();
	static bool compareX(Triangle a, Triangle b);
	static bool compareY(Triangle a, Triangle b);
	static bool compareZ(Triangle a, Triangle b);
	
	void addTriangles(TriList& triangles);
	float splitEntries(bool(*compare)(Triangle, Triangle), TriList& entries, TriList& splitA, TriList& splitB);
	bool raycast(Ray& ray, glm::vec3& hitPoint, glm::vec2& uv, unsigned int& triID);
	const AABB& getBoundingBox();
};

#endif // INCLUDED_AABBTREE