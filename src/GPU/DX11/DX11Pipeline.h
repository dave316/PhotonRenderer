#ifndef INCLUDED_DX11PIPELINE
#define INCLUDED_DX11PIPELINE

#pragma once

#include <GPU/Pipeline.h>
#include <GPU/DX11/DX11Buffer.h>
#include <GPU/DX11/DX11DescriptorPool.h>
#include <glm/glm.hpp>

namespace DX11
{
	class GraphicsPipeline : public GPU::GraphicsPipeline
	{
	public:	
		GraphicsPipeline(std::string name);
		~GraphicsPipeline();
		void setVertexInputDescripton(GPU::VertexDescription& inputDescription);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants);
		void addShaderStage(std::string code, GPU::ShaderStage stage);
		void pushConstants(uint8* data);
		void setDepthTest(bool depthTestEnable, bool depthWriteEnable);
		void setBlending(bool blendEnable);
		void setStencilTest(bool stencilTestEnable, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp);
		void setColorMask(bool red, bool green, bool blue, bool alpha);
		void setScissorTest(bool scissorTestEnable);
		void setCullMode(int mode);
		void setWindingOrder(int frontFace);
		void createProgram();
		void use();
		std::vector<int> getLayoutBindings(std::string name);

		typedef std::shared_ptr<GraphicsPipeline> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<GraphicsPipeline>(name);
		}
	private:
		std::string vsBinary;
		std::string gsBinary;
		std::string psBinary;
		DX11::Buffer::Ptr pushConstantsUBO;
		std::map<std::string, std::vector<int>> layout;
		
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;
		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11VertexShader> vertexShader;
		ComPtr<ID3D11GeometryShader> geometryShader;
		ComPtr<ID3D11PixelShader> pixelShader;

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D11_RASTERIZER_DESC rasterizerDesc;
		D3D11_BLEND_DESC blendStateDesc;

		ComPtr<ID3D11DepthStencilState> depthStencilState;
		ComPtr<ID3D11RasterizerState> rasterizerState;
		ComPtr<ID3D11BlendState> alphaEnableBlendingState;
		uint32 stencilRefValue = 0;

		unsigned int id;
		static unsigned int globalIDCount;

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	};

	class ComputePipeline : public GPU::ComputePipeline
	{
	public:
		ComputePipeline(std::string name);
		~ComputePipeline();
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts);
		void addShaderStage(std::string code, GPU::ShaderStage stage);
		void createProgram();
		void use();
		std::vector<int> getLayoutBindings(std::string name);
		typedef std::shared_ptr<ComputePipeline> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<ComputePipeline>(name);
		}

	private:
		std::string csBinary;
		std::map<std::string, std::vector<int>> layout;

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;
		ComPtr<ID3D11ComputeShader> computeShader;

		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;
	};
}

#endif // INCLUDED_DX11PIPELINE