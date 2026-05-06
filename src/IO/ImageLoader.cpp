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
		ImageData::Ptr loadPNGFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load(filename.c_str(), &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto imgData = ImageData::create(width, height);
			imgData->setData(rawData, width * height * 4);

			delete[] rawData;

			return imgData;
		}

		ImageData::Ptr loadJPGFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load(filename.c_str(), &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto imgData = ImageData::create(width, height);
			imgData->setData(rawData, width * height * 4);

			delete[] rawData;

			return imgData;
		}

		ImageData::Ptr loadHDRFromFile(const std::string& filename)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			float* rawData = stbi_loadf(filename.c_str(), &width, &height, &channels, 4);

			auto imgData = ImageData::create(width, height, 4, sizeof(float));
			imgData->setData((uint8*)rawData, width * height * 4 * sizeof(float));

			delete[] rawData;

			return imgData;
		}

#ifdef IMAGE_EXR
		ImageData::Ptr loadEXRFromFile(const std::string& filename)
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

			auto imgData = ImageData::create(width, height, 4, sizeof(float));
			imgData->setData((uint8*)rgba, width * height * 4 * sizeof(float));

			delete[] rgba;

			return imgData;
		}
#endif

#ifdef IMAGE_TIFF
		ImageData::Ptr loadTIFFFromFile(const std::string& filename)
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

			auto imgData = ImageData::create(imgwidth, imgheight);
			imgData->setData(buffer, imgwidth * imgheight * 4);

			delete[] buffer;

			_TIFFfree(raster);
			TIFFClose(tif);

			return imgData;
		}
#endif

#ifdef IMAGE_WEBP
		ImageData::Ptr loadWebPFromFile(const std::string& filename)
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

			auto imgData = ImageData::create(width, height);
			imgData->setData(data, width * height * 4);

			WebPFree(data);

			return imgData;
		}
#endif

#ifdef IMAGE_KTX
		ImageData::Ptr loadKTXFromFile(const std::string& filename)
		{
			ktxTexture2* pKtxTexture;
			KTX_error_code result;

			result = ktxTexture2_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
			if (result != KTX_SUCCESS)
			{
				std::cout << "error: could not open file " << filename << std::endl;
				return nullptr;
			}

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

			auto imgData = ImageData::create(
				pKtxTexture->baseWidth,
				pKtxTexture->baseHeight,
				4, // TODO: get from format
				1, // TODO: get from format
				pKtxTexture->numLevels,
				pKtxTexture->numLayers,
				pKtxTexture->isCompressed
			);

			// TODO: add cube map faces
			for (uint32 level = 0; level < pKtxTexture->numLevels; level++)
			{
				int w = std::max(pKtxTexture->baseWidth >> level, 1U);
				int h = std::max(pKtxTexture->baseHeight >> level, 1U);

				for (uint32 layer = 0; layer < pKtxTexture->numLayers; layer++)
				{
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, layer, 0, &offset);
					ktx_uint8_t* data = pKtxTexture->pData + offset;
					uint32 size = (uint32)ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);

					imgData->setData(data, size, level, layer);					
				}
			}
		
			ktxTexture_Destroy(ktxTexture(pKtxTexture));

			return imgData;
		}
#endif

		ImageData::Ptr loadFromFile(const std::string& filename)
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
#ifdef IMAGE_KTX
			else if (extension.compare(".ktx") == 0 || extension.compare(".ktx2") == 0)
				return loadKTXFromFile(filename);
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

		ImageData::Ptr decodePNGFromMemory(uint8* data, uint32 size)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load_from_memory(data, size, &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto imgData = ImageData::create(width, height);
			imgData->setData(rawData, width * height * 4);

			delete[] rawData;

			return imgData;
		}

		ImageData::Ptr decodeJPGFromMemory(uint8* data, uint32 size)
		{
			stbi_set_flip_vertically_on_load(false);

			int width = 0;
			int height = 0;
			int channels = 0;
			uint8* rawData = stbi_load_from_memory(data, size, &width, &height, &channels, 4);

			//std::cout << "tex size: " << width << "x" << height << "x" << channels << std::endl;

			auto imgData = ImageData::create(width, height);
			imgData->setData(rawData, width * height * 4);

			delete[] rawData;

			return imgData;
		}

#ifdef IMAGE_WEBP
		ImageData::Ptr decodeWebPFromMemory(uint8* data, uint32 size)
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

			auto imgData = ImageData::create(width, height);
			imgData->setData(rawData, width * height * 4);

			WebPFree(rawData);

			return imgData;
		}
#endif

#ifdef IMAGE_KTX
		ImageData::Ptr decodeKTXFromMemory(uint8* data, uint32 size)
		{
			std::cout << "decode KTX from memory" << std::endl;

			ktxTexture2* pKtxTexture;
			KTX_error_code result;
			result = ktxTexture2_CreateFromMemory(data, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
			if (result != KTX_SUCCESS)
			{
				std::cout << "error: create KTX texture from memory!" << std::endl;
				return nullptr;
			}

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
			//std::cout << "compressed: " << (pKtxTexture->isCompressed ? "true" : "false") << std::endl;
			//std::cout << "array: " << (pKtxTexture->isArray ? "true" : "false") << std::endl;
			//std::cout << "cubemap: " << (pKtxTexture->isCubemap ? "true" : "false") << std::endl;
			//std::cout << "video: " << (pKtxTexture->isVideo ? "true" : "false") << std::endl;

			auto imgData = ImageData::create(
				pKtxTexture->baseWidth,
				pKtxTexture->baseHeight,
				4, // TODO: get from format
				1, // TODO: get from format
				pKtxTexture->numLevels,
				pKtxTexture->numLayers,
				pKtxTexture->isCompressed
			);

			// TODO: add cube map faces
			for (uint32 level = 0; level < pKtxTexture->numLevels; level++)
			{
				int w = std::max(pKtxTexture->baseWidth >> level, 1U);
				int h = std::max(pKtxTexture->baseHeight >> level, 1U);

				for (uint32 layer = 0; layer < pKtxTexture->numLayers; layer++)
				{
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, layer, 0, &offset);
					ktx_uint8_t* data = pKtxTexture->pData + offset;
					uint32 size = (uint32)ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);

					imgData->setData(data, size, level, layer);
				}
			}

			ktxTexture_Destroy(ktxTexture(pKtxTexture));

			return imgData;
		}
#endif

		ImageData::Ptr decodeFromMemory(uint8* data, uint32 size, std::string mimeType)
		{
			if (mimeType.compare("image/jpeg") == 0)
				return IO::ImageLoader::decodeJPGFromMemory(data, size);
			else if (mimeType.compare("image/png") == 0)
				return IO::ImageLoader::decodePNGFromMemory(data, size);
#ifdef IMAGE_WEBP
			else if (mimeType.compare("image/webp") == 0)
				return IO::ImageLoader::decodeWebPFromMemory(data, size);
#endif
#ifdef IMAGE_KTX
			else if (mimeType.compare("image/ktx2") == 0)
				return IO::ImageLoader::decodeKTXFromMemory(data, size);
#endif
			else
				std::cout << "error: not supported mime type: " << mimeType << std::endl;

			return nullptr;
		}
	}
}