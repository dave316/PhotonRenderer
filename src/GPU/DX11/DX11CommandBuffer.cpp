#include "DX11CommandBuffer.h"
#include "DX11Enums.h"
#include "DX11Device.h"
#include <iostream>
unsigned int DX11::CommandBuffer::globalIDCount = 0;
namespace DX11
{
	DXGI_FORMAT getIndexType(GPU::IndexType type)
	{
		DXGI_FORMAT dxFormat = DXGI_FORMAT_R32_UINT;
		switch (type)
		{
			case GPU::IndexType::uInt8:		dxFormat = DXGI_FORMAT_R8_UINT;	break; // Does this even work?
			case GPU::IndexType::uint16:	dxFormat = DXGI_FORMAT_R16_UINT; break;
			case GPU::IndexType::uint32:	dxFormat = DXGI_FORMAT_R32_UINT; break;
		}
		return dxFormat;
	}

	class CmdBeginRenderPass : public Command
	{
	public:
		CmdBeginRenderPass(DX11::Framebuffer::Ptr framebuffer) :
			framebuffer(framebuffer)
		{

		}
		~CmdBeginRenderPass() {}

		void execute()
		{
			auto dxFramebuffer = std::dynamic_pointer_cast<Framebuffer>(framebuffer);
			dxFramebuffer->bindRenderTarget();
			if (dxFramebuffer->clearOnLoad())
				dxFramebuffer->clearRenderTarget();
		}

		typedef std::shared_ptr<CmdBeginRenderPass> Ptr;
		static Ptr create(DX11::Framebuffer::Ptr framebuffer)
		{
			return std::make_shared<CmdBeginRenderPass>(framebuffer);
		}

	private:
		DX11::Framebuffer::Ptr framebuffer;

		CmdBeginRenderPass(const CmdBeginRenderPass&) = delete;
		CmdBeginRenderPass& operator=(const CmdBeginRenderPass&) = delete;
	};

	class CmdEndRenderPass : public Command
	{
	public:
		CmdEndRenderPass()
		{

		}

		void execute()
		{
			//deviceContext->OMSetRenderTargets(0, NULL, NULL);
			ID3D11RenderTargetView* rtv[1] = { NULL };
			deviceContext->OMSetRenderTargets(0, rtv, NULL);

			// TODO: unbind all shader resources wenn render pass ends!!!
			ID3D11ShaderResourceView* views[1] = { NULL };
			for (int i = 0; i < 26; i++)
			{
				deviceContext->PSSetShaderResources(i, 1, views);
			}
		}

		typedef std::shared_ptr<CmdEndRenderPass> Ptr;
		static Ptr create()
		{
			return std::make_shared<CmdEndRenderPass>();
		}

	private:
		CmdEndRenderPass(const CmdEndRenderPass&) = delete;
		CmdEndRenderPass& operator=(const CmdEndRenderPass&) = delete;
	};

	class CmdSetViewport : public Command
	{
	public:
		CmdSetViewport(float x, float y, float width, float height) :
			x(x), y(y), width(width), height(height)
		{
		}

		void execute()
		{
			D3D11_VIEWPORT viewPort;
			viewPort.Width = (float)width;
			viewPort.Height = (float)height;
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
			viewPort.TopLeftX = x;
			viewPort.TopLeftY = y;
			deviceContext->RSSetViewports(1, &viewPort);
		}

		typedef std::shared_ptr<CmdSetViewport> Ptr;
		static Ptr create(float x, float y, float width, float height)
		{
			return std::make_shared<CmdSetViewport>(x, y, width, height);
		}

	private:
		float x;
		float y;
		float width;
		float height;

		CmdSetViewport(const CmdSetViewport&) = delete;
		CmdSetViewport& operator=(const CmdSetViewport&) = delete;
	};

	class CmdSetScissor : public Command
	{
	public:
		CmdSetScissor(int32 x, int32 y, uint32 width, uint32 height) :
			x(x), y(y), width(width), height(height)
		{
		}

		void execute()
		{
			D3D11_RECT rect;
			rect.left = x;
			rect.top = y;
			rect.right = x + width;
			rect.bottom = y + height;
			deviceContext->RSSetScissorRects(1, &rect);
		}

		typedef std::shared_ptr<CmdSetScissor> Ptr;
		static Ptr create(int32 x, int32 y, uint32 width, uint32 height)
		{
			return std::make_shared<CmdSetScissor>(x, y, width, height);
		}

	private:
		LONG x;
		LONG y;
		LONG width;
		LONG height;

