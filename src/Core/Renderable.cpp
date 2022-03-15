#include "Renderable.h"

#include <iostream>

//Renderable::Renderable(Mesh::Ptr mesh, Material::Ptr material) : 
//	mesh(mesh),
//	material(material)
//{
//
//}

//void Renderable::addMesh(std::string name, Mesh::Ptr mesh, Material::Ptr material)
//{
//	this->name = name;
//	Primitive p;
//	p.mesh = mesh;
//	p.materials.push_back(material);
//	primitives.push_back(p);
//}

void Renderable::addPrimitive(RenderPrimitive& primitve)
{
	primitives.push_back(primitve);
}

Renderable::~Renderable()
{
	//std::cout << "renderable: " << mesh->getName() << " destroyed" << std::endl;
}

void Renderable::render(Shader::Ptr shader)
{
	for (auto& p : primitives)
	{
		auto mat = p.getMaterial();
		//std::string name = mat->getShader();
		//Shader::Ptr shader = shaders[name];

		//shader->use();
		shader->setUniform("material.computeFlatNormals", p.computeFlatNormals);
		
		if (mat->isDoubleSided())
			glDisable(GL_CULL_FACE);

		if (mat->useBlending())
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		mat->setUniforms(shader);
		p.mesh->draw();

		if (mat->useBlending())
			glDisable(GL_BLEND);

		if (mat->isDoubleSided())
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
	}
}

void Renderable::switchMaterial(unsigned int materialIndex)
{
	// what if there are multiple primitives?? 
	// only switch if variant is present, otherwise use default material
	for (auto& p : primitives)
		p.switchMaterial(materialIndex);
}

bool Renderable::useBlending()
{
	for (auto& p : primitives)
		if (p.getMaterial()->useBlending())
			return true;
	return false;
}

bool Renderable::isTransmissive()
{
	for (auto& p : primitives)
		if (p.getMaterial()->isTransmissive())
			return true;
	return false;
}

std::vector<float> Renderable::getWeights()
{
	return morphWeights;
}

void Renderable::setSkin(Skin& skin)
{
	this->skin = skin;
	skinnedMesh = true;
}

void Renderable::setMorphWeights(std::vector<float>& weights)
{
	this->morphWeights = weights;
	morphTagets = true;
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
	return primitives[0].getMaterial()->getShader();
}

std::string Renderable::getName()
{
	return name;
}

std::vector<Vertex> Renderable::getVertices()
{
	std::vector<Vertex> vertices;
	for (auto& p : primitives)
	{
		auto v = p.mesh->getVertices();
		vertices.insert(vertices.end(), v.begin(), v.end());
	}
	return vertices;
}

void Renderable::print()
{
	std::cout << "primitives: " << primitives.size() << std::endl;
}

void Renderable::flipWindingOrder()
{
	for (auto p : primitives)
		p.mesh->flipWindingOrder();
}

Skin Renderable::getSkin()
{
	return skin;
}

AABB Renderable::getBoundingBox()
{
	AABB boundingBox;
	for (auto& p : primitives)
		boundingBox.expand(p.mesh->getBoundingBox());
	return boundingBox;
}

void Renderable::computeJoints(std::vector<Entity::Ptr>& nodes)
{
	skin.computeJoints(nodes);
}