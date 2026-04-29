#include "GLImage.h"
#include "GLCommandBuffer.h"

#include <iostream>

namespace GL
{
	Image::Image(GPU::ImageParameters params) :
		GPU::Image(params)
	{
		target = getTarget(type);
		internalFormat = getFormat(format);

		glGenTextures(1, &texture);
		glBindTexture(target, texture);

		switch (target)
		{
		case GL_TEXTURE_1D:
			glTexStorage1D(target, levels, internalFormat, extent.width);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_CUBE_MAP:
			glTexStorage2D(target, levels, internalFormat, extent.width, extent.height);
			break;
		case GL_TEXTURE_3D:
			glTexStorage3D(target, levels, internalFormat, extent.width, extent.height, extent.depth);
			break;
		case GL_TEXTURE_2D_ARRAY:
			glTexStorage3D(target, levels, internalFormat, extent.width, extent.height, layers);
			break;
		case GL_TEXTURE_CUBE_MAP_ARRAY: // TODO: for cube map arrays the layers parameter should be: layers = arrayLayers * 6
			glTexStorage3D(target, levels, internalFormat, extent.width, extent.height, layers);
			break;
		}
	}

	Image::~Image()
	{
		glDeleteTextures(1, &texture);
	}

	void Image::uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level)
	{
		if (format >= GPU::Format::BC7_RGBA)
		{
			glBindTexture(target, texture);
			glCompressedTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, extent.width >> level, extent.height >> level, internalFormat, dataSize, data);
		}
		else
		{
			GLenum dataformat = getDataFormat(format);
			GLenum dataType = getDataType(format);
			if (type == GPU::ViewType::View2D)
			{
				glBindTexture(target, texture);
				glTexSubImage2D(target, level, 0, 0, extent.width >> level, extent.height >> level, dataformat, dataType, data);
			}
			if (type == GPU::ViewType::ViewCubeMap)
			{
				glBindTexture(target, texture);
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, 0, 0, extent.width >> level, extent.height >> level, dataformat, dataType, data);
			}
		}
	}

	void Image::uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize)
	{
		GLenum dataformat = getDataFormat(format);
		GLenum dataType = getDataType(format);
		if (type == GPU::ViewType::View2DArray)
		{
			glBindTexture(target, texture);
			glTexSubImage3D(target, 0, 0, 0, 0, extent.width, extent.height, layers, dataformat, dataType, data);
		}
		if (type == GPU::ViewType::View3D)
		{
			glBindTexture(target, texture);
			glTexSubImage3D(target, 0, 0, 0, 0, extent.width, extent.height, extent.depth, dataformat, dataType, data);
		}
	}

	void Image::generateMipmaps(GPU::CommandBuffer::Ptr cmdBuf)
	{
		//glBindTexture(target, texture);
		//glGenerateMipmap(target);
		auto glCmdBuf = std::dynamic_pointer_cast<CommandBuffer>(cmdBuf);
		glCmdBuf->generateMipmap(target, texture);
	}

	void Image::setImageLayout()
	{

	}

	void Image::layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf)
	{

	}

	void Image::layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{

	}

	GPU::ImageView::Ptr Image::createImageView()
	{
		return ImageView::create(texture, type, format, GPU::SubResourceRange(0, 0, levels, layers));
	}

	GPU::ImageView::Ptr Image::createImageView(GPU::ViewType viewType, GPU::SubResourceRange range)
	{
		return ImageView::create(texture, viewType, format, range);
	}

	GLenum Image::getTexTarget()
	{
		return target;
	}

	GLuint Image::getTexture()
	{
		return texture;
	}

	typedef std::shared_ptr<Image> Ptr;
	static Ptr create(GPU::ImageParameters params)
	{
		return std::make_shared<Image>(params);
	}
}