#ifndef INCLUDED_PIPELINE
#define INCLUDED_PIPELINE

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <Platform/Types.h>
#include <GPU/DescriptorPool.h>
#include <GPU/Sampler.h>
#include <GPU/Enums.h>

namespace GPU
{
	struct VertexInputAttribute
	{
		std::string name;
		uint32 index = 0;
		uint32 location;
		uint32 binding;
		uint32 offset;
		VertexAttribFormat format;

		VertexInputAttribute(uint32 location, uint32 binding, VertexAttribFormat format, uint32 offset) :
			location(location),
			binding(binding),
			format(format),
			offset(offset)
		{

		}
	};

	struct VertexDescription
	{
		uint32 binding = 0;
		uint32 stride = 0;
		VertexInputeRate inputRate = VertexInputeRate::Vertex;
		std::vector<VertexInputAttribute> inputAttributes;
	};

	struct PushConstant
	{
		ShaderStage stage;
		uint32 offset = 0;
		uint32 size = 0;
		PushConstant(ShaderStage stage, uint32 offset, uint32 size) :
			stage(stage),
			offset(offset),
			size(size)
		{

		}
	};

	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(std::string name) : name(name) {}
		virtual ~GraphicsPipeline() = 0 {}
		virtual void setVertexInputDescripton(VertexDescription& inputDescription) = 0;
		virtual void setLayout(DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts) = 0;
		virtual void setLayout(DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<PushConstant> pushConstants) = 0;
		virtual void addShaderStage(std::string code, GPU::ShaderStage stage) = 0;
		virtual void setDepthTest(bool depthTestEnable, bool depthWriteEnable) = 0;
		virtual void setBlending(bool blendEnable) = 0;
		virtual void setStencilTest(bool stencilTestEnable, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp) = 0;
		virtual void setColorMask(bool red, bool green, bool blue, bool alpha) = 0;
		virtual void setScissorTest(bool scissorTestEnable) = 0;
		virtual void setCullMode(int cullMode) = 0;
		virtual void setWindingOrder(int frontFace) = 0;
		virtual void createProgram() = 0;
		std::string getPipelineName()
		{
			return name;
		}
		typedef std::shared_ptr<GraphicsPipeline> Ptr;

	private:
		std::string name;
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	};

	class ComputePipeline
	{
	public:
		ComputePipeline(std::string name) : name(name) {}
		virtual ~ComputePipeline() = 0 {}
		virtual void setLayout(DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts) = 0;
		virtual void addShaderStage(std::string code, GPU::ShaderStage stage) = 0;
		virtual void createProgram() = 0;
		std::string getPipelineName()
		{
			return name;
		}
		typedef std::shared_ptr<ComputePipeline> Ptr;

	private:
		std::string name;
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;
	};
}

#endif // INCLUDED_PIPELINE