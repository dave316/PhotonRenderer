#ifndef INCLUDED_VKPIPELINE
#define INCLUDED_VKPIPELINE

#pragma once

#include <GPU/Pipeline.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	class GraphicsPipeline : public GPU::GraphicsPipeline
	{
	public:	
		GraphicsPipeline(vk::RenderPass renderPass, std::string name, int numAttachments);
		~GraphicsPipeline();

		void setVertexInputDescripton(GPU::VertexDescription& inputDescription);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants);
		void addShaderStage(std::string code, GPU::ShaderStage stage);
		void setDepthTest(bool depthTestEnable, bool depthWriteEnable);
		void setBlending(bool blendEnable);
		void setStencilTest(bool stencilTestEnable, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp);
		void setColorMask(bool red, bool green, bool blue, bool alpha);
		void setScissorTest(bool scissorTestEnable);
		void setCullMode(int mode);
		void setWindingOrder(int frontFace);
		void createProgram();
		vk::PipelineLayout getPipelineLayout();
		vk::Pipeline getPipeline();
		
		typedef std::shared_ptr<GraphicsPipeline> Ptr;
		static Ptr create(vk::RenderPass renderPass, std::string name, int numAttachments)
		{
			return std::make_shared<GraphicsPipeline>(renderPass, name, numAttachments);
		}
	private:
		vk::Device device;
		vk::RenderPass renderPass;

		// Pipeline objects
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;
		vk::PipelineCache cache;

		// shader modules
		std::vector<vk::ShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

		// vertex input
		vk::VertexInputBindingDescription vertexInputBinding;
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;

		// state infos
		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments;
		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::PipelineDynamicStateCreateInfo dynamicState;
		std::vector<vk::DynamicState> dynamicStateEnables;

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
		vk::PipelineLayout getPipelineLayout();
		vk::Pipeline getPipeline();

		typedef std::shared_ptr<ComputePipeline> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<ComputePipeline>(name);
		}
	private:
		vk::Device device;

		// Pipeline objects
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;
		vk::PipelineCache cache;

		// shader modules
		std::vector<vk::ShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;
	};
}

#endif // INCLUDED_VKPIPELINE