#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize.h>

#ifdef WITH_KTX
#include <ktx.h>
#include <vulkan/vulkan_core.h>
#include <half.hpp>
using namespace half_float;
#endif

#ifdef WITH_UNITY
#include <tinyexr.h>
#include <tiffio.h>
#endif

#include "ImageDecoder.h"

namespace IO
{
	uint32 findPowerOfTwoSize(uint32 size)
	{
		uint32 e = 0;
		while (size > 1)
		{
			size = (size >> 1);
			e++;
		}			
		return std::pow(2.0, e);
	}

	ImageF32::Ptr resizeImage(ImageF32::Ptr src, uint32 width, uint32 height)
	{
		uint32 w = src->getWidth();
		uint32 h = src->getHeight();
		uint32 c = src->getChannels();

		auto dst = ImageF32::create(width, height, c);
		stbir_resize_float(src->getRawPtr(), w, h, 0, dst->getRawPtr(), width, height, 0, c);
		return dst;
	}

	ImageUI8::Ptr resizeImage(ImageUI8::Ptr src, uint32 width, uint32 height)
	{
		uint32 w = src->getWidth();
		uint32 h = src->getHeight();
		uint32 c = src->getChannels();

		auto dst = ImageUI8::create(width, height, c);
		stbir_resize_uint8(src->getRawPtr(), w, h, 0, dst->getRawPtr(), width, height, 0, c);
		return dst;
	}

	ImageUI8::Ptr decodeFromMemory(const uint8* buffer, uint32 size)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_set_flip_vertically_on_load(false);

		uint8* data = stbi_load_from_memory(buffer, size, &width, &height, &channels, 0);

		auto img = ImageUI8::create(width, height, channels);
		img->setFromMemory(data, width * height * channels);
		delete[] data;
		return img;
	}

	ImageUI8::Ptr decodeFromMemory(std::vector<uint8>& buffer)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_set_flip_vertically_on_load(false);

		uint8* data = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, &channels, 0);
		auto img = ImageUI8::create(width, height, channels);
		img->setFromMemory(data, width * height * channels);
		delete[] data;
		return img;
	}

	ImageUI8::Ptr decodeFromFile(const std::string& filename, bool flipY)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_set_flip_vertically_on_load(flipY);

		uint8* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

		//if (data == nullptr)
		//	std::cout << stbi_failure_reason() << std::endl;

		//std::cout << "loaded image " << width << "x" << height << "x" << channels << std::endl;

		// TODO: check errors etc.
		auto img = ImageUI8::create(width, height, channels);
		img->setFromMemory(data, width * height * channels);

		delete[] data;

		return img;
	}

	ImageF32::Ptr decodeFloat(const std::string& filename, bool flipY)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_set_flip_vertically_on_load(flipY);

		uint8* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
		
		//if (data == nullptr)
		//	std::cout << stbi_failure_reason() << std::endl;

		std::cout << "loaded image " << width << "x" << height << "x" << channels << std::endl;

		// TODO: check errors etc.
		float* buffer = new float[width * height];
		for (int i = 0; i < width * height; i++)
			buffer[i] = (1.0f - (static_cast<float>(data[i*3]) / 255.0f));
		auto img = ImageF32::create(width, height, 1);
		img->setFromMemory(buffer, width * height * 1);

		delete[] data;
		delete[] buffer;

		return img;
	}

	void saveToFile(const std::string& filename, ImageUI8::Ptr img)
	{
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filename.c_str(), img->getWidth(), img->getHeight(), img->getChannels(), img->getRawPtr(), img->getWidth() * 4);
	}

	void saveHDR(const std::string& filename, ImageF32::Ptr image)
	{
		stbi_flip_vertically_on_write(true);
		stbi_write_hdr(filename.c_str(), image->getWidth(), image->getHeight(), image->getChannels(), image->getRawPtr());
	}

#ifdef WITH_UNITY
	ImageUI8::Ptr decodeTIFFromFile(const std::string& filename)
	{
		TIFFRGBAImage tiffRGBA;
		uint32* raster;

		size_t npixels;
		int imgwidth, imgheight, imgcomponents;

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
				for(int c = 0; c < 4; c++)
					buffer[row * imgwidth * 4 + col * 4 + c] = ((uint8*)raster)[r * imgwidth * 4 + col * 4 + c];
			}
		}

		auto img = ImageUI8::create(imgwidth, imgheight, 4);
		img->setFromMemory(buffer, imgwidth * imgheight * 4);
		delete[] buffer;
		_TIFFfree(raster);
		TIFFClose(tif);

		return img;
	}

	ImageF32::Ptr decodeEXRFromFile(const std::string& filename)
	{
		int w, h;
		const char* err;
		float* rgba;

		int ret = LoadEXR(&rgba, &w, &h, filename.c_str(), &err);
		if (ret != 0)
			std::cout << err << std::endl;

		//std::cout << "loaded exr image: " << w << "x" << h << std::endl;

		auto img = ImageF32::create(w, h, 4);
		img->setFromMemory(rgba, w * h * 4);

		delete[] rgba;

		return img;
	}
