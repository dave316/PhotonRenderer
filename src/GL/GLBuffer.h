#ifndef INCLUDED_GLBUFFER
#define INCLUDED_GLBUFFER

#pragma once

#include <GL/glew.h>

#include <vector>

namespace GL
{
	template<GLenum Target, typename DataType>
	class Buffer
	{
		GLuint id;
		GLsizei numElements;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	public:
		Buffer()
		{
			glGenBuffers(1, &id);
		}

		~Buffer()
		{
			glDeleteBuffers(1, &id);
		}

		void upload(std::vector<DataType>& data, GLenum usage = GL_STATIC_DRAW)
		{
			numElements = (GLsizei)data.size();

			bind();
			glBufferData(Target, data.size() * sizeof(DataType), data.data(), usage);
			unbind();
		}

		void upload(const void* data, int size, GLenum usage = GL_STATIC_DRAW)
		{
			numElements = (GLsizei)size;

			bind();
			glBufferData(Target, size * sizeof(DataType), data, usage);
			unbind();
		}

		void bindBase(GLuint index)
		{
			glBindBufferBase(Target, index, id);
		}

		void bind()
		{
			glBindBuffer(Target, id);
		}

		void unbind()
		{
			glBindBuffer(Target, 0);
		}

		GLsizei size() const
		{
			return numElements;
		}
		
		operator GLuint() const
		{
			return id;
		}
	};

	template<typename DataType>
	using VertexBuffer = Buffer<GL_ARRAY_BUFFER, typename DataType>;

	template<typename DataType>
	using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER, typename DataType>;

	template<typename DataType>
	using UniformBuffer = Buffer<GL_UNIFORM_BUFFER, typename DataType>;
}

#endif // INCLUDED_GLBUFFER