#ifndef INCLUDED_TEXTUREIMPORTER
#define INCLUDED_TEXTUREIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		struct Mipmaps
		{
		public:
			Mipmaps();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Mipmaps& value)
			{
				value.print("    ");
				return stream;
			}

		//private:
			YamlField<uint32> mipMapMode = { "mipMapMode", 0 };
			YamlField<bool> enableMipMap = { "enableMipMap", false };
			YamlField<bool> sRGBTexture = { "sRGBTexture", false };
			YamlField<bool> linearTexture = { "linearTexture", false };
			YamlField<bool> fadeOut = { "fadeOut", false };
			YamlField<bool> borderMipMap = { "borderMipMap", false };
			YamlField<bool> mipMapsPreserveCoverage = { "mipMapsPreserveCoverage", false };
			YamlField<float> alphaTestReferenceValue = { "alphaTestReferenceValue", 0.0f };
			YamlField<float> mipMapFadeDistanceStart = { "mipMapFadeDistanceStart", 0.0f };
			YamlField<float> mipMapFadeDistanceEnd = { "mipMapFadeDistanceEnd", 0.0f };
		};

		struct Bumpmap
		{
		public:
			Bumpmap();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Bumpmap& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<bool> convertToNormalMap = { "convertToNormalMap", false };
			YamlField<bool> externalNormalMap = { "externalNormalMap", false };
			YamlField<float> heightScale = { "heightScale", 0.0f };
			YamlField<uint32> normalMapFilter = { "normalMapFilter", 0 };
		};

		struct TextureSettings
		{
		public:
			TextureSettings();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, TextureSettings& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<int32> filterMode = { "filterMode", 0 };
			YamlField<int32> aniso = { "aniso", 0 };
			YamlField<int32> mipBias = { "mipBias", 0 };
			YamlField<int32> wrapU = { "wrapU", 0 };
			YamlField<int32> wrapV = { "wrapV", 0 };
			YamlField<int32> wrapW = { "wrapW", 0 };
		};

		struct PlatformSettings
		{
		public:
			PlatformSettings();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, PlatformSettings& value)
			{
				value.print("    ");
				return stream;
			}

		//private:
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<std::string> buildTarget = { "buildTarget", "" };
			YamlField<uint32> maxTextureSize = { "maxTextureSize", 0 };
			YamlField<uint32> resizeAlgorithm = { "resizeAlgorithm", 0 };
			YamlField<int32> textureFormat = { "textureFormat", 0 };
			YamlField<uint32> textureCompression = { "textureCompression", 0 };
			YamlField<uint32> compressionQuality = { "compressionQuality", 0 };
			YamlField<bool> crunchedCompression = { "crunchedCompression", false };
			YamlField<bool> allowsAlphaSplitting = { "allowsAlphaSplitting", false };
			YamlField<bool> overridden = { "overridden", false };
			YamlField<bool> androidETC2FallbackOverride = { "androidETC2FallbackOverride", false };
			YamlField<bool> forceMaximumCompressionQuality_BC6H_BC7 = { "forceMaximumCompressionQuality_BC6H_BC7", false };
		};

		class TextureImporter : public Importer
		{
		public:
			TextureImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			const std::string name = "TextureImporter";
			YamlField<std::map<int64, std::string>> fileIDToRecycleName = { "fileIDToRecycleName", std::map<int64, std::string>() };
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<Mipmaps> mipmaps = { "mipmaps", {} };
			YamlField<Bumpmap> bumpmap = { "bumpmap", {} };
			YamlField<bool> isReadable = { "isReadable", false };
			YamlField<bool> streamingMipmaps = { "streamingMipmaps", false };
			YamlField<uint32> streamingMipmapsPriority = { "streamingMipmapsPriority", 0 };
			YamlField<bool> vTOnly = { "vTOnly", false };
			YamlField<bool> grayScaleToAlpha = { "grayScaleToAlpha", false };
			YamlField<uint32> generateCubemap = { "generateCubemap", 0 };
			YamlField<uint32> cubemapConvolution = { "cubemapConvolution", 0 };
			YamlField<bool> seamlessCubemap = { "seamlessCubemap", false };
			YamlField<int32> textureFormat = { "textureFormat", 0 };
			YamlField<uint32> maxTextureSize = { "maxTextureSize", 0 };
			YamlField<TextureSettings> textureSettings = { "textureSettings", {} };
			YamlField<uint32> nPOTScale = { "nPOTScale", 0 };
			YamlField<bool> lightmap = { "lightmap", false };
			YamlField<uint32> compressionQuality = { "compressionQuality", 0 };
			YamlField<uint32> spriteMode = { "spriteMode", 0 };
			YamlField<uint32> spriteExtrude = { "spriteExtrude", 0 };
			YamlField<uint32> spriteMeshType = { "spriteMeshType", 0 };
			YamlField<uint32> alignment = { "alignment", 0 };
			YamlField<glm::vec2> spritePivot = { "spritePivot", glm::vec2(0) };
			YamlField<uint32> spritePixelsToUnits = { "spritePixelsToUnits", 0 };
			YamlField<glm::vec4> spriteBorder = { "spriteBorder", glm::vec4(0) };
			YamlField<bool> spriteGenerateFallbackPhysicsShape = { "spriteGenerateFallbackPhysicsShape", false };
			YamlField<uint32> alphaUsage = { "alphaUsage", 0 };
			YamlField<bool> alphaIsTransparency = { "alphaIsTransparency", false };
			YamlField<int32> spriteTessellationDetail = { "spriteTessellationDetail", 0 };
			YamlField<uint32> textureType = { "textureType", 0 };
			YamlField<uint32> textureShape = { "textureShape", 0 };
			YamlField<uint32> singleChannelComponent = { "singleChannelComponent", 0 };
			YamlField<uint32> flipbookRows = { "flipbookRows", 0 };
			YamlField<uint32> flipbookColumns = { "flipbookColumns", 0 };
			YamlField<uint32> maxTextureSizeSet = { "maxTextureSizeSet", 0 };
			YamlField<uint32> compressionQualitySet = { "compressionQualitySet", 0 };
			YamlField<uint32> textureFormatSet = { "textureFormatSet", 0 };
			YamlField<bool> ignorePngGamma = { "ignorePngGamma", false };
			YamlField<bool> applyGammaDecoding = { "applyGammaDecoding", false };
			YamlField<uint32> cookieLightType = { "cookieLightType", 0 };
			YamlField<std::vector<PlatformSettings>> platformSettings = { "platformSettings", std::vector<PlatformSettings>() };
			// TODO: add spriteSheet.. no idea what the contents are!
			YamlField<std::string> spritePackingTag = { "spritePackingTag", "" };
			YamlField<bool> pSDRemoveMatte = { "pSDRemoveMatte", false };
			YamlField<bool> pSDShowRemoveMatteOption = { "pSDShowRemoveMatteOption", false };
		};

		bool read(ryml::ConstNodeRef n, Mipmaps* value);
		bool read(ryml::ConstNodeRef n, Bumpmap* value);
		bool read(ryml::ConstNodeRef n, TextureSettings* value);
		bool read(ryml::ConstNodeRef n, PlatformSettings* value);
	}
}

#endif // INCLUDED_TEXTUREIMPORTER