#endif

	ImageF32::Ptr decodeHDRFromFile(const std::string& filename, bool flipY)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_set_flip_vertically_on_load(flipY);

		float* data = stbi_loadf(filename.c_str(), &width, &height, &channels, 0);

		// TODO: check errors etc.
		auto img = ImageF32::create(width, height, channels);
		img->setFromMemory(data, width * height * channels);

		delete[] data;

		return img;
	}

#ifdef WITH_KTX
	void saveCubemapKTX(std::string filename, TextureCubeMap::Ptr cubemap)
	{
		unsigned int size = cubemap->getFaceSize();

		KTX_error_code result;
		ktxTexture2* texture;
		ktxTextureCreateInfo info;
		info.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
		//info.vkFormat = VK_FORMAT_R8G8B8_UNORM;
		info.baseWidth = size;
		info.baseHeight = size;
		info.baseDepth = 1;
		info.numDimensions = 2;
		info.numLevels = cubemap->getLevels();
		info.numLayers = 1;
		info.numFaces = 6;
		info.isArray = KTX_FALSE;
		info.generateMipmaps = KTX_FALSE;

		result = ktxTexture2_Create(&info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);

		auto tempCM = cubemap->copy();
		GLuint texID = tempCM->getID();

		for (int level = 0; level < cubemap->getLevels(); level++)
		{
			int s = size / std::pow(2, level);
			unsigned int imgSize = s * s * 3;
			unsigned int bufSize = imgSize * 6;
			float* buffer = new float[bufSize];

			glGetTextureImage(texID, level, GL_RGB, GL_FLOAT, bufSize * sizeof(float), buffer); // TODO: get format and type from texture
			for (int f = 0; f < 6; f++)
			{
				unsigned char* dataPtr = (unsigned char*)&buffer[imgSize * f];
				result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, 0, f, dataPtr, imgSize * sizeof(float));
			}

			delete[] buffer;
		}

		ktxTexture_WriteToNamedFile(ktxTexture(texture), filename.c_str());
		ktxTexture_Destroy(ktxTexture(texture));
	}

	void saveTextureKTX(const std::string& filename, Texture2D::Ptr texture)
	{
		// TODO: get size from texture
		int size = 512;

		KTX_error_code result;
		ktxTexture2* ktxTex;
		ktxTextureCreateInfo info;
		info.vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		info.baseWidth = 512;
		info.baseHeight = 512;
		info.baseDepth = 1;
		info.numDimensions = 2;
		info.numLevels = 1;
		info.numLayers = 1;
		info.numFaces = 1;
		info.isArray = KTX_FALSE;
		info.generateMipmaps = KTX_FALSE;
		result = ktxTexture2_Create(&info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktxTex);

		unsigned int bufSize = size * size * 4;
		half* buffer = new half[bufSize];
		glGetTextureImage(texture->getID(), 0, GL_RGBA, GL_HALF_FLOAT, bufSize * sizeof(half), buffer);

		ktxTexture_SetImageFromMemory(ktxTexture(ktxTex), 0, 0, 0, (unsigned char*)buffer, bufSize * sizeof(half));
		ktxTexture_WriteToNamedFile(ktxTexture(ktxTex), filename.c_str());
		ktxTexture_Destroy(ktxTexture(ktxTex));

		delete[] buffer;
	}

	TextureCubeMap::Ptr loadCubemapKTX(const std::string& filename)
	{
		ktxTexture2* texture;
		KTX_error_code result;

		result = ktxTexture2_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
		if (texture->isCompressed)
			result = ktxTexture2_TranscodeBasis(texture, KTX_TTF_BC7_RGBA, 0);

		auto tex = TextureCubeMap::create(texture->baseWidth, texture->baseHeight, GL::RGB32F);
		tex->bind();
		GLuint id = tex->getID();
		GLenum target, error;
		result = ktxTexture_GLUpload(ktxTexture(texture), &id, &target, &error);

		//std::cout << "format: " << texture->vkFormat << std::endl;
		//std::cout << "levels: " << texture->numLevels << std::endl;
		if (texture->numLevels > 1)
			tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		ktxTexture_Destroy(ktxTexture(texture));

		return tex;
	}

	Texture2D::Ptr loadTextureKTXLIB(const std::string& filename)
	{
		// TODO: check tex info (cubemaps, mipmaps, arrays, etc) and load accordingly
		// TODO: check available texture compression formats before uploading
		// TODO: check errors...
		ktxTexture2* texture;
		KTX_error_code result;

		result = ktxTexture2_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
		if (texture->isCompressed)
			result = ktxTexture2_TranscodeBasis(texture, KTX_TTF_BC7_RGBA, 0);

		auto tex = Texture2D::create(texture->baseWidth, texture->baseHeight, GL::RGBA8);
		tex->bind();
		GLuint id = tex->getID();
		GLenum target, error;
		result = ktxTexture_GLUpload(ktxTexture(texture), &id, &target, &error);

		ktxTexture_Destroy(ktxTexture(texture));

		return tex;
	}

	Texture2D::Ptr loadTextureKTX(const std::string& filename)
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

		Texture2D::Ptr texture = Texture2D::create(pKtxTexture->baseWidth, pKtxTexture->baseHeight, GL::RGBA8);

		if (result == KTX_SUCCESS)
		{
			GLint internalFormat = 0;
			if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_UNORM_BLOCK)
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			else if (pKtxTexture->vkFormat == VK_FORMAT_BC1_RGB_SRGB_BLOCK)
				internalFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_UNORM_BLOCK)
				internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
			else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK)
				internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

			//std::cout << "tex format " << internalFormat << std::endl;

			texture->bind();
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			for (int level = 0; level < pKtxTexture->numLevels; level++)
			{
				int w = std::max(pKtxTexture->baseWidth >> level, 1U);
				int h = std::max(pKtxTexture->baseHeight >> level, 1U);
				int imgSize = ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);
				ktx_size_t offset = 0;
				ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, 0, 0, &offset);
				ktx_uint8_t* pData = pKtxTexture->pData + offset;
				//std::cout << "mip size: " << w << "x" << h << std::endl;
				glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, w, h, 0, imgSize, pData);
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		ktxTexture_Destroy(ktxTexture(pKtxTexture));

		return texture;
	}

	void compressImage2D(const std::string& filename, ImageUI8::Ptr image, bool sRGB)
	{
		KTX_error_code result;
		ktxTexture2* ktxTex;
		ktxTextureCreateInfo info;
		
		if(sRGB)
			info.vkFormat = VK_FORMAT_R8G8B8_SRGB; // TODO: figure out format needed from image
		else
			info.vkFormat = VK_FORMAT_R8G8B8_UNORM; // TODO: figure out format needed from image
		info.baseWidth = image->getWidth();
		info.baseHeight = image->getHeight();
		info.baseDepth = 1;
		info.numDimensions = 2;
		info.numLevels = 1;
		info.numLayers = 1;
		info.numFaces = 1;
		info.isArray = KTX_FALSE;
		info.generateMipmaps = KTX_FALSE; // TODO: generate mipmaps
		result = ktxTexture2_Create(&info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktxTex);
		if (result != KTX_SUCCESS)
		{
			std::cout << "error creating ktx texture " << ktxErrorString(result) << std::endl;
			return;
		}

		uint8* buffer = image->getRawPtr();
		uint32 bufSize = image->getSize();

		ktxTexture_SetImageFromMemory(ktxTexture(ktxTex), 0, 0, 0, buffer, bufSize);

		if (sRGB)
		{
			ktxBasisParams params = { 0 };
			params.structSize = sizeof(params);
			params.threadCount = 8;
			params.compressionLevel = 5;
			params.qualityLevel = 255;
			params.uastc = KTX_FALSE;

			result = ktxTexture2_CompressBasisEx(ktxTex, &params);
		}			
		else
		{
			ktxBasisParams params = { 0 };
			params.structSize = sizeof(params);
			params.threadCount = 8;
			params.uastc = KTX_TRUE;
			params.uastcFlags = KTX_PACK_UASTC_LEVEL_VERYSLOW;
			params.uastcRDO = KTX_TRUE;
			params.uastcRDOQualityScalar = 0.25;
			params.uastcRDODictSize = std::numeric_limits<unsigned short>::max();
			
			//params.normalMap = KTX_TRUE;
			result = ktxTexture2_CompressBasisEx(ktxTex, &params);
			result = ktxTexture2_DeflateZstd(ktxTex, 22);
		}	

		ktxTexture_WriteToNamedFile(ktxTexture(ktxTex), filename.c_str());
		ktxTexture_Destroy(ktxTexture(ktxTex));
	}
#endif
}