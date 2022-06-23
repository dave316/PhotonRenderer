#include "GLTFMaterial.h"

#include <IO/Image/ImageDecoder.h>
#include <base64/base64.h>

namespace IO
{
	namespace glTF
	{
		glm::mat3 getTexTransform(const json::Value& texTransNode)
		{
			glm::vec2 offset(0.0f);
			glm::vec2 scale(1.0f);
			float rotation = 0.0f;

			if (texTransNode.HasMember("offset"))
			{
				offset.x = texTransNode["offset"].GetArray()[0].GetFloat();
				offset.y = texTransNode["offset"].GetArray()[1].GetFloat();
			}
			if (texTransNode.HasMember("rotation"))
			{
				rotation = texTransNode["rotation"].GetFloat();
			}
			if (texTransNode.HasMember("scale"))
			{
				scale.x = texTransNode["scale"].GetArray()[0].GetFloat();
				scale.y = texTransNode["scale"].GetArray()[1].GetFloat();
			}

			glm::mat3 T(1.0f);
			T[2][0] = offset.x;
			T[2][1] = offset.y;

			glm::mat3 S(1.0f);
			S[0][0] = scale.x;
			S[1][1] = scale.y;

			glm::mat3 R(1.0f);
			R[0][0] = glm::cos(rotation);
			R[1][0] = glm::sin(rotation);
			R[0][1] = -glm::sin(rotation);
			R[1][1] = glm::cos(rotation);

			return T * R * S;
		}

		void setTextureInfo(ImageData& imageData, const json::Value& node, const std::string& texNodeName, Material::Ptr material, std::string texInfoStr, bool sRGB)
		{
			if (node.HasMember(texNodeName.c_str()))
			{
				auto& texNode = node[texNodeName.c_str()];
				unsigned int texIndex = texNode["index"].GetInt();
				auto tex = imageData.loadTexture(texIndex, sRGB);
				if (tex != nullptr)
				{
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);

					int texCoordIdx = 0;
					if (texNode.HasMember("texCoord"))
						texCoordIdx = texNode["texCoord"].GetInt();
					material->addProperty(texInfoStr + ".uvIndex", texCoordIdx);

					if (texNode.HasMember("extensions"))
					{
						auto& extNode = texNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
						{
							glm::vec2 offset(0.0f);
							glm::vec2 scale(1.0f);
							float rotation = 0.0f;
							auto& texTransformNode = extNode["KHR_texture_transform"];
							if (texTransformNode.HasMember("offset"))
							{
								offset.x = texTransformNode["offset"].GetArray()[0].GetFloat();
								offset.y = texTransformNode["offset"].GetArray()[1].GetFloat();
							}
							if (texTransformNode.HasMember("rotation"))
							{
								rotation = texTransformNode["rotation"].GetFloat();
							}
							if (texTransformNode.HasMember("scale"))
							{
								scale.x = texTransformNode["scale"].GetArray()[0].GetFloat();
								scale.y = texTransformNode["scale"].GetArray()[1].GetFloat();
							}

							tex->setOffset(offset);
							tex->setRotation(rotation);
							tex->setScale(scale);
						}
					}

					glm::mat3 texTransform = tex->getUVTransform();
					material->addProperty(texInfoStr + ".uvTransform", texTransform);
				}
				else
				{
					std::cout << "texture index " << texIndex << " not found" << std::endl;
				}
			}
			else
			{
				material->addProperty(texInfoStr + ".use", false);
			}
		}

		glm::vec4 getVec4FromNode(const json::Value& parentNode, const std::string& nodeName, glm::vec4 defaultValue = glm::vec4(1.0f))
		{
			glm::vec4 value = defaultValue;
			if (parentNode.HasMember(nodeName.c_str()))
			{
				const auto& node = parentNode[nodeName.c_str()];
				auto array = node.GetArray();
				value.r = array[0].GetFloat();
				value.g = array[1].GetFloat();
				value.b = array[2].GetFloat();
				value.a = array[3].GetFloat();
			}
			return value;
		}