		CmdSetScissor(const CmdSetScissor&) = delete;
		CmdSetScissor& operator=(const CmdSetScissor&) = delete;
	};

	class CmdBindGraphicsPipeline : public Command
	{
	public:
		CmdBindGraphicsPipeline(DX11::GraphicsPipeline::Ptr pipeline) :
			pipeline(pipeline)
		{
		}

		void execute()
		{
			pipeline->use();
		}

		typedef std::shared_ptr<CmdBindGraphicsPipeline> Ptr;
		static Ptr create(DX11::GraphicsPipeline::Ptr pipeline)
		{
			return std::make_shared<CmdBindGraphicsPipeline>(pipeline);
		}

	private:
		DX11::GraphicsPipeline::Ptr pipeline;

		CmdBindGraphicsPipeline(const CmdBindGraphicsPipeline&) = delete;
		CmdBindGraphicsPipeline& operator=(const CmdBindGraphicsPipeline&) = delete;
	};

	class CmdBindComputePipeline : public Command
	{
	public:
		CmdBindComputePipeline(DX11::ComputePipeline::Ptr pipeline) :
			pipeline(pipeline)
		{
		}

		void execute()
		{
			pipeline->use();
		}

		typedef std::shared_ptr<CmdBindComputePipeline> Ptr;
		static Ptr create(DX11::ComputePipeline::Ptr pipeline)
		{
			return std::make_shared<CmdBindComputePipeline>(pipeline);
		}

	private:
		DX11::ComputePipeline::Ptr pipeline;

		CmdBindComputePipeline(const CmdBindComputePipeline&) = delete;
		CmdBindComputePipeline& operator=(const CmdBindComputePipeline&) = delete;
	};

	class CmdPushConstants : public Command
	{
	public:
		CmdPushConstants(DX11::GraphicsPipeline::Ptr pipeline, uint8* data, uint32 size) :
			pipeline(pipeline),
			size(size)
		{
			this->data = new uint8[size];
			std::memcpy(this->data, data, size);
		}

		void execute()
		{
			pipeline->pushConstants(data);
		}

		typedef std::shared_ptr<CmdPushConstants> Ptr;
		static Ptr create(DX11::GraphicsPipeline::Ptr pipeline, uint8* data, uint32 size)
		{
			return std::make_shared<CmdPushConstants>(pipeline, data, size);
		}

	private:
		uint8* data;
		uint32 size;
		DX11::GraphicsPipeline::Ptr pipeline;

		CmdPushConstants(const CmdPushConstants&) = delete;
		CmdPushConstants& operator=(const CmdPushConstants&) = delete;
	};


	class CmdBindDescriptorSets : public Command
	{
	public:
		CmdBindDescriptorSets(DX11::DescriptorSet::Ptr descriptorSet, std::vector<int> bindings) :
			descriptorSet(descriptorSet),
			bindings(bindings)
		{
		}

		void execute()
		{
			//auto bindings = pipeline->getLayoutBindings(descriptorSet->getLayoutName());
			descriptorSet->bind(deviceContext, bindings);
		}

		typedef std::shared_ptr<CmdBindDescriptorSets> Ptr;
		static Ptr create(DX11::DescriptorSet::Ptr descriptorSet, std::vector<int> bindings)
		{
			return std::make_shared<CmdBindDescriptorSets>(descriptorSet, bindings);
		}

	private:
		DX11::DescriptorSet::Ptr descriptorSet;
		std::vector<int> bindings;

		CmdBindDescriptorSets(const CmdBindDescriptorSets&) = delete;
		CmdBindDescriptorSets& operator=(const CmdBindDescriptorSets&) = delete;
	};

	class CmdBindVertexBuffers : public Command
	{
	public:
		CmdBindVertexBuffers(DX11::Buffer::Ptr buffer) :
			buffer(buffer)
		{
		}

		void execute()
		{
			auto dx11Buffer = buffer->getBuffer();
			uint32 offset = 0;
			uint32 stride = buffer->getStride();
			deviceContext->IASetVertexBuffers(0, 1, dx11Buffer.GetAddressOf(), &stride, &offset);
		}

		typedef std::shared_ptr<CmdBindVertexBuffers> Ptr;
		static Ptr create(DX11::Buffer::Ptr buffer)
		{
			return std::make_shared<CmdBindVertexBuffers>(buffer);
		}

	private:
		DX11::Buffer::Ptr buffer;

