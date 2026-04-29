#include "FileIO.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace IO
{
	std::vector<std::string> getAllFileNames(const std::string& path, const std::string& extension)
	{
		if (!fs::exists(path))
			std::cout << "path " << path << " does not exist!" << std::endl;

		if (!fs::is_directory(path))
			std::cout << "path " << path << " is not a directory!" << std::endl;

		std::vector<std::string> fileNames;
		for (auto& file : fs::directory_iterator(path))
		{
			if (fs::is_regular_file(file))
			{
				std::string fileName = file.path().filename().string();
				if (extension.empty())
				{
					fileNames.push_back(fileName);
				}
				else
				{
					std::string ext = file.path().extension().string();
					if (ext == extension)
					{
						fileNames.push_back(fileName);
					}
				}
			}
		}
		return fileNames;
	}

	void loadBinary(std::string fileName, std::string& buffer)
	{
		std::ifstream file(fileName, std::ios::binary);
		if (file.is_open())
		{
			file.seekg(0, std::ios::end);
			unsigned int size = (unsigned int)file.tellg();
			file.seekg(0, std::ios::beg);

			buffer.resize(size);

			file.read(&buffer[0], size);
			file.close();
		}
		else
		{
			std::cout << "could not open file " << fileName << std::endl;
		}
	}

	std::string loadTxtFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::binary);
		std::stringstream ss;

		if (file.is_open())
		{
			ss << file.rdbuf();
		}
		else
		{
			std::cout << "could not open file " << fileName << std::endl;
		}
		return ss.str();
	}

	std::string loadExpanded(const std::string& fileName)
	{
		std::string code = loadTxtFile(fileName);
		std::stringstream is(code);
		std::string line;
		std::string expandedCode = "";
		std::getline(is, line);

		while (std::getline(is, line))
		{
			if (!line.empty() && line.at(0) == '#')
			{
				size_t index = line.find_first_of(" ");
				std::string directive = line.substr(0, index);
				if (directive.compare("#include") == 0)
				{
					size_t start = line.find_first_of("\"") + 1;
					size_t end = line.find_last_of("\"");
					size_t index = fileName.find_last_of("/");
					std::string subFile = line.substr(start, end - start);
					std::string includeFile = fileName.substr(0, index) + "/" + subFile;
					std::string includeCode = loadTxtFile(includeFile);
					expandedCode += includeCode;
				}
				else
				{
					expandedCode += line + "\n";
				}
			}
			else
			{
				expandedCode += line + "\n";
			}
		}

		expandedCode += '\0';

		return expandedCode;
	}
}
