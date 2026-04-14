#ifndef INCLUDED_VKCOMMANDBUFFER
#define INCLUDED_VKCOMMANDBUFFER

#pragma once

#include <GPU/CommandBuffer.h>
#include <GPU/VK/VKFramebuffer.h>
#include <GPU/VK/VKPipeline.h>
#include <GPU/VK/VKDescriptorSet.h>
#include <GPU/VK/VKBuffer.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	vk::IndexType getIndexType(GPU::IndexType type);

	class CommandBuffer : public GPU::CommandBuffer
	{
	public:
		CommandBuffer(vk::CommandBuffer commandBuffer);
		~CommandBuffer();

		void begin();
		void end();
		void beginRenderPass(GPU::Framebuffer::Ptr framebuffer);
		//void beginRenderPass(uint32 width, uint32 height, vk::RenderPass renderpass, vk::Framebuffer framebuffer);
		void endRenderPass();
		void setViewport(float x, float y, float width, float height);
		void setScissor(int32 x, int32 y, uint32 width, uint32 height);
		void bindPipeline(GPU::GraphicsPipeline::Ptr pipeline);
		void bindPipeline(GPU::ComputePipeline::Ptr pipeline);
		void pushConstants(GPU::GraphicsPipeline::Ptr pipeline, GPU::ShaderStage stage, uint32 offset, uint32 size, const void* values);
		void bindDescriptorSets(GPU::GraphicsPipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet);
		void bindDescriptorSets(GPU::ComputePipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet);
		void bindVertexBuffers(GPU::Buffer::Ptr vertexBuffer);
		void bindIndexBuffers(GPU::Buffer::Ptr indexBuffer, GPU::IndexType indexType);
		void setCullMode(int mode);
		void drawIndexed(uint32 indexCount, GPU::Topology topology);
		void drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset);
		void drawArrays(uint32 vertexCount);
		void dispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ);
		void pipelineBarrier();
		void flush();
		vk::CommandBuffer getCommandBuffer();
		vk::Semaphore getSemaphore();

		typedef std::shared_ptr<CommandBuffer> Ptr;
		static Ptr create(vk::CommandBuffer commandBuffer)
		{
			return std::make_shared<CommandBuffer>(commandBuffer);
		}

	private:
		vk::Device device;
		vk::Queue queue;
		vk::Fence fence;
		vk::Semaphore semaphore;
		vk::CommandBuffer commandBuffer;
		
		PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT = nullptr;
		PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT = nullptr;
		
		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}

#endif // INCLUDED_VKCOMMANDBUFFER