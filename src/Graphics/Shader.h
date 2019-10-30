#ifndef INCLUDED_SHADER
#define INCLUDED_SHADER

#pragma once

#include <GL/GLProgram.h>
#include <GL/GLShader.h>

class Shader
{
private:
	std::string name;
	GL::Program program;

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
public:
	Shader(const std::string& name);

	template<typename ShaderType>
	bool compile(const std::string& code)
	{
		ShaderType shader;
		if (shader.compile(code.c_str()))
			program.attachShader(shader);
		else
		{
			std::cout << "error compiling shader " << name << std::endl;
			std::cout << shader.getErrorLog() << std::endl;
			return false;
		}
		return true;
	}
	bool link();
	void use();

	template<typename T>
	void setUniform(const std::string& name, const T& value0)
	{
		program.setUniform(name, value0);
	}

	template<typename T>
	void setUniform(const std::string& name, const T& value0, const T& value1)
	{
		program.setUniform(name, value0, value1);
	}

	template<typename T>
	void setUniform(const std::string& name, const T& value0, const T& value1, const T& value2)
	{
		program.setUniform(name, value0, value1, value2);
	}

	template<typename T>
	void setUniform(const std::string& name, const T& value0, const T& value1, const T& value2, const T& value3)
	{
		program.setUniform(name, value0, value1, value2, value3);
	}

	template<typename T>
	void setUniform(const std::string& name, const std::vector<T>& value)
	{
		program.setUniform(name, value);
	}

	typedef std::shared_ptr<Shader> Ptr;
	static Ptr create(const std::string& name);
	std::string getName() const;
};

#endif // INCLUDED_SHADER