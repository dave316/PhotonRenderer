#ifndef INCLUDED_SHADERLOADER
#define INCLUDED_SHADERLOADER

#pragma once

#include <Graphics/Shader.h>

namespace IO
{
	std::string loadTxtFile(const std::string& fileName);
	std::string loadExpanded(const std::string& fileName, const std::string& matFile);
	std::vector<std::string> getAllFileNames(const std::string& path, const std::string& extension = "");
	std::vector<std::string> getAllDirNames(const std::string& path);
	std::vector<Shader::Ptr> loadShadersFromPath(const std::string& path);
}

#endif // INCLUDED_SHADERLOADER