		CmdBindVertexBuffers(const CmdBindVertexBuffers&) = delete;
		CmdBindVertexBuffers& operator=(const CmdBindVertexBuffers&) = delete;
	};


	class CmdBindIndexBuffers : public Command
	{
	public:
		CmdBindIndexBuffers(DX11::Buffer::Ptr indexBuffer, GPU::IndexType indexType) :
			indexBuffer(indexBuffer),
			indexType(getIndexType(indexType))
		{
		}

		void execute()
		{
			auto dx11Buffer = indexBuffer->getBuffer();
			deviceContext->IASetIndexBuffer(dx11Buffer.Get(), indexType, 0);
		}

		typedef std::shared_ptr<CmdBindIndexBuffers> Ptr;
		static Ptr create(DX11::Buffer::Ptr indexBuffer, GPU::IndexType indexType)
		{
			return std::make_shared<CmdBindIndexBuffers>(indexBuffer, indexType);
		}

	private:
		DX11::Buffer::Ptr indexBuffer;
		DXGI_FORMAT indexType;

		CmdBindIndexBuffers(const CmdBindIndexBuffers&) = delete;
		CmdBindIndexBuffers& operator=(const CmdBindIndexBuffers&) = delete;
	};

	class CmdSetCullMode : public Command
	{
	public:
		CmdSetCullMode(ComPtr<ID3D11RasterizerState> rasterState) :
			rasterState(rasterState)
		{
		}

		void execute()
		{
			deviceContext->RSSetState(rasterState.Get());
		}

		typedef std::shared_ptr<CmdSetCullMode> Ptr;
		static Ptr create(ComPtr<ID3D11RasterizerState> rasterState)
		{
			return std::make_shared<CmdSetCullMode>(rasterState);
		}

	private:
		ComPtr<ID3D11RasterizerState> rasterState;

		CmdSetCullMode(const CmdSetCullMode&) = delete;
		CmdSetCullMode& operator=(const CmdSetCullMode&) = delete;
	};

	class CmdDrawIndexed : public Command
	{
	public:
		CmdDrawIndexed(uint32 indexCount, GPU::Topology topology) :
			indexCount(indexCount),
			topology(topology)
		{
		}

		void execute()
		{
			deviceContext->IASetPrimitiveTopology(getTopology(topology)); // TODO: put in pipeline
			deviceContext->DrawIndexed(indexCount, 0, 0);
		}

		typedef std::shared_ptr<CmdDrawIndexed> Ptr;
		static Ptr create(uint32 indexCount, GPU::Topology topology)
		{
			return std::make_shared<CmdDrawIndexed>(indexCount, topology);
		}

	private:
		uint32 indexCount;
		GPU::Topology topology;

		CmdDrawIndexed(const CmdDrawIndexed&) = delete;
		CmdDrawIndexed& operator=(const CmdDrawIndexed&) = delete;
	};

	class CmdDrawIndexedBaseVertex : public Command
	{
	public:
		CmdDrawIndexedBaseVertex(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) :
			indexCount(indexCount),
			indexOffset(indexOffset),
			vertexOffset(vertexOffset)
		{
		}

		void execute()
		{
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO: put in pipeline
			deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
		}

		typedef std::shared_ptr<CmdDrawIndexedBaseVertex> Ptr;
		static Ptr create(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
		{
			return std::make_shared<CmdDrawIndexedBaseVertex>(indexCount, indexOffset, vertexOffset);
		}

	private:
		uint32 indexCount;
		uint32 indexOffset;
		uint32 vertexOffset;

		CmdDrawIndexedBaseVertex(const CmdDrawIndexedBaseVertex&) = delete;
		CmdDrawIndexedBaseVertex& operator=(const CmdDrawIndexedBaseVertex&) = delete;
	};

	class CmdDrawArrays : public Command
	{
	public:
		CmdDrawArrays(uint32 vertexCount) :
			vertexCount(vertexCount)
		{
		}

		void execute()
		{
			//glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		}

		typedef std::shared_ptr<CmdDrawArrays> Ptr;
		static Ptr create(uint32 vertexCount)
		{
			return std::make_shared<CmdDrawArrays>(vertexCount);
		}

	private:
		uint32 vertexCount;

		CmdDrawArrays(const CmdDrawArrays&) = delete;
		CmdDrawArrays& operator=(const CmdDrawArrays&) = delete;
	};

	class CmdDispatchCompute : public Command
	{
	public:
		CmdDispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ) :
			grpCountX(grpCountX),
			grpCountY(grpCountY),
			grpCountZ(grpCountZ)
		{
		}

