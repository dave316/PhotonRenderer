#include "Texture.h"

Texture2D::Texture2D(unsigned int width, unsigned int height, GL::TextureFormat format) :
	width(width),
	height(height),
	format(format)
{
	texture.upload(nullptr, width, height, format);
	texture.setFilter(GL::LINEAR, GL::LINEAR);
	texture.setWrap(GL::REPEAT, GL::REPEAT);
}

void Texture2D::upload(void* data)
{
	texture.upload(data, width, height, format);
}

void Texture2D::download(void* data)
{
	texture.download(data, format);
}

void Texture2D::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void Texture2D::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT)
{
	texture.setWrap(wrapS, wrapT);
}

void Texture2D::generateMipmaps()
{
	texture.generateMipmaps();
}

void Texture2D::bind()
{
	texture.bind();
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

	setFilter(GL::LINEAR, GL::LINEAR);
	setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
}

void TextureCubeMap::upload(void** data)
{
	for (int f = 0; f < NUM_CUBEMAP_FACES; f++)
	{
		texture.upload(data[f], width, height, format, GL::POS_X + f);
	}
}

void TextureCubeMap::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void TextureCubeMap::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR)
{
	texture.setWrap(wrapS, wrapT, wrapR);
}

void TextureCubeMap::setCompareMode()
{
	texture.setCompareMode();
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