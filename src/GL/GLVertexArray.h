#ifndef INCLUDED_GLVERTEXARRAY
#define INCLUDED_GLVERTEXARRAY

#pragma once

#include <GL/glew.h>

#include <vector>

namespace GL
{
	class VertexArray
	{
		GLuint id;

		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;
	public:
		VertexArray()
		{
			glGenVertexArrays(1, &id);
		}

		~VertexArray()
		{
			glDeleteVertexArrays(1, &id);
		}

		void addAttrib(GLuint index, GLint size, GLuint bufferID, GLsizei stride, const void* pointer)
		{
			bind();
			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, bufferID);
			glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
			glBindBuffer(GL_ARRAY_BUFFER, bufferID);
			unbind();
		}

		void bind()
		{
			glBindVertexArray(id);
		}

		void unbind()
		{
			glBindVertexArray(0);
		}
	};
}

#endif // INCLUDED_GLVERTEXARRAY