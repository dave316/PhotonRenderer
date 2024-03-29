#ifndef INCLUDED_GLTEXTURE
#define INCLUDED_GLTEXTURE

#pragma once

#include <GL/glew.h>
#include <iostream>

namespace GL
{
	enum TextureFormat
	{
		R8,
		RG8,
		RGB8,
		RGBA8,
		SRGB8,
		SRGBA8,
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH32F,
		D24_S8,
		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
		R16F,
		RG16F,
		RGB16F,
		RGBA16F,
	};

	enum TextureFilter
	{
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR,
		NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
		LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
	};
	
	enum TextureWrap
	{
		REPEAT = GL_REPEAT,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
	};
	
	enum CupeMapFace
	{
		POS_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEG_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		POS_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		NEG_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		POS_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		NEG_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	static GLint getInternalFormat(TextureFormat format)
	{
		GLint internalFormat;
		switch (format)
		{
		case R8:		internalFormat = GL_RED; break;
		case RG8:		internalFormat = GL_RG8; break;
		case RGB8:		internalFormat = GL_RGB8; break;
		case RGBA8:		internalFormat = GL_RGBA8; break;
		case SRGB8:		internalFormat = GL_SRGB8; break;
		case SRGBA8:	internalFormat = GL_SRGB8_ALPHA8; break;
		case DEPTH16:	internalFormat = GL_DEPTH_COMPONENT16; break;
		case DEPTH24:	internalFormat = GL_DEPTH_COMPONENT24; break;
		case DEPTH32:	internalFormat = GL_DEPTH_COMPONENT32; break;
		case DEPTH32F:	internalFormat = GL_DEPTH_COMPONENT32F; break;
		case D24_S8:	internalFormat = GL_DEPTH24_STENCIL8; break;
		case R32F:		internalFormat = GL_R32F; break;
		case RG32F:		internalFormat = GL_RG32F; break;
		case RGB32F:	internalFormat = GL_RGB32F; break;
		case RGBA32F:	internalFormat = GL_RGBA32F; break;
		case R16F:		internalFormat = GL_R16F; break;
		case RG16F:		internalFormat = GL_RG16F; break;
		case RGB16F:	internalFormat = GL_RGB16F; break;
		case RGBA16F:	internalFormat = GL_RGBA16F; break;
		default:		internalFormat = GL_RGBA8; break;
		}
		return internalFormat;
	}

	static GLenum getDataFormat(TextureFormat format)
	{
		GLenum dataFormat = GL_RGB;
		switch (format)
		{
		case R8:
		case R16F:
		case R32F:
			dataFormat = GL_RED;
			break;
		case RG8:
		case RG16F:
		case RG32F:
			dataFormat = GL_RG;
			break;
		case RGB8:
		case RGB16F:
		case RGB32F:
		case SRGB8:  
			dataFormat = GL_RGB;
			break;
		case RGBA8:
		case RGBA16F:
		case RGBA32F:
		case SRGBA8:
			dataFormat = GL_RGBA;
			break;
		case DEPTH16:
		case DEPTH24:
		case DEPTH32:
		case DEPTH32F:
			dataFormat = GL_DEPTH_COMPONENT;
			break;
		case D24_S8: 
			dataFormat = GL_DEPTH_STENCIL;
			break;
		default:		
			dataFormat = GL_RGBA; 
			break;
		}
		return dataFormat;
	}

	static GLenum getDataType(TextureFormat format)
	{
		GLenum dataType;
		switch (format)
		{
		case R8:
		case RG8:
		case RGB8:
		case RGBA8:
		case SRGB8:
		case SRGBA8:
			dataType = GL_UNSIGNED_BYTE;
			break;
		case R16F:
		case RG16F:
		case RGB16F:
		case RGBA16F:
		case DEPTH16:
			dataType = GL_FLOAT;
			break;
		case R32F:
		case RG32F:
		case RGB32F:
		case RGBA32F:
		case DEPTH24:
		case DEPTH32:
		case DEPTH32F:
			dataType = GL_FLOAT;
			break;
		case D24_S8:
			dataType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			break;
		default:
			dataType = GL_UNSIGNED_BYTE; 
			break;
		}
		return dataType;
	}

	template <GLenum Target>
	class Texture
	{
		GLuint id;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

	public:
		Texture()
		{
			glGenTextures(1, &id);
			//std::cout << "tex ID: " << (int)id << std::endl;
		}

		~Texture()
		{
			glDeleteTextures(1, &id);
		}