		void execute()
		{
			deviceContext->Dispatch(grpCountX, grpCountY, grpCountZ);
			deviceContext->CSSetShader(NULL, NULL, 0);

			ID3D11ShaderResourceView* srvs[1] = { NULL };
			for (int i = 0; i < 10; i++)
				deviceContext->CSSetShaderResources(i, 1, srvs);
			ID3D11UnorderedAccessView* uavs[1] = { NULL };
			for (int i = 0; i < 8; i++)
				deviceContext->CSSetUnorderedAccessViews(i, 1, uavs, NULL);
		}

		typedef std::shared_ptr<CmdDispatchCompute> Ptr;
		static Ptr create(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ)
		{
			return std::make_shared<CmdDispatchCompute>(grpCountX, grpCountY, grpCountZ);
		}

	private:
		uint32 grpCountX;
		uint32 grpCountY;
		uint32 grpCountZ;

		CmdDispatchCompute(const CmdDispatchCompute&) = delete;
		CmdDispatchCompute& operator=(const CmdDispatchCompute&) = delete;
	};

	CommandBuffer::CommandBuffer() :
		deviceContext(Device::getInstance().getDeviceContext())
	{
		auto device = Device::getInstance().getDevice();

		id = globalIDCount;
		globalIDCount++;

		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;

		ComPtr<ID3D11RasterizerState> rasterizerStateDefault;
		HRESULT result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStateDefault.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}
		rasterStates.push_back(rasterizerStateDefault);

		ComPtr<ID3D11RasterizerState> rasterizerStateFront;
		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStateFront.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}
		rasterStates.push_back(rasterizerStateFront);

		ComPtr<ID3D11RasterizerState> rasterizerStateBack;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		result = device->CreateRasterizerState(&rasterizerDesc, rasterizerStateBack.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}
		rasterStates.push_back(rasterizerStateBack);

