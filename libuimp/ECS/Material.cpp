#include "Material.h"

namespace Unity
{
	Material::Material(const std::string& name, const std::string& shaderFile, bool internalShader) :
		name(name),
		shaderFile(shaderFile),
		internalShader(internalShader)
	{

	}

	Material::~Material()
	{

	}
}