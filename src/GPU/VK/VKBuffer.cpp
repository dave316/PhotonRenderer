#include "VKBuffer.h"
#include "VKDevice.h"
#include <iostream>

uint32 VK::Buffer::bufferCount = 0;

namespace VK
{
	Buffer::Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride) :
		allocator(Device::getInstance().getAllocator()),
		GPU::Buffer(usage, size, stride)
	{
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.usage = static_cast<vk::BufferUsageFlags>((int)usage);
		bufferCreateInfo.size = size;

		VmaAllocationCreateInfo memoryInfo{};
		if (usage & GPU::BufferUsage::VertexBuffer || usage & GPU::BufferUsage::IndexBuffer)
		{
			memoryInfo.flags = 0;
			memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}
		else
		{
			memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}

		VmaAllocationInfo allocInfo{};
		auto result = vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &memoryInfo, reinterpret_cast<VkBuffer*>(&buffer), &allocation, &allocInfo);
		if (result != VK_SUCCESS)
		{
			std::cout << "error: could not allocate memory for VkBuffer!" << std::endl;
		}

		bufferID = bufferCount;
		//std::cout << "buffer " << bufferID << " created" << std::endl;
		bufferCount++;
	}

	Buffer::~Buffer()
	{
		vmaDestroyBuffer(allocator, buffer, allocation);
		//std::cout << "buffer " << bufferID << " destroyed" << std::endl;
	}

	void Buffer::init(GPU::BufferUsage usage, uint32 size, uint32 stride)
	{
		this->usage = usage;
		this->size = size;
		this->stride = stride;

		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.usage = static_cast<vk::BufferUsageFlags>((int)usage);
		bufferCreateInfo.size = size;

		VmaAllocationCreateInfo memoryInfo{};
		//if (usage & GPU::BufferUsage::VertexBuffer || usage & GPU::BufferUsage::IndexBuffer)
		//{
		//	memoryInfo.flags = 0;
		//	memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
		//}
		//else
		{
			memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}

		VmaAllocationInfo allocInfo{};
		auto result = vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &memoryInfo, reinterpret_cast<VkBuffer*>(&buffer), &allocation, &allocInfo);
		if (result != VK_SUCCESS)
		{
			std::cout << "error: could not allocate memory for VkBuffer!" << std::endl;
		}
	}

	void Buffer::destroy()
	{
		vmaDestroyBuffer(allocator, buffer, allocation);
	}

	void Buffer::map()
	{
		vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(&data));
	}

	void Buffer::unmap()
	{
		if (data)
		{
			vmaUnmapMemory(allocator, allocation);
			data = nullptr;
		}		
	}

	void Buffer::flush(uint32 size, uint32 offset)
	{
		vmaFlushAllocation(allocator, allocation, offset, size);
	}

	void Buffer::uploadMapped(void* data)
	{
		uint8* dataPtr = nullptr;
		vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(&dataPtr));
		std::memcpy(dataPtr, data, size);
		vmaFlushAllocation(allocator, allocation, 0, size);
		vmaUnmapMemory(allocator, allocation);
	}

	void Buffer::uploadStaged(void* data)
	{
		auto stagingBuffer = VK::Buffer::create(GPU::BufferUsage::TransferSrc, size, 0);
		stagingBuffer->uploadMapped(data);

		//auto& ctx = VK::Context::getInstance();

		auto copyCmd = Device::getInstance().allocateCommandBuffer();

		//auto cmdBuf = ctx.allocateCommandBuffer();
		//auto copyCmd = std::dynamic_pointer_cast<VK::CommandBuffer>(cmdBuf)->getCommandBuffer();
		copyCmd.begin(vk::CommandBufferBeginInfo());

		vk::BufferCopy region;
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = size;
		copyCmd.copyBuffer(stagingBuffer->getBuffer(), buffer, region);

		copyCmd.end();

		auto device = Device::getInstance().getDevice();
		auto queue = Device::getInstance().getSuitableGraphicsQueue();

		auto fence = device.createFence({});

		vk::SubmitInfo submitInfo({}, {}, copyCmd);
		queue.submit(submitInfo, fence);

		vk::Result vkResult = device.waitForFences(fence, true, 100000000000);
		if (vkResult != vk::Result::eSuccess)
		{
			std::cout << "error executing command buffer!" << std::endl;
		}

		device.resetFences(fence);
		device.destroyFence(fence);
		//device.waitIdle();
	}

	uint8* Buffer::getMappedPointer()
	{
		return data;
	}

	GPU::Descriptor::Ptr Buffer::getDescriptor()
	{
		auto bufferDescriptor = vk::DescriptorBufferInfo(buffer, 0, VK_WHOLE_SIZE);
		return VK::BufferDescriptor::create(bufferDescriptor);
	}

	vk::Buffer Buffer::getBuffer()
	{
		return buffer;
	}
}