		//std::cout << "created command buffer " << std::to_string(id) << std::endl;
	}
	 
	CommandBuffer::~CommandBuffer()
	{
		//std::cout << "destroyed command buffer " << std::to_string(id) << std::endl;
	}

	void CommandBuffer::begin()
	{
		commands.clear();
	}

	void CommandBuffer::end()
	{
		// TODO: prevent new commands being added!
	}

	void CommandBuffer::beginRenderPass(GPU::Framebuffer::Ptr framebuffer)
	{
		//auto dxFramebuffer = std::dynamic_pointer_cast<Framebuffer>(framebuffer);
		//dxFramebuffer->bindRenderTarget();
		//if (dxFramebuffer->clearOnLoad())
		//	dxFramebuffer->clearRenderTarget();

		auto glFramebuffer = std::dynamic_pointer_cast<Framebuffer>(framebuffer);
		commands.push_back(CmdBeginRenderPass::create(glFramebuffer));
	}

	void CommandBuffer::endRenderPass()
	{
		////deviceContext->OMSetRenderTargets(0, NULL, NULL);
		//ID3D11RenderTargetView* rtv[1] = { NULL };
		//deviceContext->OMSetRenderTargets(0, rtv, NULL);

		//// TODO: unbind all shader resources wenn render pass ends!!!
		//ID3D11ShaderResourceView* views[1] = { NULL };
		//for (int i = 0; i < 26; i++)
		//{
		//	deviceContext->PSSetShaderResources(i, 1, views);
		//}			

		commands.push_back(CmdEndRenderPass::create());
	}

	void CommandBuffer::setViewport(float x, float y, float width, float height)
	{
		//D3D11_VIEWPORT viewPort;
		//viewPort.Width = (float)width;
		//viewPort.Height = (float)height;
		//viewPort.MinDepth = 0.0f;
		//viewPort.MaxDepth = 1.0f;
		//viewPort.TopLeftX = 0.0f;
		//viewPort.TopLeftY = 0.0f;
		//deviceContext->RSSetViewports(1, &viewPort);

		commands.push_back(CmdSetViewport::create(x, y, width, height));
	}

	void CommandBuffer::setScissor(int32 x, int32 y, uint32 width, uint32 height)
	{
		//D3D11_RECT rect;
		//rect.left = x;
		//rect.top = y;
		//rect.right = x + width;
		//rect.bottom = y + height;
		//deviceContext->RSSetScissorRects(1, &rect);

		commands.push_back(CmdSetScissor::create(x, y, width, height));
	}

	void CommandBuffer::bindPipeline(GPU::GraphicsPipeline::Ptr pipeline)
	{
		auto dx11Pipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		//dx11Pipeline->use();

		commands.push_back(CmdBindGraphicsPipeline::create(dx11Pipeline));
	}

	void CommandBuffer::bindPipeline(GPU::ComputePipeline::Ptr pipeline)
	{
		auto dx11Pipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		//dx11Pipeline->use();

		commands.push_back(CmdBindComputePipeline::create(dx11Pipeline));
	}

	void CommandBuffer::pushConstants(GPU::GraphicsPipeline::Ptr pipeline, GPU::ShaderStage stage, uint32 offset, uint32 size, const void* values)
	{
		auto dx11Pipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		//dx11Pipeline->pushConstants((uint8*)values);

		commands.push_back(CmdPushConstants::create(dx11Pipeline, (uint8*)values, size));
	}

	void CommandBuffer::bindDescriptorSets(GPU::GraphicsPipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto dx11Pipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		auto dx11DescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		auto bindings = dx11Pipeline->getLayoutBindings(dx11DescriptorSet->getLayoutName());
		//dx11DescriptorSet->bind(deviceContext, bindings);
		
		commands.push_back(CmdBindDescriptorSets::create(dx11DescriptorSet, bindings));
	}

	void CommandBuffer::bindDescriptorSets(GPU::ComputePipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto dx11Pipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		auto dx11DescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		auto bindings = dx11Pipeline->getLayoutBindings(dx11DescriptorSet->getLayoutName());
		//dx11DescriptorSet->bind(deviceContext, bindings);

		commands.push_back(CmdBindDescriptorSets::create(dx11DescriptorSet, bindings));
	}

	void CommandBuffer::bindVertexBuffers(GPU::Buffer::Ptr vertexBuffer)
	{
		auto dx11VBO = std::dynamic_pointer_cast<Buffer>(vertexBuffer);
		//auto dx11Buffer = dx11VBO->getBuffer();
		//uint32 offset = 0;
		//uint32 stride = dx11VBO->getStride();
		//deviceContext->IASetVertexBuffers(0, 1, dx11Buffer.GetAddressOf(), &stride, &offset);

		commands.push_back(CmdBindVertexBuffers::create(dx11VBO));
	}

	void CommandBuffer::bindIndexBuffers(GPU::Buffer::Ptr indexBuffer, GPU::IndexType indexType)
	{
		auto dx11IBO = std::dynamic_pointer_cast<Buffer>(indexBuffer);
		//auto dx11Buffer = dx11IBO->getBuffer();
		//deviceContext->IASetIndexBuffer(dx11Buffer.Get(), getIndexType(indexType), 0);

		commands.push_back(CmdBindIndexBuffers::create(dx11IBO, indexType));
	}

	void CommandBuffer::setCullMode(int mode)
	{
		// TODO: direct3D only has a global rasterizer state that can be set
		//deviceContext->RSSetState(rasterStates[mode].Get());

		commands.push_back(CmdSetCullMode::create(rasterStates[mode]));
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, GPU::Topology topology)
	{
		//deviceContext->IASetPrimitiveTopology(getTopology(topology)); // TODO: put in pipeline
		//deviceContext->DrawIndexed(indexCount, 0, 0);

		commands.push_back(CmdDrawIndexed::create(indexCount, topology));
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
	{
		//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO: put in pipeline
		//deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);

		commands.push_back(CmdDrawIndexedBaseVertex::create(indexCount, indexOffset, vertexOffset));
	}

	void CommandBuffer::drawArrays(uint32 vertexCount)
	{
		// TODO: draw array buffers
	}

	void CommandBuffer::dispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ)
	{
		//deviceContext->Dispatch(grpCountX, grpCountY, grpCountZ);
		//deviceContext->CSSetShader(NULL, NULL, 0);

		//ID3D11ShaderResourceView* srvs[1] = { NULL };
		//for (int i = 0; i < 10; i++)
		//	deviceContext->CSSetShaderResources(i, 1, srvs);
		//ID3D11UnorderedAccessView* uavs[1] = { NULL };
		//for (int i = 0; i < 8; i++)
		//	deviceContext->CSSetUnorderedAccessViews(i, 1, uavs, NULL);

		commands.push_back(CmdDispatchCompute::create(grpCountX, grpCountY, grpCountZ));
	}

	void CommandBuffer::pipelineBarrier()
	{
	}

	void CommandBuffer::flush()
	{
		for (auto cmd : commands)
			cmd->execute();
	}
}