		void upload2D(const void* data, int width, int height, TextureFormat format, GLenum target = Target)
		{
			GLint internalFormat = getInternalFormat(format);
			GLenum dataFormat = getDataFormat(format);
			GLenum dataType = getDataType(format);

			bind(); 
			glTexImage2D(target, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
			unbind();
		}

		void upload3D(const void* data, int width, int height, int depth, TextureFormat format, GLenum target = Target)
		{
			GLint internalFormat = getInternalFormat(format);
			GLenum dataFormat = getDataFormat(format);
			GLenum dataType = getDataType(format);

			bind();
			//glTexImage2D(target, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
			glTexImage3D(target, 0, internalFormat, width, height, depth, 0, dataFormat, dataType, data);
			unbind();
		}

		void download(void* data, GL::TextureFormat format)
		{
			GLenum dataFormat = getDataFormat(format);
			GLenum dataType = getDataType(format);

			bind();
			glGetTexImage(Target, 0, dataFormat, dataType, (GLvoid*)data);
			unbind();
		}

		void setFilter(TextureFilter minFilter, TextureFilter magFilter)
		{
			//GLint minFilter = GL_LINEAR;
			//GLint magFilter = GL_LINEAR;

			//switch (filter)
			//{
			//case TextureFilter::NEAREST:
			//	minFilter = GL_NEAREST;
			//	magFilter = GL_NEAREST;
			//	break;
			//case TextureFilter::LINEAR:
			//	minFilter = GL_LINEAR;
			//	magFilter = GL_LINEAR;
			//	break;
			//case TextureFilter::NEAREST_MIPMAP_NEAREST:
			//	minFilter = GL_NEAREST_MIPMAP_NEAREST;
			//	magFilter = GL_NEAREST;
			//	break;
			//case TextureFilter::NEAREST_MIPMAP_LINEAR:
			//	minFilter = GL_NEAREST_MIPMAP_LINEAR;
			//	magFilter = GL_NEAREST;
			//	break;
			//case TextureFilter::LINEAR_MIPMAP_NEAREST:
			//	minFilter = GL_LINEAR_MIPMAP_NEAREST;
			//	magFilter = GL_LINEAR;
			//	break;
			//case TextureFilter::LINEAR_MIPMAP_LINEAR:
			//	minFilter = GL_LINEAR_MIPMAP_LINEAR;
			//	magFilter = GL_LINEAR;
			//	break;
			//}

			bind();
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, magFilter);
			unbind();
		}

		void setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR = TextureWrap::REPEAT)
		{
			//GLint wrapMode;

			//switch (wrap)
			//{
			//case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
			//case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
			//case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
			//case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
			//}

			//bind();
			//glTexParameteri(Target, GL_TEXTURE_WRAP_S, wrapMode);
			//glTexParameteri(Target, GL_TEXTURE_WRAP_T, wrapMode);
			//glTexParameteri(Target, GL_TEXTURE_WRAP_R, wrapMode);
			//unbind();
		}

		void setCompareMode()
		{
			bind();
			glTexParameteri(Target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(Target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			unbind();
		}

		void generateMipmaps()
		{
			bind();
			glGenerateMipmap(Target);
			unbind();
		}

		void bind()
		{
			glBindTexture(Target, id);
		}

		void unbind()
		{
			glBindTexture(Target, 0);
		}

		void use(GLuint unit)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			bind();
		}

		operator GLuint() const
		{
			return id;
		}
	};

	typedef Texture<GL_TEXTURE_1D> Texture1D;
	typedef Texture<GL_TEXTURE_2D> Texture2D;
	typedef Texture<GL_TEXTURE_3D> Texture3D;
	typedef Texture<GL_TEXTURE_1D_ARRAY> Texture1DArray;
	typedef Texture<GL_TEXTURE_2D_ARRAY> Texture2DArray;
	typedef Texture<GL_TEXTURE_CUBE_MAP> TextureCubeMap;
	typedef Texture<GL_TEXTURE_CUBE_MAP_ARRAY> TextureCubeMapArray;
	typedef Texture<GL_TEXTURE_2D_MULTISAMPLE> Texture2DMultisample;
	typedef Texture<GL_TEXTURE_2D_MULTISAMPLE_ARRAY> Texture2DMultisampleArray;

	template<>
	void Texture2D::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap)
	{
		//GLint wrapMode = GL_REPEAT;

		//switch (wrap)
		//{
		//case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
		//case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
		//case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
		//case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
		//}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	template<>
	void Texture3D::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
	{
		//GLint wrapMode = GL_REPEAT;

		//switch (wrap)
		//{
		//case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
		//case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
		//case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
		//case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
		//}

		glBindTexture(GL_TEXTURE_3D, id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	template<>
	void TextureCubeMap::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
	{
		//GLint wrapMode;

		//switch (wrap)
		//{
		//case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
		//case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
		//case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
		//case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
		//}

		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	template<>
	void Texture2DArray::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	template<>
	void TextureCubeMapArray::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}

#endif // INCLUDED_GLTEXTURE
