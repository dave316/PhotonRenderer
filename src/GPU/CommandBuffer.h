#ifndef INCLUDED_COMMANDBUFFER
#define INCLUDED_COMMANDBUFFER

#pragma once

#include <memory>
#include <Platform/Types.h>

#include "Pipeline.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"
#include "Buffer.h"

namespace GPU
{
	enum class IndexType
	{
		uInt8,
		uint16,
		uint32
	};

	class CommandBuffer
	{
	public:
		CommandBuffer() {}
		virtual ~CommandBuffer() = 0 {}
		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void beginRenderPass(Framebuffer::Ptr framebuffer) = 0;
		virtual void endRenderPass() = 0;
		virtual void setViewport(float x, float y, float width, float height) = 0;
		virtual void setScissor(int32 x, int32 y, uint32 width, uint32 height) = 0;
		virtual void bindPipeline(GraphicsPipeline::Ptr pipeline) = 0;
		virtual void bindPipeline(ComputePipeline::Ptr pipeline) = 0;
		virtual void pushConstants(GraphicsPipeline::Ptr pipeline, ShaderStage stage, uint32 offset, uint32 size, const void* values) = 0;
		virtual void bindDescriptorSets(GraphicsPipeline::Ptr pipeline, DescriptorSet::Ptr descriptorSet, uint32 firstSet) = 0;
		virtual void bindDescriptorSets(ComputePipeline::Ptr pipeline, DescriptorSet::Ptr descriptorSet, uint32 firstSet) = 0;
		virtual void bindVertexBuffers(Buffer::Ptr vertexBuffer) = 0;
		virtual void bindIndexBuffers(Buffer::Ptr indexBuffer, IndexType indexType) = 0;
		virtual void setCullMode(int mode) = 0;
		virtual void drawIndexed(uint32 indexCount, GPU::Topology topology) = 0;
		virtual void drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) = 0;
		virtual void drawArrays(uint32 vertexCount) = 0;
		virtual void dispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ) = 0;
		virtual void pipelineBarrier() = 0;
		virtual void flush() = 0;
		typedef std::shared_ptr<CommandBuffer> Ptr;

	private:
		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}

#endif // INCLUDED_COMMANDBUFFER