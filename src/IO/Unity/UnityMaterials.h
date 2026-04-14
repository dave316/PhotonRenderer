#ifndef INCLUDED_UNITYMATERIALS
#define INCLUDED_UNITYMATERIALS

#pragma once

#include "UnityYAML.h"

#include <Graphics/Material.h>
 
namespace IO
{
	namespace Unity
	{
		struct UniformProperty
		{
			std::string name;
			std::string type;
			float defaultValue;
		};

		struct Metadata
		{
			std::string filePath;
			ryml::Tree root;
		};

		glm::vec3 sRGBToLinear(glm::vec3 sRGB, float gamma = 2.2f);
		glm::vec4 sRGBAlphaToLinear(glm::vec4 sRGBAlpha, float gamma = 2.2f);

		void resizeImageUint8(uint8* src, uint32 srcW, uint32 srcH, uint8* dst, uint32 dstW, uint32 dstH);
		void resizeImageFP32(float* src, uint32 srcW, uint32 srcH, float* dst, uint32 dstW, uint32 dstH);
		std::string loadTxtFile(const std::string& fileName);

		class Materials
		{
		public:
			Materials(uint32 maxTexSize) : maxTexSize(maxTexSize)
			{}
			void setMetadata(std::map<std::string, Metadata>& metaData)
			{
				this->metaData = metaData;
			}

			void addTexture(pr::Material::Ptr material, Texture& texture, float defaultValue = 0.0f, bool loadUncompressed = false);
			pr::Texture2D::Ptr loadTextureGUID(const std::string& guid, bool loadUncompressed);
			pr::Material::Ptr loadDefaultMaterial(Material& unityMaterial);
			pr::Material::Ptr loadSpecGlossMaterial(Material& unityMaterial);
			pr::Material::Ptr loadCustomMaterial(Material& unityMaterial, std::string shaderFile);
			bool loadUniformsFromJSON(std::string jsonFileContent, std::vector<UniformProperty>& uniforms);
			bool loadUniformsFromJSONSerialized(std::string jsonFileContent, std::vector<UniformProperty>& uniforms);
			pr::Material::Ptr loadShadergraphMaterial(Material& unityMaterial, std::string shaderPath);
			pr::Material::Ptr loadMaterialGUID(const std::string& guid);

		private:
			unsigned int maxTexSize = 0;
			std::map<std::string, Metadata> metaData;
			std::map<std::string, int> textures;
			std::map<std::string, pr::Texture2D::Ptr> textureCache;
		};
	}
}

#endif // INCLUDED_UNITYMATERIALS