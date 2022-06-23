#include "TextureData.h"

#include <IO/Image/ImageDecoder.h>
#include <base64/base64.h>

#ifdef WITH_KTX
#include <ktx.h>
#include <vkformat_enum.h>
#endif

namespace IO
{
	namespace glTF
	{
		void ImageData::loadData(const json::Document& doc, const std::string& path, BinaryData& binData)
		{
			if (!doc.HasMember("images"))
				return;

			for (auto& imageNode : doc["images"].GetArray())
			{
				ImageInfo imageInfo;
				if (imageNode.HasMember("uri"))
				{
					std::string uri = imageNode["uri"].GetString();
					if (uri.find(':') == std::string::npos) // file uri
					{
						int index; // TODO: add proper parsing of URIs
						while ((index = uri.find("%20")) >= 0)
							uri.replace(index, 3, " ");

						std::string fullPath = path + "/" + uri;
						auto idx = uri.find_last_of('.') + 1;
						auto len = uri.length();
						auto fileExt = uri.substr(idx, len - idx);

#ifdef WITH_KTX
						if (fileExt.compare("ktx2") == 0)
							imageInfo.compressed = true; // TODO: check if actually compressed
						else
#endif
							imageInfo.image = IO::decodeFromFile(fullPath);

						imageInfo.uri = fullPath;
					}
					else // data uri
					{
						int sepIndex = uri.find_last_of(',');
						int dataStart = sepIndex + 1;
						int dataLen = uri.length() - dataStart;
						std::string dataURI = uri.substr(0, sepIndex); // TODO: check if media type is correct etc...
						std::string dataBase64 = uri.substr(dataStart, dataLen);
						std::string dataBinary = base64_decode(dataBase64);
						std::vector<unsigned char> data;
						data.insert(data.end(), dataBinary.begin(), dataBinary.end());

						imageInfo.image = IO::decodeFromMemory(data);
					}
				}
				if (imageNode.HasMember("mimeType")) // TODO: check mime type
					imageInfo.mimeType = imageNode["mimeType"].GetString();
				if (imageNode.HasMember("bufferView"))
				{
					uint32 bufferView = imageNode["bufferView"].GetInt();

					std::vector<unsigned char> data;
					BufferView& bv = binData.bufferViews[bufferView];
					Buffer& buffer = binData.buffers[bv.buffer];
					int offset = bv.byteOffset;
					data.resize(bv.byteLength);
					memcpy(data.data(), &buffer.data[offset], bv.byteLength);

					imageInfo.image = IO::decodeFromMemory(data);
				}
				images.push_back(imageInfo);
			}

			if (doc.HasMember("samplers"))
			{
				for (auto& samplerNode : doc["samplers"].GetArray())
				{
					TextureSampler sampler;
					if (samplerNode.HasMember("minFilter"))
						sampler.minFilter = samplerNode["minFilter"].GetInt();
					if (samplerNode.HasMember("magFilter"))
						sampler.magFilter = samplerNode["magFilter"].GetInt();
					if (samplerNode.HasMember("wrapS"))
						sampler.wrapS = samplerNode["wrapS"].GetInt();
					if (samplerNode.HasMember("wrapT"))
						sampler.wrapT = samplerNode["wrapT"].GetInt();
					samplers.push_back(sampler);
				}
			}

			if (doc.HasMember("textures"))
			{
				for (auto& textureNode : doc["textures"].GetArray())
				{
					TextureInfo texInfo;
					if (textureNode.HasMember("source"))
						texInfo.source = textureNode["source"].GetInt();
					if (textureNode.HasMember("extensions"))
					{
						auto& extNode = textureNode["extensions"];
						if (extNode.HasMember("KHR_texture_basisu"))
							texInfo.source = extNode["KHR_texture_basisu"]["source"].GetInt();
					}
					if (textureNode.HasMember("sampler"))
						texInfo.sampler = textureNode["sampler"].GetInt();
					else
						texInfo.sampler = -1;
					textures.push_back(texInfo);
				}
			}
		}

		Texture2D::Ptr ImageData::loadTexture(uint32 index, bool sRGB)
		{
			TextureInfo& texInfo = textures[index];
			if (texInfo.texture) // texture has been loaded already
				return texInfo.texture;

			TextureSampler sampler;
			if(texInfo.sampler >= 0)
				sampler = samplers[texInfo.sampler];

			auto& imageInfo = images[texInfo.source];

#ifdef WITH_KTX
			if (imageInfo.compressed) // TODO: add normal ktx textures
			{
				texInfo.texture = IO::loadTextureKTX(imageInfo.uri);
			}
			else
#endif
			{
				auto image = imageInfo.image;

				if (sampler.wrapS == GL::TextureWrap::REPEAT ||
					sampler.wrapT == GL::TextureWrap::REPEAT ||
					sampler.wrapS == GL::TextureWrap::MIRRORED_REPEAT ||
					sampler.wrapT == GL::TextureWrap::MIRRORED_REPEAT ||
					sampler.minFilter >= GL::TextureFilter::NEAREST_MIPMAP_NEAREST &&
					sampler.minFilter <= GL::TextureFilter::LINEAR_MIPMAP_LINEAR)
				{
					if (!image->isPowerOfTwo())
					{
						// this can lead to a stretched image
						uint32 w = IO::findPowerOfTwoSize(image->getWidth());
						uint32 h = IO::findPowerOfTwoSize(image->getHeight());
						image = IO::resizeImage(image, w, h);
					}
				}

				texInfo.texture = image->upload(sRGB);
			}

			if (sampler.minFilter >= GL::TextureFilter::NEAREST_MIPMAP_NEAREST && 
				sampler.minFilter <= GL::TextureFilter::LINEAR_MIPMAP_LINEAR)
				texInfo.texture->generateMipmaps();

			texInfo.texture->setFilter(GL::TextureFilter(sampler.minFilter), GL::TextureFilter(sampler.magFilter));
			texInfo.texture->setWrap(GL::TextureWrap(sampler.wrapS), GL::TextureWrap(sampler.wrapT));
			return texInfo.texture;
		}
	}
}
