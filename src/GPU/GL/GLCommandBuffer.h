#ifndef INCLUDED_GLCOMMANDBUFFER
#define INCLUDED_GLCOMMANDBUFFER

#pragma once

#include <GPU/CommandBuffer.h>
#include <GPU/GL/GLBuffer.h>
#include <GPU/GL/GLPipeline.h>
#include <GPU/GL/GLDescriptorSet.h>
#include <GPU/GL/GLFramebuffer.h>

namespace GL
{
	class Command
	{
	public:
		Command() {}
		virtual ~Command() = 0 {}
		virtual void execute() = 0;
		typedef std::shared_ptr<Command> Ptr;
	private:
		Command(const Command&) = delete;
		Command& operator=(const Command&) = delete;
	};

	class CommandBuffer : public GPU::CommandBuffer
	{
	public:
		CommandBuffer();
		~CommandBuffer();

		void begin();
		void end();
		void beginRenderPass(GPU::Framebuffer::Ptr framebuffer);
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

		void generateMipmap(GLenum target, GLuint texture);

		typedef std::shared_ptr<CommandBuffer> Ptr;
		static Ptr create()
		{
			return std::make_shared<CommandBuffer>();
		}

	private:
		std::vector<Command::Ptr> commands;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}

#endif // INCLUDED_GLCOMMANDBUFFER