#include "Renderable.h"

#include <iostream>

Renderable::Renderable(Mesh::Ptr mesh, Material::Ptr material) : 
	mesh(mesh),
	material(material)
{

}

Renderable::~Renderable()
{
	//std::cout << "renderable: " << mesh->getName() << " destroyed" << std::endl;
}

void Renderable::setAnimation(Animation::Ptr anim)
{
	this->animation = anim;
}

void Renderable::update(float dt)
{
	if (animation != nullptr)
		animation->update(dt);
}

void Renderable::render(GL::Program& program)
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

	material->setUniforms(program);
	mesh->draw();
}

void Renderable::print()
{
	//std::cout << "meshes: " << meshes.size() << std::endl;
}

bool Renderable::hasAnimations()
{
	return (animation != nullptr);
}

glm::mat4 Renderable::getTransform()
{
	if (animation != nullptr)
	{
		return animation->getTransform();
	}
		
	return glm::mat4();
}
