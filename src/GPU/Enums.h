#ifndef INCLUDED_ENUMS
#define INCLUDED_ENUMS

#pragma once

namespace GPU
{
	enum class BufferUsage
	{
		TransferSrc = 0x1,
		TransferDst = 0x2,
		UniformBuffer = 0x10,
		StorageBuffer = 0x20,
		IndexBuffer = 0x40,
		VertexBuffer = 0x80
	};

	inline constexpr BufferUsage operator| (BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline constexpr int operator& (BufferUsage a, BufferUsage b)
	{
		return static_cast<int>(a) & static_cast<int>(b);
	}

	enum class VertexInputeRate
	{
		Vertex,
		Instance
	};

	enum class VertexAttribFormat
	{
		Vector1F,
		Vector2F,
		Vector3F,
		Vector4F,
		Vectur4UC
	};

	enum class DescriptorType
	{
		UniformBuffer,
		CombinedImageSampler,
		StorageImage
	};

	enum class ShaderStage
	{
		Vertex = 0x1,
		Geometry = 0x8,
		Fragment = 0x10,
		Compute = 0x20
	};

	inline constexpr ShaderStage operator| (ShaderStage a, ShaderStage b)
	{
		return static_cast<ShaderStage>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline constexpr int operator& (ShaderStage a, ShaderStage b)
	{
		return static_cast<int>(a) & static_cast<int>(b);
	}

	enum class Topology
	{
		Points,
		Lines,
		LineLoop,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan,
	};
}

#endif // INCLUDED_ENUMS