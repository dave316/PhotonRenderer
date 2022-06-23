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
			//std::cout << "activate tex unit " << numTextures << std::endl;
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
	// This is super slow! Don't set everything if it is not needed...

	auto defaultMaterial = Material::create();
	defaultMaterial->setShader("Default");

	// PBR MetallicRough parameters
	defaultMaterial->addProperty("material.baseColorFactor", glm::vec4(1.0));
	defaultMaterial->addProperty("material.roughnessFactor", 1.0f);
	defaultMaterial->addProperty("material.metallicFactor", 1.0f);
	defaultMaterial->addProperty("material.occlusionStrength", 1.0f);
	defaultMaterial->addProperty("material.emissiveFactor", glm::vec3(0.0));
	defaultMaterial->addProperty("material.alphaCutOff", 0.0f);
	defaultMaterial->addProperty("material.alphaMode", 0);
	defaultMaterial->addProperty("baseColorTex.use", false);
	defaultMaterial->addProperty("metalRoughTex.use", false);
	defaultMaterial->addProperty("normalTex.use", false);
	defaultMaterial->addProperty("emissiveTex.use", false);
	defaultMaterial->addProperty("occlusionTex.use", false);

	//// Sheen
	//defaultMaterial->addProperty("material.sheenColorFactor", glm::vec3(0));
	//defaultMaterial->addProperty("material.sheenRoughnessFactor", 0.0f);
	//defaultMaterial->addProperty("sheenColorTex.use", false);
	//defaultMaterial->addProperty("sheenRoughTex.use", false);

	//// Clearcoat
	//defaultMaterial->addProperty("material.clearcoatFactor", 0.0f);
	//defaultMaterial->addProperty("material.clearcoatRoughnessFactor", 0.0f);
	//defaultMaterial->addProperty("clearCoatTex.use", false);
	//defaultMaterial->addProperty("clearCoatRoughTex.use", false);
	//defaultMaterial->addProperty("clearCoatNormalTex.use", false);

	//// Transmission
	//defaultMaterial->addProperty("material.transmissionFactor", 0.0f);
	//defaultMaterial->addProperty("transmissionTex.use", false);

	//// Volume
	//defaultMaterial->addProperty("material.thicknessFactor", 0.0f);
	//defaultMaterial->addProperty("material.attenuationDistance", 0.0f);
	//defaultMaterial->addProperty("material.attenuationColor", glm::vec3(1.0f));
	//defaultMaterial->addProperty("thicknessTex.use", false);

	// IOR
	defaultMaterial->addProperty("material.ior", 1.5f);

	//// Specular
	//defaultMaterial->addProperty("material.specularFactor", 1.0f);
	//defaultMaterial->addProperty("material.specularColorFactor", glm::vec3(1.0f));
	//defaultMaterial->addProperty("specularTex.use", false);
	//defaultMaterial->addProperty("specularColorTex.use", false);

	//// Iridescence
	//defaultMaterial->addProperty("material.iridescenceFactor", 0.0f);
	//defaultMaterial->addProperty("material.iridescenceIOR", 1.8f);
	//defaultMaterial->addProperty("material.iridescenceThicknessMin", 400.0f);
	//defaultMaterial->addProperty("material.iridescenceThicknessMax", 1200.0f);
	//defaultMaterial->addProperty("iridescenceTex.use", false);
	//defaultMaterial->addProperty("iridescenceThicknessTex.use", false);

	//// Anisotropy
	//defaultMaterial->addProperty("material.anisotropyFactor", 0.0f);
	//defaultMaterial->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
	//defaultMaterial->addProperty("anisotropyTex.use", false);
	//defaultMaterial->addProperty("anisotropyDirectionTex.use", false);

	// Unlit
	defaultMaterial->addProperty("material.unlit", false);

	//defaultMaterial->addProperty("material.computeFlatNormals", false);
	defaultMaterial->addProperty("useSpecGlossMat", false);

	return defaultMaterial;
}