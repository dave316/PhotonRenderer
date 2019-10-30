#include "Shader.h"

Shader::Shader(const std::string& name) :
	name(name)
{

}

bool Shader::link()
{
	if (program.link())
		program.loadUniforms();
	else
	{
		std::cout << "error linking shader program " << name << std::endl;
		std::cout << program.getErrorLog() << std::endl;
		return false;
	}
	return true;
}

void Shader::use()
{
	program.use();
}

Shader::Ptr Shader::create(const std::string& name)
{
	return std::make_shared<Shader>(name);
}

std::string Shader::getName() const
{
	return name;
}