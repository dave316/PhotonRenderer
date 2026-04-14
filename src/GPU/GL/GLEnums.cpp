#include "GLEnums.h"

namespace GL
{
	GLenum getBufferTarget(GPU::BufferUsage usage)
	{
		GLenum target;
		if (usage & GPU::BufferUsage::VertexBuffer)
			target = target = GL_ARRAY_BUFFER;
		else if (usage & GPU::BufferUsage::IndexBuffer)
			target = GL_ELEMENT_ARRAY_BUFFER;
		else if (usage & GPU::BufferUsage::UniformBuffer)
			target = GL_UNIFORM_BUFFER;
		return target;
	}

	GLenum getShaderType(GPU::ShaderStage stage)
	{
		GLenum type;
		switch (stage)
		{
			case GPU::ShaderStage::Vertex: type = GL_VERTEX_SHADER; break;
			case GPU::ShaderStage::Geometry: type = GL_GEOMETRY_SHADER; break;
			case GPU::ShaderStage::Fragment: type = GL_FRAGMENT_SHADER; break;
		}
		return type;
	}

	GLenum getSize(GPU::VertexAttribFormat format)
	{
		GLint size;
		switch (format)
		{
			case GPU::VertexAttribFormat::Vector1F: size = 1; break;
			case GPU::VertexAttribFormat::Vector2F: size = 2; break;
			case GPU::VertexAttribFormat::Vector3F: size = 3; break;
			case GPU::VertexAttribFormat::Vector4F: size = 4; break;
			case GPU::VertexAttribFormat::Vectur4UC: size = 4; break;
		}
		return size;
	}

	GLenum getTopology(GPU::Topology topology)
	{
		return (GLenum)topology;
	}
}