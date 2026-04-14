#include "UnityTestImporter.h"
#include <Importer/TextureImporter.h>
#include <IO/ImageLoader.h>
#include <fstream>
#include <stb_image_resize2.h>
#include <ktx.h>
#include <rapidjson/document.h>
#include <Utils/IBL.h>
namespace json = rapidjson;
UnityTestImporter::UnityTestImporter()
{

}

UnityTestImporter::~UnityTestImporter()
{

}

pr::Material::Ptr getDefaultMaterial()
{
	auto mat = pr::Material::create("Default", "Default");
	mat->addProperty("baseColor", glm::vec4(1));
	mat->addProperty("emissive", glm::vec4(0));
	mat->addProperty("roughness", 1.0f);
	mat->addProperty("metallic", 0.0f);
	mat->addProperty("occlusion", 1.0f);
	mat->addProperty("normalScale", 1.0f);
	mat->addProperty("alphaMode", 0);
	mat->addProperty("alphaCutOff", 0.5f);
	mat->addProperty("computeFlatNormals", false);
	mat->addProperty("ior", 1.5f);
	std::vector<std::string> texNames = {
		"baseColorTex",
		"normalTex",
		"metalRoughTex",
		"emissiveTex",
		"occlusionTex"
	};
	for (int i = 0; i < texNames.size(); i++)
		mat->addTexture(texNames[i], nullptr);
	return mat;
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

pr::Texture2D::Ptr UnityTestImporter::loadTextureGUID(Unity::YAML::Metadata metadata)
{
	auto texImporter = (Unity::YAML::TextureImporter*)metadata.importer;
	bool sRGB = false;
	unsigned int maxSize = 0;
	for (auto settings : texImporter->platformSettings.value)
	{
		if (settings.buildTarget.value.compare("DefaultTexturePlatform") == 0)
			maxSize = settings.maxTextureSize;
	}
	sRGB = texImporter->mipmaps.value.sRGBTexture;

	//maxSize = 1024; //std::min(maxSize, maxTexSize);

	std::string ext = metadata.extension;

	pr::Texture2D::Ptr tex = nullptr;
	if (ext.compare(".hdr") == 0)
	{
		std::cout << "loading texture " << metadata.filepath << std::endl;

		auto img = IO::ImageLoader::loadFromFile(metadata.filepath);

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
		std::string fn = cachePath + metadata.guid + ".ktx2";
		std::ifstream file(fn);
		if (!file.is_open())
		{
			file.close();

			std::cout << "loading texture " << metadata.filepath << std::endl;

			auto img = IO::ImageLoader::loadFromFile(metadata.filepath);
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

	textureCache.insert(std::make_pair(metadata.guid, tex));
	return tex;
}

void UnityTestImporter::addTexture(pr::Material::Ptr material, Unity::TextureProperty::Ptr textureProp, float defaultValue, bool isMainTex)
{
	pr::Texture2D::Ptr tex = nullptr;
	std::string guid = textureProp->getGUID();

	if (textureCache.find(guid) != textureCache.end())
		tex = textureCache[guid];
	else if (importer.hasMetadata(guid))
	{
		auto metadata = importer.getMetadata(guid);
		tex = loadTextureGUID(metadata);
	}	

	pr::TextureInfo info;
	info.offset = textureProp->getOffset();
	info.scale = textureProp->getScale();
	info.defaultValue = defaultValue;
	info.isMainTexture = isMainTex;
	material->addTexture("", tex, info); // TODO: add uniform name
}

pr::Material::Ptr UnityTestImporter::loadStandardMaterial(Unity::Material::Ptr unityMaterial)
{
	auto material = pr::Material::create(unityMaterial->getName(), "UnityDefault");
	material->addProperty("baseColor", glm::vec4(1.0f));
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
	if (unityMaterial->hasKeyword("_EMISSION"))
		emissive = true;

	int alphaMode = 0;
	float alphaCutOff = 0.0f;
	if (unityMaterial->hasProperty("_Mode"))
	{
		float mode = unityMaterial->getValue<float>("_Mode");
		if (mode == 1.0f)
		{
			if (unityMaterial->hasProperty("_Cutoff"))
				alphaCutOff = unityMaterial->getValue<float>("_Cutoff");
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

	if (unityMaterial->hasProperty("_BumpScale"))
		material->setProperty("normalScale", unityMaterial->getValue<float>("_BumpScale"));

	float glossMapScale = 1.0f;
	float glossiness = 1.0f;
	float metallic = 0.0f;
	float occlusionStrength = 1.0f;

	if (unityMaterial->hasProperty("_GlossMapScale"))
		glossMapScale = unityMaterial->getValue<float>("_GlossMapScale");
	if (unityMaterial->hasProperty("_Glossiness"))
		glossiness = unityMaterial->getValue<float>("_Glossiness");
	if (unityMaterial->hasProperty("_Metallic"))
		metallic = unityMaterial->getValue<float>("_Metallic");
	if (unityMaterial->hasProperty("_OcclusionStrength"))
		occlusionStrength = unityMaterial->getValue<float>("_OcclusionStrength");

	material->setProperty("glossiness", glossiness);
	material->setProperty("glossMapScale", glossMapScale);
	material->setProperty("metallic", metallic);
	material->setProperty("occlusion", occlusionStrength);

	glm::vec2 offset = glm::vec2(0);
	glm::vec2 scale = glm::vec2(1);
	if (auto texProp = unityMaterial->getTexture("_MainTex"))
	{
		addTexture(material, texProp, 0.0f, true);
		offset = texProp->getOffset();
		scale = texProp->getScale();
	}		
	else if (auto texProp = unityMaterial->getTexture("_Albedo"))
	{
		addTexture(material, texProp, 0.0f, true);
		offset = texProp->getOffset();
		scale = texProp->getScale();
	}		
	else
		material->addTexture("", nullptr);

	unityMaterial->setTransform(offset, scale);

	if (auto texProp = unityMaterial->getTexture("_BumpMap"))
		addTexture(material, texProp);
	else if (auto texProp = unityMaterial->getTexture("_Normal"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

	if (auto texProp = unityMaterial->getTexture("_MetallicGlossMap"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

	if (emissive && unityMaterial->getTexture("_EmissionMap"))
	{
		auto texProp = unityMaterial->getTexture("_EmissionMap");
		addTexture(material, texProp);
	}		
	else
		material->addTexture("", nullptr);

	if (auto texProp = unityMaterial->getTexture("_OcclusionMap"))
		addTexture(material, texProp);
	else if (auto texProp = unityMaterial->getTexture("_AmbientOcclusion"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

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

	if (unityMaterial->hasProperty("_Color"))
	{
		auto srgba = unityMaterial->getValue<glm::vec4>("_Color");
		auto rgba = sRGBAlphaToLinear(srgba, 2.2f);
		if (unityMaterial->getTexture("_MainTex"))
			rgba.a = 1.0f;
		material->setProperty("baseColor", rgba);
	}

	if (emissive && unityMaterial->hasProperty("_EmissionColor"))
	{
		auto srgba = unityMaterial->getValue<glm::vec4>("_EmissionColor");
		glm::vec4 emissionColor = sRGBAlphaToLinear(srgba, 2.2f);
		material->setProperty("emissive", glm::vec4(glm::vec3(emissionColor), 1.0f));
	}

	return material;
}

pr::Material::Ptr UnityTestImporter::loadSpecGlossMaterial(Unity::Material::Ptr unityMaterial)
{
	auto material = pr::Material::create(unityMaterial->getName(), "UnitySpecGloss");
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

	bool emissive = false;
	if (unityMaterial->hasKeyword("_EMISSION"))
		emissive = true;

	int alphaMode = 0;
	float alphaCutOff = 0.0f; 
	if (unityMaterial->hasProperty("_Mode"))
	{
		float mode = unityMaterial->getValue<float>("_Mode");
		if (mode == 1.0f)
		{
			if (unityMaterial->hasProperty("_Cutoff"))
				alphaCutOff = unityMaterial->getValue<float>("_Cutoff");
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

	if (unityMaterial->hasProperty("_BumpScale"))
		material->setProperty("normalScale", unityMaterial->getValue<float>("_BumpScale"));

	float glossMapScale = 1.0f;
	float glossiness = 1.0f;
	float occlusionStrength = 1.0f;

	if (unityMaterial->hasProperty("_GlossMapScale"))
		glossMapScale = unityMaterial->getValue<float>("_GlossMapScale");
	if (unityMaterial->hasProperty("_Glossiness"))
		glossiness = unityMaterial->getValue<float>("_Glossiness");
	if (unityMaterial->hasProperty("_OcclusionStrength"))
		occlusionStrength = unityMaterial->getValue<float>("_OcclusionStrength");

	material->setProperty("glossiness", glossiness);
	material->setProperty("glossMapScale", glossMapScale);
	material->setProperty("occlusion", occlusionStrength);

	glm::vec2 offset = glm::vec2(0);
	glm::vec2 scale = glm::vec2(1);
	if (auto texProp = unityMaterial->getTexture("_MainTex"))
	{
		addTexture(material, texProp, 0.0f, true);
		offset = texProp->getOffset();
		scale = texProp->getScale();
	}
	else if (auto texProp = unityMaterial->getTexture("_Albedo"))
	{
		addTexture(material, texProp, 0.0f, true);
		offset = texProp->getOffset();
		scale = texProp->getScale();
	}
	else
		material->addTexture("", nullptr);

	unityMaterial->setTransform(offset, scale);

	if (auto texProp = unityMaterial->getTexture("_SpecGlossMap"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

	if (auto texProp = unityMaterial->getTexture("_BumpMap"))
		addTexture(material, texProp);
	else if (auto texProp = unityMaterial->getTexture("_Normal"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

	if (emissive && unityMaterial->getTexture("_EmissionMap"))
	{
		auto texProp = unityMaterial->getTexture("_EmissionMap");
		addTexture(material, texProp);
	}
	else
		material->addTexture("", nullptr);

	if (auto texProp = unityMaterial->getTexture("_OcclusionMap"))
		addTexture(material, texProp);
	else if (auto texProp = unityMaterial->getTexture("_AmbientOcclusion"))
		addTexture(material, texProp);
	else
		material->addTexture("", nullptr);

	if (unityMaterial->hasProperty("_Color"))
	{
		auto srgba = unityMaterial->getValue<glm::vec4>("_Color");
		auto rgba = sRGBAlphaToLinear(srgba, 2.2f);
		if (unityMaterial->getTexture("_MainTex"))
			rgba.a = 1.0f;
		material->setProperty("baseColor", rgba);
	}

	if (emissive && unityMaterial->hasProperty("_EmissionColor"))
	{
		auto srgba = unityMaterial->getValue<glm::vec4>("_EmissionColor");
		glm::vec4 emissionColor = sRGBAlphaToLinear(srgba, 2.2f);
		material->setProperty("emissive", glm::vec4(glm::vec3(emissionColor), 1.0f));
	}

	if (unityMaterial->hasProperty("_SpecColor"))
	{
		auto srgba = unityMaterial->getValue<glm::vec4>("_SpecColor");
		auto rgba = sRGBAlphaToLinear(srgba, 2.2f);
		material->setProperty("specularColor", rgba);
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

bool UnityTestImporter::loadUniformsFromJSON(std::string jsonFileContent, std::vector<UniformProperty>& sortedUniforms)
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
		if (doc.HasMember("m_TwoSided"))
		{
			isDoubleSided = doc["m_TwoSided"].GetBool();
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

bool UnityTestImporter::loadUniformsFromJSONSerialized(std::string jsonFileContent, std::vector<UniformProperty>& sortedUniforms)
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

pr::Material::Ptr UnityTestImporter::loadCustomMaterial(Unity::Material::Ptr unityMaterial, std::string shaderPath)
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

	std::string content = Unity::YAML::loadTxtFile(shaderPath);
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

	//auto floatMap = unityMaterial.floats;
	//auto colMap = unityMaterial.colors;
	//auto texMap = unityMaterial.texEnvs;

	auto material = pr::Material::create(unityMaterial->getName(), shaderName);
	for (auto u : sortedUniforms)
	{
		if (u.type.compare("float4") == 0) 
		{
			if (unityMaterial->hasProperty(u.name))
			{
				auto color = unityMaterial->getValue<glm::vec4>(u.name);
				material->addProperty(u.name, color);
			}
			else
			{
				std::string texName = u.name.substr(0, u.name.length() - 3);

				if (unityMaterial->getTexture(texName))
				{
					auto texProp = unityMaterial->getTexture(texName);
					glm::vec2 offset = texProp->getOffset();
					glm::vec2 scale = texProp->getScale();
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
			if (unityMaterial->hasProperty(u.name))
			{
				auto value = unityMaterial->getValue<float>(u.name);
				material->addProperty(u.name, value);
			}
			else
			{
				material->addProperty(u.name, 1.0f);
			}
		}
		else if (u.type.compare("sampler2D") == 0)
		{
			if (unityMaterial->getTexture(u.name))
			{
				addTexture(material, unityMaterial->getTexture(u.name));
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

pr::Material::Ptr UnityTestImporter::loadShadergraphMaterial(Unity::Material::Ptr unityMaterial, std::string shaderPath)
{
	int idx = (int)shaderPath.find_last_of("/") + 1;
	int len = (int)shaderPath.length();
	std::string shaderFile = shaderPath.substr(idx, len - idx);
	std::string shaderName = "Unity" + shaderFile.substr(0, shaderFile.find_last_of('.'));
	//if (shaderName.compare("UnityLit_SSS_Cutout") == 0)
	//	std::cout << "TOP LEL" << std::endl;
	std::string jsonFileContent = Unity::YAML::loadTxtFile(shaderPath);
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

	//auto floatMap = unityMaterial.floats;
	//auto colMap = unityMaterial.colors;
	//auto texMap = unityMaterial.texEnvs;

	auto material = pr::Material::create(unityMaterial->getName(), shaderName);
	material->setDoubleSided(isDoubleSided);

	if (unityMaterial->hasProperty("_AlphaCutoffEnable"))
	{
		if (unityMaterial->getValue<float>("_AlphaCutoffEnable") > 0.0f)
			material->setAlphaMode(1, 0.5f);
	}		

	for (auto u : uniforms)
	{
		if (u.type.compare("vec4") == 0)
		{
			if (unityMaterial->hasProperty(u.name))
			//if (colMap.find(u.name) != colMap.end())
			{
				//glm::vec4 rgba = sRGBAlphaToLinear(colMap[u.name]);
				auto color = unityMaterial->getValue<glm::vec4>(u.name);
				material->addProperty(u.name, color);
			}
			else
			{
				std::string texName = u.name.substr(0, u.name.length() - 3);

				if (unityMaterial->getTexture(texName))
				//if (texMap.find(texName) != texMap.end())
				{
					auto texProp = unityMaterial->getTexture(texName);
					glm::vec2 offset = texProp->getOffset();
					glm::vec2 scale = texProp->getScale();
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
			if (unityMaterial->hasProperty(u.name))
			{
				auto color = unityMaterial->getValue<glm::vec4>(u.name);
				glm::vec2 value = glm::vec2(color.x, color.y);
				material->addProperty(u.name, value);
			}
			else
			{
				material->addProperty(u.name, glm::vec2(1, 1));
			}
		}
		else if (u.type.compare("float") == 0)
		{
			if (unityMaterial->hasProperty(u.name))
			{
				float value = unityMaterial->getValue<float>(u.name);
				material->addProperty(u.name, value);
			}
			else
			{
				material->addProperty(u.name, 1.0f);
			}
		}
		else if (u.type.compare("bool") == 0)
		{
			if (unityMaterial->hasProperty(u.name))
			{
				float value = unityMaterial->getValue<float>(u.name);
				material->addProperty(u.name, (int)value);
			}
			else
			{
				material->addProperty(u.name, 0);
			}
		}
		else if (u.type.compare("TextureInfo") == 0)
		{
			if (unityMaterial->getTexture(u.name))
			{
				// TODO: quick hack to mark the main texture from shader graph material
				//		 this is important so the alpha mask gets passed to the shadow pass
				//		 check if there is any other way to identify the main texture from unity...
				bool isMainTex = false;
				if (u.name.compare("_MainTex") == 0 ||
					u.name.compare("_Albedo") == 0 ||
					u.name.compare("_BaseColorMap") == 0)
					isMainTex = true;

				auto texProp = unityMaterial->getTexture(u.name);
				addTexture(material, texProp, u.defaultValue, isMainTex);
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

pr::Material::Ptr UnityTestImporter::loadMaterial(Unity::Material::Ptr unityMaterial)
{
	//std::cout << "loading material " << unityMaterial->getName() << std::endl;

	if (unityMaterial == nullptr)
		return getDefaultMaterial();

	std::string shaderFile = unityMaterial->getShaderFile();
	if (shaderFile.compare("UnityDefault") == 0)
		return loadStandardMaterial(unityMaterial);
	else if (shaderFile.compare("UnitySpecGloss") == 0)
		return loadSpecGlossMaterial(unityMaterial);
	else
	{
		int idx = (int)shaderFile.find_last_of("/") + 1;
		int len = (int)shaderFile.length();
		std::string shaderName = shaderFile.substr(idx, len - idx);
		idx = static_cast<int>(shaderName.find_last_of(".") + 1);
		std::string ext = shaderName.substr(idx, shaderName.length() - idx);

		if (ext.compare("shader") == 0)
			return loadCustomMaterial(unityMaterial, shaderFile);
		else if (ext.compare("shadergraph") == 0)
			return loadShadergraphMaterial(unityMaterial, shaderFile);
		else
			std::cout << "unknow shader file extension: " << ext << std::endl;
	}
}

pr::TextureCubeMap::Ptr loadReflectionProbe(std::string fn)
{
	auto img = IO::ImageLoader::loadEXRFromFile(fn);
	uint32 w = img->getWidth();
	uint32 h = img->getHeight();
	float* data = (float*)img->getRawPtr();

	uint32 levels = static_cast<uint32>(std::floor(std::log2(h))) + 1;
	auto reflectionProbe = pr::TextureCubeMap::create(h, GPU::Format::RGBA32F, levels, GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled);
	reflectionProbe->setAddressMode(GPU::AddressMode::ClampToEdge);

	uint32 faceImgSize = h * h * 4;
	for (uint32 face = 0; face < 6; face++)
	{
		uint32 faceOffset = face * h;
		float* faceBuffer = new float[faceImgSize];
		for (uint32 y = 0; y < h; y++)
			for (uint32 x = 0; x < h; x++)
				for (uint32 c = 0; c < 4; c++) // need to flip y.. but why?
					faceBuffer[y * h * 4 + x * 4 + c] = data[(h - y - 1) * w * 4 + (faceOffset + x) * 4 + c];

		// swap top and bottom faces... thanks unity
		if (face == 2)
			reflectionProbe->upload((uint8*)faceBuffer, faceImgSize * sizeof(float), 3);
		else if (face == 3)
			reflectionProbe->upload((uint8*)faceBuffer, faceImgSize * sizeof(float), 2);
		else reflectionProbe->upload((uint8*)faceBuffer, faceImgSize * sizeof(float), face);

		delete[] faceBuffer;
	}

	return reflectionProbe;
}

pr::Entity::Ptr UnityTestImporter::traverse(Unity::GameObject::Ptr gameObject, pr::Entity::Ptr parent)
{
	auto entity = pr::Entity::create(gameObject->getName(), parent);
	entity->setActive(gameObject->isActive());

	if (gameObject->getComponent<Unity::Transform>())
	{
		auto unityTransform = gameObject->getComponent<Unity::Transform>();
		auto transform = entity->getComponent<pr::Transform>();
		glm::vec3 pos = unityTransform->getLocalPosition();
		glm::quat rot = unityTransform->getLocalRotation();
		glm::vec3 scale = unityTransform->getLocalScale();

		// TODO: this is wrong! Assimp loads everything in right-handed coords
		//		 while Unity uses left-handed coords. So we cannot convert all Transforms.
		//		 We have to flip only the Unity ones!!!
		//// Unity to OpenGL convertion
		//pos.x = -pos.x;
		//rot.y = -rot.y;
		//rot.z = -rot.z;

		transform->setLocalPosition(pos);
		transform->setLocalRotation(rot);
		transform->setLocalScale(scale);
	}
	
	if (gameObject->getComponent<Unity::MeshRenderer>() && gameObject->getComponent<Unity::MeshFilter>())
	{
		auto mr = gameObject->getComponent<Unity::MeshRenderer>();
		auto mf = gameObject->getComponent<Unity::MeshFilter>();

		auto unityMesh = mf->getMesh();

		if (unityMesh)
		{
			if (unityMesh->subMeshes.size() != mr->getNumMaterials())
				std::cout << "error: sub meshes and materials don't match!" << std::endl;

			auto name = unityMesh->getName();
			auto mesh = pr::Mesh::create(unityMesh->getName());
			bool isTransparent = false;
			for (int m = 0; m < mr->getNumMaterials(); m++)
			{
				if (m < unityMesh->subMeshes.size())
				{
					Unity::SubMeshInfo& smInfo = unityMesh->subMeshes[m];
					TriangleSurface surface;
					for (int i = 0; i < smInfo.vertexCount; i++)
					{
						uint32 idx = smInfo.firstVertex + i;
						Vertex v;
						v.position = unityMesh->vertices[idx];
						if (i < unityMesh->colors.size())
							v.color = unityMesh->colors[idx];
						if (i < unityMesh->normals.size())
							v.normal = unityMesh->normals[idx];
						if (i < unityMesh->uv1.size())
							v.texCoord0 = unityMesh->uv1[idx];
						if (i < unityMesh->uv2.size())
							v.texCoord1 = unityMesh->uv2[idx];
						if (i < unityMesh->tangents.size())
							v.tangent = unityMesh->tangents[idx];
						surface.vertices.push_back(v);
					}
					for (int i = 0; i < smInfo.indexCount; i++)
					{
						uint32 idx = smInfo.indexStart + i;
						surface.indices.push_back(unityMesh->triangles[idx] - smInfo.firstVertex);
					}

					pr::SubMesh s;
					s.primitive = pr::Primitive::create(name, surface, GPU::Topology::Triangles);
					s.primitive->createData();
					s.primitive->uploadData();
					s.material = loadMaterial(mr->getMaterial(m));

					if (s.material->isTransparent())
						isTransparent = true;

					mesh->addSubMesh(s);
				}
				else
				{
					std::cout << "error: no submesh for material index " << m << std::endl;
				}
			}

			auto r = pr::Renderable::create(mesh);
			if (isTransparent)
				r->setType(pr::RenderType::Transparent);
			r->setEnabled(mr->isEnabled());
			r->setDiffuseMode(mr->getDiffuseMode());
			r->setLightMapIndex(mr->getLMIndex());
			r->setLightMapST(mr->getLMOffset(), mr->getLMScale());
			r->setReflectionProbe(mr->getReflName(), mr->getRPIndex());
			entity->addComponent(r);
		}
	}
	
	if (gameObject->getComponent<Unity::ReflectionProbe>())
	{
		auto rp = gameObject->getComponent<Unity::ReflectionProbe>();
		glm::vec3 boxSize = rp->getBoxSize();
		glm::vec3 boxOffset = rp->getBoxOffset();
		boxOffset.x = -boxOffset.x;

		glm::vec3 boxMin = boxOffset - boxSize * 0.5f;
		glm::vec3 boxMax = boxOffset + boxSize * 0.5f;
		Box box(boxMin, boxMax);

		std::string fn = rp->getSourceFile();
		std::cout << "loading reflection probe " << fn << std::endl;

		auto cubemap = loadReflectionProbe(fn);
		auto probe = pr::LightProbe::create(cubemap, box);
		entity->addComponent(probe);
	}

	if (gameObject->getComponent<Unity::Light>())
	{
		auto l = gameObject->getComponent<Unity::Light>();
		switch (l->getType())
		{
			case 1:
			{
				auto color = sRGBToLinear(l->getColor(), 2.2f);
				auto light = pr::Light::create(pr::LightType::DIRECTIONAL, color, l->getIntensity(), l->getRange());
				entity->addComponent(light);
				break;
			}
			default:
				std::cout << "not supported light type: " << std::endl;
		}
	}

	auto unityTransform = gameObject->getComponent<Unity::Transform>();
	for (uint32 i = 0; i < unityTransform->numChildren(); i++)
	{
		auto unityChildTransform = unityTransform->getChild(i);
		auto unityChildGameObject = unityChildTransform->getGameObject();
		auto child = traverse(unityChildGameObject, entity);
		entity->addChild(child);
	}

	return entity;
}

pr::Scene::Ptr UnityTestImporter::importScene(const std::string& assetPath, const std::string& sceneFile)
{	
	auto unityScene = importer.importScene(assetPath, sceneFile);

	auto scene = pr::Scene::create("UnityScene");
	for (uint32 i = 0; i < unityScene->numRootNodes(); i++)
	{
		auto rootGO = unityScene->getRootNode(i);
		std::cout << "parsing root node " << rootGO->getName() << std::endl;
		auto rootEntity = traverse(rootGO, nullptr);
		scene->addRoot(rootEntity);

		for (auto e : rootEntity->getChildrenWithComponent<pr::Renderable>())
		{
			std::string name = e->getName();
			std::string lod = name.substr(name.length() - 4, 4);

			if (lod.compare("LOD1") == 0 ||
				lod.compare("LOD2") == 0)
				e->setActive(false);
		}
	}

	loadLightMaps(scene);
	loadDirectionMaps(scene);
	getLightProbes(scene);

	auto unitySkyboxMat = unityScene->getSkyboxMaterial();
	float rotation = unitySkyboxMat->getValue<float>("_Rotation");
	auto texProp = unitySkyboxMat->getTexture("_Tex");
	auto texGuid = texProp->getGUID();
	auto texMetadata = importer.getMetadata(texGuid);
	std::string envFn = texMetadata.filepath;

	auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
	uint32 width = panoImg->getWidth();
	uint32 height = panoImg->getHeight();
	uint8* data = panoImg->getRawPtr();
	uint32 dataSize = width * height * sizeof(float) * 4;
	auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
	panoTex->upload(data, dataSize);
	panoTex->createData();
	panoTex->uploadData();
	auto skybox = IBL::convertEqui2CM(panoTex, 1024, rotation);
	scene->setSkybox(skybox);

	return scene;
}

pr::Entity::Ptr UnityTestImporter::importPrefab(const std::string& assetPath, const std::string& prefabFile)
{
	importer.loadMetadata(assetPath); // TODO: this needs to be done only once!
	auto prefab = importer.importPrefab(assetPath + "/" + prefabFile);
	auto root = traverse(prefab, nullptr);
	return root;
}

void UnityTestImporter::loadLightMaps(pr::Scene::Ptr scene)
{
	uint32 numImages = (uint32)importer.lightData.lightMaps.size();
	uint32 size = 2048;
	uint32 imageSize = size * size * 4;
	float* buffer = new float[numImages * imageSize];

	for (uint32 i = 0; i < numImages; i++)
	{
		auto guid = importer.lightData.lightMaps[i];
		if (importer.hasMetadata(guid))
		{
			auto metadata = importer.getMetadata(guid);
			std::cout << "loading lightmap " << metadata.filepath << std::endl;

			//uint32 w, h;
			//auto dataPtr = decodeEXRFromFile(metaData[guid].filePath, w, h);
			auto img = IO::ImageLoader::loadEXRFromFile(metadata.filepath);
			uint32 w = img->getWidth();
			uint32 h = img->getHeight();
			float* dataPtr = (float*)img->getRawPtr();

			float* newDataPtr = new float[size * size * 4];
			resizeImageFP32(dataPtr, w, h, newDataPtr, size, size);

			//delete[] dataPtr;
			dataPtr = newDataPtr;

			uint32 offset = i * imageSize;
			std::memcpy(buffer + offset, dataPtr, imageSize * sizeof(float));

			delete[] dataPtr;
		}
	}

	GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
	auto lightMaps = pr::Texture2DArray::create(size, size, numImages, GPU::Format::RGBA32F, 1, flags);
	lightMaps->upload((uint8*)buffer, numImages * imageSize * sizeof(float));

	delete[] buffer;

	scene->setLightMaps(lightMaps);
}

void UnityTestImporter::loadDirectionMaps(pr::Scene::Ptr scene)
{
	uint32 numImages = (uint32)importer.lightData.directionMaps.size();
	uint32 size = 2048;
	uint32 imageSize = size * size * 4;
	uint8* buffer = new uint8[numImages * imageSize];

	for (uint32 i = 0; i < numImages; i++)
	{
		auto guid = importer.lightData.directionMaps[i];
		if (importer.hasMetadata(guid))
		{
			auto metadata = importer.getMetadata(guid);
			std::cout << "loading direction map " << metadata.filepath << std::endl;

			//uint32 w, h;
			//auto dataPtr = loadFromFile(metaData[guid].filePath, w, h);

			auto img = IO::ImageLoader::loadFromFile(metadata.filepath);
			uint32 w = img->getWidth();
			uint32 h = img->getHeight();
			uint8* dataPtr = img->getRawPtr();

			uint8* newDataPtr = new uint8[size * size * 4];
			resizeImageUint8(dataPtr, w, h, newDataPtr, size, size);
			dataPtr = newDataPtr;

			uint32 offset = i * imageSize;
			std::memcpy(buffer + offset, dataPtr, imageSize);

			delete[] dataPtr;
		}
	}

	GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
	auto dirMaps = pr::Texture2DArray::create(size, size, numImages, GPU::Format::RGBA8, 1, flags);
	dirMaps->upload((uint8*)buffer, numImages * imageSize);

	delete[] buffer;

	//return lightMaps;

	scene->setDirMaps(dirMaps);
}

void UnityTestImporter::getLightProbes(pr::Scene::Ptr scene)
{
	pr::SHLightProbes probes;
	for (auto t : importer.lightData.tetrahedras)
	{
		IO::Unity::Tetrahedron tetra;
		for (int i = 0; i < 4; i++)
		{
			tetra.indices[i] = t.indices[i];
			tetra.neighbors[i] = t.neighbors[i];
		}
		tetra.matrix = t.matrix;
		probes.tetrahedras.push_back(tetra);
	}
	for (auto& sh9 : importer.lightData.probeCoeffs)
	{
		IO::Unity::SH9 sh;
		for (int i = 0; i < 27; i++)
			sh.coefficients[i] = sh9.coefficients[i];
		probes.coeffs.push_back(sh);
	}
	probes.positions = importer.lightData.probePositions;
	scene->setSHProbes(probes);
}