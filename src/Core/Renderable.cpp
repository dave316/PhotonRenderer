#include "Renderable.h"

#include <iostream>

//Renderable::Renderable(Mesh::Ptr mesh, Material::Ptr material) : 
//	mesh(mesh),
//	material(material)
//{
//
//}

void Renderable::addMesh(Mesh::Ptr mesh, Material::Ptr material)
{
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
		if (p.material->blend())
		{
			glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		p.material->setUniforms(shader);
		p.mesh->draw();

		if (p.material->blend())
			glDisable(GL_BLEND);
	}
}

bool Renderable::useBlending()
{
	for (auto& p : primitives)
		if (p.material->blend())
			return true;
	return false;
}

void Renderable::print()
{
	//std::cout << "meshes: " << meshes.size() << std::endl;
}
