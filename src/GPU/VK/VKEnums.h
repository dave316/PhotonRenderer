#ifndef INCLUDED_VKENUMS
#define INCLUDED_VKENUMS

#pragma once

#include <GPU/Enums.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	vk::Format getVertexFormat(GPU::VertexAttribFormat vertexFormat);
	vk::ShaderStageFlagBits getShaderStage(GPU::ShaderStage stage);
	vk::PrimitiveTopology getTopology(GPU::Topology topology);
}

#endif // INCLUDED_VKENUMS