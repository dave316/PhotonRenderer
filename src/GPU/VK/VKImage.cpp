#include "VKImage.h"
#include "VKDevice.h"
namespace VK
{
	vk::ImageType getImageType(GPU::ViewType view)
	{
		vk::ImageType imageType;
		switch (view)
		{
		case GPU::ViewType::View1D:
		case GPU::ViewType::View1DArray:
			imageType = vk::ImageType::e1D;
			break;
		case GPU::ViewType::View2D:
		case GPU::ViewType::View2DArray:
		case GPU::ViewType::ViewCubeMap:
		case GPU::ViewType::ViewCubeMapArray:
			imageType = vk::ImageType::e2D;
			break;
		case GPU::ViewType::View3D:
			imageType = vk::ImageType::e3D;
			break;
		}
		return imageType;
	}

	Image::Image(GPU::ImageParameters params) :
		device(Device::getInstance().getDevice()),
		allocator(Device::getInstance().getAllocator()),
		GPU::Image(params)
	{
		vk::ImageCreateInfo imageCreateInfo;
		imageCreateInfo.imageType = getImageType(type);
		imageCreateInfo.format = getFormat(format);
		imageCreateInfo.mipLevels = levels;
		imageCreateInfo.arrayLayers = layers;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = vk::Extent3D(extent.width, extent.height, extent.depth);
		imageCreateInfo.usage = static_cast<vk::ImageUsageFlags>((int)usage);

		if (type == GPU::ViewType::ViewCubeMap || type == GPU::ViewType::ViewCubeMapArray)
			imageCreateInfo.flags |= vk::ImageCreateFlagBits::eCubeCompatible;
		//if (type == GPU::ViewType::View2DArray)
		//	imageCreateInfo.flags |= vk::ImageCreateFlagBits::e2DArrayCompatible;

		VmaAllocationCreateInfo memoryInfo{};
		VmaAllocationInfo allocInfo{};

		auto result = vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &memoryInfo, reinterpret_cast<VkImage*>(&image), &allocation, &allocInfo);
		if (result != VK_SUCCESS)
		{
			std::cout << "error: could not allocate memory for VkImage!" << std::endl;
		}
	}

	Image::~Image()
	{
		vmaDestroyImage(allocator, image, allocation);
	}

	void Image::uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level)
	{
		auto stagingBuffer = VK::Buffer::create(GPU::BufferUsage::TransferSrc, dataSize, 0);
		stagingBuffer->uploadMapped(data);

		vk::BufferImageCopy bufferCopyRegion;
		bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = extent.width >> level;
		bufferCopyRegion.imageExtent.height = extent.height >> level;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		auto copyCmd = std::dynamic_pointer_cast<VK::CommandBuffer>(cmdBuf)->getCommandBuffer();
		copyCmd.begin(vk::CommandBufferBeginInfo());

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = level;
		subresourceRange.baseArrayLayer = layer;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = {};
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
		copyCmd.copyBufferToImage(stagingBuffer->getBuffer(), image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion);

		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemoryBarrier);
		copyCmd.end();

		imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		cmdBuf->flush();
	}

	void Image::uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize)
	{
		auto stagingBuffer = VK::Buffer::create(GPU::BufferUsage::TransferSrc, dataSize, 0);
		stagingBuffer->uploadMapped(data);

		vk::BufferImageCopy bufferCopyRegion;
		bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = layers;
		bufferCopyRegion.imageExtent.width = extent.width;
		bufferCopyRegion.imageExtent.height = extent.height;
		bufferCopyRegion.imageExtent.depth = extent.depth;
		bufferCopyRegion.bufferOffset = 0;

		auto copyCmd = std::dynamic_pointer_cast<VK::CommandBuffer>(cmdBuf)->getCommandBuffer();
		copyCmd.begin(vk::CommandBufferBeginInfo());

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.levelCount = levels;
		subresourceRange.layerCount = layers;

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = {};
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
		copyCmd.copyBufferToImage(stagingBuffer->getBuffer(), image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion);

		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemoryBarrier);
		copyCmd.end();

		imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		cmdBuf->flush();
	}

	void Image::generateMipmaps(GPU::CommandBuffer::Ptr commandBuffer)
	{
		auto cmdBuf = std::dynamic_pointer_cast<VK::CommandBuffer>(commandBuffer)->getCommandBuffer();
		{
			vk::ImageSubresourceRange subresourceRange;
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = layers;

			vk::ImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
		}

		{
			vk::ImageSubresourceRange subresourceRange;
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			subresourceRange.baseMipLevel = 1;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.levelCount = levels - 1;
			subresourceRange.layerCount = layers;

			vk::ImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcAccessMask = {};
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
		}

		for (uint32 i = 1; i < levels; i++)
		{
			vk::ImageBlit imageBlit = {};

			imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageBlit.srcSubresource.layerCount = layers;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(extent.width >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(extent.height >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;

			imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageBlit.dstSubresource.layerCount = layers;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(extent.width >> i);
			imageBlit.dstOffsets[1].y = int32_t(extent.height >> i);
			imageBlit.dstOffsets[1].z = 1;

			vk::ImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = layers;

			{
				vk::ImageMemoryBarrier imageMemoryBarrier;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange = mipSubRange;
				imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
				imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
			}

			cmdBuf.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, imageBlit, vk::Filter::eLinear);

			{
				vk::ImageMemoryBarrier imageMemoryBarrier;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange = mipSubRange;
				imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
				imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);
			}
		}

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.levelCount = levels;
		subresourceRange.layerCount = layers;

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemoryBarrier);
	}

	void Image::setImageLayout()
	{
		imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	void Image::layoutTransition(vk::CommandBuffer cmdBuf, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask)
	{
		vk::ImageSubresourceRange subresourceRange;
		if (format == GPU::Format::DEPTH32)
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		else
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.levelCount = levels;
		subresourceRange.layerCount = layers;

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, nullptr, nullptr, imageMemoryBarrier);

		imageLayout = newLayout;
	}

	void Image::layoutTransition(vk::CommandBuffer cmdBuf, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32 mip)
	{
		vk::ImageSubresourceRange subresourceRange;
		if (format == GPU::Format::DEPTH32)
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		else
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = mip;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, nullptr, nullptr, imageMemoryBarrier);

		imageLayout = newLayout;
	}

	void Image::layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf)
	{
		auto vkCmdBuf = std::dynamic_pointer_cast<VK::CommandBuffer>(cmdBuf)->getCommandBuffer();
		layoutTransition(vkCmdBuf, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, {}, vk::AccessFlagBits::eShaderRead);
	}

	void Image::layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{
		auto vkCmdBuf = std::dynamic_pointer_cast<VK::CommandBuffer>(cmdBuf)->getCommandBuffer();
		layoutTransition(vkCmdBuf, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, {}, vk::AccessFlagBits::eShaderRead);
	}

	GPU::ImageView::Ptr Image::createImageView()
	{
		return ImageView::create(image, type, format, GPU::SubResourceRange(0, 0, levels, layers));
	}

	GPU::ImageView::Ptr Image::createImageView(GPU::ViewType type, GPU::SubResourceRange subRange)
	{
		return ImageView::create(image, type, format, subRange);
	}

	vk::Image Image::getImage()
	{
		return image;
	}

	vk::ImageLayout Image::getLayout()
	{
		return imageLayout;
	}
}
