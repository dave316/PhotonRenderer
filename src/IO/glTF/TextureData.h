#ifndef INCLUDED_TEXTUREDATA
#define INCLUDED_TEXTUREDATA

#pragma once

#include <Graphics/Texture.h>
#include <IO/glTF/BinaryData.h>
#include <IO/Image/Image.h>
#include <rapidjson/document.h>

namespace json = rapidjson;
namespace IO
{
	namespace glTF
	{
		struct TextureSampler
		{
			uint32 minFilter = GL::TextureFilter::LINEAR_MIPMAP_LINEAR;
			uint32 magFilter = GL::TextureFilter::LINEAR;
			uint32 wrapS = GL::TextureWrap::REPEAT;
			uint32 wrapT = GL::TextureWrap::REPEAT;
		};

		struct TextureInfo
		{
			Texture2D::Ptr texture = nullptr;
			int sampler = 0;
			int source = 0;
		};

		struct ImageInfo
		{
			ImageUI8::Ptr image = nullptr;
			std::string uri;
			std::string mimeType;
			bool compressed = false;
		};

		class ImageData
		{
		private:
			std::vector<ImageInfo> images;
			std::vector<TextureInfo> textures;
			std::vector<TextureSampler> samplers;
		public:
			void loadData(const json::Document& doc, const std::string& path, BinaryData& binData);
			Texture2D::Ptr loadTexture(uint32 index, bool sRGB);
		};
	}
}

#endif // INCLUDED_TEXTUREDATA