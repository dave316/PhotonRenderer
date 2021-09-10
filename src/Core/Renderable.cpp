#include "Renderable.h"

#include <iostream>

//Renderable::Renderable(Mesh::Ptr mesh, Material::Ptr material) : 
//	mesh(mesh),
//	material(material)
//{
//
//}

void Renderable::addMesh(std::string name, Mesh::Ptr mesh, Material::Ptr material)
{
	this->name = name;
	Primitive p;
	p.mesh = mesh;
	p.material = material;
	primitives.push_back(p);
}

Renderable::~Renderable()
{
	//std::cout << "renderable: " << mesh->getName() << " destroyed" << std::endl;
}

void Renderable::render(Shader::Ptr shader)
{
	//program.setUniform("M", M);
	//program.setUniform("w0", weights.x);
	//program.setUniform("w1", weights.y);
	//for (auto mesh : meshes)
	//{
	//	auto matIndex = mesh->getMaterialIndex();
	//	if (matIndex < materials.size())
	//	{
	//		materials[matIndex]->setUniforms(program);
	//	}
	//	mesh->draw();
	//}

	for (auto& p : primitives)
	{
		if (p.material->isDoubleSided())
			glDisable(GL_CULL_FACE);

		if (p.material->useBlending())
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		p.material->setUniforms(shader);
		p.mesh->draw();

		if (p.material->useBlending())
			glDisable(GL_BLEND);

		if (p.material->isDoubleSided())
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
	}
}

bool Renderable::useBlending()
{
	for (auto& p : primitives)
		if (p.material->useBlending())
			return true;
	return false;
}

bool Renderable::isTransmissive()
{
	for (auto& p : primitives)
		if (p.material->isTransmissive())
			return true;
	return false;
}

// TODO: get whole array of morph weights
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
	//std::cout << "meshes: " << meshes.size() << std::endl;
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

void Renderable::computeJoints(std::vector<Entity::Ptr>& nodes)
{
	skin.computeJoints(nodes);
}