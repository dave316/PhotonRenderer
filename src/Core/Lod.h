#ifndef INCLUDED_LOD
#define INCLUDED_LOD

#pragma once

#include "Component.h"
#include "Renderable.h"

class Lod : public Component
{
private:

	std::vector<std::pair<float, Renderable::Ptr>> heights;
	std::map<float, Renderable::Ptr> distances;
	glm::vec3 localReferencePoint;
	float size;

public:
	Lod(float size, glm::vec3 refPoint);
	void addLevel(float height, Renderable::Ptr renderable);
	void computeDistances(float fovy, float scale);
	void selectLod(glm::vec3 cameraPosition, glm::mat4 localToWorld);
	void print();
	float getSize();
	glm::vec3 getReferencePoint();
	std::vector<std::pair<float, Renderable::Ptr>> getLevels();
	typedef std::shared_ptr<Lod> Ptr;
	static Ptr create(float size, glm::vec3 refPoint)
	{
		return std::make_shared<Lod>(size, refPoint);
	}
};

#endif // INCLUDED_LOD