#include "Texture.h"

Texture2D::Texture2D(unsigned int width, unsigned int height, GL::TextureFormat format) :
	width(width),
	height(height),
	format(format)
{
	texture.upload(nullptr, width, height, format);
	texture.setFilter(GL::LINEAR);
	texture.setWrap(GL::REPEAT);
}

void Texture2D::upload(void* data)
{
	texture.upload(data, width, height, format);
}

void Texture2D::setFilter(GL::TextureFilter filter)
{
	texture.setFilter(filter);
}

void Texture2D::setWrap(GL::TextureWrap wrap)
{
	texture.setWrap(wrap);
}

void Texture2D::generateMipmaps()
{
	texture.generateMipmaps();
}

void Texture2D::use(GLuint unit)
{
	texture.use(unit);
}

GLuint Texture2D::getID()
{
	return texture;
}

const unsigned int NUM_CUBEMAP_FACES = 6;

TextureCubeMap::TextureCubeMap(unsigned int width, unsigned int height, GL::TextureFormat format):
	width(width),
	height(height),
	format(format)
{
	for (int f = 0; f < NUM_CUBEMAP_FACES; f++)
		texture.upload(nullptr, width, height, format, GL::POS_X + f);

	setFilter(GL::LINEAR);
	setWrap(GL::CLAMP_TO_EDGE);
}

void TextureCubeMap::upload(void** data)
{
	for (int f = 0; f < NUM_CUBEMAP_FACES; f++)
	{
		texture.upload(data[f], width, height, format, GL::POS_X + f);
	}
}

void TextureCubeMap::setFilter(GL::TextureFilter filter)
{
	texture.setFilter(filter);
}

void TextureCubeMap::setWrap(GL::TextureWrap wrap)
{
	texture.setWrap(wrap);
}

void TextureCubeMap::generateMipmaps()
{
	texture.generateMipmaps();
}

void TextureCubeMap::use(GLuint unit)
{
	texture.use(unit);
}

GLuint TextureCubeMap::getID()
{
	return texture;
}