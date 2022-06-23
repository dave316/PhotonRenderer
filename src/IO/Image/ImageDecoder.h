#ifndef INCLUDED_IMAGEDECODER
#define INCLUDED_IMAGEDECODER

#pragma once

#include "Image.h"

namespace IO
{	
	uint32 findPowerOfTwoSize(uint32 size);
	ImageUI8::Ptr resizeImage(ImageUI8::Ptr image, uint32 width, uint32 height);

	ImageUI8::Ptr decodeFromMemory(const uint8* data, uint32 size);
	ImageUI8::Ptr decodeFromMemory(std::vector<uint8>& buffer);
	ImageUI8::Ptr decodeFromFile(const std::string& filename, bool flipY = false);
	ImageF32::Ptr decodeHDRFromFile(const std::string& filename, bool flipY = false);

	// TODO: put texture load functions somewhere else
	void saveCubemapKTX(std::string filename, TextureCubeMap::Ptr cubemap);
	void saveTextureKTX(const std::string& filename, Texture2D::Ptr texture);
	TextureCubeMap::Ptr loadCubemapKTX(const std::string& filename);
	Texture2D::Ptr loadTextureKTX(const std::string& filename);
	Texture2D::Ptr loadTextureKTXLIB(const std::string& filename);
}

#endif // INCLUDED_IMAGEDECODER