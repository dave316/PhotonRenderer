#ifndef INCLUDED_VKBUFFER
#define INCLUDED_VKBUFFER

#pragma once

#include <GPU/Buffer.h>
#include <GPU/VK/VKDescriptor.h>
#include <GPU/VK/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	class Buffer : public GPU::Buffer
	{
	public:
		Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride);
		~Buffer();
		void init(GPU::BufferUsage usage, uint32 size, uint32 stride);
		void destroy();
		void map();
		void unmap();
		void flush(uint32 size, uint32 offset);
		void uploadMapped(void* data);
		void uploadStaged(void* data);
		uint8* getMappedPointer();
		GPU::Descriptor::Ptr getDescriptor();
		vk::Buffer getBuffer();

		typedef std::shared_ptr<Buffer> Ptr;
		static Ptr create(GPU::BufferUsage usage, uint32 size, uint32 stride)
		{
			return std::make_shared<Buffer>(usage, size, stride);
		}
		static uint32 bufferCount;
		uint32 getBufferID()
		{
			return bufferID;
		}
	private:
		uint8* data = nullptr;
		uint32 bufferID;
		vk::Buffer buffer;
		VmaAllocator allocator;
		VmaAllocation allocation = VK_NULL_HANDLE;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	};
}

#endif // INCLUDED_VKBUFFER