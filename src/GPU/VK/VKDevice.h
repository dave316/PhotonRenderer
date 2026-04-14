#ifndef INCLUDED_VKDEVICE
#define INCLUDED_VKDEVICE

#pragma once

#include <GPU/VK/VKPlatform.h>

namespace VK
{
	struct Queue
	{
		vk::Queue queue;
		uint32_t familyIndex = 0;
		uint32_t index = 0;
		vk::Bool32 canPresent = false;
		vk::QueueFamilyProperties properties{};
	};

	class Device
	{
	public:
		Device();
		~Device();

		void init(vk::Instance instance);
		void destroy();

		vk::Device getDevice() {
			return device;
		}
		vk::CommandPool getCommandPool()
		{
			return cmdPool;
		}
		vk::CommandBuffer allocateCommandBuffer()
		{
			return device.allocateCommandBuffers({ cmdPool, vk::CommandBufferLevel::ePrimary, 1 }).front();
		}
		VmaAllocator getAllocator()
		{
			return allocator;
		}
		vk::Queue getSuitableGraphicsQueue();
		uint32_t getQueueFamilyIndex(vk::QueueFlagBits queueFlags);
		uint32_t getQueueByFlags(vk::QueueFlags requiredQueueFlags, uint32_t queueIndex);
		void waitIdle();

		static Device& getInstance()
		{
			static Device instance;
			return instance;
		}

	private:
		vk::PhysicalDevice gpu;
		vk::Device device;
		vk::CommandPool cmdPool;
		std::vector<std::vector<Queue>> queues;
		VmaAllocator allocator;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
	};
}

#endif // INCLUDED_VKDEVICE