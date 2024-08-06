#ifndef INCLUDED_UNITYMATERIALS
#define INCLUDED_UNITYMATERIALS

#ifdef WITH_UNITY

#pragma once

#include "UnityYAML.h"

#include <Graphics/Material.h>

namespace IO
{
	namespace Unity
	{
		struct Metadata
		{
			std::string filePath;
			ryml::Tree root;
		};

		class Materials
		{
		private:
			unsigned int maxTexSize = 0;
			std::map<std::string, Metadata> metaData;
			std::map<std::string, int> textures;
			std::map<std::string, Texture2D::Ptr> textureCache;
		public:
			Materials(unsigned int maxTexSize) : maxTexSize(maxTexSize) {}
			void setMetadata(std::map<std::string, Metadata>& metaData) 
			{
				this->metaData = metaData;
			}
			Material::Ptr loadMaterialGUID(const std::string& guid);

			// default shader
			Material::Ptr loadDefaultMaterial(UnityMaterial& unityMaterial);
			Material::Ptr loadSpecGlossMaterial(UnityMaterial& unityMaterial);

			// archviz shader
			Material::Ptr loadLayerMaterial(UnityMaterial& unityMaterial, bool use3Layers);
			Material::Ptr loadRainMaterial(UnityMaterial& unityMaterial);
			Material::Ptr loadRainMaterialPOM(UnityMaterial& unityMaterial);
			Material::Ptr loadRainRefractionMaterial(UnityMaterial& unityMaterial);
			Material::Ptr loadPomMaterial(UnityMaterial& unityMaterial);
			Material::Ptr loadVelvetMaterial(UnityMaterial& unityMaterial);
			Material::Ptr loadGlassMaterial(UnityMaterial& unityMaterial);

			// viking shader
			Material::Ptr loadDefaultShaderGraph(UnityMaterial& unityMaterial);
			Material::Ptr loadTerrainShaderGraph(UnityMaterial& unityMaterial);
			Material::Ptr loadGrassShaderGraph(UnityMaterial& unityMaterial);
			Material::Ptr loadShaderBakedLit(UnityMaterial& unityMaterial);

			void loadTexture(Material::Ptr material, std::string texPrefix, TexEnv& texInfo, bool useTransform = true);
			Texture2D::Ptr loadTextureGUID(const std::string& guid);
			Texture2D::Ptr loadMetalRoughTex(const std::string& guid, std::string& occGuid, float glossMapScale);
		};
	}
}

#endif
#endif // INCLUDED_UNITYMATERIALS
