#ifndef INCLUDED_GLDESCRIPTOR
#define INCLUDED_GLDESCRIPTOR

#pragma once

#include <GPU/Descriptor.h>
#include <GPU/GL/GLPlatform.h>

namespace GL
{
	class BufferDescriptor : public GPU::BufferDescriptor
	{
	public:
		BufferDescriptor(GLuint buffer);
		GLuint getBuffer();

		typedef std::shared_ptr<BufferDescriptor> Ptr;
		static Ptr create(GLuint buffer)
		{
			return std::make_shared<BufferDescriptor>(buffer);
		}

	private:
		GLuint buffer;
	};

	class ImageDescriptor : public GPU::ImageDescriptor
	{
	public:
		ImageDescriptor(GLenum target, GLuint texture, GLuint sampler);
		GLuint getTarget();
		GLuint getTexture();
		GLuint getSampler();

		typedef std::shared_ptr<ImageDescriptor> Ptr;
		static Ptr create(GLenum target, GLuint texture, GLuint sampler)
		{
			return std::make_shared<ImageDescriptor>(target, texture, sampler);
		}

	private:
		GLenum target;
		GLuint texture;
		GLuint sampler;
	};
}

#endif // INCLUDED_GLDESCRIPTOR