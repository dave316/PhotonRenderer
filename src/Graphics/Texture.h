#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#pragma once

#include <GL/GLTexture.h>
#include <memory>

class Texture
{
public:
	virtual GLuint getID() = 0;
	virtual void use(GLuint unit) = 0;
	typedef std::shared_ptr<Texture> Ptr;
};

class Texture2D : public Texture
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
	GLuint getID();

	typedef std::shared_ptr<Texture2D> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format)
	{
		return std::make_shared<Texture2D>(width, height, format);
	}
};

class TextureCubeMap : public Texture
{
	GL::TextureCubeMap texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;

public:
	TextureCubeMap(unsigned int width, unsigned int height, GL::TextureFormat format);
	void upload(void** data);
	void setFilter(GL::TextureFilter filter);
	void setWrap(GL::TextureWrap wrap);
	void generateMipmaps();
	void use(GLuint unit);
	GLuint getID();

	typedef std::shared_ptr<TextureCubeMap> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format)
	{
		return std::make_shared<TextureCubeMap>(width, height, format);
	}
};

#endif // INCLUDED_TEXTURE