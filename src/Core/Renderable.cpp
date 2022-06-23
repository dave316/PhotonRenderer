#include "Renderable.h"

#include <iostream>

void Renderable::setMesh(Mesh::Ptr mesh)
{
	this->mesh = mesh;
	morphTagets = mesh->useMorphTargets();
}

Renderable::~Renderable()
{
	//std::cout << "renderable: " << mesh->getName() << " destroyed" << std::endl;
}

void Renderable::render(Shader::Ptr shader)
{
	mesh->draw(shader);
}

void Renderable::switchMaterial(int materialIndex)
{
	mesh->switchVariant(materialIndex);
}

bool Renderable::useBlending()
{
	return mesh->useBlending();
}

bool Renderable::isTransmissive()
{
	return mesh->isTransmissive();
}

std::vector<float> Renderable::getWeights()
{
	return mesh->getWeights();
}

void Renderable::setSkin(Skin::Ptr skin)
{
	this->skin = skin;
	skinnedMesh = true;
}

bool Renderable::isSkinnedMesh()
{
	return skinnedMesh;
}

bool Renderable::useMorphTargets()
{
	return morphTagets;
}

std::string Renderable::getShader()
{
	return mesh->getShader();
}

Skin::Ptr Renderable::getSkin()
{
	return skin;
}

Mesh::Ptr Renderable::getMesh()
{
	return mesh;
}

AABB Renderable::getBoundingBox()
{
	return mesh->getBoundingBox();
}

void Renderable::computeJoints(std::vector<Entity::Ptr>& nodes)
{
	skin->computeJoints(nodes);
}