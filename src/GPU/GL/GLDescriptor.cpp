#include "GLDescriptor.h"

namespace GL
{
	BufferDescriptor::BufferDescriptor(GLuint buffer) :
		buffer(buffer)
	{

	}

	GLuint BufferDescriptor::getBuffer()
	{
		return buffer;
	}

	ImageDescriptor::ImageDescriptor(GLenum target, GLuint texture, GLuint sampler) :
		target(target),
		texture(texture),
		sampler(sampler)
	{

	}

	GLuint ImageDescriptor::getTarget()
	{
		return target;
	}

	GLuint ImageDescriptor::getTexture()
	{
		return texture;
	}

	GLuint ImageDescriptor::getSampler()
	{
		return sampler;
	}
}