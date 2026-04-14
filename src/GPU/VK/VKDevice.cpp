#include "VKDevice.h"
#include <iostream>
namespace VK
{
	Device::Device()
	{

	}

	Device::~Device()
	{

	}

	void Device::init(vk::Instance instance)
	{
		// physical GPU
		bool highPriorityGraphicsQueue = true;
		auto gpus = instance.enumeratePhysicalDevices();
		gpu = gpus[0];
		std::cout << "found " << gpus.size() << " GPUs" << std::endl;
		for (auto& g : gpus)
		{
			bool found = false;
			auto props = g.getProperties();
			std::cout << props.deviceName << std::endl;
			if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				//size_t queueCount = g.getQueueFamilyProperties().size();
				//for (size_t queueIdx = 0; queueIdx < queueCount; queueIdx++)
				//{
				//	if (g.getSurfaceSupportKHR(queueIdx, surface))
				//	{
				//		gpu = g;
				//		found = true;
				//		break;
				//	}
				//}
			}
			if (found)
				break;
		}

		// queues
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = gpu.getQueueFamilyProperties();
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(queueFamilyProperties.size());
		std::vector<std::vector<float>> queuePriorities(queueFamilyProperties.size());
		for (uint32_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilyProperties.size(); queueFamilyIdx++)
		{
			const auto& queueFamilyProps = queueFamilyProperties[queueFamilyIdx];
			if (highPriorityGraphicsQueue)
			{
				uint32_t graphicsQueueFamilyIdx = getQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
				if (graphicsQueueFamilyIdx == queueFamilyIdx)
				{
					queuePriorities[queueFamilyIdx].reserve(queueFamilyProps.queueCount);
					queuePriorities[queueFamilyIdx].push_back(1.0f);
					for (uint32_t i = 1; i < queueFamilyProps.queueCount; i++)
						queuePriorities[queueFamilyIdx].push_back(0.5);
				}
				else
				{
					queuePriorities[queueFamilyIdx].resize(queueFamilyProps.queueCount, 0.5f);
				}
			}
			else
			{
				queuePriorities[queueFamilyIdx].resize(queueFamilyProps.queueCount, 0.5);
			}

			vk::DeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[queueFamilyIdx];

			queueCreateInfo.queueFamilyIndex = queueFamilyIdx;
			queueCreateInfo.queueCount = queueFamilyProps.queueCount;
			queueCreateInfo.pQueuePriorities = queuePriorities[queueFamilyIdx].data();
		}

		// device
		std::vector<const char*> requiredDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
		};
		auto deviceExtensions = gpu.enumerateDeviceExtensionProperties(); // TODO: check if required extensions available
		auto features = gpu.getFeatures();

		//std::cout << "geometry shader: " << features.geometryShader << std::endl;
		//std::cout << "--- Device Extensions ---" << std::endl;
		//for (auto ext : deviceExtensions)
		//	std::cout << ext.extensionName << std::endl;

		auto descriptorIndexingFeatures = vk::PhysicalDeviceDescriptorIndexingFeatures();
		descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
		descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
		descriptorIndexingFeatures.runtimeDescriptorArray = true;
		descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = true;

		auto extendedDynamicStateFeatures = vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT();
		extendedDynamicStateFeatures.extendedDynamicState = true;
		extendedDynamicStateFeatures.pNext = &descriptorIndexingFeatures;

		vk::DeviceCreateInfo deviceInfo({}, queueCreateInfos, {}, requiredDeviceExtensions, &features, &extendedDynamicStateFeatures);
		device = gpu.createDevice(deviceInfo);
		queues.resize(queueFamilyProperties.size());

		for (uint32_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilyProperties.size(); queueFamilyIdx++)
		{
			vk::QueueFamilyProperties& queueFamilyProps = queueFamilyProperties[queueFamilyIdx];
			//vk::Bool32 presentSupported = gpu.getSurfaceSupportKHR(queueFamilyIdx, surface);
			for (uint32_t queueIdx = 0; queueIdx < queueFamilyProps.queueCount; queueIdx++)
			{
				Queue q;
				q.familyIndex = queueFamilyIdx;
				q.index = queueIdx;
				q.canPresent = true;
				q.properties = queueFamilyProps;
				q.queue = device.getQueue(queueFamilyIdx, queueIdx);
				queues[queueFamilyIdx].push_back(q);
			}
		}

		uint32_t queueFamilyIndex = getQueueByFlags(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute, 0);
		vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
		cmdPool = device.createCommandPool(cmdPoolInfo);

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.physicalDevice = static_cast<VkPhysicalDevice>(gpu);
		allocatorInfo.device = static_cast<VkDevice>(device);
		allocatorInfo.instance = static_cast<VkInstance>(instance);
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

		VkResult vkResult = vmaCreateAllocator(&allocatorInfo, &allocator);
		if (vkResult != VK_SUCCESS)
		{
			std::cout << "error creating memory allocator!" << std::endl;
		}
	}

	void Device::destroy()
	{
		vmaDestroyAllocator(allocator);

		queues.clear();

		device.destroyCommandPool(cmdPool);
		device.destroy();
	}
	
	vk::Queue Device::getSuitableGraphicsQueue()
	{
		for (size_t queueFamilyIdx = 0; queueFamilyIdx < queues.size(); queueFamilyIdx++)
		{
			Queue& firstQueue = queues[queueFamilyIdx][0];
			uint32_t queueCount = firstQueue.properties.queueCount;
			if (firstQueue.canPresent && 0 < queueCount)
			{
				return queues[queueFamilyIdx][0].queue;
			}
		}
		auto idx = getQueueByFlags(vk::QueueFlagBits::eGraphics, 0);
		return queues[idx][0].queue;
	}

	uint32_t Device::getQueueFamilyIndex(vk::QueueFlagBits queueFlags)
	{
		const auto& queueFamilyProperties = gpu.getQueueFamilyProperties();
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			if (queueFamilyProperties[i].queueFlags & queueFlags)
				return i;
		}
		return 0;
	}

	uint32_t Device::getQueueByFlags(vk::QueueFlags requiredQueueFlags, uint32_t queueIndex)
	{
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queues.size(); queueFamilyIndex++)
		{
			Queue& firstQueue = queues[queueFamilyIndex][0];
			vk::QueueFlags queueFlags = firstQueue.properties.queueFlags;
			uint32_t queueCount = firstQueue.properties.queueCount;
			if (((queueFlags & requiredQueueFlags) == requiredQueueFlags) && queueIndex < queueCount)
				return queueFamilyIndex;
		}
		return 0;
	}

	void Device::waitIdle()
	{
		device.waitIdle();
	}
}