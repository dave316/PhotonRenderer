#include "Material.h"

Material::Material()
{

}

void Material::setUniforms(Shader::Ptr shader)
{
	int numTextures = 0;
	for (auto& prop : properties)
	{
		if (prop.second->isTexture())
		{
			textures[numTextures]->use(numTextures);
			numTextures++;
		}

		prop.second->setUniform(shader);
	}
}