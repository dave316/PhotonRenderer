#include "GLImageView.h"

namespace GL
{
	GLenum getTarget(GPU::ViewType viewType)
	{
		GLenum target;
		switch (viewType)
		{
			case GPU::ViewType::View1D: target = GL_TEXTURE_1D; break;
			case GPU::ViewType::View2D: target = GL_TEXTURE_2D; break;
			case GPU::ViewType::View3D: target = GL_TEXTURE_3D; break;
			case GPU::ViewType::View1DArray: target = GL_TEXTURE_1D_ARRAY; break;
			case GPU::ViewType::View2DArray: target = GL_TEXTURE_2D_ARRAY; break;
			case GPU::ViewType::ViewCubeMap: target = GL_TEXTURE_CUBE_MAP; break;
			case GPU::ViewType::ViewCubeMapArray: target = GL_TEXTURE_CUBE_MAP_ARRAY; break;
		}
		return target;
	}

	GLenum getFormat(GPU::Format format)
	{
		GLenum glFormat = GL_RGBA8;
		switch (format)
		{
			case GPU::Format::R8: glFormat = GL_R8; break;
			case GPU::Format::RG8: glFormat = GL_RG8; break;
			case GPU::Format::RGB8: glFormat = GL_RGB8; break;
			case GPU::Format::RGBA8: glFormat = GL_RGBA8; break;
			case GPU::Format::SRGB8: glFormat = GL_SRGB8; break;
			case GPU::Format::SRGBA8: glFormat = GL_SRGB8_ALPHA8; break;
			case GPU::Format::R16F: glFormat = GL_R16F; break;
			case GPU::Format::RG16F: glFormat = GL_RG16F; break;
			case GPU::Format::RGB16F: glFormat = GL_RGB16F; break;
			case GPU::Format::RGBA16F: glFormat = GL_RGBA16F; break;
			case GPU::Format::R32F: glFormat = GL_R32F; break;
			case GPU::Format::RG32F: glFormat = GL_RG32F; break;
			case GPU::Format::RGB32F: glFormat = GL_RGB32F; break;
			case GPU::Format::RGBA32F: glFormat = GL_RGBA32F; break;
			case GPU::Format::DEPTH16: glFormat = GL_DEPTH_COMPONENT16; break;
			case GPU::Format::DEPTH24: glFormat = GL_DEPTH_COMPONENT24; break; // TODO: Depth24 does not exist in vulkan
			case GPU::Format::DEPTH32: glFormat = GL_DEPTH_COMPONENT32F; break;
			case GPU::Format::D24_S8: glFormat = GL_DEPTH24_STENCIL8; break;
			case GPU::Format::BC7_RGBA: glFormat = GL_COMPRESSED_RGBA_BPTC_UNORM; break;
			case GPU::Format::BC7_SRGB: glFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM; break;
		}
		return glFormat;
	}

	ImageView::ImageView(GLuint parentTexture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange) :
		GPU::ImageView(format, subRange)
	{
		GLenum target = getTarget(type);
		GLenum internalFormat = getFormat(format);

		glGenTextures(1, &texture);
		glTextureView(texture, target, parentTexture, internalFormat, subRange.baseMipLevel, subRange.levelCount, subRange.baseArrayLayer, subRange.layerCount);
	}

	ImageView::~ImageView()
	{
		glDeleteTextures(1, &texture);
	}

	GLuint ImageView::getTexture()
	{
		return texture;
	}
}
