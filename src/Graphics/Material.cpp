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

void resetTexInfo(Material::Ptr material, std::string name)
{
	material->addProperty(name + ".use", false);
	material->addProperty(name + ".uvIndex", 0);
	material->addProperty(name + ".uvTransform", glm::mat3(1.0f));
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
	resetTexInfo(defaultMaterial, "baseColorTex");
	resetTexInfo(defaultMaterial, "pbrTex");
	resetTexInfo(defaultMaterial, "normalTex");
	resetTexInfo(defaultMaterial, "emissiveTex");
	resetTexInfo(defaultMaterial, "occlusionTex");

	// Sheen
	defaultMaterial->addProperty("material.sheenColorFactor", glm::vec3(0));
	defaultMaterial->addProperty("material.sheenRoughnessFactor", 0.0f);
	resetTexInfo(defaultMaterial, "sheenColorTex");
	resetTexInfo(defaultMaterial, "sheenRoughTex");

	// Clearcoat
	defaultMaterial->addProperty("material.clearcoatFactor", 0.0f);
	defaultMaterial->addProperty("material.clearcoatRoughnessFactor", 0.0f);
	resetTexInfo(defaultMaterial, "clearCoatTex");
	resetTexInfo(defaultMaterial, "clearCoatRoughTex");
	resetTexInfo(defaultMaterial, "clearCoatNormalTex");

	// Transmission
	defaultMaterial->addProperty("material.transmissionFactor", 0.0f);
	resetTexInfo(defaultMaterial, "transmissionTex");

	// Volume
	defaultMaterial->addProperty("material.thicknessFactor", 0.0f);
	defaultMaterial->addProperty("material.attenuationDistance", 0.0f);
	defaultMaterial->addProperty("material.attenuationColor", glm::vec3(1.0f));
	resetTexInfo(defaultMaterial, "thicknessTex");

	// IOR
	defaultMaterial->addProperty("material.ior", 1.5f);

	// Specular
	defaultMaterial->addProperty("material.specularFactor", 1.0f);
	defaultMaterial->addProperty("material.specularColorFactor", glm::vec3(1.0f));
	resetTexInfo(defaultMaterial, "specularTex");
	resetTexInfo(defaultMaterial, "specularColorTex");

	// Iridescence
	defaultMaterial->addProperty("material.iridescenceFactor", 0.0f);
	defaultMaterial->addProperty("material.iridescenceIOR", 1.8f);
	defaultMaterial->addProperty("material.iridescenceThicknessMin", 400.0f);
	defaultMaterial->addProperty("material.iridescenceThicknessMax", 1200.0f);
	resetTexInfo(defaultMaterial, "iridescenceTex");
	resetTexInfo(defaultMaterial, "iridescenceThicknessTex");

	// Anisotropy
	defaultMaterial->addProperty("material.anisotropyFactor", 0.0f);
	defaultMaterial->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
	resetTexInfo(defaultMaterial, "anisotropyTex");
	resetTexInfo(defaultMaterial, "anisotropyDirectionTex");

	// Unlit
	defaultMaterial->addProperty("material.unlit", false);

	defaultMaterial->addProperty("material.computeFlatNormals", false);
	defaultMaterial->addProperty("useSpecGlossMat", false);

	return defaultMaterial;
}