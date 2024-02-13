#include "UnityMaterials.h"

#include <IO/ShaderLoader.h>
#include <IO/Image/ImageDecoder.h>
#include <Utils/Color.h>
#include <fstream>

namespace IO
{
	namespace Unity
	{
		Material::Ptr Materials::loadDefaultMaterial(UnityMaterial& unityMaterial)
		{
			auto material = getDefaultMaterial();
			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			int alphaMode = 0;
			float alphaCutOff = 0.5f;
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 1.0f)
				{
					if (floatMap.find("_Cutoff") != floatMap.end())
						alphaCutOff = floatMap["_Cutoff"];

					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", alphaCutOff);
				}
				else if (mode == 2.0f || mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
			}

			if (floatMap.find("_BumpScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_BumpScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float metallic = 0.0f;
			float occlusionStrength = 1.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];
			//if (floatMap.find("_Smoothness") != floatMap.end())
			//	glossiness = floatMap["_Smoothness"];
			if (floatMap.find("_Metallic") != floatMap.end())
				metallic = floatMap["_Metallic"];
			if (floatMap.find("_OcclusionStrength") != floatMap.end())
				occlusionStrength = floatMap["_OcclusionStrength"];

			glm::mat3 texTransform = glm::mat3(1.0f);
			if (texMap.find("_MainTex") != texMap.end())
			{
				glm::vec2 offset = texMap["_MainTex"].offset;
				glm::vec2 scale = texMap["_MainTex"].scale;
				glm::mat3 T(1.0f);
				T[2][0] = offset.x;
				T[2][1] = offset.y;

				glm::mat3 S(1.0f);
				S[0][0] = scale.x;
				S[1][1] = scale.y;

				texTransform = T * S;
				loadTexture(material, "baseColorTex", texMap["_MainTex"], false);
				material->addProperty("baseColorTex.uvTransform", texTransform);
			}

			if (!material->getPropertyValue<bool>("baseColorTex.use"))
			{
				if (texMap.find("_Albedo") != texMap.end())
				{
					glm::vec2 offset = texMap["_Albedo"].offset;
					glm::vec2 scale = texMap["_Albedo"].scale;
					glm::mat3 T(1.0f);
					T[2][0] = offset.x;
					T[2][1] = offset.y;

					glm::mat3 S(1.0f);
					S[0][0] = scale.x;
					S[1][1] = scale.y;

					texTransform = T * S;
					loadTexture(material, "baseColorTex", texMap["_Albedo"], false);
					material->addProperty("baseColorTex.uvTransform", texTransform);
				}
			}

			if (texMap.find("_EmissionMap") != texMap.end())
			{
				loadTexture(material, "emissiveTex", texMap["_EmissionMap"], false);
				material->addProperty("emissiveTex.uvTransform", texTransform);
			}

			if (texMap.find("_BumpMap") != texMap.end())
			{
				loadTexture(material, "normalTex", texMap["_BumpMap"], false);
				material->addProperty("normalTex.uvTransform", texTransform);
			}

			if (!material->getPropertyValue<bool>("normalTex.use"))
			{
				if (texMap.find("_Normal") != texMap.end())
				{
					loadTexture(material, "normalTex", texMap["_Normal"], false);
					material->addProperty("normalTex.uvTransform", texTransform);
				}
			}

			if (texMap.find("_MetallicGlossMap") != texMap.end())
			{
				std::string occGUID;
				if (texMap.find("_OcclusionMap") != texMap.end())
					occGUID = texMap["_OcclusionMap"].texture.guid;
				if (texMap.find("_AmbientOcclusion") != texMap.end())
					occGUID = texMap["_AmbientOcclusion"].texture.guid;

				auto guid = texMap["_MetallicGlossMap"].texture.guid;
				auto tex = loadMetalRoughTex(guid, occGUID, glossMapScale);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);

					std::string texInfoStr = "metalRoughTex";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", texTransform);

					if (!occGUID.empty())
					{
						std::string texInfoStr = "occlusionTex";
						material->addTexture(texInfoStr, tex);
						material->addProperty(texInfoStr + ".use", true);
						material->addProperty(texInfoStr + ".uvIndex", 0);
						material->addProperty(texInfoStr + ".uvTransform", texTransform);
						material->addProperty("material.occlusionStrength", occlusionStrength);
					}

					material->addProperty("material.roughnessFactor", 1.0f);
					material->addProperty("material.metallicFactor", 1.0f);
				}
				else
				{
					material->addProperty("material.roughnessFactor", 1.0f - glossiness);
					material->addProperty("material.metallicFactor", metallic);
				}
			}
			else
			{
				material->addProperty("material.roughnessFactor", 1.0f - glossiness);
				material->addProperty("material.metallicFactor", metallic);
			}

			if (texMap.find("_ParallaxMap") != texMap.end())
			{
				material->setShader("Default_POM");
				loadTexture(material, "heightTex", texMap["_ParallaxMap"], false);
				if (floatMap.find("_Parallax") != floatMap.end())
					material->addProperty("material.scale", floatMap["_Parallax"]);
				material->addProperty("material.refPlane", 0.0f);
				material->addProperty("material.curvFix", 1.0f);
				material->addProperty("material.curvatureU", 0.0f);
				material->addProperty("material.curvatureV", 0.0f);
				material->addProperty("heightTex.uvTransform", texTransform);
			}

			if (colMap.find("_Color") != colMap.end())
			{
				glm::vec4 rgba = Color::sRGBAlphaToLinear(colMap["_Color"]);
				if (texMap.find("_MainTex") != texMap.end())
					rgba.a = 1.0f;
				material->addProperty("material.baseColorFactor", rgba);
			}

			if (material->getPropertyValue<bool>("emissiveTex.use") &&
				colMap.find("_EmissionColor") != colMap.end())
			{
				glm::vec4 emissionColor = Color::sRGBAlphaToLinear(colMap["_EmissionColor"]);
				material->addProperty("material.emissiveFactor", glm::vec3(emissionColor));
				material->addProperty("material.emissiveStrength", 1.0f);
			}

