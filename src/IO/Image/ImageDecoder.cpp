#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>

#ifdef WITH_KTX
#include <ktx.h>
#include <vkformat_enum.h>
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

		//std::cout << "loaded image " << width << "x" << height << "x" << channels << std::endl;

		// TODO: check errors etc.
		auto img = ImageUI8::create(width, height, channels);
		img->setFromMemory(data, width * height * channels);

		delete[] data;

		return img;
	}

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
			if (pKtxTexture->vkFormat == VK_FORMAT_BC7_UNORM_BLOCK)
				internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
			else if (pKtxTexture->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK)
				internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

			texture->bind();
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			for (int level = 0; level < pKtxTexture->numLevels; level++)
			{
				int w = pKtxTexture->baseWidth >> level;
				int h = pKtxTexture->baseHeight >> level;
				int imgSize = ktxTexture_GetImageSize(ktxTexture(pKtxTexture), level);
				ktx_size_t offset = 0;
				ktxTexture_GetImageOffset(ktxTexture(pKtxTexture), level, 0, 0, &offset);
				ktx_uint8_t* pData = pKtxTexture->pData + offset;
				glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, w, h, 0, imgSize, pData);
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		ktxTexture_Destroy(ktxTexture(pKtxTexture));

		return texture;
	}
#endif
}