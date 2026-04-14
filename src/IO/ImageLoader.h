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
		Image::Ptr loadPNGFromFile(const std::string& filename);
		Image::Ptr loadJPGFromFile(const std::string& filename);
		Image::Ptr loadHDRFromFile(const std::string& filename);
		Image::Ptr loadEXRFromFile(const std::string& filename);
		Image::Ptr loadTIFFFromFile(const std::string& filename);
		Image::Ptr loadWebPFromFile(const std::string& filename);
		pr::Texture2D::Ptr loadKTXFromFile(const std::string& filename);
		Image::Ptr loadFromFile(const std::string& filename);
		Image::Ptr decodePNGFromMemory(uint8* data, uint32 size);
		Image::Ptr decodeJPGFromMemory(uint8* data, uint32 size);
		Image::Ptr decodeWebPFromMemory(uint8* data, uint32 size);
		pr::Texture2D::Ptr decodeKTXFromMemory(uint8* data, uint32 size);
	}
}

#endif // INCLUDED_IMAGELOADER