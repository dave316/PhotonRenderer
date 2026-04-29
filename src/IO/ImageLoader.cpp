#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <iostream>
#include <fstream>

#ifdef IMAGE_TIFF
#include <tiffio.h>
#endif
#ifdef IMAGE_EXR
#include <tinyexr.h>
#endif
#ifdef IMAGE_KTX
#include <ktx.h>
#endif
#ifdef IMAGE_WEBP
#include <webp/decode.h>
#endif

namespace fs = std::filesystem;

namespace IO
{
	namespace ImageLoader
	{
		Image::Ptr loadPNGFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load(filename.c_str(), &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4);

			delete[] rawData;

			return img;
		}

		Image::Ptr loadJPGFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load(filename.c_str(), &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4);

			delete[] rawData;

			return img;
		}

		Image::Ptr loadHDRFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			float* rawData = stbi_loadf(filename.c_str(), &width, &height, &channels, 4);

			auto img = ImageType<float>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4 * sizeof(float));

			delete[] rawData;

			return img;
		}

#ifdef IMAGE_EXR
		Image::Ptr loadEXRFromFile(const std::string& filename)
		{
			int width = 0;
			int height = 0;
			int channels = 4;
			float* rgba;
			const char* err;

			int ret = LoadEXR(&rgba, &width, &height, filename.c_str(), &err);
			if (ret != 0)
				std::cout << err << std::endl;

			//std::cout << "loaded exr image: " << width << "x" << height << std::endl;

			auto img = ImageType<float>::create(width, height, 4);
			img->setFromMemory(rgba, width * height * 4 * sizeof(float));

			delete[] rgba;

			return img;
		}
#endif

#ifdef IMAGE_TIFF
		Image::Ptr loadTIFFFromFile(const std::string& filename)
		{
			TIFFRGBAImage tiffRGBA;
			uint32* raster;

			size_t npixels;
			int imgwidth, imgheight;

			int hasABGR = 0;
			char* imgfilename = NULL;

			TIFF* tif;
			char emsg[1024];

			tif = TIFFOpen(filename.c_str(), "r");
			if (tif == NULL) {
				fprintf(stderr, "tif == NULL\n");
				exit(1);
			}
			if (TIFFRGBAImageBegin(&tiffRGBA, tif, 0, emsg)) {
				npixels = tiffRGBA.width * tiffRGBA.height;
				raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));
				if (raster != NULL) {
					if (TIFFRGBAImageGet(&tiffRGBA, raster, tiffRGBA.width, tiffRGBA.height) == 0) {
						TIFFError(filename.c_str(), emsg);
						exit(1);
					}
				}
				TIFFRGBAImageEnd(&tiffRGBA);
				//fprintf(stderr, "Read image %s (%d x %d)\n", filename.c_str(), tiffRGBA.width, tiffRGBA.height);
			}
			else {
				TIFFError(filename.c_str(), emsg);
				exit(1);
			}
			imgwidth = tiffRGBA.width;
			imgheight = tiffRGBA.height;
			uint32 imgSize = imgwidth * imgheight * 4;

			uint8_t* buffer = new uint8_t[imgSize];
			for (int row = 0; row < imgheight; row++)
			{
				int r = imgheight - row - 1;
				for (int col = 0; col < imgwidth; col++)
				{
					for (int c = 0; c < 4; c++)
						buffer[row * imgwidth * 4 + col * 4 + c] = ((uint8*)raster)[r * imgwidth * 4 + col * 4 + c];
				}
			}

			auto img = ImageType<uint8>::create(imgwidth, imgheight, 4);
			img->setFromMemory(buffer, imgwidth * imgheight * 4);
			delete[] buffer;

			_TIFFfree(raster);
			TIFFClose(tif);

			return img;
		}
#endif

