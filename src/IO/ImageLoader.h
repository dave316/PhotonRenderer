#ifndef INCLUDED_IMAGELOADER
#define INCLUDED_IMAGELOADER

#pragma once

#include <Graphics/Texture.h>

#include <string>
#include <vector>

namespace IO
{
	Texture2D::Ptr loadTextureFromMemory(std::vector<unsigned char>& data, bool sRGB);
	Texture2D::Ptr loadTexture(const std::string& filename, bool sRGB);
	Texture2D::Ptr loadTexture16(const std::string& filename, bool sRGB);
	Texture2D::Ptr loadTextureHDR(const std::string& filename);
	Texture2D::Ptr loadTextureKTX(const std::string& filename);	
}

#endif // INCLUDED_IMAGELOADER