		glm::vec3 getVec3FromNode(const json::Value& parentNode, const std::string& nodeName, glm::vec3 defaultValue = glm::vec3(1.0f))
		{
			glm::vec3 value = defaultValue;
			if (parentNode.HasMember(nodeName.c_str()))
			{
				const auto& baseColorNode = parentNode[nodeName.c_str()];
				auto array = baseColorNode.GetArray();
				value.r = array[0].GetFloat();
				value.g = array[1].GetFloat();
				value.b = array[2].GetFloat();
			}
			return value;
		}

		float getFloatFromNode(const json::Value& parentNode, const std::string& nodeName, float defaultValue = 1.0f)
		{
			float value = defaultValue;
			if (parentNode.HasMember(nodeName.c_str()))
				value = parentNode[nodeName.c_str()].GetFloat();
			return value;
		}

		Material::Ptr loadMaterial(ImageData& imageData, const json::Value& materialNode)
		{
			std::string shaderName = "Default";
			auto material = Material::create();
			//auto material = getDefaultMaterial();
			if (materialNode.HasMember("name"))
			{
				std::string name = materialNode["name"].GetString();
				//std::cout << "loading material " << name << std::endl;
			}
			if (materialNode.HasMember("doubleSided"))
			{
				bool doubleSided = materialNode["doubleSided"].GetBool();
				material->setDoubleSided(doubleSided);

				//std::cout << "material doubleSided: " << doubleSided << std::endl;
			}
			std::string alphaMode = "OPAQUE";
			int alphaModeEnum = 0;
			if (materialNode.HasMember("alphaMode"))
			{
				alphaMode = materialNode["alphaMode"].GetString();
				//std::cout << "alpha mode: " << alphaMode << std::endl;
			}

			float cutOff = 0.0f;
			if (alphaMode.compare("MASK") == 0)
			{
				alphaModeEnum = 1;
				if (materialNode.HasMember("alphaCutoff"))
				{
					cutOff = materialNode["alphaCutoff"].GetFloat();
					//std::cout << "alpha cut off: " << cutOff << std::endl;
				}
				else
				{
					cutOff = 0.5f;
				}
			}

			if (alphaMode.compare("BLEND") == 0)
			{
				alphaModeEnum = 2;
				material->setBlending(true);
			}

			material->addProperty("material.alphaMode", alphaModeEnum);
			material->addProperty("material.alphaCutOff", cutOff);

			if (materialNode.HasMember("pbrMetallicRoughness"))
			{
				auto& pbrNode = materialNode["pbrMetallicRoughness"];

				glm::vec4 baseColor = getVec4FromNode(pbrNode, "baseColorFactor");
				float roughnessFactor = getFloatFromNode(pbrNode, "roughnessFactor");
				float metallicFactor = getFloatFromNode(pbrNode, "metallicFactor");
				material->addProperty("material.baseColorFactor", baseColor);
				material->addProperty("material.roughnessFactor", roughnessFactor);
				material->addProperty("material.metallicFactor", metallicFactor);
				setTextureInfo(imageData, pbrNode, "baseColorTexture", material, "baseColorTex", true);

				//if (materialNode.HasMember("occlusionTexture") && pbrNode.HasMember("metallicRoughnessTexture"))
				//{
				//	int occTexIndex = materialNode["occlusionTexture"]["index"].GetInt();
				//	int pbrTexIndex = pbrNode["metallicRoughnessTexture"]["index"].GetInt();

				//	if (occTexIndex != pbrTexIndex)
				//	{
				//		if (textures[pbrTexIndex].isExternalFile)
				//		{
				//			Image2D<unsigned char> pbrImage(path + "/" + textures[pbrTexIndex].filename);
				//			Image2D<unsigned char> occImage(path + "/" + textures[occTexIndex].filename);
				//			unsigned char* rawData = occImage.getDataPtr();
				//			pbrImage.addChannel(0, 0, rawData, occImage.getChannels());

				//			Texture2D::Ptr pbrTex = pbrImage.upload(false);
				//			textures[pbrTexIndex].texture = pbrTex;
				//			textures[occTexIndex].texture = pbrTex;

				//			if (textures[pbrTexIndex].sampler.minFilter >= 9984 && textures[pbrTexIndex].sampler.minFilter <= 9987)
				//				textures[pbrTexIndex].texture->generateMipmaps();
				//		}
				//	}
				//}
				setTextureInfo(imageData, pbrNode, "metallicRoughnessTexture", material, "metalRoughTex", false);
			}

			if (materialNode.HasMember("normalTexture"))
			{
				auto& normalTexNode = materialNode["normalTexture"];
				unsigned int texIndex = normalTexNode["index"].GetInt();
				//auto tex = imageData.loadTexture(binData, texIndex, path, false);
				auto tex = imageData.loadTexture(texIndex, false);
				if (tex != nullptr)
				{
					//auto tex = encodeNormals(textures[texIndex], path);
					//auto tex = loadTexture(textures[texIndex], path, false);

					material->addTexture("normalTex", tex);
					material->addProperty("normalTex.use", true);

					int texCoordIdx = 0;
					if (normalTexNode.HasMember("texCoord"))
						texCoordIdx = normalTexNode["texCoord"].GetInt();
					material->addProperty("normalTex.uvIndex", texCoordIdx);

					float normalScale = 1.0f;
					if (normalTexNode.HasMember("scale"))
						normalScale = normalTexNode["scale"].GetFloat();
					material->addProperty("material.normalScale", normalScale);

					glm::mat3 texTransform = glm::mat3(1.0f);
					if (normalTexNode.HasMember("extensions"))
					{
						auto& extNode = normalTexNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
							texTransform = getTexTransform(extNode["KHR_texture_transform"]);
					}
					material->addProperty("normalTex.uvTransform", texTransform);
				}
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
			else
			{
				material->addProperty("normalTex.use", false);
			}

			if (materialNode.HasMember("occlusionTexture"))
			{
				auto& texNode = materialNode["occlusionTexture"];
				float occlusionStrength = getFloatFromNode(texNode, "strength");
				material->addProperty("material.occlusionStrength", occlusionStrength);
			}
			else
			{
				material->addProperty("material.occlusionStrength", 1.0f);
			}
			setTextureInfo(imageData, materialNode, "occlusionTexture", material, "occlusionTex", false);

			glm::vec3 emissiveFactor = getVec3FromNode(materialNode, "emissiveFactor", glm::vec3(0));
			material->addProperty("material.emissiveFactor", emissiveFactor);
			material->addProperty("material.emissiveStrength", 1.0f);
			setTextureInfo(imageData, materialNode, "emissiveTexture", material, "emissiveTex", true);

			material->addProperty("material.unlit", false);
			material->addProperty("material.ior", 1.5f);

			if (materialNode.HasMember("extensions"))
			{
				const auto& extensionNode = materialNode["extensions"];
				if (extensionNode.HasMember("KHR_materials_pbrSpecularGlossiness"))
				{
					const auto& pbrSpecGlossNode = extensionNode["KHR_materials_pbrSpecularGlossiness"];
					glm::vec4 diffuseFactor = getVec4FromNode(pbrSpecGlossNode, "diffuseFactor");
					glm::vec3 specularFactor = getVec3FromNode(pbrSpecGlossNode, "specularFactor");
					float glossFactor = getFloatFromNode(pbrSpecGlossNode, "glossinessFactor");
					material->addProperty("material.diffuseFactor", diffuseFactor);
					material->addProperty("material.specularFactor", specularFactor);
					material->addProperty("material.glossFactor", glossFactor);
					setTextureInfo(imageData, pbrSpecGlossNode, "diffuseTexture", material, "diffuseTex", true);
					setTextureInfo(imageData, pbrSpecGlossNode, "specularGlossinessTexture", material, "specGlossTex", true);

					shaderName += "_SPECGLOSS";
				}

				if (extensionNode.HasMember("KHR_materials_sheen"))
				{
					const auto& sheenNode = extensionNode["KHR_materials_sheen"];
					glm::vec3 sheenColor = getVec3FromNode(sheenNode, "sheenColorFactor", glm::vec3(0));
					float sheenRough = getFloatFromNode(sheenNode, "sheenRoughnessFactor", 0.0);
					material->addProperty("material.sheenColorFactor", sheenColor);
					material->addProperty("material.sheenRoughnessFactor", sheenRough);
					setTextureInfo(imageData, sheenNode, "sheenColorTexture", material, "sheenColortex", true);
					setTextureInfo(imageData, sheenNode, "sheenRoughnessTexture", material, "sheenRoughtex", false);

					shaderName += "_SHEEN";
				}

				if (extensionNode.HasMember("KHR_materials_clearcoat"))
				{
					const auto& clearcoatNode = extensionNode["KHR_materials_clearcoat"];
					float clearcoat = getFloatFromNode(clearcoatNode, "clearcoatFactor", 0.0f);
					float clearcoatRough = getFloatFromNode(clearcoatNode, "clearcoatRoughnessFactor", 0.0f);
					material->addProperty("material.clearcoatFactor", clearcoat);
					material->addProperty("material.clearcoatRoughnessFactor", clearcoatRough);
					setTextureInfo(imageData, clearcoatNode, "clearcoatTexture", material, "clearCoatTex", false);
					setTextureInfo(imageData, clearcoatNode, "clearcoatRoughnessTexture", material, "clearCoatRoughTex", false);
					setTextureInfo(imageData, clearcoatNode, "clearcoatNormalTexture", material, "clearCoatNormalTex", false);

					shaderName += "_CLEARCOAT";
				}

				if (extensionNode.HasMember("KHR_materials_transmission"))
				{
					const auto& transmissionNode = extensionNode["KHR_materials_transmission"];
					float transmissionFactor = getFloatFromNode(transmissionNode, "transmissionFactor", 0.0f);
					if (transmissionFactor > 0.0f)
					{
						material->addProperty("material.transmissionFactor", transmissionFactor);
						material->setTransmissive(true);
						setTextureInfo(imageData, transmissionNode, "transmissionTexture", material, "transmissionTex", false);
						shaderName += "_TRANSMISSION";

						if (extensionNode.HasMember("KHR_materials_volume"))
						{
							const auto& volumeNode = extensionNode["KHR_materials_volume"];
							float thicknessFactor = getFloatFromNode(volumeNode, "thicknessFactor", 0.0f);
							float attenuationDistance = getFloatFromNode(volumeNode, "attenuationDistance", 0.0f);
							glm::vec3 attenuationColor = getVec3FromNode(volumeNode, "attenuationColor");
							material->addProperty("material.thicknessFactor", thicknessFactor);
							material->addProperty("material.attenuationDistance", attenuationDistance);
							material->addProperty("material.attenuationColor", attenuationColor);
							setTextureInfo(imageData, volumeNode, "thicknessTexture", material, "thicknessTex", false);
						}
						else
						{
							material->addProperty("material.thicknessFactor", 0.0f);
						}
					}
				}

				if (extensionNode.HasMember("KHR_materials_diffuse_transmission"))
				{
					const auto& transmissionNode = extensionNode["KHR_materials_diffuse_transmission"];
					float transmissionFactor = getFloatFromNode(transmissionNode, "diffuseTransmissionFactor", 0.0f);
					glm::vec3 transmissionColorFactor = getVec3FromNode(transmissionNode, "diffuseTransmissionColorFactor");
					material->addProperty("material.translucencyFactor", transmissionFactor);
					material->addProperty("material.translucencyColorFactor", transmissionColorFactor);
					shaderName += "_TRANSLUCENCY";
				}

				if (extensionNode.HasMember("KHR_materials_ior"))
				{
					const auto& iorNode = extensionNode["KHR_materials_ior"];
					material->addProperty("material.ior", getFloatFromNode(iorNode, "ior", 1.5f));
				}

				if (extensionNode.HasMember("KHR_materials_specular"))
				{
					const auto& specularNode = extensionNode["KHR_materials_specular"];
					float specularFactor = getFloatFromNode(specularNode, "specularFactor");
					glm::vec3 specularColor = getVec3FromNode(specularNode, "specularColorFactor");
					material->addProperty("material.specularFactor", specularFactor);
					material->addProperty("material.specularColorFactor", specularColor);
					setTextureInfo(imageData, specularNode, "specularTexture", material, "specularTex", false);
					setTextureInfo(imageData, specularNode, "specularColorTexture", material, "specularColorTex", true);

					shaderName += "_SPECULAR";
				}

				if (extensionNode.HasMember("KHR_materials_iridescence"))
				{
					const auto& iridescenceNode = extensionNode["KHR_materials_iridescence"];
					float iridescenceFactor = getFloatFromNode(iridescenceNode, "iridescenceFactor", 0.0f);
					float iridescenceIOR = getFloatFromNode(iridescenceNode, "iridescenceIor", 1.8f);
					float iridescenceThicknessMin = getFloatFromNode(iridescenceNode, "iridescenceThicknessMinimum", 100.0f);
					float iridescenceThicknessMax = getFloatFromNode(iridescenceNode, "iridescenceThicknessMaximum", 400.0f);
					material->addProperty("material.iridescenceFactor", iridescenceFactor);
					material->addProperty("material.iridescenceIor", iridescenceIOR);
					material->addProperty("material.iridescenceThicknessMin", iridescenceThicknessMin);
					material->addProperty("material.iridescenceThicknessMax", iridescenceThicknessMax);
					setTextureInfo(imageData, iridescenceNode, "iridescenceThicknessTexture", material, "iridescenceThicknessTex", false);

					shaderName += "_IRIDESCENCE";
				}

				if (extensionNode.HasMember("KHR_materials_anisotropy"))
				{
					const auto& anisotropyNode = extensionNode["KHR_materials_anisotropy"];
					float anisotropy = getFloatFromNode(anisotropyNode, "anisotropy", 0.0f);
					glm::vec3 anisotropyDirection = getVec3FromNode(anisotropyNode, "anisotropyDirection", glm::vec3(1.0f, 0.0f, 0.0f));
					material->addProperty("material.anisotropyFactor", anisotropy);
					material->addProperty("material.anisotropyDirection", anisotropyDirection);
					setTextureInfo(imageData, anisotropyNode, "anisotropyTexture", material, "anisotropyTex", false);
					setTextureInfo(imageData, anisotropyNode, "anisotropyDirectionTexture", material, "anisotropyDirectionTex", false);

					shaderName += "_ANISOTROPY";
				}

				if (extensionNode.HasMember("KHR_materials_unlit"))
				{
					material->addProperty("material.unlit", true);
				}
				else
				{
					material->addProperty("material.unlit", false);
				}

				if (extensionNode.HasMember("KHR_materials_emissive_strength"))
				{
					const auto& emissiveNode = extensionNode["KHR_materials_emissive_strength"];
					float emissiveStrength = getFloatFromNode(emissiveNode, "emissiveStrength");
					material->addProperty("material.emissiveStrength", emissiveStrength);
				}
			}

			//if(material->getUsedTexUnits() > 2)
			//	std::cout << "material needs " << material->getUsedTexUnits() << " tex units" << std::endl;
			//if (shaderName.length() > 7)
			//	std::cout << "shader variant: " << shaderName << std::endl;
			material->setShader(shaderName);
			return material;
		}
	}
}
