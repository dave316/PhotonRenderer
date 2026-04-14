#define VMA_IMPLEMENTATION

#include "VKContext.h"
#include <Platform/Win32/Win32Window.h>
#include <GPU/VK/VKDevice.h>
#include <vulkan/vulkan_win32.h>
#include <iostream>

namespace VK
{
	Context::Context()
	{
		createInstance();

		Device::getInstance().init(instance);

		auto& deviceInstance = Device::getInstance();
		auto device = deviceInstance.getDevice();
		queue = deviceInstance.getSuitableGraphicsQueue();

		std::cout << "Created Vulkan Context" << std::endl;
	}

	Context::~Context()
	{
		auto& deviceInstance = Device::getInstance();
		auto device = deviceInstance.getDevice();

		Device::getInstance().destroy();

		vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);

		instance.destroySurfaceKHR(surface);
		instance.destroy();

		std::cout << "Destroyed Vulkan Context" << std::endl;
	}

	GPU::Buffer::Ptr Context::createBuffer(GPU::BufferUsage usage, uint32 size, uint32 stride)
	{
		return Buffer::create(usage, size, stride);
	}

	GPU::CommandBuffer::Ptr Context::allocateCommandBuffer()
	{
		auto& device = Device::getInstance().getDevice();
		auto& cmdPool = Device::getInstance().getCommandPool();

		vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers({ cmdPool, vk::CommandBufferLevel::ePrimary, 1 }).front();
		return CommandBuffer::create(cmdBuffer);
	}

	GPU::ComputePipeline::Ptr Context::createComputePipeline(std::string name)
	{
		return ComputePipeline::create(name);
	}

	GPU::DescriptorPool::Ptr Context::createDescriptorPool()
	{
		auto pool = DescriptorPool::create();
		return pool;
	}

	GPU::Framebuffer::Ptr Context::createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear)
	{
		return Framebuffer::create(width, height, layers, offscreen, clear);
	}

	GPU::GraphicsPipeline::Ptr Context::createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments)
	{
		auto vkFramebuffer = std::dynamic_pointer_cast<VK::Framebuffer>(framebuffer);
		return GraphicsPipeline::create(vkFramebuffer->getRenderPass(), name, numAttachments);
	}

	GPU::Image::Ptr Context::createImage(GPU::ImageParameters params)
	{
		return Image::create(params);
	}

	GPU::ImageDescriptor::Ptr Context::createImageDescriptor(GPU::Image::Ptr image, GPU::ImageView::Ptr view, GPU::Sampler::Ptr sampler)
	{
		vk::DescriptorImageInfo imageDesc;
		imageDesc.imageLayout = std::dynamic_pointer_cast<Image>(image)->getLayout();
		imageDesc.imageView = std::dynamic_pointer_cast<ImageView>(view)->getImageView();
		imageDesc.sampler = std::dynamic_pointer_cast<Sampler>(sampler)->getSampler();
		return ImageDescriptor::create(imageDesc);
	}

	GPU::Sampler::Ptr Context::createSampler(uint32 levels)
	{
		return Sampler::create(levels);
	}

	GPU::Swapchain::Ptr Context::createSwapchain(Window::Ptr window)
	{
		auto win32Window = std::dynamic_pointer_cast<Win32Window>(window);
		
		// surface
		vk::Win32SurfaceCreateInfoKHR surfaceInfo;
		surfaceInfo.hinstance = win32Window->getPlatformHandle();
		surfaceInfo.hwnd = win32Window->getWindowHandle();
		surface = instance.createWin32SurfaceKHR(surfaceInfo);
		if (!surface)
		{
			std::cout << "error creating window surface!" << std::endl;
		}

		auto swapchain = Swapchain::create(surface, window->getWidth(), window->getHeight());

		return swapchain;
	}

	void Context::submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		auto vkSwapchain = std::dynamic_pointer_cast<Swapchain>(swapchain);
		auto vkNextCmdBuf = std::dynamic_pointer_cast<CommandBuffer>(nextCmdBuf);
		auto cmdBuf = vkNextCmdBuf->getCommandBuffer();
		auto vkPrevSem = vkSwapchain->getSemaphore();
		auto vkNextSem = vkNextCmdBuf->getSemaphore();
		fence = vkSwapchain->getFence(); // TODO: this is not a good solution...

		vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo submitInfo;
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.setWaitSemaphores(vkPrevSem);
		submitInfo.setSignalSemaphores(vkNextSem);
		submitInfo.setCommandBuffers(cmdBuf);
		queue.submit(submitInfo, fence);
	}

	void Context::submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		auto vkPrevCmdBuf = std::dynamic_pointer_cast<CommandBuffer>(prevCmdBuf);
		auto vkNextCmdBuf = std::dynamic_pointer_cast<CommandBuffer>(nextCmdBuf);
		auto cmdBuf = vkNextCmdBuf->getCommandBuffer();
		auto vkPrevSem = vkPrevCmdBuf->getSemaphore();
		auto vkNextSem = vkNextCmdBuf->getSemaphore();

		vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo submitInfo;
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.setWaitSemaphores(vkPrevSem);
		submitInfo.setSignalSemaphores(vkNextSem);
		submitInfo.setCommandBuffers(cmdBuf);
		queue.submit(submitInfo, nullptr);
	}	

	void Context::waitDeviceIdle()
	{
		Device::getInstance().waitIdle();
	}

	void Context::createInstance()
	{
		// instance
		std::vector<const char*> requiredInstanceExtensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
		};
		std::vector<const char*> requiredValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		// TODO: check if extensions&layers are available
		//std::cout << "--- Instance Extensions ---" << std::endl;
		//auto instanceExtensions = vk::enumerateInstanceExtensionProperties();
		//for (auto instanceExt : instanceExtensions)
		//	std::cout << instanceExt.extensionName << std::endl;
		//std::cout << "--- Validation Layers ---" << std::endl;
		//auto supportedValidationLayers = vk::enumerateInstanceLayerProperties();
		//for (auto instanceExt : supportedValidationLayers)
		//	std::cout << instanceExt.layerName << std::endl;

		std::vector<const char*> activeInstanceExtensions(requiredInstanceExtensions);
		activeInstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		activeInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		std::vector<const char*> requestedValidationLayers(requiredValidationLayers);

		vk::ApplicationInfo app;
		app.pApplicationName = "SimpleViewer";
		app.pEngineName = "PhotonRenderer";
		app.apiVersion = VK_API_VERSION_1_4;
		vk::InstanceCreateInfo instanceInfo({}, &app, requestedValidationLayers, activeInstanceExtensions);

		instance = vk::createInstance(instanceInfo);

		vkCreateDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
			(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
		vkDebugReportMessageEXT =
			reinterpret_cast<PFN_vkDebugReportMessageEXT>
			(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));
		vkDestroyDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
			(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

		VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
		callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		callbackCreateInfo.pNext = nullptr;
		callbackCreateInfo.flags =
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		callbackCreateInfo.pfnCallback = &debugCallbackFunc;
		callbackCreateInfo.pUserData = nullptr;

		VkResult result = vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
		if (!callback)
		{
			std::cout << "error creating debug callback!" << std::endl;
		}
	}
}