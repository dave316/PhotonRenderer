#include "UnityMaterials.h"
#include <IO/ImageLoader.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#include <ktx.h>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include <iostream>
namespace json = rapidjson;
namespace IO
{
	namespace Unity
	{
		int countTabs2(std::string line)
		{
			int numTabs = 0;
			for (int i = 0; i < line.length(); i++)
			{
				if (line[i] == '\t')
					numTabs++;
				else
					break;
			}
			return numTabs;
		}

		glm::vec3 sRGBToLinear(glm::vec3 sRGB, float gamma)
		{
			return glm::pow(sRGB, glm::vec3(gamma));
		}

		glm::vec4 sRGBAlphaToLinear(glm::vec4 sRGBAlpha, float gamma)
		{
			glm::vec3 sRGB = glm::vec3(sRGBAlpha);
			float alpha = sRGBAlpha.a;
			glm::vec3 linearRGB = sRGBToLinear(sRGB, gamma);
			return glm::vec4(linearRGB, alpha);
		}

		void resizeImageUint8(uint8* src, uint32 srcW, uint32 srcH, uint8* dst, uint32 dstW, uint32 dstH)
		{
			stbir_resize_uint8_linear(src, srcW, srcH, 0, dst, dstW, dstH, 0, (stbir_pixel_layout)4);
		}

		void resizeImageFP32(float* src, uint32 srcW, uint32 srcH, float* dst, uint32 dstW, uint32 dstH)
		{
			stbir_resize_float_linear(src, srcW, srcH, 0, dst, dstW, dstH, 0, (stbir_pixel_layout)4);
		}

		std::string loadTxtFile(const std::string& fileName)
		{
			std::ifstream file(fileName);
			std::stringstream ss;

			if (file.is_open())
			{
				ss << file.rdbuf();
			}
			else
			{
				std::cout << "could not open file " << fileName << std::endl;
			}

			return ss.str();
		}

		uint32 findPowerOfTwoSize(uint32 size)
		{
			uint32 e = 0;
			while (size > 1)
			{
				size = (size >> 1);
				e++;
			}
			return static_cast<uint32>(std::pow(2.0, e));
		}

		pr::Texture2D::Ptr loadTextureKTX(const std::string& filename)
		{
			ktxTexture2* pKtxTexture;
			KTX_error_code result;

			result = ktxTexture2_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
			if (pKtxTexture->isCompressed)
				result = ktxTexture2_TranscodeBasis(pKtxTexture, KTX_TTF_BC7_RGBA, 0);

			//std::cout << "vk format: " << pKtxTexture->vkFormat << std::endl;
			//std::cout << "width: " << pKtxTexture->baseWidth << std::endl;
			//std::cout << "height: " << pKtxTexture->baseHeight << std::endl;
			//std::cout << "depth: " << pKtxTexture->baseDepth << std::endl;
			//std::cout << "dims: " << pKtxTexture->numDimensions << std::endl;
			//std::cout << "faces: " << pKtxTexture->numFaces << std::endl;
			//std::cout << "layers: " << pKtxTexture->numLayers << std::endl;
			//std::cout << "levels: " << pKtxTexture->numLevels << std::endl;
			//std::cout << "data: " << pKtxTexture->dataSize << std::endl;

			pr::Texture2D::Ptr texture = nullptr;
			if (result == KTX_SUCCESS)
			{
				GLint internalFormat = 0;
				GPU::Format format = GPU::Format::RGBA8;
				if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_UNORM_BLOCK)
					std::cout << "tex format RGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_SRGB_BLOCK)
					std::cout << "tex format SRGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_UNORM_BLOCK)
					format = GPU::Format::BC7_RGBA;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK)
					format = GPU::Format::BC7_SRGB;

				texture = pr::Texture2D::create(pKtxTexture->baseWidth, pKtxTexture->baseHeight, format, pKtxTexture->numLevels);

