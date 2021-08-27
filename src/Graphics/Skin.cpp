#include "Skin.h"
#include <glm/gtc/matrix_inverse.hpp>


Skin::Skin() :
	skeleton(0)
{

}

void Skin::setSkeleton(unsigned int index)
{
	skeleton = index;
}

void Skin::addJoint(int index, glm::mat4 ibm)
{
	joints.push_back(index);
	inverseBindMatrices.push_back(ibm);
}

void Skin::computeJoints(std::vector<Entity::Ptr>& nodes)
{
	jointMatrices.clear();
	normalMatrices.clear();

	Entity::Ptr parentNode = nodes[skeleton];

	for (int i = 0; i < joints.size(); i++)
	{
		int jointIdx = joints[i];
		auto node = nodes[jointIdx];
		glm::mat4 jointIBM = inverseBindMatrices[i];
		glm::mat4 nodeLocalToWorld = node->getComponent<Transform>()->getTransform();
		glm::mat4 parentWorldToLocal = glm::inverse(parentNode->getComponent<Transform>()->getTransform());
		glm::mat4 jointMatrix = nodeLocalToWorld * jointIBM;
		jointMatrix = parentWorldToLocal * jointMatrix;
		glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(jointMatrix));
		jointMatrices.push_back(jointMatrix);
		normalMatrices.push_back(normalMatrix);
	}

}

int Skin::numJoints()
{
	return joints.size();
}

std::vector<glm::mat4> Skin::getBoneTransform()
{
	return jointMatrices;
}

std::vector<glm::mat3> Skin::getNormalTransform()
{
	return normalMatrices;
}