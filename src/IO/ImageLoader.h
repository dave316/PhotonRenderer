#ifndef INCLUDED_IMAGELOADER
#define INCLUDED_IMAGELOADER

#pragma once

#include <Graphics/Texture.h>

#include <string>

namespace IO
{
	Texture2D::Ptr loadTexture(const std::string& filename);
}

#endif // INCLUDED_IMAGELOADER