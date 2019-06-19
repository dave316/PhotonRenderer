#ifndef INCLUDED_GLPROGRAM
#define INCLUDED_GLPROGRAM

#pragma once

#include <GL/glew.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace GL
{	
	class Program
	{
		GLuint id;

		std::map<std::string, GLint> uniforms;

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;
	public:
		Program()
		{
			id = glCreateProgram();
		}

		~Program()
		{
			glDeleteProgram(id);
		}

		template<typename ShaderType>
		void attachShader(const ShaderType& shader)
		{
			glAttachShader(id, shader);
		}

		template<typename ShaderType>
		void detachShader(const ShaderType& shader)
		{
			glDetachShader(id, shader);
		}

		bool link()
		{
			GLint status;
			glLinkProgram(id);
			glGetProgramiv(id, GL_LINK_STATUS, &status);
			return (status == GL_TRUE);
		}

		std::string getErrorLog() const
		{
			GLint len;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			std::string log(len + 1, '\0');
			glGetProgramInfoLog(id, len, 0, &log[0]);
			return log;
		}

		void loadUniforms()
		{
			GLint numUniforms;
			glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numUniforms);

			GLchar buffer[128];
			GLsizei length;
			GLint size;
			GLenum type;

			for (GLint i = 0; i < numUniforms; i++)
			{
				glGetActiveUniform(id, i, 128, &length, &size, &type, buffer);
				std::string name(buffer);
				uniforms[name] = glGetUniformLocation(id, name.c_str());

				//std::cout << name << " " << i << std::endl;
			}
		}

		GLint getUniformLocation(const std::string& name)
		{
			if (uniforms.find(name) != uniforms.end())
			{
				return uniforms[name];
			}
			return -1;
		}

		void use()
		{
			glUseProgram(id);
		}

		operator GLuint() const
		{
			return id;
		}

		template<typename T>
		void setUniform(const std::string& name, const T& value0);
		template<typename T>
		void setUniform(const std::string& name, const T& value0, const T& value1);
		template<typename T>
		void setUniform(const std::string& name, const T& value0, const T& value1, const T& value2);
		template<typename T>
		void setUniform(const std::string& name, const T& value0, const T& value1, const T& value2, const T& value3);
		template<typename T>
		void setUniform(const std::string& name, const std::vector<T>& value);
	};
}

#endif // INCLUDED_GLPROGRAM