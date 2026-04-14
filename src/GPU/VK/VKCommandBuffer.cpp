#include "VKCommandBuffer.h"
#include "VKEnums.h"
#include "VKDevice.h"
#include <iostream>

namespace VK
{
	vk::IndexType getIndexType(GPU::IndexType type)
	{
		vk::IndexType vkIndexType = vk::IndexType::eUint32;
		switch (type)
		{
		case GPU::IndexType::uInt8:		vkIndexType = vk::IndexType::eUint8KHR;	break; // Does this even work?
		case GPU::IndexType::uint16:	vkIndexType = vk::IndexType::eUint16; break;
		case GPU::IndexType::uint32:	vkIndexType = vk::IndexType::eUint32; break;
		}
		return vkIndexType;
	}

	CommandBuffer::CommandBuffer(vk::CommandBuffer commandBuffer) :
		commandBuffer(commandBuffer)
	{
		auto& deviceInstance = Device::getInstance();
		device = deviceInstance.getDevice();
		queue = deviceInstance.getSuitableGraphicsQueue();

		fence = device.createFence({});
		semaphore = device.createSemaphore({});

		vkCmdSetCullModeEXT = reinterpret_cast<PFN_vkCmdSetCullModeEXT>(vkGetDeviceProcAddr(device, "vkCmdSetCullModeEXT"));
		vkCmdSetPrimitiveTopologyEXT = reinterpret_cast<PFN_vkCmdSetPrimitiveTopologyEXT>(vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopologyEXT"));
	}

	CommandBuffer::~CommandBuffer()
	{
		device.destroyFence(fence);
		device.destroySemaphore(semaphore);
	}

	void CommandBuffer::begin()
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo;
		commandBuffer.begin(commandBufferBeginInfo);
	}

	void CommandBuffer::end()
	{
		commandBuffer.end();
	}

	void CommandBuffer::beginRenderPass(GPU::Framebuffer::Ptr framebuffer)
	{
		auto vkFramebuffer = std::dynamic_pointer_cast<VK::Framebuffer>(framebuffer);
		auto renderPassBeginInfo = vkFramebuffer->getRenderPassInfo();
		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	}

	//void CommandBuffer::beginRenderPass(uint32 width, uint32 height, vk::RenderPass renderpass, vk::Framebuffer framebuffer)
	//{
	//	vk::Extent2D extent(width, height);

	//	vk::ClearValue clearValues[2];
	//	clearValues[0].color = vk::ClearColorValue(0.01f, 0.033f, 0.033f, 1.0f);
	//	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	//	vk::RenderPassBeginInfo renderPassBeginInfo;
	//	renderPassBeginInfo.renderPass = renderpass;
	//	renderPassBeginInfo.framebuffer = framebuffer;
	//	renderPassBeginInfo.renderArea.offset.x = 0;
	//	renderPassBeginInfo.renderArea.offset.y = 0;
	//	renderPassBeginInfo.renderArea.extent = extent;
	//	renderPassBeginInfo.clearValueCount = 2;
	//	renderPassBeginInfo.pClearValues = clearValues;

	//	commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	//}

	void CommandBuffer::endRenderPass()
	{
		commandBuffer.endRenderPass();
	}

	void CommandBuffer::setViewport(float x, float y, float width, float height)
	{
		vk::Viewport viewport(x, y, width, height, 0.0f, 1.0f);
		commandBuffer.setViewport(0, viewport);
	}

	void CommandBuffer::setScissor(int32 x, int32 y, uint32 width, uint32 height)
	{
		vk::Offset2D offset(x, y);
		vk::Extent2D extent(width, height);
		vk::Rect2D scissor(offset, extent);
		commandBuffer.setScissor(0, scissor);
	}

	void CommandBuffer::bindPipeline(GPU::GraphicsPipeline::Ptr pipeline)
	{
		auto vkPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vkPipeline->getPipeline());
	}

	void CommandBuffer::bindPipeline(GPU::ComputePipeline::Ptr pipeline)
	{
		auto vkPipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, vkPipeline->getPipeline());
	}

	void CommandBuffer::pushConstants(GPU::GraphicsPipeline::Ptr pipeline, GPU::ShaderStage stage, uint32 offset, uint32 size, const void* values)
	{
		auto vkPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		vk::ShaderStageFlags stageFlgas = static_cast<vk::ShaderStageFlags>((int)stage);
		commandBuffer.pushConstants(vkPipeline->getPipelineLayout(), stageFlgas, offset, size, values);
	}

	void CommandBuffer::bindDescriptorSets(GPU::GraphicsPipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto vkPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		auto vkDescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkPipeline->getPipelineLayout(), firstSet, vkDescriptorSet->getDescriptorSet(), {});
	}

	void CommandBuffer::bindDescriptorSets(GPU::ComputePipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto vkPipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		auto vkDescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, vkPipeline->getPipelineLayout(), firstSet, vkDescriptorSet->getDescriptorSet(), {});

	}

	void CommandBuffer::bindVertexBuffers(GPU::Buffer::Ptr vertexBuffer)
	{
		auto vkBuffer = std::dynamic_pointer_cast<Buffer>(vertexBuffer);
		vk::DeviceSize offset = 0;
		commandBuffer.bindVertexBuffers(0, vkBuffer->getBuffer(), offset);
	}

	void CommandBuffer::bindIndexBuffers(GPU::Buffer::Ptr indexBuffer, GPU::IndexType indexType)
	{
		auto vkBuffer = std::dynamic_pointer_cast<Buffer>(indexBuffer);
		commandBuffer.bindIndexBuffer(vkBuffer->getBuffer(), 0, getIndexType(indexType));
	}

	void CommandBuffer::setCullMode(int mode)
	{
		VkCullModeFlags flags = (VkCullModeFlags)vk::CullModeFlags(mode);
		vkCmdSetCullModeEXT(commandBuffer, flags);
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, GPU::Topology topology)
	{
		VkPrimitiveTopology vkTopology = (VkPrimitiveTopology)getTopology(topology);
		vkCmdSetPrimitiveTopologyEXT(commandBuffer, vkTopology);
		commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
	{
		VkPrimitiveTopology vkTopology = (VkPrimitiveTopology)getTopology(GPU::Topology::Triangles);
		vkCmdSetPrimitiveTopologyEXT(commandBuffer, vkTopology);
		commandBuffer.drawIndexed(indexCount, 1, indexOffset, vertexOffset, 0);
	}

	void CommandBuffer::drawArrays(uint32 vertexCount)
	{
		commandBuffer.draw(vertexCount, 1, 0, 0);
	}

	void CommandBuffer::dispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ)
	{
		commandBuffer.dispatch(grpCountX, grpCountY, grpCountZ);
	}

	void CommandBuffer::pipelineBarrier()
	{

	}

	void CommandBuffer::flush()
	{
		vk::SubmitInfo submitInfo({}, {}, commandBuffer);
		queue.submit(submitInfo, fence);

		vk::Result vkResult = device.waitForFences(fence, true, 100000000000);
		if (vkResult != vk::Result::eSuccess)
		{
			std::cout << "error executing command buffer!" << std::endl;
		}
		
		device.resetFences(fence);
		//device.waitIdle();
	}

	vk::CommandBuffer CommandBuffer::getCommandBuffer()
	{
		return commandBuffer;
	}

	vk::Semaphore CommandBuffer::getSemaphore()
	{
		return semaphore;
	}
}