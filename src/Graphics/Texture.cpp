#include "Texture.h"

Texture2D::Texture2D(unsigned int width, unsigned int height) :
	width(width),
	height(height)
{

}

void Texture2D::upload(void* data)
{
	texture.upload(data, width, height);
}

void Texture2D::use(GLuint unit)
{
	texture.use(unit);
}