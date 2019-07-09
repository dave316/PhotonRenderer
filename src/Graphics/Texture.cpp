#include "Texture.h"

Texture2D::Texture2D(unsigned int width, unsigned int height, bool sRGB) :
	width(width),
	height(height),
	sRGB(sRGB)
{

}

void Texture2D::upload(void* data)
{
	texture.upload(data, width, height, sRGB);
}

void Texture2D::use(GLuint unit)
{
	texture.use(unit);
}