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

void Renderable::setMorphAnim(MorphAnimation::Ptr anim)
{
	this->morphAnim = anim;
}

void Renderable::update(float dt)
{
	if (animation != nullptr)
		animation->update(dt);

	if (morphAnim != nullptr)
		morphAnim->update(dt);
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

bool Renderable::hasMorphAnim()
{
	return (morphAnim != nullptr);
}

glm::vec2 Renderable::getWeights()
{
	if (morphAnim != nullptr)
		return morphAnim->getWeights();
	return glm::vec2();
}

glm::mat4 Renderable::getTransform()
{
	if (animation != nullptr)
		return animation->getTransform();
	   		
	return glm::mat4();
}
