#ifndef INCLUDED_GLBUFFER
#define INCLUDED_GLBUFFER

#pragma once

#include <GPU/Buffer.h>
#include <GPU/GL/GLPlatform.h>
#include <GPU/GL/GLDescriptor.h>

namespace GL
{
	class Buffer : public GPU::Buffer
	{
	public:
		Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride);
		~Buffer();
		void uploadMapped(void* data);
		void uploadStaged(void* data);
		uint8* getMappedPointer() { return data; }
		uint32 getStride() { return stride; }
		GPU::Descriptor::Ptr getDescriptor();
		GLuint getID();
		void bind();
		void bindBase(uint32 index);

    	typedef std::shared_ptr<Buffer> Ptr;
		static Ptr create(GPU::BufferUsage usage, uint32 size, uint32 stride)
		{
			return std::make_shared<Buffer>(usage, size, stride);
		}

	private:
		uint8* data = nullptr;
		GLenum target;
		GLuint buffer;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	};
}

#endif // INCLUDED_GLBUFFER