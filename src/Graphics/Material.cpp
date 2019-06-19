#include "Material.h"

Material::Material()
{

}

Material::~Material()
{
	//std::cout << "material destroyed" << std::endl;
}

void Material::setColor(glm::vec3& color)
{
	this->color = color;
}

void Material::addTexture(Texture2D::Ptr texture)
{
	textures.push_back(texture);
}

void Material::setUniforms(GL::Program& program)
{
	if (textures.size() == 1)
	{
		program.setUniform("useTexture", true);
		textures[0]->use(0);
	}		
	else
	{
		program.setUniform("useTexture", false);
		program.setUniform("material.diffuseColor", color);
	}		
}