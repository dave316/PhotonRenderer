#include "GLBuffer.h"
#include "GLEnums.h"

namespace GL
{
	Buffer::Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride) :
		GPU::Buffer(usage, size, stride),
		target(getBufferTarget(usage))
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(target, buffer);
		glBufferStorage(target, size, NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &buffer);
	}

	void Buffer::uploadMapped(void* data)
	{
		glBindBuffer(target, buffer);
		glBufferSubData(target, 0, size, data);
		//glBufferData(target, size, data, GL_STATIC_DRAW);
	}

	void Buffer::uploadStaged(void* data)
	{
		glBindBuffer(target, buffer);
		glBufferSubData(target, 0, size, data);
		//glBufferData(target, size, data, GL_STATIC_DRAW);
	}

	GPU::Descriptor::Ptr Buffer::getDescriptor()
	{
		return BufferDescriptor::create(buffer);
	}

	GLuint Buffer::getID()
	{
		return buffer;
	}

	void Buffer::bind()
	{
		glBindBuffer(target, buffer);
	}

	void Buffer::bindBase(uint32 index)
	{
		glBindBufferBase(target, index, buffer);
	}
}