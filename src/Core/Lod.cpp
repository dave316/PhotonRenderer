#include "Lod.h"

Lod::Lod(float size, glm::vec3 refPoint) :
	size(size),
	localReferencePoint(refPoint)
{	

}

void Lod::addLevel(float height, Renderable::Ptr renderable)
{
	heights.push_back(std::make_pair(height, renderable));
}

void Lod::computeDistances(float fovy, float scale)
{
	distances.clear();

	float worldSize = size * scale;
	float distance = worldSize * 0.5f / glm::tan(fovy * 0.5f); // distance at 100%
	for (auto [height, rend] : heights)
	{
		float targetDistance = distance / height;
		distances.insert(std::make_pair(targetDistance, rend));
	}
}

void Lod::selectLod(glm::vec3 cameraPosition, glm::mat4 localToWorld)
{
	glm::vec3 refPoint = glm::vec3(localToWorld * glm::vec4(localReferencePoint, 1.0f));
	float distance = glm::distance(cameraPosition, refPoint);

	for (auto [_, rend] : distances)
		rend->setEnabled(false);

	for (auto [dist, rend] : distances)
	{
		if (distance < dist)
		{
			rend->setEnabled(true);
			break;
		}
	}
}

void Lod::print()
{
	for (auto [dist, rend] : distances)
	{
		auto name = rend->getMesh()->getSubMeshes()[0].primitive->getName();
		std::cout << name << " : " << dist << std::endl;
	}		
}

float Lod::getSize()
{
	return size;
}

glm::vec3 Lod::getReferencePoint()
{
	return localReferencePoint;
}

std::vector<std::pair<float, Renderable::Ptr>> Lod::getLevels()
{
	return heights;
}