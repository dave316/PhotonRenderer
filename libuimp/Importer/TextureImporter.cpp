#include "TextureImporter.h"

namespace Unity
{
	namespace YAML
	{
		Mipmaps::Mipmaps()
		{

		}

		void Mipmaps::fromYAML(ryml::ConstNodeRef n)
		{
			n[mipMapMode.key] >> mipMapMode.value;
			n[enableMipMap.key] >> enableMipMap.value;
			n[sRGBTexture.key] >> sRGBTexture.value;
			n[linearTexture.key] >> linearTexture.value;
			n[fadeOut.key] >> fadeOut.value;
			n[borderMipMap.key] >> borderMipMap.value;
			n[mipMapsPreserveCoverage.key] >> mipMapsPreserveCoverage.value;
			n[alphaTestReferenceValue.key] >> alphaTestReferenceValue.value;
			n[mipMapFadeDistanceStart.key] >> mipMapFadeDistanceStart.value;
			n[mipMapFadeDistanceEnd.key] >> mipMapFadeDistanceEnd.value;
		}

		void Mipmaps::print(const std::string& prefix)
		{
			std::cout << std::endl;
			mipMapMode.print(prefix);
			enableMipMap.print(prefix);
			sRGBTexture.print(prefix);
			linearTexture.print(prefix);
			fadeOut.print(prefix);
			borderMipMap.print(prefix);
			mipMapsPreserveCoverage.print(prefix);
			alphaTestReferenceValue.print(prefix);
			mipMapFadeDistanceStart.print(prefix);
			std::cout << prefix << mipMapFadeDistanceEnd.key << ": " << mipMapFadeDistanceEnd.value;
		}

		Bumpmap::Bumpmap()
		{

		}

		void Bumpmap::fromYAML(ryml::ConstNodeRef n)
		{
			n[convertToNormalMap.key] >> convertToNormalMap.value;
			n[externalNormalMap.key] >> externalNormalMap.value;
			n[heightScale.key] >> heightScale.value;
			n[normalMapFilter.key] >> normalMapFilter.value;
		}

		void Bumpmap::print(const std::string& prefix)
		{
			std::cout << std::endl;
			convertToNormalMap.print(prefix);
			externalNormalMap.print(prefix);
			heightScale.print(prefix);
			std::cout << prefix << normalMapFilter.key << ": " << normalMapFilter.value;
		}

		TextureSettings::TextureSettings()
		{

		}

		void TextureSettings::fromYAML(ryml::ConstNodeRef n)
		{
			n[serializedVersion.key] >> serializedVersion.value;
			n[filterMode.key] >> filterMode.value;
			n[aniso.key] >> aniso.value;
			n[mipBias.key] >> mipBias.value;
			n[wrapU.key] >> wrapU.value;
			n[wrapV.key] >> wrapV.value;
			n[wrapW.key] >> wrapW.value;
		}

		void TextureSettings::print(const std::string& prefix)
		{
			std::cout << std::endl;
			serializedVersion.print(prefix);
			filterMode.print(prefix);
			aniso.print(prefix);
			mipBias.print(prefix);
			wrapU.print(prefix);
			wrapV.print(prefix);
			std::cout << prefix << wrapW.key << ": " << wrapW.value;
		}

		PlatformSettings::PlatformSettings()
		{

		}

		void PlatformSettings::fromYAML(ryml::ConstNodeRef n)
		{
			if (serializedVersion.isPresent = n.has_child(serializedVersion.key))
				n[serializedVersion.key] >> serializedVersion.value;
			n[buildTarget.key] >> buildTarget.value;
			n[maxTextureSize.key] >> maxTextureSize.value;
			n[resizeAlgorithm.key] >> resizeAlgorithm.value;
			n[textureFormat.key] >> textureFormat.value;
			n[textureCompression.key] >> textureCompression.value;
			n[compressionQuality.key] >> compressionQuality.value;
			n[crunchedCompression.key] >> crunchedCompression.value;
			n[allowsAlphaSplitting.key] >> allowsAlphaSplitting.value;
			n[overridden.key] >> overridden.value;
			n[androidETC2FallbackOverride.key] >> androidETC2FallbackOverride.value;
			if (forceMaximumCompressionQuality_BC6H_BC7.isPresent = n.has_child(forceMaximumCompressionQuality_BC6H_BC7.key))
				n[forceMaximumCompressionQuality_BC6H_BC7.key] >> forceMaximumCompressionQuality_BC6H_BC7.value;
		}

		void PlatformSettings::print(const std::string& prefix)
		{
			serializedVersion.print("");
			buildTarget.print(prefix);
			maxTextureSize.print(prefix);
			resizeAlgorithm.print(prefix);
			textureFormat.print(prefix);
			textureCompression.print(prefix);
			compressionQuality.print(prefix);
			crunchedCompression.print(prefix);
			allowsAlphaSplitting.print(prefix);
			overridden.print(prefix);
			androidETC2FallbackOverride.print(prefix);
			forceMaximumCompressionQuality_BC6H_BC7.print(prefix);
		}

		TextureImporter::TextureImporter()
		{
		}

