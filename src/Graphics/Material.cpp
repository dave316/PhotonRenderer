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

Material::Ptr getDefaultMaterial()
{
	auto defaultMaterial = Material::create();

	// PBR MetallicRough parameters
	defaultMaterial->addProperty("material.baseColorFactor", glm::vec4(1.0));
	defaultMaterial->addProperty("material.roughnessFactor", 1.0f);
	defaultMaterial->addProperty("material.metallicFactor", 1.0f);
	defaultMaterial->addProperty("material.occlusionFactor", 0.0f);
	defaultMaterial->addProperty("material.emissiveFactor", glm::vec3(0.0));
	defaultMaterial->addProperty("material.alphaCutOff", 0.0f);
	defaultMaterial->addProperty("material.alphaMode", 0);
	defaultMaterial->addProperty("material.useBaseColorTex", false);
	defaultMaterial->addProperty("material.usePbrTex", false);
	defaultMaterial->addProperty("material.useNormalTex", false);
	defaultMaterial->addProperty("material.useEmissiveTex", false);
	defaultMaterial->addProperty("material.useOcclusionTex", false);

	// Sheen
	defaultMaterial->addProperty("material.sheenColorFactor", glm::vec3(0));
	defaultMaterial->addProperty("material.useSheenColorTex", false);
	defaultMaterial->addProperty("material.sheenRoughnessFactor", 0.0f);
	defaultMaterial->addProperty("material.useSheenRoughTex", false);

	// Clearcoat
	defaultMaterial->addProperty("material.clearcoatFactor", 0.0f);
	defaultMaterial->addProperty("material.useClearCoatTex", false);
	defaultMaterial->addProperty("material.clearcoatRoughnessFactor", 0.0f);
	defaultMaterial->addProperty("material.useClearCoatRoughTex", false);
	defaultMaterial->addProperty("material.useClearCoatNormalTex", false);

	// Transmission
	defaultMaterial->addProperty("material.transmissionFactor", 0.0f);
	defaultMaterial->addProperty("material.useTransmissionTex", false);

	// Volume
	defaultMaterial->addProperty("material.thicknessFactor", 0.0f);
	defaultMaterial->addProperty("material.useThicknessTex", false);
	defaultMaterial->addProperty("material.attenuationDistance", 0.0f);
	defaultMaterial->addProperty("material.attenuationColor", glm::vec3(1.0f));

	// IOR
	defaultMaterial->addProperty("material.ior", 1.5f);

	// Specular
	defaultMaterial->addProperty("material.specularFactor", 1.0f);
	defaultMaterial->addProperty("material.useSpecularTex", false);
	defaultMaterial->addProperty("material.specularColorFactor", glm::vec3(1.0f));
	defaultMaterial->addProperty("material.useSpecularColorTex", false);

	// Iridescence
	defaultMaterial->addProperty("material.iridescenceFactor", 0.0f);
	defaultMaterial->addProperty("material.useIridescenceTex", false);
	defaultMaterial->addProperty("material.iridescenceIOR", 1.8f);
	defaultMaterial->addProperty("material.iridescenceThicknessMin", 400.0f);
	defaultMaterial->addProperty("material.iridescenceThicknessMax", 1200.0f);
	defaultMaterial->addProperty("material.useIridescenceThicknessTex", false);

	// Anisotropy
	defaultMaterial->addProperty("material.anisotropyFactor", 0.0f);
	defaultMaterial->addProperty("material.useAnisotropyTexture", false);
	defaultMaterial->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
	defaultMaterial->addProperty("material.useAnisotropyDirectionTexture", false);

	// Unlit
	defaultMaterial->addProperty("material.unlit", false);

	defaultMaterial->addProperty("material.computeFlatNormals", false);
	defaultMaterial->addProperty("useSpecGlossMat", false);

	return defaultMaterial;
}