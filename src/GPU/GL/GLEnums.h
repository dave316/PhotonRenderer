#ifndef INCLUDED_GLENUMS
#define INCLUDED_GLENUMS

#pragma once

#include <GPU/Enums.h>
#include <GPU/GL/GLPlatform.h>

namespace GL
{
	GLenum getBufferTarget(GPU::BufferUsage usage);
	GLenum getShaderType(GPU::ShaderStage stage);
	GLenum getSize(GPU::VertexAttribFormat format);
	GLenum getTopology(GPU::Topology topology);
}

#endif // INCLUDED_GLENUMS