			return material;
		}

		Material::Ptr Materials::loadSpecGlossMaterial(UnityMaterial& unityMaterial)
		{
			auto material = Material::create();
			material->setShader("Default_SPECGLOSS");

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			int alphaMode = 0;
			float alphaCutOff = 0.5f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 1.0f)
				{
					if (floatMap.find("_Cutoff") != floatMap.end())
						alphaCutOff = floatMap["_Cutoff"];

					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", alphaCutOff);
				}
				else if (mode == 2.0f || mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
				else
				{
					material->addProperty("material.alphaMode", 0);
				}
			}

			if (floatMap.find("_BumpScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_BumpScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float occlusionStrength = 1.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];
			if (floatMap.find("_OcclusionStrength") != floatMap.end())
				occlusionStrength = floatMap["_OcclusionStrength"];

			material->addProperty("material.occlusionStrength", occlusionStrength);

			glm::mat3 texTransform = glm::mat3(1.0f);
			if (texMap.find("_MainTex") != texMap.end())
			{
				glm::vec2 offset = texMap["_MainTex"].offset;
				glm::vec2 scale = texMap["_MainTex"].scale;
				glm::mat3 T(1.0f);
				T[2][0] = offset.x;
				T[2][1] = offset.y;

				glm::mat3 S(1.0f);
				S[0][0] = scale.x;
				S[1][1] = scale.y;

				texTransform = T * S;
				loadTexture(material, "diffuseTex", texMap["_MainTex"], false);
				material->addProperty("diffuseTex.uvTransform", texTransform);
			}
			else
			{
				material->addProperty("diffuseTex.use", false);
			}

			if (!material->getPropertyValue<bool>("diffuseTex.use"))
			{
				if (texMap.find("_Albedo") != texMap.end())
				{
					glm::vec2 offset = texMap["_Albedo"].offset;
					glm::vec2 scale = texMap["_Albedo"].scale;
					glm::mat3 T(1.0f);
					T[2][0] = offset.x;
					T[2][1] = offset.y;

					glm::mat3 S(1.0f);
					S[0][0] = scale.x;
					S[1][1] = scale.y;

					texTransform = T * S;
					loadTexture(material, "diffuseTex", texMap["_Albedo"], false);
					material->addProperty("diffuseTex.uvTransform", texTransform);
				}
				else
				{
					material->addProperty("diffuseTex.use", false);
				}
			}

			if (texMap.find("_EmissionMap") != texMap.end())
			{
				loadTexture(material, "emissiveTex", texMap["_EmissionMap"], false);
				material->addProperty("emissiveTex.uvTransform", texTransform);
			}
			else
			{
				material->addProperty("emissiveTex.use", false);
			}

			if (texMap.find("_BumpMap") != texMap.end())
			{
				loadTexture(material, "normalTex", texMap["_BumpMap"], false);
				material->addProperty("normalTex.uvTransform", texTransform);
			}
			else
			{
				material->addProperty("normalTex.use", false);
			}

			if (!material->getPropertyValue<bool>("normalTex.use"))
			{
				if (texMap.find("_Normal") != texMap.end())
				{
					loadTexture(material, "normalTex", texMap["_Normal"], false);
					material->addProperty("normalTex.uvTransform", texTransform);
				}
				else
				{
					material->addProperty("normalTex.use", false);
				}
			}

			if (texMap.find("_SpecGlossMap") != texMap.end())
			{
				loadTexture(material, "specGlossTex", texMap["_SpecGlossMap"], false);
				material->addProperty("specGlossTex.uvTransform", texTransform);
				material->addProperty("material.glossFactor", glossMapScale);
			}
			else
			{
				material->addProperty("specGlossTex.use", false);
				material->addProperty("material.glossFactor", glossiness);
			}

			if (texMap.find("_OcclusionMap") != texMap.end())
			{
				loadTexture(material, "occlusionTex", texMap["_OcclusionMap"], false);
				material->addProperty("occlusionTex.uvTransform", texTransform);
			}
			else
			{
				material->addProperty("occlusionTex.use", false);
			}

			if (colMap.find("_Color") != colMap.end())
			{
				glm::vec4 rgba = Color::sRGBAlphaToLinear(colMap["_Color"]);
				if (texMap.find("_MainTex") != texMap.end())
					rgba.a = 1.0f;
				material->addProperty("material.diffuseFactor", rgba);
			}

			if (material->getPropertyValue<bool>("emissiveTex.use"))
			{
				material->addProperty("material.emissiveFactor", glm::vec3(1.0f));
				material->addProperty("material.emissiveStrength", 1.0f);
			}
			else
			{
				material->addProperty("material.emissiveFactor", glm::vec3(0.0f));
				material->addProperty("material.emissiveStrength", 1.0f);
			}

			if (colMap.find("_SpecColor") != colMap.end() && texMap.find("_SpecGlossMap") == texMap.end())
			{
				glm::vec4 specularColor = Color::sRGBAlphaToLinear(colMap["_SpecColor"]);
				material->addProperty("material.specularFactor", glm::vec3(specularColor));
			}
			else
			{
				material->addProperty("material.specularFactor", glm::vec3(1.0f));
			}

			return material;
		}

		void Materials::loadTexture(Material::Ptr material, std::string texPrefix, TexEnv& texInfo, bool useTransform)
		{
			std::string guid = texInfo.texture.guid;
			Texture2D::Ptr tex = loadTextureGUID(guid);
			if (tex)
			{
				//tex->generateMipmaps(); // TODO: check if mipmaps already exist
				tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
				tex->setWrap(GL::REPEAT, GL::REPEAT);

				glm::mat3 texTransform = glm::mat3(1.0f);
				if (useTransform)
				{
					tex->setOffset(texInfo.offset);
					tex->setScale(texInfo.scale);
					texTransform = tex->getUVTransform();
				}

				material->addTexture(texPrefix, tex);
				material->addProperty(texPrefix + ".use", true);
				material->addProperty(texPrefix + ".uvIndex", 0);
				material->addProperty(texPrefix + ".uvTransform", texTransform);
			}
			else
			{
				material->addProperty(texPrefix + ".use", false);
			}
		}

		Material::Ptr Materials::loadLayerMaterial(UnityMaterial& unityMaterial, bool use3Layers)
		{
			auto material = Material::create();
			material->setShader("Default_LAYERED_MATERIAL");
			material->addProperty("material.use3Layers", use3Layers);

			auto floatMap = unityMaterial.floats;
			float smoothness1 = 1.0f;
			float smoothness2 = 1.0f;
			float smoothness3 = 1.0f;
			if (floatMap.find("_Smoothness1") != floatMap.end())
				smoothness1 = floatMap["_Smoothness1"];
			if (floatMap.find("_Smoothness2") != floatMap.end())
				smoothness2 = floatMap["_Smoothness2"];
			if (floatMap.find("_Metallic1_Power") != floatMap.end())
				material->addProperty("material.metallicFactor1", floatMap["_Metallic1_Power"]);
			if (floatMap.find("_Metallic2_Power") != floatMap.end())
				material->addProperty("material.metallicFactor2", floatMap["_Metallic2_Power"]);
			if (floatMap.find("_Tile1") != floatMap.end())
				material->addProperty("material.tile1", floatMap["_Tile1"]);
			if (floatMap.find("_Tile2") != floatMap.end())
				material->addProperty("material.tile2", floatMap["_Tile2"]);

			if (floatMap.find("_Color2Offset") != floatMap.end())
				material->addProperty("material.colorOffset2", floatMap["_Color2Offset"]);
			if (floatMap.find("_Color2Scale") != floatMap.end())
				material->addProperty("material.colorScale2", floatMap["_Color2Scale"]);
			if (floatMap.find("_Height2Offset") != floatMap.end())
				material->addProperty("material.heightOffset2", floatMap["_Height2Offset"]);
			if (floatMap.find("_Height2Scale") != floatMap.end())
				material->addProperty("material.heightScale2", floatMap["_Height2Scale"]);

			if (use3Layers)
			{
				if (floatMap.find("_Smoothness3") != floatMap.end())
					smoothness3 = floatMap["_Smoothness3"];
				if (floatMap.find("_Metallic3_Power") != floatMap.end())
					material->addProperty("material.metallicFactor3", floatMap["_Metallic3_Power"]);
				if (floatMap.find("_Tile3") != floatMap.end())
					material->addProperty("material.tile3", floatMap["_Tile3"]);

				if (floatMap.find("_Color3Offset") != floatMap.end())
					material->addProperty("material.colorOffset3", floatMap["_Color3Offset"]);
				if (floatMap.find("_Color3Scale") != floatMap.end())
					material->addProperty("material.colorScale3", floatMap["_Color3Scale"]);
				if (floatMap.find("_Height3Offset") != floatMap.end())
					material->addProperty("material.heightOffset3", floatMap["_Height3Offset"]);
				if (floatMap.find("_Height3Scale") != floatMap.end())
					material->addProperty("material.heightScale3", floatMap["_Height3Scale"]);
			}
			else
			{
				material->addProperty("material.colorOffset3", 0.0f);
				material->addProperty("material.colorScale3", 0.0f);
				material->addProperty("material.heightOffset3", 0.0f);
				material->addProperty("material.heightScale3", 0.0f);
			}

			material->addProperty("material.specularWeight", 1.0f);

			auto texMap = unityMaterial.texEnvs;
			if (texMap.find("_Albedo1") != texMap.end())
				loadTexture(material, "baseColorTex1", texMap["_Albedo1"], false);
			else
				material->addProperty("baseColorTex1.use", false);
			if (texMap.find("_Albedo2") != texMap.end())
				loadTexture(material, "baseColorTex2", texMap["_Albedo2"], false);
			else
				material->addProperty("baseColorTex2.use", false);
			if (texMap.find("_Albedo3") != texMap.end())
				loadTexture(material, "baseColorTex3", texMap["_Albedo3"], false);
			else
				material->addProperty("baseColorTex3.use", false);
			if (texMap.find("_Height2") != texMap.end())
				loadTexture(material, "heightTex1", texMap["_Height2"], false);
			else
				material->addProperty("heightTex1.use", false);
			if (texMap.find("_Height3") != texMap.end())
				loadTexture(material, "heightTex2", texMap["_Height3"], false);
			else
				material->addProperty("heightTex2.use", false);

			if (texMap.find("_Metallic1") != texMap.end())
			{
				auto guid = texMap["_Metallic1"].texture.guid;
				auto tex = loadMetalRoughTex(guid, std::string(), smoothness1);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);

					std::string texInfoStr = "metalRoughTex1";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
				}

				material->addProperty("material.roughnessFactor1", 1.0f);
			}
			else
			{
				material->addProperty("metalRoughTex1.use", false);
				material->addProperty("material.roughnessFactor1", 1.0f - smoothness1);
			}
			if (texMap.find("_Metallic2") != texMap.end())
			{
				auto guid = texMap["_Metallic2"].texture.guid;
				auto tex = loadMetalRoughTex(guid, std::string(), smoothness2);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);

					std::string texInfoStr = "metalRoughTex2";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
				}

				material->addProperty("material.roughnessFactor2", 1.0f);
			}
			else
			{
				material->addProperty("metalRoughTex2.use", false);
				material->addProperty("material.roughnessFactor2", 1.0f - smoothness2);
			}
			if (texMap.find("_Metallic3") != texMap.end())
			{
				auto guid = texMap["_Metallic3"].texture.guid;
				auto tex = loadMetalRoughTex(guid, std::string(), smoothness3);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);

					std::string texInfoStr = "metalRoughTex3";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
				}

				material->addProperty("material.roughnessFactor3", 1.0f);
			}
			else
			{
				material->addProperty("metalRoughTex3.use", false);
				material->addProperty("material.roughnessFactor3", 1.0f - smoothness3);
			}
			if (texMap.find("_Normal1") != texMap.end())
				loadTexture(material, "normalTex1", texMap["_Normal1"], false);
			else
				material->addProperty("normalTex1.use", false);
			if (texMap.find("_Normal2") != texMap.end())
				loadTexture(material, "normalTex2", texMap["_Normal2"], false);
			else
				material->addProperty("normalTex2.use", false);
			if (texMap.find("_Normal3") != texMap.end())
				loadTexture(material, "normalTex3", texMap["_Normal3"], false);
			else
				material->addProperty("normalTex3.use", false);

			auto colorMap = unityMaterial.colors;
			if (colorMap.find("_Color1") != colorMap.end())
				material->addProperty("material.baseColorFactor1", Color::sRGBAlphaToLinear(colorMap["_Color1"]));
			else
				material->addProperty("material.baseColorFactor1", glm::vec4(1.0f));
			if (colorMap.find("_Color2") != colorMap.end())
				material->addProperty("material.baseColorFactor2", Color::sRGBAlphaToLinear(colorMap["_Color2"]));
			else
				material->addProperty("material.baseColorFactor2", glm::vec4(1.0f));
			if (colorMap.find("_Color3") != colorMap.end())
				material->addProperty("material.baseColorFactor3", Color::sRGBAlphaToLinear(colorMap["_Color3"]));
			else
				material->addProperty("material.baseColorFactor3", glm::vec4(1.0f));

			// PBR MetallicRough parameters
			material->addProperty("material.alphaCutOff", 0.0f);
			material->addProperty("material.alphaMode", 0);
			material->addProperty("material.ior", 1.5f);

			return material;
		}

		Material::Ptr Materials::loadRainMaterial(UnityMaterial& unityMaterial)
		{
			auto material = loadDefaultMaterial(unityMaterial);
			material->setShader("Default_RAIN_MATERIAL");
			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;

			if (floatMap.find("_NormalScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_NormalScale"]);
			if (floatMap.find("_RainDrops_Power") != floatMap.end())
				material->addProperty("material.rainDropsPower", floatMap["_RainDrops_Power"]);
			if (floatMap.find("_RainDrops_Tile") != floatMap.end())
				material->addProperty("material.rainDropsTile", floatMap["_RainDrops_Tile"]);
			if (floatMap.find("_RainSpeed") != floatMap.end())
				material->addProperty("material.rainSpeed", floatMap["_RainSpeed"]);
			if (floatMap.find("_Rain_Metallic") != floatMap.end())
				material->addProperty("material.rainMetallic", floatMap["_Rain_Metallic"]);
			if (floatMap.find("_Rain_Smoothness") != floatMap.end())
				material->addProperty("material.rainSmoothness", floatMap["_Rain_Smoothness"]);
			if (texMap.find("_Mask") != texMap.end())
				loadTexture(material, "maskTex", texMap["_Mask"], false);
			if (texMap.find("_TextureSample1") != texMap.end())
				loadTexture(material, "sampleTex", texMap["_TextureSample1"], false);

			return material;
		}

		Material::Ptr Materials::loadRainMaterialPOM(UnityMaterial& unityMaterial)
		{
			auto material = getDefaultMaterial();

			material->setShader("Default_RAIN_MATERIAL_POM_POM");
			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;
			auto colMap = unityMaterial.colors;

			int alphaMode = 0;
			float alphaCutOff = 0.0f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 2.0f)
				{
					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", 0.5f);
				}
				else if (mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
			}

			if (floatMap.find("_NormalScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_NormalScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float metallic = 0.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];
			if (floatMap.find("_Metallic") != floatMap.end())
				metallic = floatMap["_Metallic"];

			if (texMap.find("_Albedo") != texMap.end())
				loadTexture(material, "baseColorTex", texMap["_Albedo"], true);
			if (texMap.find("_Normal") != texMap.end())
				loadTexture(material, "normalTex", texMap["_Normal"], false);
			if (texMap.find("_Metallic") != texMap.end())
			{
				std::string occGUID;
				if (texMap.find("_OcclusionMap") != texMap.end())
					occGUID = texMap["_OcclusionMap"].texture.guid;

				auto guid = texMap["_Metallic"].texture.guid;
				auto tex = loadMetalRoughTex(guid, occGUID, glossMapScale);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);
					tex->setOffset(texMap["_Metallic"].offset);
					tex->setScale(texMap["_Metallic"].scale);

					std::string texInfoStr = "metalRoughTex";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
				}

				material->addProperty("material.roughnessFactor", 1.0f);
				material->addProperty("material.metallicFactor", 1.0f);
			}
			else
			{
				material->addProperty("material.roughnessFactor", 1.0f - glossiness);
				material->addProperty("material.metallicFactor", metallic);
			}

			if (colMap.find("_Color") != colMap.end())
				material->addProperty("material.baseColorFactor", Color::sRGBAlphaToLinear(colMap["_Color"]));

			// rain parameters
			if (floatMap.find("_RainDrops_Power") != floatMap.end())
				material->addProperty("material.rainDropsPower", floatMap["_RainDrops_Power"]);
			if (floatMap.find("_RainDrops_Tile") != floatMap.end())
				material->addProperty("material.rainDropsTile", floatMap["_RainDrops_Tile"]);
			if (floatMap.find("_RainSpeed") != floatMap.end())
				material->addProperty("material.rainSpeed", floatMap["_RainSpeed"]);
			if (floatMap.find("_Rain_Metallic") != floatMap.end())
				material->addProperty("material.rainMetallic", floatMap["_Rain_Metallic"]);
			if (floatMap.find("_Rain_Smoothness") != floatMap.end())
				material->addProperty("material.rainSmoothness", floatMap["_Rain_Smoothness"]);
			if (texMap.find("_Mask") != texMap.end())
				loadTexture(material, "maskTex", texMap["_Mask"], false);
			if (texMap.find("_TextureSample1") != texMap.end())
				loadTexture(material, "sampleTex", texMap["_TextureSample1"], false);

			// pom parameters
			if (floatMap.find("_Scale") != floatMap.end())
				material->addProperty("material.scale", floatMap["_Scale"]);
			if (floatMap.find("_RefPlane") != floatMap.end())
				material->addProperty("material.refPlane", floatMap["_RefPlane"]);
			if (floatMap.find("_CurvFix") != floatMap.end())
				material->addProperty("material.curvFix", floatMap["_CurvFix"]);
			if (floatMap.find("_CurvatureU") != floatMap.end())
				material->addProperty("material.curvatureU", floatMap["_CurvatureU"]);
			if (floatMap.find("_CurvatureV") != floatMap.end())
				material->addProperty("material.curvatureV", floatMap["_CurvatureV"]);
			if (texMap.find("_HeightMap") != texMap.end())
				loadTexture(material, "heightTex", texMap["_HeightMap"], false);

			return material;
		}

		Material::Ptr Materials::loadRainRefractionMaterial(UnityMaterial& unityMaterial)
		{
			auto material = loadDefaultMaterial(unityMaterial);
			material->setShader("Default_RAIN_REFRACTION");
			material->setTransmissive(true);
			material->setDoubleSided(true);
			//material->setWriteDepth();

			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;

			if (floatMap.find("_RainDrops_Power") != floatMap.end())
				material->addProperty("material.rainDropsPower", floatMap["_RainDrops_Power"]);
			if (floatMap.find("_RainDrops_Tile") != floatMap.end())
				material->addProperty("material.rainDropsTile", floatMap["_RainDrops_Tile"]);
			if (floatMap.find("_RainSpeed") != floatMap.end())
				material->addProperty("material.rainSpeed", floatMap["_RainSpeed"]);
			if (floatMap.find("_Distortion") != floatMap.end())
				material->addProperty("material.distortion", floatMap["_Distortion"]);

			if (texMap.find("_BrushedMetalNormal") != texMap.end())
				loadTexture(material, "normalTex", texMap["_BrushedMetalNormal"], false);

			return material;
		}

		Material::Ptr Materials::loadPomMaterial(UnityMaterial& unityMaterial)
		{
			auto material = getDefaultMaterial();

			material->setShader("Default_POM");
			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;
			auto colMap = unityMaterial.colors;

			int alphaMode = 0;
			float alphaCutOff = 0.0f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 2.0f)
				{
					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", 0.5f);
				}
				else if (mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
			}

			if (floatMap.find("_NormalScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_NormalScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float metallic = 0.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_SmoothnessPower") != floatMap.end())
				glossiness = floatMap["_SmoothnessPower"];
			if (floatMap.find("_Metallic_Power") != floatMap.end())
				metallic = floatMap["_Metallic_Power"];
			if (floatMap.find("_OcclusionPower") != floatMap.end())
				material->addProperty("material.occlusionStrength", floatMap["_OcclusionPower"]);

			if (texMap.find("_Albedo") != texMap.end())
				loadTexture(material, "baseColorTex", texMap["_Albedo"], true);
			if (texMap.find("_Normal") != texMap.end())
				loadTexture(material, "normalTex", texMap["_Normal"], false);
			if (texMap.find("_Metallic") != texMap.end())
			{
				std::string occGUID;
				if (texMap.find("_Occlusion") != texMap.end())
					occGUID = texMap["_Occlusion"].texture.guid;

				auto guid = texMap["_Metallic"].texture.guid;
				auto tex = loadMetalRoughTex(guid, occGUID, glossiness);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);
					tex->setOffset(texMap["_Metallic"].offset);
					tex->setScale(texMap["_Metallic"].scale);

					std::string texInfoStr = "metalRoughTex";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());

					if (!occGUID.empty())
					{
						tex->setOffset(texMap["_Occlusion"].offset);
						tex->setScale(texMap["_Occlusion"].scale);

						std::string texInfoStr = "occlusionTex";
						material->addTexture(texInfoStr, tex);
						material->addProperty(texInfoStr + ".use", true);
						material->addProperty(texInfoStr + ".uvIndex", 0);
						material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
					}
				}

				material->addProperty("material.roughnessFactor", 1.0f);
				material->addProperty("material.metallicFactor", metallic);
			}
			else
			{
				material->addProperty("material.roughnessFactor", 1.0f - glossiness);
				material->addProperty("material.metallicFactor", metallic);
			}

			if (colMap.find("_Color") != colMap.end())
				material->addProperty("material.baseColorFactor", Color::sRGBAlphaToLinear(colMap["_Color"]));

			// pom parameters
			if (floatMap.find("_Scale") != floatMap.end())
				material->addProperty("material.scale", floatMap["_Scale"]);
			if (floatMap.find("_RefPlane") != floatMap.end())
				material->addProperty("material.refPlane", floatMap["_RefPlane"]);
			if (floatMap.find("_CurvFix") != floatMap.end())
				material->addProperty("material.curvFix", floatMap["_CurvFix"]);
			if (floatMap.find("_CurvatureU") != floatMap.end())
				material->addProperty("material.curvatureU", floatMap["_CurvatureU"]);
			if (floatMap.find("_CurvatureV") != floatMap.end())
				material->addProperty("material.curvatureV", floatMap["_CurvatureV"]);
			if (texMap.find("_HeightMap") != texMap.end())
				loadTexture(material, "heightTex", texMap["_HeightMap"], false);

			return material;
		}

		Material::Ptr Materials::loadVelvetMaterial(UnityMaterial& unityMaterial)
		{
			auto material = getDefaultMaterial();

			material->setShader("Default_VELVET_MATERIAL");
			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;
			auto colMap = unityMaterial.colors;

			int alphaMode = 0;
			float alphaCutOff = 0.0f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 2.0f)
				{
					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", 0.5f);
				}
				else if (mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
			}

			float smoothness = 1.0f;
			if (floatMap.find("_Smoothness") != floatMap.end())
				smoothness = floatMap["_Smoothness"];
			if (floatMap.find("_OcclusionPower") != floatMap.end())
				material->addProperty("material.occlusionStrength", floatMap["_OcclusionPower"]);
			if (floatMap.find("_NormalPower") != floatMap.end())
				material->addProperty("material.normalPower", floatMap["_NormalPower"]);
			if (floatMap.find("_NormalDetailPower") != floatMap.end())
				material->addProperty("material.normalDetailPower", floatMap["_NormalDetailPower"]);
			if (floatMap.find("_Emission_Power") != floatMap.end())
				material->addProperty("material.emissiveStrength", floatMap["_Emission_Power"]);
			if (floatMap.find("_RimPower") != floatMap.end())
				material->addProperty("material.rimPower", floatMap["_RimPower"]);

			if (texMap.find("_Albedo") != texMap.end())
				loadTexture(material, "baseColorTex", texMap["_Albedo"], true);
			if (texMap.find("_Detail_Albedo") != texMap.end())
				loadTexture(material, "baseColorDetailTex", texMap["_Detail_Albedo"], true);
			else
				material->addProperty("baseColorDetailTex.use", false);
			if (texMap.find("_Emission") != texMap.end())
				loadTexture(material, "emissiveTex", texMap["_Emission"], true);
			if (texMap.find("_Normals") != texMap.end())
				loadTexture(material, "normalTex", texMap["_Normals"], false);
			if (texMap.find("_Normals_Detail") != texMap.end())
				loadTexture(material, "normalDetailTex", texMap["_Normals_Detail"], false);
			else
				material->addProperty("normalDetailTex.use", false);
			if (texMap.find("_Noise") != texMap.end())
				loadTexture(material, "noiseTex", texMap["_Noise"], false);
			if (texMap.find("_Metallic") != texMap.end())
			{
				std::string occGUID;
				if (texMap.find("_Occlusion") != texMap.end())
					occGUID = texMap["_Occlusion"].texture.guid;

				auto guid = texMap["_Metallic"].texture.guid;
				auto tex = loadMetalRoughTex(guid, occGUID, smoothness);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);
					tex->setOffset(texMap["_Metallic"].offset);
					tex->setScale(texMap["_Metallic"].scale);

					std::string texInfoStr = "metalRoughTex";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());

					if (!occGUID.empty())
					{
						tex->setOffset(texMap["_Occlusion"].offset);
						tex->setScale(texMap["_Occlusion"].scale);

						std::string texInfoStr = "occlusionTex";
						material->addTexture(texInfoStr, tex);
						material->addProperty(texInfoStr + ".use", true);
						material->addProperty(texInfoStr + ".uvIndex", 0);
						material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
					}
				}

				material->addProperty("material.roughnessFactor", 1.0f);
				material->addProperty("material.metallicFactor", 1.0f);
			}
			else
			{
				material->addProperty("material.roughnessFactor", 1.0f - smoothness);
				material->addProperty("material.metallicFactor", 1.0f);
			}

			if (colMap.find("_AlbedoColor") != colMap.end())
				material->addProperty("material.baseColorFactor", Color::sRGBAlphaToLinear(colMap["_AlbedoColor"]));
			if (colMap.find("_EmissionColor") != colMap.end())
				material->addProperty("material.emissiveFactor", glm::vec3(Color::sRGBAlphaToLinear(colMap["_EmissionColor"])));
			if (colMap.find("_RimColor") != colMap.end())
				material->addProperty("material.rimColor", Color::sRGBAlphaToLinear(colMap["_RimColor"]));

			return material;
		}

		Material::Ptr Materials::loadGlassMaterial(UnityMaterial& unityMaterial)
		{
			auto material = getDefaultMaterial();

			material->setShader("Default_GLASS_MATERIAL");
			material->setTransmissive(true);

			auto floatMap = unityMaterial.floats;
			auto texMap = unityMaterial.texEnvs;
			auto colMap = unityMaterial.colors;

			int alphaMode = 0;
			float alphaCutOff = 0.0f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 2.0f)
				{
					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", 0.5f);
				}
				else if (mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
			}

			float smoothness = 1.0f;
			if (floatMap.find("_NormalScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_NormalScale"]);
			if (floatMap.find("_OcclusionPower") != floatMap.end())
				material->addProperty("material.occlusionStrength", floatMap["_OcclusionPower"]);
			if (floatMap.find("_IndexofRefraction") != floatMap.end())
				material->addProperty("material.indexOfRefraction", floatMap["_IndexofRefraction"]);
			if (floatMap.find("_ChromaticAberration") != floatMap.end())
				material->addProperty("material.chromaticAberration", floatMap["_ChromaticAberration"]);

			if (texMap.find("_Albedo") != texMap.end())
				loadTexture(material, "baseColorTex", texMap["_Albedo"], true);
			if (texMap.find("_NormalMap") != texMap.end())
				loadTexture(material, "normalTex", texMap["_NormalMap"], false);
			if (texMap.find("_Smoothness") != texMap.end())
			{
				std::string occGUID;
				if (texMap.find("_Occlusion") != texMap.end())
					occGUID = texMap["_Occlusion"].texture.guid;

				auto guid = texMap["_Smoothness"].texture.guid;
				auto tex = loadMetalRoughTex(guid, occGUID, 1.0f);
				if (tex)
				{
					//tex->generateMipmaps();
					tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					tex->setWrap(GL::REPEAT, GL::REPEAT);
					tex->setOffset(texMap["_Smoothness"].offset);
					tex->setScale(texMap["_Smoothness"].scale);

					std::string texInfoStr = "metalRoughTex";
					material->addTexture(texInfoStr, tex);
					material->addProperty(texInfoStr + ".use", true);
					material->addProperty(texInfoStr + ".uvIndex", 0);
					material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());

					if (!occGUID.empty())
					{
						tex->setOffset(texMap["_Occlusion"].offset);
						tex->setScale(texMap["_Occlusion"].scale);

						std::string texInfoStr = "occlusionTex";
						material->addTexture(texInfoStr, tex);
						material->addProperty(texInfoStr + ".use", true);
						material->addProperty(texInfoStr + ".uvIndex", 0);
						material->addProperty(texInfoStr + ".uvTransform", tex->getUVTransform());
					}
				}

				material->addProperty("material.roughnessFactor", 1.0f);
				material->addProperty("material.metallicFactor", 1.0f);
			}
			else
			{
				material->addProperty("material.roughnessFactor", 1.0f - smoothness);
				material->addProperty("material.metallicFactor", 1.0f);
			}

			if (colMap.find("_Color") != colMap.end())
				material->addProperty("material.baseColorFactor", Color::sRGBAlphaToLinear(colMap["_Color"]));

			return material;
		}

		Material::Ptr Materials::loadDefaultShaderGraph(UnityMaterial& unityMaterial)
		{
			auto material = Material::create();
			material->setShader("Default_SPECGLOSS");
			material->addProperty("material.emissiveFactor", glm::vec3(0.0f));
			material->addProperty("material.emissiveStrength", 1.0f);

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			int alphaMode = 0;
			float alphaCutOff = 0.5f; // TODO: fetch alpha cutoff
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 1.0f)
				{
					if (floatMap.find("_Cutoff") != floatMap.end())
						alphaCutOff = floatMap["_Cutoff"];

					material->addProperty("material.alphaMode", 1);
					material->addProperty("material.alphaCutOff", alphaCutOff);
				}
				else if (mode == 2.0f || mode == 3.0f)
				{
					material->addProperty("material.alphaMode", 2);
					material->setBlending(true);
				}
				else
				{
					material->addProperty("material.alphaMode", 0);
				}
			}

			if (floatMap.find("_BumpScale") != floatMap.end())
				material->addProperty("material.normalScale", floatMap["_BumpScale"]);
			if (floatMap.find("_OcclusionStrength") != floatMap.end())
				material->addProperty("material.occlusionStrength", floatMap["_OcclusionStrength"]);

			if (texMap.find("_MainTex") != texMap.end())
				loadTexture(material, "diffuseTex", texMap["_MainTex"], false);
			else
				material->addProperty("diffuseTex.use", false);

			if (texMap.find("_BumpMap") != texMap.end())
				loadTexture(material, "normalTex", texMap["_BumpMap"], false);
			else
				material->addProperty("normalTex.use", false);

			if (texMap.find("_OcclusionMap") != texMap.end())
				loadTexture(material, "occlusionTex", texMap["_OcclusionMap"], false);
			else
				material->addProperty("occlusionTex.use", false);

			bool useSpecGlossMap = true;
			if (floatMap.find("_SPECGLOSSMAP") != floatMap.end())
				useSpecGlossMap = floatMap["_SPECGLOSSMAP"];

			float glossiness = 0.0f;
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];

			glm::vec3 specularColor = glm::vec3(0);
			if (colMap.find("_SpecColor") != colMap.end())
				specularColor = glm::vec3(Color::sRGBAlphaToLinear(colMap["_SpecColor"]));

			if (useSpecGlossMap && (texMap.find("_SpecGlossMap") != texMap.end()))
			{
				loadTexture(material, "specGlossTex", texMap["_SpecGlossMap"], false);
				material->addProperty("material.specularFactor", glm::vec3(1.0f));
				material->addProperty("material.glossFactor", 1.0f);
			}
			else
			{
				material->addProperty("specGlossTex.use", false);
				material->addProperty("material.specularFactor", specularColor);
				material->addProperty("material.glossFactor", glossiness);
			}

			if (colMap.find("_Color") != colMap.end())
			{
				glm::vec4 rgba = Color::sRGBAlphaToLinear(colMap["_Color"]);
				material->addProperty("material.diffuseFactor", rgba);
			}

			glm::mat3 texTransform = glm::mat3(1.0f);
			if (texMap.find("_DetailAlbedoMap") != texMap.end())
			{
				glm::vec2 offset = texMap["_DetailAlbedoMap"].offset;
				glm::vec2 scale = texMap["_DetailAlbedoMap"].scale;
				glm::mat3 T(1.0f);
				T[2][0] = offset.x;
				T[2][1] = offset.y;

				glm::mat3 S(1.0f);
				S[0][0] = scale.x;
				S[1][1] = scale.y;

				texTransform = T * S;
				loadTexture(material, "detailAlbedoTex", texMap["_DetailAlbedoMap"], false);
				material->addProperty("detailAlbedoTex.uvTransform", texTransform);
			}
			else
				material->addProperty("detailAlbedoTex.use", false);

			if (texMap.find("_DetailNormalMap") != texMap.end())
			{
				loadTexture(material, "detailNormalTex", texMap["_DetailNormalMap"], false);
				material->addProperty("detailNormalTex.uvTransform", texTransform);
			}
			else
				material->addProperty("detailNormalTex.use", false);

			if (texMap.find("_DetailMask") != texMap.end())
			{
				loadTexture(material, "detailMask", texMap["_DetailMask"], false);
				material->addProperty("detailMask.uvTransform", texTransform);
			}
			else
				material->addProperty("detailMask.use", false);

			if (floatMap.find("_DetailNormalMapScale") != floatMap.end())
				material->addProperty("material.detailNormalScale", floatMap["_DetailNormalMapScale"]);

			return material;
		}

		Material::Ptr Materials::loadTerrainShaderGraph(UnityMaterial& unityMaterial)
		{
			auto material = Material::create();
			material->setShader("Default_SPECGLOSS_LAYERED_MATERIAL");
			material->addProperty("material.alphaMode", 0);
			material->addProperty("material.alphaCutOff", 0.5f);

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			glm::vec4 tiling1 = colMap["Vector2_25355DDA"];
			glm::mat3 texTransform1 = glm::mat3(1.0f);
			{
				glm::mat3 T(1.0f);
				T[2][0] = tiling1.z;
				T[2][1] = tiling1.w;
				glm::mat3 S(1.0f);
				S[0][0] = tiling1.x;
				S[1][1] = tiling1.y;
				texTransform1 = T * S;
			}

			if (texMap.find("Texture2D_86E580F7") != texMap.end())
			{
				loadTexture(material, "albedo1", texMap["Texture2D_86E580F7"], false);
				material->addProperty("albedo1.uvTransform", texTransform1);
			}
			else
				material->addProperty("albedo1.use", false);

			if (texMap.find("Texture2D_38793424") != texMap.end())
			{
				loadTexture(material, "normal1", texMap["Texture2D_38793424"], false);
				material->addProperty("normal1.uvTransform", texTransform1);
			}
			else
				material->addProperty("normal1.use", false);

			if (texMap.find("Texture2D_D4C629A1") != texMap.end())
			{
				loadTexture(material, "specular1", texMap["Texture2D_D4C629A1"], false);
				material->addProperty("specular1.uvTransform", texTransform1);
			}
			else
				material->addProperty("specular1.use", false);

			glm::vec4 tiling2 = colMap["Vector2_7270F9CB"];
			glm::mat3 texTransform2 = glm::mat3(1.0f);
			{
				glm::mat3 T(1.0f);
				T[2][0] = tiling1.z;
				T[2][1] = tiling1.w;
				glm::mat3 S(1.0f);
				S[0][0] = tiling1.x;
				S[1][1] = tiling1.y;
				texTransform2 = T * S;
			}

			if (texMap.find("Texture2D_3D8FD2A5") != texMap.end())
			{
				loadTexture(material, "albedo2", texMap["Texture2D_3D8FD2A5"], false);
				material->addProperty("albedo2.uvTransform", texTransform1);
			}
			else
				material->addProperty("albedo2.use", false);

			if (texMap.find("Texture2D_2046661A") != texMap.end())
			{
				loadTexture(material, "normal2", texMap["Texture2D_2046661A"], false);
				material->addProperty("normal2.uvTransform", texTransform1);
			}
			else
				material->addProperty("normal2.use", false);

			if (texMap.find("Texture2D_5E505606") != texMap.end())
			{
				loadTexture(material, "specular2", texMap["Texture2D_5E505606"], false);
				material->addProperty("specular2.uvTransform", texTransform1);
			}
			else
				material->addProperty("specular2.use", false);

			if (texMap.find("Texture2D_933CD9B0") != texMap.end())
			{
				loadTexture(material, "mask", texMap["Texture2D_933CD9B0"], false);
			}
			else
				material->addProperty("mask.use", false);

			return material;
		}

		Material::Ptr Materials::loadGrassShaderGraph(UnityMaterial& unityMaterial)
		{
			auto material = loadDefaultMaterial(unityMaterial);
			material->setShader("Default_TRANSLUCENCY");

			auto floatMap = unityMaterial.floats;
			float alphaCutOff = 0.5f;
			if (floatMap.find("_Cutoff") != floatMap.end())
				alphaCutOff = floatMap["_Cutoff"];

			material->addProperty("material.roughnessFactor", 1.0f);
			material->addProperty("material.translucencyFactor", 0.5f);
			material->addProperty("material.translucencyColorFactor", glm::vec3(2));
			material->addProperty("material.alphaMode", 1);
			material->addProperty("material.alphaCutOff", alphaCutOff);
			material->setBlending(false);
			material->setDoubleSided(true);

			return material;
		}

		Material::Ptr Materials::loadShaderBakedLit(UnityMaterial& unityMaterial)
		{
			auto material = loadDefaultMaterial(unityMaterial);
			auto texMap = unityMaterial.texEnvs;

			material->addProperty("material.metallicFactor", 0.0f);

			glm::mat3 texTransform = glm::mat3(1.0f);
			if (texMap.find("_BaseMap") != texMap.end())
				loadTexture(material, "baseColorTex", texMap["_BaseMap"], true);
			else
				material->addProperty("baseColorTex.use", false);

			return material;
		}

		Material::Ptr Materials::loadMaterialGUID(const std::string& guid)
		{
			if (metaData.find(guid) == metaData.end())
			{
				//std::cout << "error: could not find material with GUID: " << guid << std::endl;
				return nullptr;
			}

			//if (meshCache.find(guid) != meshCache.end())
			//{
			//	std::cout << "warning: material must be loaded form source file! (mesh: " << guid << ")" << std::endl;
			//	return nullptr;
			//}

			Metadata& data = metaData[guid];
			std::string content = IO::loadTxtFile(data.filePath);
			ryml::Tree root = ryml::parse(ryml::to_csubstr(content));
			int matIndex = 0;
			for (int i = 0; i < root.rootref().num_children(); i++) // There can be other objects in a .mat file...
			{
				if (root.docref(i).has_child("Material"))
					matIndex = i;

			}
			ryml::NodeRef stream = root.docref(matIndex);

			std::cout << "loading material " << data.filePath << std::endl;

			UnityMaterial unityMaterial;
			stream["Material"] >> unityMaterial;
			auto shaderGUID = unityMaterial.shader.guid;
			auto shaderfileID = unityMaterial.shader.fileID;

			Material::Ptr material = nullptr;
			if (shaderGUID.compare("0000000000000000f000000000000000") == 0)
			{
				if (shaderfileID == 45)
					material = loadSpecGlossMaterial(unityMaterial);
				else if (shaderfileID == 46)
					material = loadDefaultMaterial(unityMaterial);
				else
					std::cout << "default shader with fileID " << shaderfileID << " not implemented!" << std::endl;
			}
			else
			{
				if (metaData.find(shaderGUID) != metaData.end())
				{
					Metadata& shaderMetadata = metaData[shaderGUID];

					int idx = shaderMetadata.filePath.find_last_of("/") + 1;
					int len = shaderMetadata.filePath.length();
					std::string shaderName = shaderMetadata.filePath.substr(idx, len - idx);

					if (shaderName.compare("DoubleSided.shader") == 0)
					{
						material = loadDefaultMaterial(unityMaterial);

						auto floatMap = unityMaterial.floats;
						float alphaCutOff = 0.5f;
						if (floatMap.find("_Cutoff") != floatMap.end())
							alphaCutOff = floatMap["_Cutoff"];

						material->addProperty("material.alphaMode", 1);
						material->addProperty("material.alphaCutOff", alphaCutOff);
						material->setBlending(false);
						material->setDoubleSided(true);
					}
					else if (shaderName.compare("Height_Blend_2_Layer.shader") == 0)
					{
						material = loadLayerMaterial(unityMaterial, false);
					}
					else if (shaderName.compare("Height_Blend_3_Layer.shader") == 0)
					{
						material = loadLayerMaterial(unityMaterial, true);
					}
					else if (shaderName.compare("Flipbook.shader") == 0)
					{
						material = loadRainMaterial(unityMaterial);
					}
					else if (shaderName.compare("Flipbook_POM.shader") == 0)
					{
						material = loadRainMaterialPOM(unityMaterial);
					}
					else if (shaderName.compare("POM.shader") == 0)
					{
						material = loadPomMaterial(unityMaterial);
					}
					else if (shaderName.compare("Velvet.shader") == 0 ||
						shaderName.compare("Velvet_Curtains.shader") == 0)
					{
						material = loadVelvetMaterial(unityMaterial);
					}
					else if (shaderName.compare("Glass.shader") == 0)
					{
						material = loadGlassMaterial(unityMaterial);
					}
					else if (shaderName.compare("Vegetation.shader") == 0)
					{
						material = loadDefaultMaterial(unityMaterial);
						//material = loadSpecGlossMaterial(unityMaterial);
						material->setShader("Default_VERTEX_WIND");

						auto floatMap = unityMaterial.floats;
						float alphaCutOff = 0.5f;
						if (floatMap.find("_Cutoff") != floatMap.end())
							alphaCutOff = floatMap["_Cutoff"];

						material->addProperty("material.alphaMode", 1);
						material->addProperty("material.alphaCutOff", alphaCutOff);
						material->setBlending(false);
						material->setDoubleSided(true);

						if (floatMap.find("_Smoothness") != floatMap.end())
							material->addProperty("material.roughnessFactor", 1.0f - floatMap["_Smoothness"]);
						if (floatMap.find("_Specular") != floatMap.end())
							material->addProperty("material.specularWeight", glm::vec3(floatMap["_Specular"]));

						float windFrequency = unityMaterial.floats["_Wind_Frequency"];
						float windOndulation = unityMaterial.floats["_Wind_Ondulation"];
						float windPower = unityMaterial.floats["_Wind_Power"];

						material->addProperty("vertexWind.windFrequency", windFrequency);
						material->addProperty("vertexWind.windOndulation", windOndulation);
						material->addProperty("vertexWind.windPower", windPower);
					}
					else if (shaderName.compare("Rain_Refraction.shader") == 0)
					{
						material = loadRainRefractionMaterial(unityMaterial);
					}
					else if (shaderName.compare("Standard.shadergraph") == 0 ||
						shaderName.compare("Standard_AlphaClip.shadergraph") == 0)
					{
						material = loadDefaultShaderGraph(unityMaterial);
					}
					else if (shaderName.compare("TerrainSurfaceGraph.shadergraph") == 0)
					{
						material = loadTerrainShaderGraph(unityMaterial);
					}
					else if (shaderName.compare("Lit_SSS_Cutout.shadergraph") == 0)
					{
						material = loadGrassShaderGraph(unityMaterial);
					}
					else
					{
						std::cout << "warning: custom shader " << shaderName << " not implemented, fallback to default shader!" << std::endl;
						material = loadDefaultMaterial(unityMaterial);
					}
				}
				else if (shaderGUID.compare("933532a4fcc9baf4fa0491de14d08ed7") == 0) // URP/Lit
				{
					material = loadSpecGlossMaterial(unityMaterial);
				}
				else if (shaderGUID.compare("0ca6dca7396eb48e5849247ffd444914") == 0) // URP/BakedLit
				{
					material = loadShaderBakedLit(unityMaterial);
				}
				else
				{
					std::cout << "custom shader not found (GUID: " << shaderGUID << "), fallback to default shader!" << std::endl;
					material = loadDefaultMaterial(unityMaterial);
				}
			}

			return material;
		}

		Texture2D::Ptr Materials::loadTextureGUID(const std::string& guid)
		{
			if (textureCache.find(guid) != textureCache.end())
				return textureCache[guid];

			if (metaData.find(guid) == metaData.end())
				return nullptr;

			Metadata& data = metaData[guid];
			std::cout << "loading texture " << data.filePath << std::endl;

			auto stream = data.root.rootref();
			bool sRGB = false;
			unsigned int maxSize = 0;

			if (stream.has_child("TextureImporter"))
			{
				ryml::NodeRef const& platformSettings = stream["TextureImporter"]["platformSettings"];
				for (auto& settings : platformSettings)
				{
					std::string target;
					settings["buildTarget"] >> target;
					if (target.compare("DefaultTexturePlatform") == 0)
						settings["maxTextureSize"] >> maxSize;
				}

				ryml::NodeRef const& mipmapSettings = stream["TextureImporter"]["mipmaps"];
				mipmapSettings["sRGBTexture"] >> sRGB;
			}

			maxSize = std::min(maxSize, maxTexSize);

			if (textures.find(data.filePath) == textures.end())
				textures.insert(std::make_pair(data.filePath, 0));
			textures[data.filePath]++;

			int index = data.filePath.find_last_of(".") + 1;
			int len = data.filePath.length() - index;
			std::string ext = data.filePath.substr(index, len);

			Texture2D::Ptr tex = nullptr;
			if (ext.compare("hdr") == 0)
			{
				auto image = IO::decodeHDRFromFile(data.filePath);
				tex = image->upload(false);
				tex->generateMipmaps();
			}
			else
			{
				std::string cachePath = "../../../../cache/";
				std::string fn = cachePath + guid + ".ktx2";
				std::ifstream file(fn);
				if (!file.is_open())
				{
					//auto image = IO::decodeTIFFromFile(data.filePath);
					auto image = IO::decodeFromFile(data.filePath);
					if (!image->isPowerOfTwo())
					{
						uint32 w = IO::findPowerOfTwoSize(image->getWidth());
						uint32 h = IO::findPowerOfTwoSize(image->getHeight());
						image = IO::resizeImage(image, w, h);
					}

					int w = image->getWidth();
					int h = image->getHeight();

					if (w > maxSize || h > maxSize)
					{
						float scale = (float)maxSize / (float)std::max(w, h);
						image = IO::resizeImage(image, w * scale, h * scale);
					}

					tex = image->upload(sRGB);
					tex->generateMipmaps();
				}
				else
				{
					file.close();
					//return nullptr;
					tex = IO::loadTextureKTX(fn);
				}
			}
			textureCache.insert(std::make_pair(guid, tex));
			return tex;
		}

		Texture2D::Ptr Materials::loadMetalRoughTex(const std::string& guid, std::string& occGuid, float glossMapScale)
		{
			if (occGuid.empty())
				if (textureCache.find(guid) != textureCache.end())
					return textureCache[guid];

			if (metaData.find(guid) == metaData.end())
			{
				//std::cout << "warning: no texture with GUID " << guid << " found" << std::endl;
				return nullptr;
			}

			std::string cachePath = "../../../../cache/";
			std::string fn = cachePath + guid + ".ktx2";
			std::ifstream file(fn);
			if (file.is_open())
			{
				file.close();
				auto tex = IO::loadTextureKTX(fn);
				textureCache.insert(std::make_pair(guid, tex));
				if (metaData.find(occGuid) == metaData.end())
					occGuid = "";
				return tex;
			}

			ImageUI8::Ptr occImage = nullptr;
			if (metaData.find(occGuid) != metaData.end())
			{
				Metadata& data2 = metaData[occGuid];
				std::cout << "loading texture " << data2.filePath << std::endl;
				if (textures.find(data2.filePath) == textures.end())
					textures.insert(std::make_pair(data2.filePath, 0));
				textures[data2.filePath]++;
				occImage = IO::decodeFromFile(data2.filePath);
				if (!occImage->isPowerOfTwo())
				{
					uint32 w = IO::findPowerOfTwoSize(occImage->getWidth());
					uint32 h = IO::findPowerOfTwoSize(occImage->getHeight());
					occImage = IO::resizeImage(occImage, w, h);
				}
			}
			else
				occGuid = "";

			Metadata& data = metaData[guid];
			std::cout << "loading texture " << data.filePath << std::endl;
			auto stream = data.root.rootref();
			bool sRGB = false;
			unsigned int maxSize = 0;
			if (stream.has_child("TextureImporter"))
			{
				stream["TextureImporter"]["maxTextureSize"] >> maxSize;
				ryml::NodeRef const& mipmapSettings = stream["TextureImporter"]["mipmaps"];
				mipmapSettings["sRGBTexture"] >> sRGB;
			}


			maxSize = std::min(maxSize, maxTexSize);

			if (textures.find(data.filePath) == textures.end())
				textures.insert(std::make_pair(data.filePath, 0));
			textures[data.filePath]++;

			auto image = IO::decodeFromFile(data.filePath);

			if (!image->isPowerOfTwo())
			{
				uint32 w = IO::findPowerOfTwoSize(image->getWidth());
				uint32 h = IO::findPowerOfTwoSize(image->getHeight());
				image = IO::resizeImage(image, w, h);
			}

			int w = image->getWidth();
			int h = image->getHeight();
			if (w > maxSize || h > maxSize)
			{
				float scale = (float)maxSize / (float)std::max(w, h);
				w *= scale;
				h *= scale;
				image = IO::resizeImage(image, w, h);
			}

			if (occImage)
				occImage = IO::resizeImage(occImage, w, h);

			// convert from metal(rgb), gloss(a) + AO(rgb) to occlusion(r), rough(g), metal(b)
			uint32 numChannels = image->getChannels();
			uint8* occ = occImage ? occImage->getRawPtr() : nullptr;
			uint32 numOccChannels = occImage ? occImage->getChannels() : 0;
			uint8* src = image->getRawPtr();
			uint8* dst = new uint8[w * h * 3];
			for (int i = 0; i < w * h; i++)
			{
				if (occImage)
				{
					float ao = 1.0f;
					if (numOccChannels == 3)
						ao = occ[i * 3] / 255.0f;
					else if (numOccChannels == 1)
						ao = occ[i] / 255.0f;
					if (sRGB)
						ao = glm::pow(ao, 2.2);
					dst[i * 3] = ao * 255.0f;
				}
				else
					dst[i * 3] = 0;
				if (numChannels == 4)
					dst[i * 3 + 1] = 255 - glossMapScale * src[i * numChannels + 3]; // roughness = 1 - glossiness
				else
					dst[i * 3 + 1] = 255 - glossMapScale * 255;
				float metallic = src[i * numChannels] / 255.0f;
				if (sRGB)
					metallic = glm::pow(metallic, 2.2);
				dst[i * 3 + 2] = metallic * 255.0f;
			}

			auto dstImage = ImageUI8::create(w, h, 3);
			dstImage->setFromMemory(dst, w * h * 3);
			delete[] dst;
			//return dstImage->upload(false);

			auto tex = dstImage->upload(false);
			tex->generateMipmaps();
			textureCache.insert(std::make_pair(guid, tex));
			return tex;
		}
	}
}
