#include "ShaderLoader.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace IO
{
	std::string loadTxtFile(const std::string& fileName)
	{
		std::ifstream file(fileName);
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

	std::string loadExpanded(const std::string& fileName, const std::string& matFile)
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

					// TODO: find a better way todo this, maybe a json file that contains shader/material variants
					if (!matFile.empty() && subFile.compare("Material.glsl") == 0)
						includeFile = fileName.substr(0, index) + "/Materials/" + matFile; 

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

		//std::ofstream outFile(fileName + ".complete");
		//outFile << expandedCode;
		//outFile.close();

		return expandedCode;
	}

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

	std::vector<std::string> getAllDirNames(const std::string& path)
	{
		if (!fs::exists(path))
			std::cout << "path " << path << " does not exist!" << std::endl;

		if (!fs::is_directory(path))
			std::cout << "path " << path << " is not a directory!" << std::endl;

		std::vector<std::string> dirNames;
		for (auto& file : fs::directory_iterator(path))
		{
			if (fs::is_directory(file))
			{
				dirNames.push_back(file.path().string());
			}
		}
		return dirNames;
	}

	std::vector<Shader::Ptr> loadShadersFromPath(const std::string& path)
	{
		auto filenames = getAllFileNames(path); // check subdirectories...
		std::vector<Shader::Ptr> shaderList;
		std::map<std::string, std::vector<std::string>> shaderFiles;
		for (auto fn : filenames)
		{
			int index = fn.find_last_of('.');
			std::string name = fn.substr(0, index);
			if (shaderFiles.find(name) == shaderFiles.end())
				shaderFiles[name] = std::vector<std::string>();
			shaderFiles[name].push_back(fn);
		}

		std::vector<std::string> matfiles = {
			"Default.glsl",
			//"Default.glsl",
			//"HeightBlend3.glsl",
			//"Rain.glsl",
			//"RainPOM.glsl",
			//"RainRefraction.glsl",
			//"Velvet.glsl",
			//"Glass.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			//"Default.glsl",
			"SpecGloss.glsl",
			//"TerrainBlend.glsl",
		};

		// TODO: where to put this? config file? data driven? 
		std::vector<std::vector<std::string>> shaderVariants = {
			{},
			//{ "POM" },
			//{ "LAYERED_MATERIAL" },
			//{ "RAIN_MATERIAL" },
			//{ "RAIN_MATERIAL_POM", "POM"},
			//{ "RAIN_REFRACTION" },
			//{ "VELVET_MATERIAL" },
			//{ "GLASS_MATERIAL" },
			//{ "VERTEX_WIND" },
			//{ "SHEEN" }, 
			//{ "CLEARCOAT" }, 
			//{ "TRANSMISSION" }, 
			//{ "TRANSLUCENCY" },
			//{ "SPECULAR" }, 
			//{ "IRIDESCENCE" },
			//{ "ANISOTROPY" },
			//{ "SHEEN", "SPECULAR" },
			//{ "CLEARCOAT", "SPECULAR" },
			//{ "CLEARCOAT", "TRANSMISSION" },
			//{ "TRANSMISSION", "IRIDESCENCE" },
			//{ "TRANSMISSION", "SPECULAR" },
			//{ "TRANSMISSION", "SPECULAR", "IRIDESCENCE" },
			//{ "TRANSMISSION", "TRANSLUCENCY" },
			//{ "SPECULAR", "IRIDESCENCE" },
			//{ "ANISOTROPY", "IRIDESCENCE" },
			{ "SPECGLOSS" },
			//{ "SPECGLOSS", "LAYERED_MATERIAL" }
		};

		for (auto shaderName : shaderFiles)
		{
			auto name = shaderName.first;
			auto stageList = shaderName.second;
			if (stageList.size() == 1)
			{
				auto shaderFile = stageList[0];
				int index = shaderFile.find_last_of('.') + 1;
				int len = shaderFile.length() - index;

				std::string stage = shaderFile.substr(index, len);
				if (stage.compare("glsl") == 0)
					continue;
			}			
			
			{
				if (name.compare("Default") == 0)
				{
					std::string vsCode = "#version 460 core\n";
					vsCode += loadExpanded(path + "/" + shaderName.second[1], "");

					GL::VertexShader vs;
					if (!vs.compile(vsCode.c_str()))
					{
						std::cout << "error compiling shader " << name << std::endl;
						std::cout << vs.getErrorLog() << std::endl;
						continue;
					}

					for (int i = 0; i < shaderVariants.size(); i++)
					{
						auto variants = shaderVariants[i];
						auto matFile = matfiles[i];

						std::string fsCode = loadExpanded(path + "/" + shaderName.second[0], matFile);

						std::string expandedCode = "#version 460 core\n";
						bool specGlossMat = false;
						for (auto v : variants)
						{
							expandedCode += "#define " + v + "\n";
							if (v.compare("SPECGLOSS") == 0)
								specGlossMat = true;
						}
						if (!specGlossMat)
							expandedCode += "#define METAL_ROUGH_MATERIAL\n";

						expandedCode += "#define DEBUG_OUTPUT\n";
						expandedCode += fsCode;

						GL::FragmentShader fs;
						if (!fs.compile(expandedCode.c_str()))
						{
							std::cout << expandedCode << std::endl;
							std::cout << "error compiling shader " << name << std::endl;
							std::cout << fs.getErrorLog() << std::endl;
							continue;
						}

						std::string variantName = name;
						for (auto v : variants)
							variantName += '_' + v;

						std::cout << "compiling shader " << variantName << std::endl;

						auto shader = Shader::create(variantName);

						if (variantName.compare("Default_VERTEX_WIND") == 0)
						{
							std::string vsCode = "#version 460 core\n";
							vsCode += "#define VERTEX_WIND\n";
							vsCode += loadExpanded(path + "/" + shaderName.second[1], "");

							GL::VertexShader windVS;
							if (!windVS.compile(vsCode.c_str()))
							{
								std::cout << "error compiling shader " << name << std::endl;
								std::cout << windVS.getErrorLog() << std::endl;
								continue;
							}
							shader->attach(windVS);
						}
						else
						{
							shader->attach(vs);
						}
						shader->attach(fs);
						if (shader->link())
						{
							shaderList.push_back(shader);
						}
						else
						{
							std::cout << "error loading shader " << name << std::endl;
						}
					}
				}
				else
				{
					std::cout << "compiling shader " << name << std::endl;

					bool success = true;
					auto shader = Shader::create(name);
					for (auto shaderFile : shaderName.second)
					{
						int index = shaderFile.find_last_of('.') + 1;
						int len = shaderFile.length() - index;

						std::string stage = shaderFile.substr(index, len);
						std::string code = loadExpanded(path + "/" + shaderFile, "");
						code = "#version 460 core\n" + code;
						if (stage.compare("vert") == 0)
							success = shader->compile<GL::VertexShader>(code);
						else if (stage.compare("geom") == 0)
							success = shader->compile<GL::GeometryShader>(code);
						else if (stage.compare("frag") == 0)
							success = shader->compile<GL::FragmentShader>(code);
						else if (stage.compare("comp") == 0)
							success = shader->compile<GL::ComputeShader>(code);

						if (!success)
							break;
					}

					if (success)
					{
						shader->link();
						shaderList.push_back(shader);
					}
					else
					{
						std::cout << "error loading shader " << name << std::endl;
					}
				}
			}
		}
		return shaderList;
	}
}