#ifdef IMAGE_WEBP
		Image::Ptr loadWebPFromFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				std::cout << "error opening file " << filename << std::endl;
				return nullptr;
			}

			auto size = file.tellg();
			file.seekg(0, std::ios::beg);
			std::vector<uint8> buffer(size);
			file.read((char*)buffer.data(), buffer.size());
			file.close();

			int w, h;
			if (!WebPGetInfo(buffer.data(), buffer.size(), &w, &h))
			{
				std::cout << "error getting WebP info!" << std::endl;
				return nullptr;
			}

			//std::cout << "loaded WebP info, image size: " << w << "x" << h << std::endl;

			int width;
			int height;
			uint8* data = WebPDecodeRGBA(buffer.data(), buffer.size(), &width, &height);
			if (data == nullptr)
			{
				std::cout << "error decoding WebP image!" << std::endl;
				return nullptr;
			}

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(data, width * height * 4);
			WebPFree(data);

			return img;
		}
#endif

#ifdef IMAGE_KTX
		pr::Texture2D::Ptr loadKTXFromFile(const std::string& filename)
		{
			ktxTexture2* pKtxTexture;
			KTX_error_code result;

			result = ktxTexture2_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
			if (pKtxTexture->isCompressed)
				result = ktxTexture2_TranscodeBasis(pKtxTexture, KTX_TTF_BC7_RGBA, 0);

			//std::cout << "vk format: " << pKtxTexture->vkFormat << std::endl;
			//std::cout << "width: " << pKtxTexture->baseWidth << std::endl;
			//std::cout << "height: " << pKtxTexture->baseHeight << std::endl;
			//std::cout << "depth: " << pKtxTexture->baseDepth << std::endl;
			//std::cout << "dims: " << pKtxTexture->numDimensions << std::endl;
			//std::cout << "faces: " << pKtxTexture->numFaces << std::endl;
			//std::cout << "layers: " << pKtxTexture->numLayers << std::endl;
			//std::cout << "levels: " << pKtxTexture->numLevels << std::endl;
			//std::cout << "data: " << pKtxTexture->dataSize << std::endl;

			pr::Texture2D::Ptr texture = nullptr;
			if (result == KTX_SUCCESS)
			{
				GLint internalFormat = 0;
				GPU::Format format = GPU::Format::RGBA8;
				if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_UNORM_BLOCK)
					std::cout << "tex format RGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_SRGB_BLOCK)
					std::cout << "tex format SRGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_UNORM_BLOCK)
					format = GPU::Format::BC7_RGBA;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK)
					format = GPU::Format::BC7_SRGB;

				texture = pr::Texture2D::create(pKtxTexture->baseWidth, pKtxTexture->baseHeight, format, pKtxTexture->numLevels, GPU::ImageUsage::Sampled);

				for (uint32 level = 0; level < pKtxTexture->numLevels; level++)
				{
					int w = std::max(pKtxTexture->baseWidth >> level, 1U);
					int h = std::max(pKtxTexture->baseHeight >> level, 1U);
					uint32 imgSize = (uint32)ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, 0, 0, &offset);
					ktx_uint8_t* pData = pKtxTexture->pData + offset;
					texture->upload(pData, imgSize, level);
				}
			}

			ktxTexture_Destroy(ktxTexture(pKtxTexture));

			return texture;
		}
#endif

		Image::Ptr loadFromFile(const std::string& filename)
		{
			auto p = fs::path(filename);
			auto extension = p.extension().string();

			if (extension.compare(".png") == 0)
				return loadPNGFromFile(filename);
			else if (extension.compare(".jpg") == 0 || extension.compare(".jpeg") == 0)
				return loadJPGFromFile(filename);
			else if (extension.compare(".hdr") == 0)
				return loadHDRFromFile(filename);
#ifdef IMAGE_EXR
			else if (extension.compare(".exr") == 0)
				return loadEXRFromFile(filename);
#endif
#ifdef IMAGE_TIFF
			else if (extension.compare(".tif") == 0)
				return loadTIFFFromFile(filename);
#endif
#ifdef IMAGE_WEBP
			else if (extension.compare(".webp") == 0)
				return loadWebPFromFile(filename);
#endif
			else
				std::cout << "extension " << extension << " not supported!" << std::endl;

			return nullptr;
		}

		Image::Ptr decodePNGFromMemory(uint8* data, uint32 size)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load_from_memory(data, size, &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4);

			delete[] rawData;

			return img;
		}

		Image::Ptr decodeJPGFromMemory(uint8* data, uint32 size)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load_from_memory(data, size, &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4);

			delete[] rawData;

			return img;
		}

