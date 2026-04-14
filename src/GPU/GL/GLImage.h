#ifndef INCLUDED_GLIMAGE
#define INCLUDED_GLIMAGE

#pragma once
#include <GPU/Image.h>
#include <GPU/GL/GLImageView.h>
#include <GPU/GL/GLPlatform.h>

namespace GL
{
	static GLenum getDataFormat(GPU::Format format)
	{
		GLenum glFormat = GL_RGBA;
		switch (format)
		{
			case GPU::Format::RGBA8: glFormat = GL_RGBA; break;
			case GPU::Format::SRGBA8: glFormat = GL_RGBA; break;
			case GPU::Format::RG16F: glFormat = GL_RG; break;
			case GPU::Format::RGB16F: glFormat = GL_RGB; break;
			case GPU::Format::RGBA16F: glFormat = GL_RGBA; break;
			case GPU::Format::R32F: glFormat = GL_RED; break;
			case GPU::Format::RG32F: glFormat = GL_RG; break;
			case GPU::Format::RGB32F: glFormat = GL_RGB; break;
			case GPU::Format::RGBA32F: glFormat = GL_RGBA; break;
			case GPU::Format::DEPTH16: glFormat = GL_DEPTH_COMPONENT; break;
			case GPU::Format::D24_S8: glFormat = GL_DEPTH_STENCIL; break;
		}
		return glFormat;
	}

	static GLenum getDataType(GPU::Format format)
	{
		GLenum glFormat = GL_UNSIGNED_BYTE;
		switch (format)
		{
			case GPU::Format::RGBA8: glFormat = GL_UNSIGNED_BYTE; break;
			case GPU::Format::SRGBA8: glFormat = GL_UNSIGNED_BYTE; break;
			case GPU::Format::RG16F: glFormat = GL_HALF_FLOAT; break;
			case GPU::Format::RGB16F: glFormat = GL_HALF_FLOAT; break;
			case GPU::Format::RGBA16F: glFormat = GL_HALF_FLOAT; break;
			case GPU::Format::R32F: glFormat = GL_FLOAT; break;
			case GPU::Format::RG32F: glFormat = GL_FLOAT; break;
			case GPU::Format::RGB32F: glFormat = GL_FLOAT; break;
			case GPU::Format::RGBA32F: glFormat = GL_FLOAT; break;
			case GPU::Format::DEPTH16: glFormat = GL_HALF_FLOAT; break;
			case GPU::Format::D24_S8: glFormat = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; break;
		}
		return glFormat;
	}

	class Image : public GPU::Image
	{
	public:
		Image(GPU::ImageParameters params);
		~Image();
		void uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level);
		void uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize);
		void generateMipmaps(GPU::CommandBuffer::Ptr cmdBuf);
		void setImageLayout();
		void layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf);
		void layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::ImageView::Ptr createImageView();
		GPU::ImageView::Ptr createImageView(GPU::ViewType viewType, GPU::SubResourceRange range);
		GLenum getTexTarget();
		GLuint getTexture();

		typedef std::shared_ptr<Image> Ptr;
		static Ptr create(GPU::ImageParameters params)
		{
			return std::make_shared<Image>(params);
		}

	private:
		GLuint texture;
		GLenum target;
		GLenum internalFormat;		

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
	};
}

#endif // INCLUDED_GLIMAGE