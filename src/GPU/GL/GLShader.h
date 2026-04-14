#ifndef INCLUDED_GLSHADER
#define INCLUDED_GLSHADER

#pragma once

#include <GPU/GL/GLPlatform.h>
#include <string>

namespace GL
{	
	class Shader
	{
		GLuint id;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
	public:
		Shader(GLenum shaderType)
		{
			id = glCreateShader(shaderType);
		}

		~Shader()
		{
			glDeleteShader(id);
		}

		bool compile(const char* code)
		{
			glShaderSource(id, 1, &code, 0);
			glCompileShader(id);

			GLint status;
			glGetShaderiv(id, GL_COMPILE_STATUS, &status);

			return (status == GL_TRUE);
		}

		std::string getErrorLog() const
		{
			GLint len;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			std::string log(len + 1, '\0');
			glGetShaderInfoLog(id, len, 0, &log[0]);
			return log;
		}

		operator GLuint() const
		{
			return id;
		}
	};
}

#endif // INCLUDED_GLSHADER