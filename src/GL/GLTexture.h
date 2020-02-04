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
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH32F,
		RGB32F,
		RG16F
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
		case RGB8:	internalFormat = GL_RGB8; break;
		case RGBA8:	internalFormat = GL_RGBA8; break;
		case SRGB8:	internalFormat = GL_SRGB8; break;
		case SRGBA8:internalFormat = GL_SRGB8_ALPHA8; break;
		case DEPTH16:	internalFormat = GL_DEPTH_COMPONENT16; break;
		case DEPTH24:	internalFormat = GL_DEPTH_COMPONENT24; break;
		case DEPTH32:	internalFormat = GL_DEPTH_COMPONENT32; break;
		case DEPTH32F:	internalFormat = GL_DEPTH_COMPONENT32F; break;
		case RGB32F: internalFormat = GL_RGB32F; break;
		case RG16F: internalFormat = GL_RG16F; break;
		default:	internalFormat = GL_RGBA8; break;
		}
		return internalFormat;
	}

	static GLenum getDataFormat(TextureFormat format)
	{
		GLenum dataFormat = GL_RGB;
		switch (format)
		{
		case RG16F:
			dataFormat = GL_RG;
			break;
		case RGB8:
		case RGB32F:
		case SRGB8:  
			dataFormat = GL_RGB;
			break;
		case RGBA8:
		case SRGBA8:
			dataFormat = GL_RGBA;
			break;
		case DEPTH16:
		case DEPTH24:
		case DEPTH32:
		case DEPTH32F:
			dataFormat = GL_DEPTH_COMPONENT;
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
		case RG16F:
		case DEPTH16:
			dataType = GL_HALF_FLOAT;
			break;
		case RGB32F:
		case DEPTH24:
		case DEPTH32:
		case DEPTH32F:
			dataType = GL_FLOAT;
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

		void upload(const void* data, int width, int height, TextureFormat format, GLenum target = Target)
		{
			GLint internalFormat = getInternalFormat(format);
			GLenum dataFormat = getDataFormat(format);
			GLenum dataType = getDataType(format);

			// TODO: add wrap & filtering methods
			bind(); 
			glTexImage2D(target, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
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
			//unbind();
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
	typedef Texture<GL_TEXTURE_2D_MULTISAMPLE> Texture2DMultisample;
	typedef Texture<GL_TEXTURE_2D_MULTISAMPLE_ARRAY> Texture2DMultisampleArray;

	template<>
	void Texture2D::setWrap(TextureWrap wrap)
	{
		GLint wrapMode;

		switch (wrap)
		{
		case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
		case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
		case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
		case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	template<>
	void TextureCubeMap::setWrap(TextureWrap wrap)
	{
		GLint wrapMode;

		switch (wrap)
		{
		case TextureWrap::REPEAT: wrapMode = GL_REPEAT;	break;
		case TextureWrap::MIRRORED_REPEAT: wrapMode = GL_MIRRORED_REPEAT; break;
		case TextureWrap::CLAMP_TO_EDGE: wrapMode = GL_CLAMP_TO_EDGE; break;
		case TextureWrap::CLAMP_TO_BORDER: wrapMode = GL_CLAMP_TO_BORDER; break;
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

}

#endif // INCLUDED_GLTEXTURE
