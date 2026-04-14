#include "VKPipeline.h"
#include <GPU/VK/VKDescriptorPool.h>
#include <GPU/VK/VKDevice.h>
#include <GPU/VK/VKSampler.h>
#include <GPU/VK/VKEnums.h>
#include <iostream>

namespace VK
{
	GraphicsPipeline::GraphicsPipeline(vk::RenderPass renderPass, std::string name, int numAttachments) :
		renderPass(renderPass),
		GPU::GraphicsPipeline(name),
		device(Device::getInstance().getDevice())
	{
		// Default settings
		inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList);
		
		rasterizationState.polygonMode = vk::PolygonMode::eFill;
		rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
		
		blendAttachments.resize(numAttachments);
		for (int i = 0; i < numAttachments; i++)
		{
			blendAttachments[i].colorWriteMask = vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA;

			blendAttachments[i].blendEnable = VK_TRUE;
			blendAttachments[i].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
			blendAttachments[i].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			blendAttachments[i].colorBlendOp = vk::BlendOp::eAdd;
			blendAttachments[i].srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			blendAttachments[i].dstAlphaBlendFactor = vk::BlendFactor::eZero;
			blendAttachments[i].alphaBlendOp = vk::BlendOp::eAdd;
		}
		colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, {}, {}, blendAttachments);

		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
		depthStencilState.stencilTestEnable = VK_TRUE;
		depthStencilState.front.failOp = vk::StencilOp::eKeep;
		depthStencilState.front.depthFailOp = vk::StencilOp::eReplace;
		depthStencilState.front.passOp = vk::StencilOp::eReplace;
		depthStencilState.front.compareOp = vk::CompareOp::eAlways;
		depthStencilState.front.compareMask = 0xFF;
		depthStencilState.front.writeMask = 0xFF;
		depthStencilState.front.reference = 0;
		depthStencilState.back = depthStencilState.front;

		viewportState = vk::PipelineViewportStateCreateInfo({}, 1, nullptr, 1, nullptr);

		multisampleState = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1);

		dynamicStateEnables.push_back(vk::DynamicState::eViewport);
		dynamicStateEnables.push_back(vk::DynamicState::eScissor);
		dynamicStateEnables.push_back(vk::DynamicState::eCullModeEXT);
		dynamicStateEnables.push_back(vk::DynamicState::ePrimitiveTopologyEXT);
		dynamicState = vk::PipelineDynamicStateCreateInfo({}, dynamicStateEnables);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		device.destroyPipeline(pipeline);
		device.destroyPipelineCache(cache);
		device.destroyPipelineLayout(layout);
		for (auto m : shaderModules)
			device.destroyShaderModule(m);
	}

	void GraphicsPipeline::setVertexInputDescripton(GPU::VertexDescription& inputDescription)
	{
		vertexInputBinding.binding = inputDescription.binding;
		vertexInputBinding.stride = inputDescription.stride;
		vertexInputBinding.inputRate = static_cast<vk::VertexInputRate>((int)inputDescription.inputRate);
		for (auto& attrib : inputDescription.inputAttributes)
		{
			vk::VertexInputAttributeDescription vertexInputAttrib;
			vertexInputAttrib.location = attrib.location;
			vertexInputAttrib.binding = attrib.binding;
			vertexInputAttrib.offset = attrib.offset;
			vertexInputAttrib.format = getVertexFormat(attrib.format);
			vertexInputAttributes.push_back(vertexInputAttrib);
		}

		if (vertexInputAttributes.empty())
		{
			vertexInputState = vk::PipelineVertexInputStateCreateInfo({}, {}, {});
		}
		else
		{
			vertexInputState = vk::PipelineVertexInputStateCreateInfo({}, vertexInputBinding, vertexInputAttributes);
		}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		if(!setLayouts.empty())
		{
			std::vector<vk::DescriptorSetLayout> layouts;
			auto vkDescriptorPool = std::dynamic_pointer_cast<VK::DescriptorPool>(descriptorPool);
			for (auto layoutName : setLayouts)
				layouts.push_back(vkDescriptorPool->getLayout(layoutName));
			vk::PipelineLayoutCreateInfo layoutInfo({}, layouts);
			layout = device.createPipelineLayout(layoutInfo);
		}
		else
		{
			vk::PipelineLayoutCreateInfo layoutInfo({}, {});
			layout = device.createPipelineLayout(layoutInfo);
		}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants)
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		auto vkDescriptorPool = std::dynamic_pointer_cast<VK::DescriptorPool>(descriptorPool);
		for (auto layoutName : setLayouts)
			layouts.push_back(vkDescriptorPool->getLayout(layoutName));
		std::vector<vk::PushConstantRange> pushConstantRanges;
		for (auto pushConstant : pushConstants)
		{
			vk::PushConstantRange pushConstantRange;
			pushConstantRange.stageFlags = static_cast<vk::ShaderStageFlags>((int)pushConstant.stage);
			pushConstantRange.offset = pushConstant.offset;
			pushConstantRange.size = pushConstant.size;
			pushConstantRanges.push_back(pushConstantRange);
		}

		vk::PipelineLayoutCreateInfo layoutInfo({}, layouts, pushConstantRanges);
		layout = device.createPipelineLayout(layoutInfo);
	}

	void GraphicsPipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		// TODO: need to check if stage already exist etc.
		vk::ShaderModule shaderModule;
		vk::ShaderModuleCreateInfo shaderModuleInfo;
		shaderModuleInfo.codeSize = code.length();
		shaderModuleInfo.pCode = (uint32_t*)code.c_str();

		shaderModule = device.createShaderModule(shaderModuleInfo);
		shaderModules.push_back(shaderModule);

		vk::PipelineShaderStageCreateInfo shaderStageInfo;
		shaderStageInfo.stage = getShaderStage(stage);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		shaderStageInfos.push_back(shaderStageInfo);
	}

	void GraphicsPipeline::setDepthTest(bool depthTestEnable, bool depthWriteEnable)
	{
		depthStencilState.depthTestEnable = depthTestEnable;
		depthStencilState.depthWriteEnable = depthWriteEnable;
	}

	void GraphicsPipeline::setBlending(bool blendEnable)
	{
		for (int i = 0; i < blendAttachments.size(); i++)
			blendAttachments[i].blendEnable = blendEnable;
	}

	void GraphicsPipeline::setStencilTest(bool stencilTestEnable, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp)
	{
		depthStencilState.stencilTestEnable = stencilTestEnable;
		depthStencilState.front.compareOp = getCompareOp(compareOp);
		depthStencilState.front.reference = refValue;
		depthStencilState.front.writeMask = stencilMask;
		depthStencilState.back = depthStencilState.front;
	}

	void GraphicsPipeline::setColorMask(bool red, bool green, bool blue, bool alpha)
	{
		vk::ColorComponentFlags writeMask = vk::ColorComponentFlags(0);
		if (red)
			writeMask |= vk::ColorComponentFlagBits::eR;
		if (green)
			writeMask |= vk::ColorComponentFlagBits::eG;
		if (blue)
			writeMask |= vk::ColorComponentFlagBits::eB;
		if (alpha)
			writeMask |= vk::ColorComponentFlagBits::eA;
		blendAttachments[0].colorWriteMask = writeMask;
		colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, {}, {}, blendAttachments);
	}

	void GraphicsPipeline::setScissorTest(bool scissorTestEnable)
	{
		// TODO: add or remove to dynamic state
	}

	void GraphicsPipeline::setCullMode(int cullMode)
	{
		rasterizationState.cullMode = static_cast<vk::CullModeFlags>(cullMode);
	}

	void GraphicsPipeline::setWindingOrder(int frontFace)
	{
		rasterizationState.frontFace = static_cast<vk::FrontFace>(frontFace);
	}

	void GraphicsPipeline::createProgram()
	{
		vk::GraphicsPipelineCreateInfo pipelineInfo(
			{},
			shaderStageInfos,
			&vertexInputState,
			&inputAssemblyState,
			{},
			&viewportState,
			&rasterizationState,
			&multisampleState,
			&depthStencilState,
			&colorBlendState,
			&dynamicState,
			layout,
			renderPass,
			{},
			{},
			-1
		);

		cache = device.createPipelineCache({});

		vk::Result result;
		std::tie(result, pipeline) = device.createGraphicsPipeline(cache, pipelineInfo);
		if (result != vk::Result::eSuccess)
		{
			std::cout << "error creating graphics pipeline!" << std::endl;
		}
	}

	vk::PipelineLayout GraphicsPipeline::getPipelineLayout()
	{
		return layout;
	}

	vk::Pipeline GraphicsPipeline::getPipeline()
	{
		return pipeline;
	}

	ComputePipeline::ComputePipeline(std::string name) :
		GPU::ComputePipeline(name),
		device(Device::getInstance().getDevice())
	{

	}
	ComputePipeline::~ComputePipeline()
	{
		device.destroyPipeline(pipeline);
		device.destroyPipelineCache(cache);
		device.destroyPipelineLayout(layout);
		for (auto m : shaderModules)
			device.destroyShaderModule(m);
	}

	void ComputePipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		if (!setLayouts.empty())
		{
			std::vector<vk::DescriptorSetLayout> layouts;
			auto vkDescriptorPool = std::dynamic_pointer_cast<VK::DescriptorPool>(descriptorPool);
			for (auto layoutName : setLayouts)
				layouts.push_back(vkDescriptorPool->getLayout(layoutName));
			vk::PipelineLayoutCreateInfo layoutInfo({}, layouts);
			layout = device.createPipelineLayout(layoutInfo);
		}
		else
		{
			vk::PipelineLayoutCreateInfo layoutInfo({}, {});
			layout = device.createPipelineLayout(layoutInfo);
		}
	}

	void ComputePipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		vk::ShaderModule shaderModule;
		vk::ShaderModuleCreateInfo shaderModuleInfo;
		shaderModuleInfo.codeSize = code.length();
		shaderModuleInfo.pCode = (uint32_t*)code.c_str();

		shaderModule = device.createShaderModule(shaderModuleInfo);
		shaderModules.push_back(shaderModule);

		vk::PipelineShaderStageCreateInfo shaderStageInfo;
		shaderStageInfo.stage = getShaderStage(stage);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		shaderStageInfos.push_back(shaderStageInfo);
	}

	void ComputePipeline::createProgram()
	{
		vk::ComputePipelineCreateInfo pipelineInfo;
		pipelineInfo.layout = layout;
		pipelineInfo.stage = shaderStageInfos[0];

		cache = device.createPipelineCache({});

		vk::Result result;
		std::tie(result, pipeline) = device.createComputePipeline(cache, pipelineInfo);
		if (result != vk::Result::eSuccess)
		{
			std::cout << "error creating graphics pipeline!" << std::endl;
		}
	}

	vk::PipelineLayout ComputePipeline::getPipelineLayout()
	{
		return layout;
	}

	vk::Pipeline ComputePipeline::getPipeline()
	{
		return pipeline;
	}
}