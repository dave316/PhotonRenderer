#ifndef INCLUDED_SKIN
#define INCLUDED_SKIN

#pragma once

#include <Core/Entity.h>

class Skin
{
private:
	std::string name;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<int> joints;
	std::vector<glm::mat4> jointMatrices;
	std::vector<glm::mat3> normalMatrices;
	unsigned int skeleton;

public:
	Skin(const std::string& name);
	void setSkeleton(unsigned int index);
	void addJoint(int index, glm::mat4 ibm);
	void computeJoints(std::vector<Entity::Ptr>& nodes);
	int numJoints();
	std::vector<int> getJoints();
	std::vector<glm::mat4> getBoneTransform();
	std::vector<glm::mat3> getNormalTransform();

	typedef std::shared_ptr<Skin> Ptr;
	static Ptr create(const std::string& name)
	{
		return std::make_shared<Skin>(name);
	}
};

#endif // INCLUDED_SKIN