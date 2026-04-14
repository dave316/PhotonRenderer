#include "VKEnums.h"

namespace VK
{
	vk::Format getVertexFormat(GPU::VertexAttribFormat vertexFormat)
	{
		vk::Format format = vk::Format::eUndefined;
		switch (vertexFormat)
		{
		case GPU::VertexAttribFormat::Vector1F: format = vk::Format::eR32Sfloat; break;
		case GPU::VertexAttribFormat::Vector2F: format = vk::Format::eR32G32Sfloat; break;
		case GPU::VertexAttribFormat::Vector3F: format = vk::Format::eR32G32B32Sfloat; break;
		case GPU::VertexAttribFormat::Vector4F: format = vk::Format::eR32G32B32A32Sfloat; break;
		case GPU::VertexAttribFormat::Vectur4UC: format = vk::Format::eR8G8B8A8Unorm; break;
		}
		return format;
	}

	vk::ShaderStageFlagBits getShaderStage(GPU::ShaderStage stage)
	{
		vk::ShaderStageFlagBits stageFlag;
		switch (stage)
		{
		case GPU::ShaderStage::Vertex: stageFlag = vk::ShaderStageFlagBits::eVertex; break;
		case GPU::ShaderStage::Geometry: stageFlag = vk::ShaderStageFlagBits::eGeometry; break;
		case GPU::ShaderStage::Fragment: stageFlag = vk::ShaderStageFlagBits::eFragment; break;
		case GPU::ShaderStage::Compute: stageFlag = vk::ShaderStageFlagBits::eCompute; break;
		}
		return stageFlag;

	}

	vk::PrimitiveTopology getTopology(GPU::Topology topology)
	{
		vk::PrimitiveTopology vkTopology;
		switch (topology)
		{
			case GPU::Topology::Points: vkTopology = vk::PrimitiveTopology::ePointList; break;
			case GPU::Topology::Lines: vkTopology = vk::PrimitiveTopology::eLineList; break;
			case GPU::Topology::LineLoop: vkTopology = vk::PrimitiveTopology::eLineStrip; break; // TODO: no line loop in vulkan
			case GPU::Topology::LineStrip: vkTopology = vk::PrimitiveTopology::eLineStrip; break;
			case GPU::Topology::Triangles: vkTopology = vk::PrimitiveTopology::eTriangleList; break;
			case GPU::Topology::TriangleStrip: vkTopology = vk::PrimitiveTopology::eTriangleStrip; break;
			case GPU::Topology::TriangleFan: vkTopology = vk::PrimitiveTopology::eTriangleFan; break;
		}
		return vkTopology;
	}
}