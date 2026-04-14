#include "VKFramebuffer.h"
#include "VKDevice.h"
namespace VK
{
	Framebuffer::Framebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear) :
		device(Device::getInstance().getDevice()),
		width(width),
		height(height),
		layers(layers),
		offscreen(offscreen),
		clear(clear)
	{

	}

	Framebuffer::~Framebuffer()
	{
		device.destroyFramebuffer(framebuffer);
		device.destroyRenderPass(renderPass);
	}

	void Framebuffer::setClearColor(glm::vec4 color)
	{
		this->clearColor = color;
	}

	uint32 Framebuffer::getWidth()
	{
		return width;
	}

	uint32 Framebuffer::getHeight()
	{
		return height;
	}

	glm::vec4 Framebuffer::getClearColor()
	{
		return clearColor;
	}

	void Framebuffer::addAttachment(GPU::ImageView::Ptr imageView)
	{
		auto format = imageView->getViewFormat();
		vk::ClearValue clearValue;
		if (format == GPU::Format::DEPTH32 || format == GPU::Format::D24_S8)
			clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		else
			clearValue.color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

		auto view = std::dynamic_pointer_cast<ImageView>(imageView)->getImageView();
		attachments.push_back(view);
		clearValues.push_back(clearValue);
		formats.push_back(getFormat(format));
	}

	void Framebuffer::addAttachment(vk::ImageView view, vk::Format format)
	{
		vk::ClearValue clearValue;
		if (format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint)
			clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		else
			clearValue.color = vk::ClearColorValue(0.3f, 0.0f, 0.0f, 1.0f);

		attachments.push_back(view);
		clearValues.push_back(clearValue);
		formats.push_back(format);
	}

	void Framebuffer::createRenderPass()
	{
		bool useDepthAttachment = false;
		bool useStencilAttachment = false;
		int numColorAttachments = 0;

		for (auto format : formats)
		{
			if (format == vk::Format::eD24UnormS8Uint)
			{
				useDepthAttachment = true;
				useStencilAttachment = true;
			}
			else if (format == vk::Format::eD16Unorm ||
					 format == vk::Format::eD32Sfloat)
				useDepthAttachment = true;
			else
				numColorAttachments++;
		}			

		std::vector<vk::AttachmentDescription> attachments;
		for (int i = 0; i < numColorAttachments; i++)
		{
			if (offscreen)
			{
				vk::AttachmentDescription colorAttachment;
				colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;
				colorAttachment.samples = vk::SampleCountFlagBits::e1;
				if (clear)
					colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
				else
					colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
				colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
				colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				if (clear)
					colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
				else
					colorAttachment.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				attachments.push_back(colorAttachment);
			}
			else
			{
				vk::AttachmentDescription colorAttachment;
				// TODO: get format from attachment
				colorAttachment.format = vk::Format::eB8G8R8A8Srgb;
				colorAttachment.samples = vk::SampleCountFlagBits::e1;
				colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
				colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
				colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
				colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
				attachments.push_back(colorAttachment);
			}
		}

		if (useDepthAttachment)
		{
			vk::AttachmentDescription depthAttachment;
			if (useStencilAttachment)
				depthAttachment.format = vk::Format::eD24UnormS8Uint;
			else
				depthAttachment.format = vk::Format::eD32Sfloat;
			depthAttachment.samples = vk::SampleCountFlagBits::e1;
			if (clear)
				depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			else
				depthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
			depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			if (clear)
				depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
			else
				depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			if (numColorAttachments == 0)
				depthAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			else
				depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			attachments.push_back(depthAttachment);
		}

		std::vector<vk::SubpassDependency> dependencies(2);
		if (offscreen)
		{
			if (numColorAttachments == 0)
			{
				dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[0].dstSubpass = 0;
				dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
				dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
				dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
				dependencies[0].dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
				dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

				dependencies[1].srcSubpass = 0;
				dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
				dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
				dependencies[1].srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
				dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
				dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;
			}
			else
			{
				dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[0].dstSubpass = 0;
				dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
				dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
				dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
				dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

				dependencies[1].srcSubpass = 0;
				dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
				dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
				dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
				dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;
			}
		}
		else
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
			dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
			dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
			dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
			dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
			dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
			dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;
		}

		std::vector<vk::AttachmentReference> colorRefs;
		for (int i = 0; i < numColorAttachments; i++)
			colorRefs.push_back(vk::AttachmentReference(i, vk::ImageLayout::eColorAttachmentOptimal));

		vk::AttachmentReference depthRef(numColorAttachments, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		if (useDepthAttachment)
		{
			vk::SubpassDescription subpassDesc({}, vk::PipelineBindPoint::eGraphics, {}, colorRefs, {}, &depthRef);
			vk::RenderPassCreateInfo rpInfo({}, attachments, subpassDesc, dependencies);

			renderPass = device.createRenderPass(rpInfo);
		}
		else
		{
			vk::SubpassDescription subpassDesc({}, vk::PipelineBindPoint::eGraphics, {}, colorRefs);
			vk::RenderPassCreateInfo rpInfo({}, attachments, subpassDesc, dependencies);

			renderPass = device.createRenderPass(rpInfo);
		}
	}

	void Framebuffer::createFramebuffer()
	{
		createRenderPass();

		vk::FramebufferCreateInfo framebufferCI({}, renderPass, attachments, width, height, layers);
		framebuffer = device.createFramebuffer(framebufferCI);
	}

	vk::RenderPassBeginInfo Framebuffer::getRenderPassInfo()
	{
		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		return renderPassBeginInfo;
	}

	vk::RenderPass Framebuffer::getRenderPass()
	{
		return renderPass;
	}

	vk::Framebuffer Framebuffer::getFramebuffer()
	{
		return framebuffer;
	}
}