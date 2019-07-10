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