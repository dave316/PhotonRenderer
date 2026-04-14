#ifndef INCLUDED_BUFFER
#define INCLUDED_BUFFER

#pragma once

#include "Descriptor.h"
#include <GPU/Enums.h>
#include <Platform/Types.h>
#include <memory>
#include <vector>

namespace GPU
{
	class Buffer
	{
	public:
		Buffer(BufferUsage usage, uint32 size, uint32 stride) :
			usage(usage),
			size(size),
			stride(stride)
		{

		}
		virtual ~Buffer() = 0 {}
		virtual void uploadMapped(void* data) = 0;
		virtual void uploadStaged(void* data) = 0;
		virtual Descriptor::Ptr getDescriptor() = 0;
		virtual uint8* getMappedPointer() = 0;
		uint32 getSize()
		{
			return size;
		}
		uint32 getStride()
		{
			return stride;
		}
		typedef std::shared_ptr<Buffer> Ptr;
	protected:
		uint32 size = 0;
		uint32 stride = 0;
		BufferUsage usage;
	private:
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	};
}

#endif // INCLUDED_BUFFER