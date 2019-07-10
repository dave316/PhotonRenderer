#ifndef INCLUDED_GLTEXTURE
#define INCLUDED_GLTEXTURE

#pragma once

#include <GL/glew.h>

namespace GL
{
	enum TextureFormat
	{
		RGB8,
		RGBA8,
		SRGB8,
		SRGBA8,
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

	static GLint getInternalFormat(TextureFormat format)
	{
		GLint internalFormat;
		switch (format)
		{
		case RGB8:	internalFormat = GL_RGB8; break;
		case RGBA8:	internalFormat = GL_RGBA8; break;
		case SRGB8:	internalFormat = GL_SRGB8; break;
		case SRGBA8:internalFormat = GL_SRGB8_ALPHA8; break;
		default:	internalFormat = GL_RGBA8; break;
		}
		return internalFormat;
	}

	static GLenum getDataFormat(TextureFormat format)
	{
		GLenum dataFormat = GL_RGB;
		switch (format)
		{
		case RGB8:
		case SRGB8:
			dataFormat = GL_RGB;
			break;
		case RGBA8:
		case SRGBA8:
			dataFormat = GL_RGBA;
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
		case RGB8:
		case RGBA8:
		case SRGB8:
		case SRGBA8:
			dataType = GL_UNSIGNED_BYTE;
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
		}

		~Texture()
		{
			glDeleteTextures(1, &id);
		}

		void upload(const void* data, int width, int height, TextureFormat format)
		{
			GLint internalFormat = getInternalFormat(format);
			GLenum dataFormat = getDataFormat(format);
			GLenum dataType = getDataType(format);

			// TODO: add wrap & filtering methods
			bind(); 
			glTexImage2D(Target, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
			unbind();
		}

		void setFilter(TextureFilter filter)
		{
			GLint minFilter = GL_LINEAR;
			GLint magFilter = GL_LINEAR;

			switch (filter)
			{
			case TextureFilter::NEAREST:
				minFilter = GL_NEAREST;
				magFilter = GL_NEAREST;
				break;
			case TextureFilter::LINEAR:
				minFilter = GL_LINEAR;
				magFilter = GL_LINEAR;
				break;
			case TextureFilter::NEAREST_MIPMAP_NEAREST:
				minFilter = GL_NEAREST_MIPMAP_NEAREST;
				magFilter = GL_NEAREST;
				break;
			case TextureFilter::NEAREST_MIPMAP_LINEAR:
				minFilter = GL_NEAREST_MIPMAP_LINEAR;
				magFilter = GL_NEAREST;
				break;
			case TextureFilter::LINEAR_MIPMAP_NEAREST:
				minFilter = GL_LINEAR_MIPMAP_NEAREST;
				magFilter = GL_LINEAR;
				break;
			case TextureFilter::LINEAR_MIPMAP_LINEAR:
				minFilter = GL_LINEAR_MIPMAP_LINEAR;
				magFilter = GL_LINEAR;
				break;
			}

			bind();
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, magFilter);
			unbind();
		}

		void setWrap(TextureWrap wrap)
		{
			GLint wrapMode;

			switch (wrap)
			{
			case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
			case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
			case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
			case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
			}

			bind();
			glTexParameteri(Target, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(Target, GL_TEXTURE_WRAP_T, wrapMode);
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
	};

	typedef Texture<GL_TEXTURE_2D> Texture2D;
}

#endif // INCLUDED_GLTEXTURE
