#ifndef INCLUDED_IMAGELOADER
#define INCLUDED_IMAGELOADER

#pragma once

#include "Image.h"

#include <string>
#include <vector>

#include <Graphics/Texture.h>

namespace IO
{
	namespace ImageLoader
	{
		// functions for loading images from file
		ImageData::Ptr loadPNGFromFile(const std::string& filename);
		ImageData::Ptr loadJPGFromFile(const std::string& filename);
		ImageData::Ptr loadHDRFromFile(const std::string& filename);
#ifdef IMAGE_TIFF
		ImageData::Ptr loadTIFFFromFile(const std::string& filename);
#endif
#ifdef IMAGE_EXR
		ImageData::Ptr loadEXRFromFile(const std::string& filename);
#endif
#ifdef IMAGE_WEBP
		ImageData::Ptr loadWebPFromFile(const std::string& filename);
#endif
		ImageData::Ptr loadFromFile(const std::string& filename);

		// functions for loading images from memory
		ImageData::Ptr decodePNGFromMemory(uint8* data, uint32 size);
		ImageData::Ptr decodeJPGFromMemory(uint8* data, uint32 size);
#ifdef IMAGE_WEBP
		ImageData::Ptr decodeWebPFromMemory(uint8* data, uint32 size);
#endif
#ifdef IMAGE_KTX
		ImageData::Ptr decodeKTXFromMemory(uint8* data, uint32 size);
#endif
		ImageData::Ptr decodeFromMemory(uint8* data, uint32 size, std::string mimeType);
	}
}

#endif // INCLUDED_IMAGELOADER