#include "VKSwapchain.h"
#include "VKDevice.h"
namespace VK
{
	Swapchain::Swapchain(vk::SurfaceKHR surface, uint32 width, uint32 height) :
		GPU::Swapchain(width, height),
		surface(surface)
	{
		auto& deviceInstance = Device::getInstance();
		device = deviceInstance.getDevice();
		queue = deviceInstance.getSuitableGraphicsQueue();
		for (int i = 0; i < 2; i++)
			presentComplete[i] = device.createSemaphore({});

		depthFormat = vk::Format::eD32Sfloat;

		initSwapchain();
		initDepthStencil();
		initFramebuffers();
		currentBuffer = 0;
	}

	Swapchain::~Swapchain()
	{
		std::cout << "destroying swapchain" << std::endl;

		device.waitIdle();
		framebuffers.clear();

		for (int i = 0; i < 2; i++)
		{
			device.destroyFence(fences[i]);
			device.destroySemaphore(presentComplete[i]);
		}

		for (auto buffer : buffers)
			device.destroyImageView(buffer.view);
		buffers.clear();
		device.destroySwapchainKHR(swapchain);
		
	}

	void Swapchain::initSwapchain()
	{
		vk::Extent2D surfaceExtent = {};
		surfaceExtent.width = width;
		surfaceExtent.height = height;

		uint32_t imageCount = 2;

		vk::SwapchainKHR oldSwapchain = swapchain;

		surfaceFormat.format = vk::Format::eB8G8R8A8Srgb;
		surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

		vk::SwapchainCreateInfoKHR info;
		info.surface = surface;
		info.minImageCount = imageCount;
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent.width = surfaceExtent.width;
		info.imageExtent.height = surfaceExtent.height;
		info.imageArrayLayers = 1;
		info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		info.imageSharingMode = vk::SharingMode::eExclusive;
		info.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		info.presentMode = vk::PresentModeKHR::eMailbox;
		info.clipped = true;
		info.oldSwapchain = oldSwapchain;

		swapchain = device.createSwapchainKHR(info);
		if (!swapchain)
		{
			std::cout << "error creating swapchain!" << std::endl;
		}

		//if (oldSwapchain)
		//{
		//	//for (auto buffer : buffers)
		//	device.destroyImageView(buffer.view);
		//	//device.destroyImage(buffer.image);
		//	//buffers.clear();
		//	device.destroySwapchainKHR(oldSwapchain);
		//}

		auto swapchainImages = device.getSwapchainImagesKHR(swapchain);
		std::cout << "created " << swapchainImages.size() << " swapchain images" << std::endl;

		buffers.clear();
		buffers.reserve(swapchainImages.size());
		for (auto& image : swapchainImages)
		{
			vk::ImageViewCreateInfo colorAttachmentView;
			colorAttachmentView.image = image;
			colorAttachmentView.viewType = vk::ImageViewType::e2D;
			colorAttachmentView.format = surfaceFormat.format;
			colorAttachmentView.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
			colorAttachmentView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;

			SwapchainBuffer buffer;
			buffer.image = image;
			buffer.view = device.createImageView(colorAttachmentView);
			buffers.push_back(buffer);
		}
	}

	void Swapchain::initDepthStencil()
	{
		GPU::ImageParameters params;
		params.type = GPU::ViewType::View2D;
		params.format = GPU::Format::DEPTH32;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = 1;
		params.levels = 1;
		params.usage = GPU::ImageUsage::DepthStencilAttachment;
		depthBuffer = Image::create(params);
		depthView = depthBuffer->createImageView();
	}

	void Swapchain::initFramebuffers()
	{
		framebuffers.clear();

		for (auto& buffer : buffers)
		{
			auto fbo = VK::Framebuffer::create(width, height, 1, false, true);
			fbo->addAttachment(buffer.view, surfaceFormat.format);
			fbo->addAttachment(depthView);
			fbo->createFramebuffer();
			framebuffers.push_back(fbo);
		}
	}

	void Swapchain::resize(uint32 width, uint32 height)
	{
		this->width = width;
		this->height = height;

		queue.waitIdle();
		initSwapchain();
		initDepthStencil();
		initFramebuffers();
		currentBuffer = 0;
	}

	int Swapchain::acquireNextFrame()
	{
		if (fences[currentBuffer])
		{
			auto waitResult = device.waitForFences(fences[currentBuffer], true, std::numeric_limits<uint64_t>::max());
			device.resetFences(fences[currentBuffer]);
		}
		else
		{
			fences[currentBuffer] = device.createFence({});
		}

		vk::Result result;
		try
		{
			uint32 index;
			std::tie(result, index) = device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), presentComplete[currentBuffer], nullptr);
			//std::cout << "index: " << index << std::endl;
		}
		catch (vk::OutOfDateKHRError&)
		{
			result = vk::Result::eErrorOutOfDateKHR;
		}
		if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		{
			std::cout << "error swapchain out of date!" << std::endl;
		}
		return currentBuffer;
	}

	void Swapchain::present(GPU::CommandBuffer::Ptr lastBuffer)
	{
		auto vkCmdBuf = std::dynamic_pointer_cast<CommandBuffer>(lastBuffer);
		auto cmdBuf = vkCmdBuf->getCommandBuffer();
		auto waitSem = vkCmdBuf->getSemaphore();

		vk::PresentInfoKHR presentInfo({}, swapchain, currentBuffer);
		presentInfo.setWaitSemaphores(waitSem);
		vk::Result presentResult;
		try
		{
			presentResult = queue.presentKHR(presentInfo);
		}
		catch (const vk::SystemError& e)
		{
			if (e.code() == vk::Result::eErrorOutOfDateKHR)
			{
				//resize(window);
				return;
			}
			else
			{
				throw;
			}
		}
		currentBuffer = (currentBuffer + 1) % 2;
	}

	GPU::Framebuffer::Ptr Swapchain::getFramebuffer(uint32 index)
	{
		return framebuffers[index];
	}
}

