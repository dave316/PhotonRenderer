#ifndef INCLUDED_GLTEXTURE
#define INCLUDED_GLTEXTURE

#pragma once

#include <GL/glew.h>

namespace GL
{
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

		void upload(const void* data, int width, int height)
		{
			bind();
			glTexImage2D(Target, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
