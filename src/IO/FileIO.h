#ifndef INCLUDED_FILEIO
#define INCLUDED_FILEIO

#pragma once

#include <string>
#include <vector>

namespace IO
{
	std::vector<std::string> getAllFileNames(const std::string& path, const std::string& extension);
	void loadBinary(std::string fileName, std::string& buffer);
	std::string loadTxtFile(const std::string& fileName);
	std::string loadExpanded(const std::string& fileName);
}

#endif // INCLUDED_FILEIO