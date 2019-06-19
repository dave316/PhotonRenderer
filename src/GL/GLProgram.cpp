#include "GLProgram.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GL
{
	// single float
	template<> void Program::setUniform<GLfloat>(const std::string& name, const GLfloat& value0)
	{
		glProgramUniform1f(id, getUniformLocation(name), value0);
	}

	template<> void Program::setUniform<GLfloat>(const std::string& name, const GLfloat& value0, const GLfloat& value1)
	{
		glProgramUniform2f(id, getUniformLocation(name), value0, value1);
	}

	template<> void Program::setUniform<GLfloat>(const std::string& name, const GLfloat& value0, const GLfloat& value1, const GLfloat& value2)
	{
		glProgramUniform3f(id, getUniformLocation(name), value0, value1, value2);
	}

	template<> void Program::setUniform<GLfloat>(const std::string& name, const GLfloat& value0, const GLfloat& value1, const GLfloat& value2, const GLfloat& value3)
	{
		glProgramUniform4f(id, getUniformLocation(name), value0, value1, value2, value3);
	}

	// single int
	template<> void Program::setUniform<GLint>(const std::string& name, const GLint& value0)
	{
		glProgramUniform1i(id, getUniformLocation(name), value0);
	}

	template<> void Program::setUniform<GLint>(const std::string& name, const GLint& value0, const GLint& value1)
	{
		glProgramUniform2i(id, getUniformLocation(name), value0, value1);
	}

	template<> void Program::setUniform<GLint>(const std::string& name, const GLint& value0, const GLint& value1, const GLint& value2)
	{
		glProgramUniform3i(id, getUniformLocation(name), value0, value1, value2);
	}

	template<> void Program::setUniform<GLint>(const std::string& name, const GLint& value0, const GLint& value1, const GLint& value2, const GLint& value3)
	{
		glProgramUniform4i(id, getUniformLocation(name), value0, value1, value2, value3);
	}

	template<> void Program::setUniform<bool>(const std::string& name, const bool& value)
	{
		glProgramUniform1i(id, getUniformLocation(name), value);
	}

	// multiple float
	template<> void Program::setUniform<GLfloat>(const std::string& name, const std::vector<GLfloat>& values)
	{
		glProgramUniform1fv(id, getUniformLocation(name), (GLsizei)values.size(), (GLfloat*)&values[0]);
	}

	template<> void Program::setUniform<glm::vec2>(const std::string& name, const glm::vec2& value)
	{
		glProgramUniform2fv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::vec2>(const std::string& name, const std::vector<glm::vec2>& values)
	{
		glProgramUniform2fv(id, getUniformLocation(name), (GLsizei)values.size(), (GLfloat*)&values[0]);
	}

	template<> void Program::setUniform<glm::vec3>(const std::string& name, const glm::vec3& value)
	{
		glProgramUniform3fv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::vec3>(const std::string& name, const std::vector<glm::vec3>& values)
	{
		glProgramUniform3fv(id, getUniformLocation(name), (GLsizei)values.size(), (GLfloat*)&values[0]);
	}

	template<> void Program::setUniform<glm::vec4>(const std::string& name, const glm::vec4& value)
	{
		glProgramUniform4fv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::vec4>(const std::string& name, const std::vector<glm::vec4>& values)
	{
		glProgramUniform4fv(id, getUniformLocation(name), (GLsizei)values.size(), (GLfloat*)&values[0]);
	}

	// multiple int
	template<> void Program::setUniform<GLint>(const std::string& name, const std::vector<GLint>& values)
	{
		glProgramUniform1iv(id, getUniformLocation(name), (GLsizei)values.size(), (GLint*)&values[0]);
	}

	template<> void Program::setUniform<glm::ivec2>(const std::string& name, const glm::ivec2& value)
	{
		glProgramUniform2iv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::ivec2>(const std::string& name, const std::vector<glm::ivec2>& values)
	{
		glProgramUniform2iv(id, getUniformLocation(name), (GLsizei)values.size(), (GLint*)&values[0]);
	}

	template<> void Program::setUniform<glm::ivec3>(const std::string& name, const glm::ivec3& value)
	{
		glProgramUniform3iv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::ivec3>(const std::string& name, const std::vector<glm::ivec3>& values)
	{
		glProgramUniform3iv(id, getUniformLocation(name), (GLsizei)values.size(), (GLint*)&values[0]);
	}

	template<> void Program::setUniform<glm::ivec4>(const std::string& name, const glm::ivec4& value)
	{
		glProgramUniform4iv(id, getUniformLocation(name), 1, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::ivec4>(const std::string& name, const std::vector<glm::ivec4>& values)
	{
		glProgramUniform4iv(id, getUniformLocation(name), (GLsizei)values.size(), (GLint*)&values[0]);
	}

	// matrices
	template<> void Program::setUniform<glm::mat2>(const std::string& name, const glm::mat2& value)
	{
		glProgramUniformMatrix2fv(id, getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::mat2>(const std::string& name, const std::vector<glm::mat2>& values)
	{
		glProgramUniformMatrix2fv(id, getUniformLocation(name), (GLsizei)values.size(), GL_FALSE, (GLfloat*)&values[0]);
	}

	template<> void Program::setUniform<glm::mat3>(const std::string& name, const glm::mat3& value)
	{
		glProgramUniformMatrix3fv(id, getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::mat3>(const std::string& name, const std::vector<glm::mat3>& values)
	{
		glProgramUniformMatrix3fv(id, getUniformLocation(name), (GLsizei)values.size(), GL_FALSE, (GLfloat*)&values[0]);
	}

	template<> void Program::setUniform<glm::mat4>(const std::string& name, const glm::mat4& value)
	{
		glProgramUniformMatrix4fv(id, getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	template<> void Program::setUniform<glm::mat4>(const std::string& name, const std::vector<glm::mat4>& values)
	{
		glProgramUniformMatrix4fv(id, getUniformLocation(name), (GLsizei)values.size(), GL_FALSE, (GLfloat*)values.data());
	}
}
