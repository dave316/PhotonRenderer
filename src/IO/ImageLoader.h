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
		ImageData::Ptr loadPNGFromFile(const std::string& filename);
		ImageData::Ptr loadJPGFromFile(const std::string& filename);
		ImageData::Ptr loadHDRFromFile(const std::string& filename);
		ImageData::Ptr loadFromFile(const std::string& filename);
		ImageData::Ptr decodePNGFromMemory(uint8* data, uint32 size);
		ImageData::Ptr decodeJPGFromMemory(uint8* data, uint32 size);
#ifdef IMAGE_TIFF
		ImageData::Ptr loadTIFFFromFile(const std::string& filename);
#endif
#ifdef IMAGE_EXR
		ImageData::Ptr loadEXRFromFile(const std::string& filename);
#endif
#ifdef IMAGE_WEBP
		ImageData::Ptr loadWebPFromFile(const std::string& filename);
		ImageData::Ptr decodeWebPFromMemory(uint8* data, uint32 size);
		pr::Texture2D::Ptr decodeKTXFromMemory(uint8* data, uint32 size);
#endif
#ifdef IMAGE_KTX
		pr::Texture2D::Ptr loadKTXFromFile(const std::string& filename);
#endif
		//pr::Texture2D::Ptr loadTextureFromFile(const std::string& filename, bool useSRGB);
	}
}

#endif // INCLUDED_IMAGELOADER