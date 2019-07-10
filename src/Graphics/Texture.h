#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#pragma once

#include <GL/GLTexture.h>
#include <memory>

class Texture2D
{
	GL::Texture2D texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;
	

public:
	Texture2D(unsigned int width, unsigned int height, GL::TextureFormat format);
	void upload(void* data);
	void setFilter(GL::TextureFilter filter);
	void setWrap(GL::TextureWrap wrap);
	void generateMipmaps();
	void use(GLuint unit);

	typedef std::shared_ptr<Texture2D> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format)
	{
		return std::make_shared<Texture2D>(width, height, format);
	}
};

#endif // INCLUDED_TEXTURE