#ifdef IMAGE_WEBP
		Image::Ptr decodeWebPFromMemory(uint8* data, uint32 size)
		{
			int w, h;
			if (!WebPGetInfo(data, size, &w, &h))
			{
				std::cout << "error getting WebP info!" << std::endl;
				return nullptr;
			}

			std::cout << "loaded WebP info, image size: " << w << "x" << h << std::endl;

			int width;
			int height;
			uint8* rawData = WebPDecodeRGBA(data, size, &width, &height);
			if (rawData == nullptr)
			{
				std::cout << "error decoding WebP image!" << std::endl;
				return nullptr;
			}

			auto img = ImageType<uint8>::create(width, height, 4);
			img->setFromMemory(rawData, width * height * 4);
			WebPFree(rawData);

			return img;
		}
#endif

#ifdef IMAGE_KTX
		pr::Texture2D::Ptr decodeKTXFromMemory(uint8* data, uint32 size)
		{
			ktxTexture2* pKtxTexture;
			KTX_error_code result;
			result = ktxTexture2_CreateFromMemory(data, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
			if (pKtxTexture->isCompressed)
				result = ktxTexture2_TranscodeBasis(pKtxTexture, KTX_TTF_BC7_RGBA, 0);

			std::cout << "vk format: " << pKtxTexture->vkFormat << std::endl;
			std::cout << "width: " << pKtxTexture->baseWidth << std::endl;
			std::cout << "height: " << pKtxTexture->baseHeight << std::endl;
			std::cout << "depth: " << pKtxTexture->baseDepth << std::endl;
			std::cout << "dims: " << pKtxTexture->numDimensions << std::endl;
			std::cout << "faces: " << pKtxTexture->numFaces << std::endl;
			std::cout << "layers: " << pKtxTexture->numLayers << std::endl;
			std::cout << "levels: " << pKtxTexture->numLevels << std::endl;
			std::cout << "data: " << pKtxTexture->dataSize << std::endl;
			std::cout << "compressed: " << (pKtxTexture->isCompressed ? "true" : "false") << std::endl;
			std::cout << "array: " << (pKtxTexture->isArray ? "true" : "false") << std::endl;
			std::cout << "cubemap: " << (pKtxTexture->isCubemap ? "true" : "false") << std::endl;
			std::cout << "video: " << (pKtxTexture->isVideo ? "true" : "false") << std::endl;

			pr::Texture2D::Ptr texture = nullptr;
			if (result == KTX_SUCCESS)
			{
				GLint internalFormat = 0;
				GPU::Format format = GPU::Format::RGBA8;
				if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_UNORM_BLOCK)
					std::cout << "tex format RGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_SRGB_BLOCK)
					std::cout << "tex format SRGB_S3TC_DXT1" << std::endl;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_UNORM_BLOCK)
					format = GPU::Format::BC7_RGBA;
				else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK)
					format = GPU::Format::BC7_SRGB;

				texture = pr::Texture2D::create(pKtxTexture->baseWidth, pKtxTexture->baseHeight, format, pKtxTexture->numLevels, GPU::ImageUsage::Sampled);
				for (uint32 level = 0; level < pKtxTexture->numLevels; level++)
				{
					int w = std::max(pKtxTexture->baseWidth >> level, 1U);
					int h = std::max(pKtxTexture->baseHeight >> level, 1U);
					uint32 imgSize = (uint32)ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, 0, 0, &offset);
					ktx_uint8_t* pData = pKtxTexture->pData + offset;
					texture->upload(pData, imgSize, level);
				}
			}

			ktxTexture_Destroy(ktxTexture(pKtxTexture));

			return texture;
		}
#endif

		pr::Texture2D::Ptr loadTextureFromFile(const std::string& filename, bool useSRGB)
		{
			auto img = IO::ImageLoader::loadFromFile(filename);

			uint32 width = img->getWidth();
			uint32 height = img->getHeight();
			uint32 channels = img->getChannels();
			uint32 elemSize = img->getElementSize();
			uint8* data = img->getRawPtr();
			uint32 dataSize = width * height * channels * elemSize;

			GPU::Format format = useSRGB ? GPU::Format::SRGBA8 : GPU::Format::RGBA8;
			return pr::Texture2D::create(width, height, format);
		}
	}
}