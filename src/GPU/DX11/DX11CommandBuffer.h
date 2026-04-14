#ifndef INCLUDED_DX11COMMANDBUFFER
#define INCLUDED_DX11COMMANDBUFFER

#pragma once

#include <GPU/CommandBuffer.h>
#include <GPU/DX11/DX11Buffer.h>
#include <GPU/DX11/DX11Device.h>
#include <GPU/DX11/DX11Pipeline.h>
#include <GPU/DX11/DX11DescriptorSet.h>
#include <GPU/DX11/DX11Framebuffer.h>

namespace DX11
{
	class Command
	{
	public:
		Command()
		{
			deviceContext = Device::getInstance().getDeviceContext();
		}
		virtual ~Command() = 0 {}
		virtual void execute() = 0;
		typedef std::shared_ptr<Command> Ptr;

	protected:
		ComPtr<ID3D11DeviceContext> deviceContext;
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

		typedef std::shared_ptr<CommandBuffer> Ptr;
		static Ptr create()
		{
			return std::make_shared<CommandBuffer>();
		}

	private:
		ComPtr<ID3D11DeviceContext> deviceContext;
		std::vector<ComPtr<ID3D11RasterizerState>> rasterStates;
		std::vector<Command::Ptr> commands;

		unsigned int id;
		static unsigned int globalIDCount;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}

#endif // INCLUDED_DX11COMMANDBUFFER