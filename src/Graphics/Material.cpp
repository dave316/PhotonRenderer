#include "Material.h"

Material::Material()
{

}

//Material::~Material()
//{
//	//std::cout << "material destroyed" << std::endl;
//}

//void Material::setColor(glm::vec4& color)
//{
//	this->color = color;
//}
//
//void Material::addTexture(Texture2D::Ptr texture)
//{
//	textures.push_back(texture);
//}

void Material::setUniforms(GL::Program& program)
{
	int numTextures = 0;
	for (auto& prop : properties)
	{
		if (prop.second->isTexture())
		{
			textures[numTextures]->use(numTextures);
			numTextures++;
		}

		prop.second->setUniform(program);
	}


/*	if (textures.size() == 1)
	{
		program.setUniform("material.useBaseColorTex", true);
		textures[0]->use(0);
	}
	else if (textures.size() == 2)
	{
		program.setUniform("material.useBaseColorTex", true);
		textures[0]->use(0);

		program.setUniform("material.useNormalTex", true);
		textures[1]->use(1);
	}
	else if (textures.size() == 3)
	{
		program.setUniform("material.useBaseColorTex", true);
		textures[0]->use(0);

		program.setUniform("material.useNormalTex", true);
		textures[1]->use(1);

		program.setUniform("material.usePbrTex", true);
		textures[2]->use(2);
	}
	else if (textures.size() == 4)
	{
		program.setUniform("material.useBaseColorTex", true);
		textures[0]->use(0);

		program.setUniform("material.useNormalTex", true);
		textures[2]->use(2);

		program.setUniform("material.usePbrTex", true);
		textures[1]->use(1);

		program.setUniform("material.useEmissiveTex", true);
		textures[3]->use(3);
	}
	else
	{
		program.setUniform("material.useBaseColorTex", false);
		program.setUniform("material.useNormalTex", false);
		program.setUniform("material.usePbrTex", false);
		program.setUniform("material.useEmissiveTex", false);
		program.setUniform("material.baseColorFactor", color);
	}	*/	
}