		void TextureImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			if (n.has_child(fileIDToRecycleName.key))
				n[fileIDToRecycleName.key] >> fileIDToRecycleName.value; 
			n[serializedVersion.key] >> serializedVersion.value;
			n[mipmaps.key] >> mipmaps.value;
			n[bumpmap.key] >> bumpmap.value;
			n[isReadable.key] >> isReadable.value;
			if (streamingMipmaps.isPresent = n.has_child(streamingMipmaps.key))
				n[streamingMipmaps.key] >> streamingMipmaps.value;
			if (streamingMipmapsPriority.isPresent = n.has_child(streamingMipmapsPriority.key))
				n[streamingMipmapsPriority.key] >> streamingMipmapsPriority.value; 
			if (vTOnly.isPresent = n.has_child(vTOnly.key))
				n[vTOnly.key] >> vTOnly.value;
			n[grayScaleToAlpha.key] >> grayScaleToAlpha.value;
			n[generateCubemap.key] >> generateCubemap.value;
			n[cubemapConvolution.key] >> cubemapConvolution.value;
			n[seamlessCubemap.key] >> seamlessCubemap.value; 
			n[textureFormat.key] >> textureFormat.value;
			n[maxTextureSize.key] >> maxTextureSize.value;
			n[textureSettings.key] >> textureSettings.value;
			n[nPOTScale.key] >> nPOTScale.value;
			n[lightmap.key] >> lightmap.value;
			n[compressionQuality.key] >> compressionQuality.value;
			n[spriteMode.key] >> spriteMode.value;
			n[spriteExtrude.key] >> spriteExtrude.value;
			n[spriteMeshType.key] >> spriteMeshType.value;
			n[alignment.key] >> alignment.value;
			read(n[spritePivot.key], spritePivot.value);
			n[spritePixelsToUnits.key] >> spritePixelsToUnits.value;
			// TODO: need to differentiate between color and vector! (.xyzw vs .rgba)
			//read(n[spriteBorder.key], spriteBorder.value);
			if (n.has_child(spriteGenerateFallbackPhysicsShape.key))
				n[spriteGenerateFallbackPhysicsShape.key] >> spriteGenerateFallbackPhysicsShape.value;
			n[alphaUsage.key] >> alphaUsage.value;
			n[alphaIsTransparency.key] >> alphaIsTransparency.value;
			n[spriteTessellationDetail.key] >> spriteTessellationDetail.value;
			n[textureType.key] >> textureType.value;
			n[textureShape.key] >> textureShape.value;
			if (n.has_child(singleChannelComponent.key))
				n[singleChannelComponent.key] >> singleChannelComponent.value;
			if (n.has_child(flipbookRows.key))
				n[flipbookRows.key] >> flipbookRows.value;
			if (n.has_child(flipbookColumns.key))
				n[flipbookColumns.key] >> flipbookColumns.value;
			n[maxTextureSizeSet.key] >> maxTextureSizeSet.value;
			n[compressionQualitySet.key] >> compressionQualitySet.value;
			n[textureFormatSet.key] >> textureFormatSet.value;
			if (n.has_child(ignorePngGamma.key))
				n[ignorePngGamma.key] >> ignorePngGamma.value;
			if (n.has_child(applyGammaDecoding.key))
				n[applyGammaDecoding.key] >> applyGammaDecoding.value;
			if (n.has_child(cookieLightType.key))
				n[cookieLightType.key] >> cookieLightType.value;
			n[platformSettings.key] >> platformSettings.value;
			if (!n[spritePackingTag.key].val_is_null())
				n[spritePackingTag.key] >> spritePackingTag.value;
			if (n.has_child(pSDRemoveMatte.key))
				n[pSDRemoveMatte.key] >> pSDRemoveMatte.value;
			if (n.has_child(pSDShowRemoveMatteOption.key))
				n[pSDShowRemoveMatteOption.key] >> pSDShowRemoveMatteOption.value;
		}

		void TextureImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			std::cout << subPrefix << fileIDToRecycleName.key << ":" << std::endl;
			for (auto&& [id, name] : fileIDToRecycleName.value)
				std::cout << subPrefix << "  " << id << ": " << name << std::endl;
			serializedVersion.print(subPrefix);
			mipmaps.print(subPrefix);
			bumpmap.print(subPrefix);
			isReadable.print(subPrefix);
			streamingMipmaps.print(subPrefix);
			streamingMipmapsPriority.print(subPrefix);
			vTOnly.print(subPrefix);
			grayScaleToAlpha.print(subPrefix);
			generateCubemap.print(subPrefix);
			cubemapConvolution.print(subPrefix);
			seamlessCubemap.print(subPrefix);
			textureFormat.print(subPrefix);
			maxTextureSize.print(subPrefix);
			textureSettings.print(subPrefix);
			nPOTScale.print(subPrefix);
			lightmap.print(subPrefix);
			compressionQuality.print(subPrefix);
			spriteMode.print(subPrefix);
			spriteExtrude.print(subPrefix);
			spriteMeshType.print(subPrefix);
			alignment.print(subPrefix);
			spritePivot.print(subPrefix);
			spritePixelsToUnits.print(subPrefix);
			spriteBorder.print(subPrefix);
			spriteGenerateFallbackPhysicsShape.print(subPrefix);
			alphaUsage.print(subPrefix);
			alphaIsTransparency.print(subPrefix);
			spriteTessellationDetail.print(subPrefix);
			textureType.print(subPrefix);
			textureShape.print(subPrefix);
			singleChannelComponent.print(subPrefix);
			flipbookRows.print(subPrefix);
			flipbookColumns.print(subPrefix);
			maxTextureSizeSet.print(subPrefix);
			compressionQualitySet.print(subPrefix);
			textureFormatSet.print(subPrefix);
			ignorePngGamma.print(subPrefix);
			applyGammaDecoding.print(subPrefix);
			cookieLightType.print(subPrefix);
			std::cout << subPrefix << platformSettings.key << ":" << std::endl;
			for (auto& ps : platformSettings.value)
				std::cout << subPrefix << "- " << ps;
			spritePackingTag.print(subPrefix);
			pSDRemoveMatte.print(subPrefix);
			pSDShowRemoveMatteOption.print(subPrefix);
			Importer::print(subPrefix);
		}

		bool read(ryml::ConstNodeRef n, Mipmaps* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Bumpmap* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, TextureSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, PlatformSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}