				for (uint32 level = 0; level < pKtxTexture->numLevels; level++)
				{
					int w = std::max(pKtxTexture->baseWidth >> level, 1U);
					int h = std::max(pKtxTexture->baseHeight >> level, 1U);
					uint32 imgSize = (uint32)ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, 0, 0, &offset);
					ktx_uint8_t* pData = pKtxTexture->pData + offset;
					texture->upload(pData, imgSize, level);
				}
			}

			ktxTexture_Destroy(ktxTexture(pKtxTexture));

			return texture;
		}

		pr::Texture2D::Ptr Materials::loadTextureGUID(const std::string& guid, bool loadUncompressed)
		{
			if (textureCache.find(guid) != textureCache.end())
				return textureCache[guid];

			if (metaData.find(guid) == metaData.end())
				return nullptr;

			Metadata& data = metaData[guid];

			int idx = (int)data.filePath.find_last_of('/') + 1;
			int l = (int)data.filePath.length() - idx;
			std::string filename = data.filePath.substr(idx, l);

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

			int index = (int)data.filePath.find_last_of(".") + 1;
			int len = (int)data.filePath.length() - index;
			std::string ext = data.filePath.substr(index, len);

			pr::Texture2D::Ptr tex = nullptr;
			if (ext.compare("hdr") == 0)
			{
				std::cout << "loading texture " << data.filePath << std::endl;

				auto img = IO::ImageLoader::loadFromFile(data.filePath);

				uint32 width = img->getWidth();
				uint32 height = img->getHeight();
				uint8* dataPtr = img->getRawPtr();
				uint32 dataSize = width * height * 4;

				GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
				GPU::Format format = GPU::Format::RGBA32F;

				// generate mipmaps
				uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1;
				tex = pr::Texture2D::create(width, height, format, levels);
				tex->upload((uint8*)dataPtr, dataSize * sizeof(float));
				tex->generateMipmaps();
				tex->setAddressMode(GPU::AddressMode::Repeat);
				tex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
			}
			else
			{
				std::string cachePath = "../../../../cache/";
				std::string fn = cachePath + guid + ".ktx2";
				std::ifstream file(fn);
				if (!file.is_open() || loadUncompressed)
				{
					file.close();

					std::cout << "loading texture " << data.filePath << std::endl;

					auto img = IO::ImageLoader::loadFromFile(data.filePath);
					uint32 width = img->getWidth();
					uint32 height = img->getHeight();
					uint8* dataPtr = img->getRawPtr();

					uint32 dataSize = width * height * 4;

					if (width > maxSize || height > maxSize)
					{
						float scale = (float)maxSize / (float)std::max(width, height);
						uint32 w = static_cast<uint32>(width * scale);
						uint32 h = static_cast<uint32>(height * scale);
						uint8* newDataPtr = new uint8[w * h * 4];
						resizeImageUint8(dataPtr, width, height, newDataPtr, w, h);

						width = w;
						height = h;

						//delete[] dataPtr;
						dataPtr = newDataPtr;
						dataSize = width * height * 4;
					}

					GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
					GPU::Format format = sRGB ? GPU::Format::SRGBA8 : GPU::Format::RGBA8;

					// generate mipmaps
					uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1;
					tex = pr::Texture2D::create(width, height, format, levels);
					tex->upload(dataPtr, dataSize);
					tex->generateMipmaps();
					tex->setAddressMode(GPU::AddressMode::Repeat);
					tex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
				}
				else
				{
					std::cout << "loading texture " << fn << std::endl;

					file.close();
					tex = loadTextureKTX(fn);
					tex->setAddressMode(GPU::AddressMode::Repeat);
					tex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
				}
			}

			tex->createData();
			tex->uploadData();

			textureCache.insert(std::make_pair(guid, tex));
			return tex;
		}

		void Materials::addTexture(pr::Material::Ptr material, Texture& texture, float defaultValue, bool loadUncompressed)
		{
			auto tex = loadTextureGUID(texture.texture.guid, loadUncompressed);

			pr::TextureInfo info;
			info.offset = texture.offset;
			info.scale = texture.scale;
			info.defaultValue = defaultValue;
			material->addTexture("", tex, info); // TODO: add uniform name
			//if (tex)
			//{
			//	material->addTexture(tex, 0, defaultValue, texTransform);
			//}
			//else
			//{
			//	material->addTexture(nullptr);
			//}
		}

		glm::mat3 getTexTransform(Texture& texInfo)
		{
			glm::vec2 offset = texInfo.offset;
			glm::vec2 scale = texInfo.scale;
			glm::mat3 T(1.0f);
			T[2][0] = offset.x;
			T[2][1] = offset.y;

			glm::mat3 S(1.0f);
			S[0][0] = scale.x;
			S[1][1] = scale.y;

			return T * S;
		}

		pr::Material::Ptr Materials::loadDefaultMaterial(Material& unityMaterial)
		{
			//auto mat = pr::Material::create("Default", "Default");
			//mat->addProperty("baseColor", glm::vec4(1));
			//mat->addProperty("emissive", glm::vec4(0));
			//mat->addProperty("roughness", 1.0f);
			//mat->addProperty("metallic", 0.0f);
			//mat->addProperty("occlusion", 1.0f);
			//mat->addProperty("normalScale", 1.0f);
			//mat->addProperty("alphaMode", 0);
			//mat->addProperty("alphaCutOff", 0.5f);
			//mat->addProperty("computeFlatNormals", 0);
			//mat->addProperty("ior", 1.5f);
			//for (int i = 0; i < 5; i++)
			//	mat->addTexture(nullptr);
			//return mat;

			auto material = pr::Material::create(unityMaterial.name, "UnityDefault");
			material->addProperty("baseColor", glm::vec4(1));
			material->addProperty("emissive", glm::vec4(glm::vec3(0), 1));
			material->addProperty("glossiness", 1.0f);
			material->addProperty("glossMapScale", 1.0f);
			material->addProperty("metallic", 0.0f);
			material->addProperty("occlusion", 1.0f);
			material->addProperty("normalScale", 1.0f);
			material->addProperty("alphaMode", 0);
			material->addProperty("alphaCutOff", 0.5f);
			material->addProperty("ior", 1.5f);

			bool emissive = false;
			if (unityMaterial.keywords.find("_EMISSION") != unityMaterial.keywords.end())
				emissive = true;

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
					material->setProperty("alphaMode", 1);
					material->setProperty("alphaCutOff", alphaCutOff);
				}
				else if (mode == 2.0f || mode == 3.0f)
				{
					material->setProperty("alphaMode", 2);
					material->setShaderName("UnityDefaultTransparency");
					material->setTransparent(true);
				}
			}

			if (floatMap.find("_BumpScale") != floatMap.end())
				material->setProperty("normalScale", floatMap["_BumpScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float metallic = 0.0f;
			float occlusionStrength = 1.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];
			if (floatMap.find("_Metallic") != floatMap.end())
				metallic = floatMap["_Metallic"];
			if (floatMap.find("_OcclusionStrength") != floatMap.end())
				occlusionStrength = floatMap["_OcclusionStrength"];

			material->setProperty("glossiness", glossiness);
			material->setProperty("glossMapScale", glossMapScale);
			material->setProperty("metallic", metallic);
			material->setProperty("occlusion", occlusionStrength);

			glm::vec2 offset = glm::vec2(0);
			glm::vec2 scale = glm::vec2(1);
			if (texMap.find("_MainTex") != texMap.end())
			{
				offset = texMap["_MainTex"].offset;
				scale = texMap["_MainTex"].scale;
				addTexture(material, texMap["_MainTex"]);
			}
			else
			{
				if (texMap.find("_Albedo") != texMap.end())
				{
					offset = texMap["_Albedo"].offset;
					scale = texMap["_Albedo"].scale;
					addTexture(material, texMap["_Albedo"]);
				}
				else
				{
					material->addTexture("", nullptr);
				}
			}

			// The default shader uses the same tex transform for all main texture maps
			for (auto&& [_, tex] : texMap)
			{
				tex.offset = offset;
				tex.scale = scale;
			}

			if (texMap.find("_BumpMap") != texMap.end())
			{
				addTexture(material, texMap["_BumpMap"]);
			}				
			else
			{
				if (texMap.find("_Normal") != texMap.end())
					addTexture(material, texMap["_Normal"]);
				else
					material->addTexture("", nullptr);
			}

			if (texMap.find("_MetallicGlossMap") != texMap.end())
				addTexture(material, texMap["_MetallicGlossMap"], 0.0f, true);
			else
				material->addTexture("", nullptr);

			if (emissive && texMap.find("_EmissionMap") != texMap.end())
				addTexture(material, texMap["_EmissionMap"]);
			else
				material->addTexture("", nullptr);

			if (texMap.find("_OcclusionMap") != texMap.end())
			{
				addTexture(material, texMap["_OcclusionMap"]);
			}
			else
			{
				if (texMap.find("_AmbientOcclusion") != texMap.end())
					addTexture(material, texMap["_AmbientOcclusion"]);
				else
					material->addTexture("", nullptr);
			}

			//if (texMap.find("_ParallaxMap") != texMap.end())
			//{
			//	material->setShader("Default_POM");
			//	loadTexture(material, "heightTex", texMap["_ParallaxMap"], false);
			//	if (floatMap.find("_Parallax") != floatMap.end())
			//		material->addProperty("material.scale", floatMap["_Parallax"]);
			//	material->addProperty("material.refPlane", 0.0f);
			//	material->addProperty("material.curvFix", 1.0f);
			//	material->addProperty("material.curvatureU", 0.0f);
			//	material->addProperty("material.curvatureV", 0.0f);
			//	material->addProperty("heightTex.uvTransform", texTransform);
			//}

			if (colMap.find("_Color") != colMap.end())
			{
				glm::vec4 rgba = sRGBAlphaToLinear(colMap["_Color"]);
				if (texMap.find("_MainTex") != texMap.end())
					rgba.a = 1.0f;
				material->setProperty("baseColor", rgba);
			}

			if (emissive && colMap.find("_EmissionColor") != colMap.end())
			{
				glm::vec4 emissionColor = sRGBAlphaToLinear(colMap["_EmissionColor"]);
				material->setProperty("emissive", glm::vec4(glm::vec3(emissionColor), 1.0f));
			}

			//if (material->isTransparent())
			//	material->setShaderName("DefaultTransparency");

			return material;
		}

		pr::Material::Ptr Materials::loadSpecGlossMaterial(Material& unityMaterial)
		{
			auto material = pr::Material::create(unityMaterial.name, "UnitySpecGloss");
			material->addProperty("baseColor", glm::vec4(1));
			material->addProperty("specularColor", glm::vec4(0));
			material->addProperty("emissive", glm::vec4(glm::vec3(0), 1));
			material->addProperty("glossiness", 1.0f);
			material->addProperty("glossMapScale", 1.0f);
			material->addProperty("occlusion", 1.0f);
			material->addProperty("normalScale", 1.0f);
			material->addProperty("alphaMode", 0);
			material->addProperty("alphaCutOff", 0.5f);
			material->addProperty("padding1", 0);
			material->addProperty("padding2", 0);

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			bool emissive = false;
			if (unityMaterial.keywords.find("_EMISSION") != unityMaterial.keywords.end())
				emissive = true;

			int alphaMode = 0;
			float alphaCutOff = 0.5f;
			if (floatMap.find("_Mode") != floatMap.end())
			{
				float mode = floatMap["_Mode"];
				if (mode == 1.0f)
				{
					if (floatMap.find("_Cutoff") != floatMap.end())
						alphaCutOff = floatMap["_Cutoff"];
					material->setProperty("alphaMode", 1);
					material->setProperty("alphaCutOff", alphaCutOff);
				}
				else if (mode == 2.0f || mode == 3.0f)
				{
					material->setProperty("alphaMode", 2);
					material->setShaderName("UnitySpecGlossTransparency");
					material->setTransparent(true);
				}
			}

			if (floatMap.find("_BumpScale") != floatMap.end())
				material->setProperty("normalScale", floatMap["_BumpScale"]);

			float glossMapScale = 1.0f;
			float glossiness = 1.0f;
			float occlusionStrength = 1.0f;

			if (floatMap.find("_GlossMapScale") != floatMap.end())
				glossMapScale = floatMap["_GlossMapScale"];
			if (floatMap.find("_Glossiness") != floatMap.end())
				glossiness = floatMap["_Glossiness"];
			if (floatMap.find("_OcclusionStrength") != floatMap.end())
				occlusionStrength = floatMap["_OcclusionStrength"];

			material->setProperty("glossiness", glossiness);
			material->setProperty("glossMapScale", glossMapScale);
			material->setProperty("occlusion", occlusionStrength);

			glm::vec2 offset = glm::vec2(0);
			glm::vec2 scale = glm::vec2(1);
			if (texMap.find("_MainTex") != texMap.end())
			{
				offset = texMap["_MainTex"].offset;
				scale = texMap["_MainTex"].scale;
				addTexture(material, texMap["_MainTex"]);
			}
			else
			{
				if (texMap.find("_Albedo") != texMap.end())
				{
					offset = texMap["_Albedo"].offset;
					scale = texMap["_Albedo"].scale;
					addTexture(material, texMap["_Albedo"]);
				}
				else
				{
					material->addTexture("", nullptr);
				}
			}

			// The default shader uses the same tex transform for all main texture maps
			for (auto&& [_, tex] : texMap)
			{
				tex.offset = offset;
				tex.scale = scale;
			}

			if (texMap.find("_SpecGlossMap") != texMap.end())
				addTexture(material, texMap["_SpecGlossMap"]);
			else
				material->addTexture("", nullptr);

			if (texMap.find("_BumpMap") != texMap.end())
			{
				addTexture(material, texMap["_BumpMap"]);
			}
			else
			{
				if (texMap.find("_Normal") != texMap.end())
					addTexture(material, texMap["_Normal"]);
				else
					material->addTexture("", nullptr);
			}

			if (emissive && texMap.find("_EmissionMap") != texMap.end())
				addTexture(material, texMap["_EmissionMap"]);
			else
				material->addTexture("", nullptr);

			if (texMap.find("_OcclusionMap") != texMap.end())
			{
				addTexture(material, texMap["_OcclusionMap"]);
			}
			else
			{
				if (texMap.find("_AmbientOcclusion") != texMap.end())
					addTexture(material, texMap["_AmbientOcclusion"]);
				else
					material->addTexture("", nullptr);
			}

			if (colMap.find("_Color") != colMap.end())
			{
				glm::vec4 rgba = sRGBAlphaToLinear(colMap["_Color"]);
				if (texMap.find("_MainTex") != texMap.end())
					rgba.a = 1.0f;
				material->setProperty("baseColor", rgba);
			}

			if (emissive && colMap.find("_EmissionColor") != colMap.end())
			{
				glm::vec4 emissionColor = sRGBAlphaToLinear(colMap["_EmissionColor"]);
				material->setProperty("emissive", glm::vec4(glm::vec3(emissionColor), 1.0f));
			}

			if (colMap.find("_SpecColor") != colMap.end())
			{
				glm::vec4 rgba = sRGBAlphaToLinear(colMap["_SpecColor"]);
				material->setProperty("specularColor", rgba);
			}

			return material;
		}

		pr::Material::Ptr Materials::loadCustomMaterial(Material& unityMaterial, std::string shaderPath)
		{
			//std::cout << shaderPath << std::endl;

			int idx = (int)shaderPath.find_last_of("/") + 1;
			int len = (int)shaderPath.length();
			std::string shaderFile = shaderPath.substr(idx, len - idx);
			std::string shaderName = "Unity" + shaderFile.substr(0, shaderFile.find_last_of('.'));

			if (shaderFile.compare("Rain_Refraction.shader") == 0 ||
				shaderFile.compare("Glass.shader") == 0)
			{
				shaderName += "Transmission";
			}				

			std::string content = loadTxtFile(shaderPath);
			std::stringstream ss(content);
			std::string line;

			std::vector<UniformProperty> uniforms;
			while (std::getline(ss, line))
			{
				if (line.find("uniform") != std::string::npos)
				{
					//std::cout << line << std::endl;

					std::stringstream lineSS(line);
					UniformProperty uniform;
					std::string name;
					lineSS >> uniform.type >> uniform.type >> name;

					if (lineSS.eof())
						name = name.substr(0, name.length() - 1);
					
					uniform.name = name;
					uniforms.push_back(uniform);
				}					
			}

			std::vector<UniformProperty> sortedUniforms;
			for (auto u : uniforms)
			{
				if (u.type.compare("float4") == 0 || u.type.compare("half4") == 0)
				{
					UniformProperty uniform;
					uniform.name = u.name;
					uniform.type = "float4";
					sortedUniforms.push_back(uniform);
				}					
			}
			int numFloats = 0;
			for (auto u : uniforms)
			{
				if (u.type.compare("float") == 0 || u.type.compare("half") == 0)
				{
					UniformProperty uniform;
					uniform.name = u.name;
					uniform.type = "float";
					sortedUniforms.push_back(uniform);
					numFloats++;
				}
			}
			int numPadding = 4 - numFloats % 4;
			if (numPadding < 4)
			{
				for (int i = 0; i < numPadding; i++)
				{
					UniformProperty uPadding;
					uPadding.name = "padding" + std::to_string(i);
					uPadding.type = "float";
					sortedUniforms.push_back(uPadding);
				}
			}
			for (auto u : uniforms)
				if (u.type.compare("sampler2D") == 0)
					sortedUniforms.push_back(u);

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			auto material = pr::Material::create(unityMaterial.name, shaderName);
			for (auto u : sortedUniforms)
			{
				if (u.type.compare("float4") == 0) // TODO: fetch from color map and 
												   // add texture coords from property def
				{
					if (colMap.find(u.name) != colMap.end())
					{
						material->addProperty(u.name, colMap[u.name]);
					}
					else
					{
						std::string texName = u.name.substr(0, u.name.length() - 3);

						if (texMap.find(texName) != texMap.end())
						{
							glm::vec2 offset = texMap[texName].offset;
							glm::vec2 scale = texMap[texName].scale;
							material->addProperty(u.name, glm::vec4(scale, offset));
						}
						else
						{
							material->addProperty(u.name, glm::vec4(1, 1, 0, 0));
						}
					}
				}
				else if (u.type.compare("float") == 0)
				{
					if (floatMap.find(u.name) != floatMap.end())
					{
						material->addProperty(u.name, floatMap[u.name]);
					}
					else
					{
						material->addProperty(u.name, 1.0f);
					}
				}
				else if (u.type.compare("sampler2D") == 0)
				{
					if (texMap.find(u.name) != texMap.end())
					{
						addTexture(material, texMap[u.name]);
					}
					else
					{
						material->addTexture("", nullptr);
					}
				}
			}

			// TODO: get this from the shader (Cull Off)
			material->setDoubleSided(true);

			if (shaderFile.compare("Rain_Refraction.shader") == 0 ||
				shaderFile.compare("Glass.shader") == 0)
			{
				material->setTransparent(true);
			}

			return material;
		}

		float getDefaultValue(int mode)
		{
			float value = 0.0f;
			switch (mode)
			{
				case 0: value = 1.0f; break; // white
				case 1: value = 0.0f; break; // black
				case 2: value = glm::pow(0.5f, 2.2f); break; // gray
				case 3: value = 0.0f; break; // normal
			}
			return value;
		}

		bool Materials::loadUniformsFromJSON(std::string jsonFileContent, std::vector<UniformProperty>& sortedUniforms)
		{
			std::map<std::string, std::string> jsonDocs;
			std::stringstream ss(jsonFileContent);
			std::string line;
			std::string content = "";
			std::string headerID;
			bool firstID = true;
			bool isDoubleSided = false;
			while (std::getline(ss, line))
			{
				content += line + "\n";
				//std::cout << line << std::endl;
				if (line.length() == 1 && line.compare("}") == 0)
				{
					json::Document document;
					document.Parse(content.c_str());
					std::string objID = document["m_ObjectId"].GetString();
					jsonDocs[objID] = content;
					content = "";

					if (firstID)
					{
						headerID = objID;
						firstID = false;
					}						
				}
			}

			json::Document document;
			document.Parse(jsonDocs[headerID].c_str());
			std::vector<std::string> properties;
			{
				auto arrayNode = document.FindMember("m_Properties");
				for (auto& node : arrayNode->value.GetArray())
				{
					if (node.HasMember("m_Id"))
						properties.push_back(node["m_Id"].GetString());
				}
			}
			std::vector<std::string> keywords;
			{
				auto arrayNode = document.FindMember("m_Keywords");
				for (auto& node : arrayNode->value.GetArray())
				{
					if (node.HasMember("m_Id"))
						keywords.push_back(node["m_Id"].GetString());
				}
			}
			if (document.HasMember("m_ActiveTargets"))
			{
				std::string objectID = document["m_ActiveTargets"][0]["m_Id"].GetString();
				std::string content = jsonDocs[objectID];
				json::Document doc;
				doc.Parse(content.c_str());
				if (doc.HasMember("m_RenderFace"))
				{
					int renderFace = doc["m_RenderFace"].GetInt();
					if (renderFace == 0) // culling disabled
					{
						std::cout << "culling disabled" << std::endl;
						isDoubleSided = true;
					}						
				}
			}

			std::vector<UniformProperty> uniforms;
			//for (int i = 0; i < jsonDocs.size(); i++)

			for (auto propID : properties)
			{
				std::string content = jsonDocs[propID];
				json::Document document;
				document.Parse(content.c_str());

				std::string type = document["m_Type"].GetString();
				std::string name = document["m_OverrideReferenceName"].GetString();
				if (name.empty())
					name = document["m_DefaultReferenceName"].GetString();
				std::string glslType = "";
				if (type.compare("UnityEditor.ShaderGraph.Internal.ColorShaderProperty") == 0)
					glslType = "vec4";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.BooleanShaderProperty") == 0)
					glslType = "bool";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector1ShaderProperty") == 0)
					glslType = "float";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector2ShaderProperty") == 0)
					glslType = "vec2";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector3ShaderProperty") == 0)
					glslType = "vec3";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector4ShaderProperty") == 0)
					glslType = "vec4";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Texture2DShaderProperty") == 0)
					glslType = "TextureInfo";
				else
					std::cout << "error: unknown property type: " << type << std::endl;
				UniformProperty uniform;
				uniform.type = glslType;
				uniform.name = name;
				if (document.HasMember("m_DefaultType"))
					uniform.defaultValue = getDefaultValue(document["m_DefaultType"].GetInt());
				else
					uniform.defaultValue = 0.0f;
				uniforms.push_back(uniform);
			}

			for (auto keyID : keywords)
			{
				std::string content = jsonDocs[keyID];
				json::Document document;
				document.Parse(content.c_str());

				std::string type = document["m_Type"].GetString();
				std::string name = document["m_OverrideReferenceName"].GetString();
				std::string glslType = "";
				if (type.compare("UnityEditor.ShaderGraph.ShaderKeyword") == 0)
					glslType = "bool";
				else
					std::cout << "error: unknown keyword type: " << type << std::endl;
				UniformProperty uniform;
				uniform.type = glslType;
				uniform.name = name.substr(0, name.length() - 3);
				uniform.defaultValue = 0.0f;
				uniforms.push_back(uniform);
			}

			//std::vector<UniformProperty> sortedUniforms;
			for (auto u : uniforms)
				if (u.type.compare("vec4") == 0)
					sortedUniforms.push_back(u);
			int numFloats = 0;
			for (auto u : uniforms)
			{
				if (u.type.compare("float") == 0)
				{
					sortedUniforms.push_back(u);
					numFloats++;
				}
			}
			for (auto u : uniforms)
			{
				if (u.type.compare("bool") == 0)
				{
					sortedUniforms.push_back(u);
					numFloats++;
				}
			}
			int numPadding = 4 - numFloats % 4;
			if (numPadding < 4)
			{
				for (int i = 0; i < numPadding; i++)
				{
					UniformProperty uPadding;
					uPadding.name = "padding" + std::to_string(i);
					uPadding.type = "float";
					uPadding.defaultValue = 0.0f;
					sortedUniforms.push_back(uPadding);
				}
			}
			for (auto u : uniforms)
				if (u.type.compare("TextureInfo") == 0)
					sortedUniforms.push_back(u);

			return isDoubleSided;
		}

		bool Materials::loadUniformsFromJSONSerialized(std::string jsonFileContent, std::vector<UniformProperty>& sortedUniforms)
		{
			json::Document document;
			document.Parse(jsonFileContent.c_str());

			std::vector<UniformProperty> uniforms;
			auto properties = document.FindMember("m_SerializedProperties");
			for (auto& prop : properties->value.GetArray())
			{
				json::Document propDoc;
				std::string jsonPropData = prop["JSONnodeData"].GetString();
				propDoc.Parse(jsonPropData.c_str());
				std::string name = propDoc["m_DefaultReferenceName"].GetString();
				std::string type = prop["typeInfo"]["fullName"].GetString();
				std::string glslType = "";
				if (type.compare("UnityEditor.ShaderGraph.Internal.ColorShaderProperty") == 0)
					glslType = "vec4";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.BooleanShaderProperty") == 0)
					glslType = "bool";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector1ShaderProperty") == 0)
					glslType = "float";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector2ShaderProperty") == 0)
					glslType = "vec2";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector3ShaderProperty") == 0)
					glslType = "vec3";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Vector4ShaderProperty") == 0)
					glslType = "vec4";
				else if (type.compare("UnityEditor.ShaderGraph.Internal.Texture2DShaderProperty") == 0)
					glslType = "TextureInfo";
				else
					std::cout << "error: unknown property type: " << type << std::endl;
				UniformProperty uniform;
				uniform.type = glslType;
				uniform.name = name;
				if (document.HasMember("m_DefaultType"))
					uniform.defaultValue = getDefaultValue(document["m_DefaultType"].GetInt());
				else
					uniform.defaultValue = 0.0f;
				uniforms.push_back(uniform);
			}

			//std::vector<UniformProperty> sortedUniforms;
			for (auto u : uniforms)
				if (u.type.compare("vec4") == 0)
					sortedUniforms.push_back(u);
			for (auto u : uniforms)
				if (u.type.compare("vec2") == 0)
					sortedUniforms.push_back(u);
			int numFloats = 0;
			for (auto u : uniforms)
			{
				if (u.type.compare("float") == 0)
				{
					sortedUniforms.push_back(u);
					numFloats++;
				}
			}
			for (auto u : uniforms)
			{
				if (u.type.compare("bool") == 0)
				{
					sortedUniforms.push_back(u);
					numFloats++;
				}
			}
			int numPadding = 4 - numFloats % 4;
			if (numPadding < 4)
			{
				for (int i = 0; i < numPadding; i++)
				{
					UniformProperty uPadding;
					uPadding.name = "padding" + std::to_string(i);
					uPadding.type = "float";
					uPadding.defaultValue = 0.0f;
					sortedUniforms.push_back(uPadding);
				}
			}
			for (auto u : uniforms)
				if (u.type.compare("TextureInfo") == 0)
					sortedUniforms.push_back(u);
			return false;
		}

		pr::Material::Ptr Materials::loadShadergraphMaterial(Material& unityMaterial, std::string shaderPath)
		{
			int idx = (int)shaderPath.find_last_of("/") + 1;
			int len = (int)shaderPath.length();
			std::string shaderFile = shaderPath.substr(idx, len - idx);
			std::string shaderName = "Unity" + shaderFile.substr(0, shaderFile.find_last_of('.'));

			std::string jsonFileContent = loadTxtFile(shaderPath);
			std::stringstream ss(jsonFileContent);
			std::string line;
			std::getline(ss, line);
			std::getline(ss, line);

			bool isDoubleSided = false;
			std::vector<UniformProperty> uniforms;
			if (line.find("m_SerializedProperties") != std::string::npos)
			{
				isDoubleSided = loadUniformsFromJSONSerialized(jsonFileContent, uniforms);
			}
			else
			{
				isDoubleSided = loadUniformsFromJSON(jsonFileContent, uniforms);
			}

			//for (auto u : sortedUniforms)
			//	std::cout << u.name << " " << u.type << std::endl;

			auto floatMap = unityMaterial.floats;
			auto colMap = unityMaterial.colors;
			auto texMap = unityMaterial.texEnvs;

			auto material = pr::Material::create(unityMaterial.name, shaderName);
			material->setDoubleSided(isDoubleSided);
			for (auto u : uniforms)
			{
				if (u.type.compare("vec4") == 0)
				{
					if (colMap.find(u.name) != colMap.end())
					{
						glm::vec4 rgba = sRGBAlphaToLinear(colMap[u.name]);
						material->addProperty(u.name, rgba);
					}
					else
					{
						std::string texName = u.name.substr(0, u.name.length() - 3);

						if (texMap.find(texName) != texMap.end())
						{
							glm::vec2 offset = texMap[texName].offset;
							glm::vec2 scale = texMap[texName].scale;
							material->addProperty(u.name, glm::vec4(scale, offset));
						}
						else
						{
							material->addProperty(u.name, glm::vec4(1, 1, 0, 0));
						}
					}
				}
				else if (u.type.compare("vec2") == 0)
				{
					if (colMap.find(u.name) != colMap.end())
					{
						glm::vec2 value = glm::vec2(colMap[u.name].x, colMap[u.name].y);
						material->addProperty(u.name, value);
					}
					else
					{
						material->addProperty(u.name, glm::vec2(1, 1));
					}
				}
				else if (u.type.compare("float") == 0)
				{
					if (floatMap.find(u.name) != floatMap.end())
					{
						material->addProperty(u.name, floatMap[u.name]);
					}
					else
					{
						material->addProperty(u.name, 1.0f);
					}
				}
				else if (u.type.compare("bool") == 0)
				{
					if (floatMap.find(u.name) != floatMap.end())
					{
						material->addProperty(u.name, (int)floatMap[u.name]);
					}
					else
					{
						material->addProperty(u.name, 0);
					}
				}
				else if (u.type.compare("TextureInfo") == 0)
				{
					if (texMap.find(u.name) != texMap.end())
					{
						std::cout << "texture map " << u.name << " default value: " << u.defaultValue << std::endl;
						addTexture(material, texMap[u.name], u.defaultValue);
					}
					else
					{
						pr::TextureInfo info;
						info.defaultValue = u.defaultValue;
						material->addTexture(u.name, nullptr, info);
					}
				}
			}

			//std::cout << "loaded " << std::to_string(properties.size()) << std::endl;
			return material;
		}

		pr::Material::Ptr Materials::loadMaterialGUID(const std::string& guid)
		{
			if (metaData.find(guid) == metaData.end())
			{
				std::cout << "error: could not find material with GUID: " << guid << std::endl;
				return nullptr;
			}

			Metadata& data = metaData[guid];
			std::string content = loadTxtFile(data.filePath);
			ryml::Tree root = ryml::parse_in_arena(ryml::to_csubstr(content));
			int matIndex = 0;
			for (int i = 0; i < root.rootref().num_children(); i++) // There can be other objects in a .mat file...
			{
				if (root.docref(i).has_child("Material"))
					matIndex = i;

			}
			ryml::NodeRef stream = root.docref(matIndex);

			std::cout << "loading material " << data.filePath << std::endl;

			Material unityMaterial;
			stream["Material"] >> unityMaterial;
			auto shaderGUID = unityMaterial.shader.guid;
			auto shaderfileID = unityMaterial.shader.fileID;

			pr::Material::Ptr material = nullptr;
			if (shaderGUID.compare("0000000000000000f000000000000000") == 0)
			{
				if (shaderfileID == 45)
					material = loadSpecGlossMaterial(unityMaterial);
				else if (shaderfileID == 46)
					material = loadDefaultMaterial(unityMaterial);
				else
					std::cout << "default shader with fileID " << shaderfileID << " not implemented!" << std::endl;
			}
			else if (shaderGUID.compare("933532a4fcc9baf4fa0491de14d08ed7") == 0)
			{
				std::cout << "URP Lit Shader!" << std::endl;
				material = loadSpecGlossMaterial(unityMaterial);
			}
			else
			{
				if (metaData.find(shaderGUID) != metaData.end())
				{
					Metadata& shaderMetadata = metaData[shaderGUID];
					
					int idx = (int)shaderMetadata.filePath.find_last_of("/") + 1;
					int len = (int)shaderMetadata.filePath.length();
					std::string shaderName = shaderMetadata.filePath.substr(idx, len - idx);
					idx = static_cast<int>(shaderName.find_last_of(".") + 1);
					std::string ext = shaderName.substr(idx, shaderName.length() - idx);

					if (ext.compare("shader") == 0)
						material = loadCustomMaterial(unityMaterial, shaderMetadata.filePath);
					else if (ext.compare("shadergraph") == 0)
					{
						material = loadShadergraphMaterial(unityMaterial, shaderMetadata.filePath);
						//if (shaderName.compare("Lit_SSS_Cutout.shadergraph") == 0)
						//{
						//	material = loadShadergraphMaterial(unityMaterial, shaderMetadata.filePath);
						//}
						////if (shaderName.compare("Standard.shadergraph") == 0 ||
						////	shaderName.compare("Standard_AlphaClip.shadergraph") == 0 ||
						////	shaderName.compare("TerrainSurfaceGraph.shadergraph") == 0)
						////{
						////	material = loadShadergraphMaterial(unityMaterial, shaderMetadata.filePath);
						////}
						//else
						//{
						//	std::cout << "importing shadergraph not implemented! (file: " << shaderName << ")" << std::endl;
						//	material = loadDefaultMaterial(unityMaterial);
						//}
					}
					else
						std::cout << "unknow shader extension: " << ext << std::endl;
				}
				else
				{
					std::cout << "custom shader not found (GUID: " << shaderGUID << "), fallback to default shader!" << std::endl;
					material = loadDefaultMaterial(unityMaterial);
				}
			}

			return material;
		}
	}
}
