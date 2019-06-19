#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#pragma once

#include <GL/GLTexture.h>
#include <memory>

class Texture2D
{
	GL::Texture2D texture;
	unsigned int width;
	unsigned int height;

public:
	Texture2D(unsigned int width, unsigned int height);
	void upload(void* data);
	void use(GLuint unit);

	typedef std::shared_ptr<Texture2D> Ptr;
	static Ptr create(unsigned int width, unsigned int height)
	{
		return std::make_shared<Texture2D>(width, height);
	}
};

#endif // INCLUDED_TEXTURE