#include "UnityYAML.h"
#include <Importer/AssemblyDefinitionImporter.h>
#include <Importer/AudioImporter.h>
#include <Importer/DefaultImporter.h>
#include <Importer/ModelImporter.h>
#include <Importer/MonoImporter.h>
#include <Importer/NativeFormatImporter.h>
#include <Importer/PackageManifestImporter.h>
#include <Importer/PrefabImporter.h>
#include <Importer/ScriptedImporter.h>
#include <Importer/ShaderImporter.h>
#include <Importer/TextScriptImporter.h>
#include <Importer/TextureImporter.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

namespace fs = std::filesystem;

namespace Unity
{
	namespace YAML
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

		void loadMetadata(const std::string& assetPath, std::map<std::string, Metadata>& metaData)
		{
			std::map<std::string, uint32> stats;
			for (const auto& entry : fs::recursive_directory_iterator(assetPath))
			{
				if (entry.is_regular_file())
				{
					auto p = entry.path();
					auto metaPath = p.string();
					auto extension = p.extension().string();
					auto filename = p.filename().string();

					if (extension.compare(".meta") == 0)
					{
						std::replace(metaPath.begin(), metaPath.end(), '\\', '/');
						std::string filepath = metaPath.substr(0, metaPath.length() - 5); // remove .meta
						std::string content = loadTxtFile(metaPath);

						ryml::Tree root = ryml::parse_in_arena(ryml::to_csubstr(content));
						auto stream = root.rootref();

						//std::cout << filename << std::endl;

						Metadata data;
						data.filepath = filepath;
						data.filename = fs::path(filepath).filename().string();
						data.extension = fs::path(filepath).extension().string();
						stream["fileFormatVersion"] >> data.fileFormat;
						stream["guid"] >> data.guid;
						if (stream.has_child("folderAsset"))
						{
							std::string value;
							stream["folderAsset"] >> value;
							data.isFolder = value.compare("yes") == 0 ? true : false;
						}
						if (stream.has_child("timeCreated"))
							stream["timeCreated"] >> data.timeCreated;
						if (stream.has_child("licenseType"))
							stream["licenseType"] >> data.licenseType;

						std::string key = "";
						if (stream.has_child("DefaultImporter"))
						{
							key = "DefaultImporter";
							data.importer = new DefaultImporter;
							stream[key.c_str()] >> *(data.importer);
							//((DefaultImporter*)importer)->print("");
						}
						else if (stream.has_child("NativeFormatImporter"))
						{
							key = "NativeFormatImporter";
							data.importer = new NativeFormatImporter;
							stream[key.c_str()] >> *(data.importer);
							//((NativeFormatImporter*)importer)->print("");
						}
						else if (stream.has_child("ModelImporter"))
						{
							key = "ModelImporter";
							data.importer = new ModelImporter;
							stream[key.c_str()] >> *(data.importer);
							//((ModelImporter*)importer)->print("");
						}
						else if (stream.has_child("TextureImporter"))
						{
							//std::cout << filename << std::endl;
							key = "TextureImporter";
							data.importer = new TextureImporter;
							stream[key.c_str()] >> *(data.importer);
						}
						else if (stream.has_child("ShaderImporter"))
						{
							key = "ShaderImporter";
							data.importer = new ShaderImporter;
							stream[key.c_str()] >> *(data.importer);
							//((ShaderImporter*)importer)->print("");
						}
						else if (stream.has_child("AudioImporter"))
						{
							key = "AudioImporter";
							data.importer = new AudioImporter;
							stream[key.c_str()] >> *(data.importer);
							//((AudioImporter*)importer)->print("");
						}
						else if (stream.has_child("TextScriptImporter"))
						{
							key = "TextScriptImporter";
							data.importer = new TextScriptImporter;
							stream[key.c_str()] >> *(data.importer);
							//((TextScriptImporter*)importer)->print("");
						}
						else if (stream.has_child("MonoImporter"))
						{
							key = "MonoImporter";
							data.importer = new MonoImporter;
							stream[key.c_str()] >> *(data.importer);
							//((MonoImporter*)importer)->print("");
						}
						else if (stream.has_child("AssemblyDefinitionImporter"))
						{
							key = "AssemblyDefinitionImporter";
							data.importer = new AssemblyDefinitionImporter;
							stream[key.c_str()] >> *(data.importer);
							//((MonoImporter*)importer)->print("");
						}
						else if (stream.has_child("PrefabImporter"))
						{
							key = "PrefabImporter";
							data.importer = new PrefabImporter;
							stream[key.c_str()] >> *(data.importer);
							//((MonoImporter*)importer)->print("");
						}
						else if (stream.has_child("ScriptedImporter"))
						{
							key = "ScriptedImporter";
							data.importer = new ScriptedImporter;
							stream[key.c_str()] >> *(data.importer);
							//((MonoImporter*)importer)->print("");
						}
						else if (stream.has_child("PackageManifestImporter"))
						{
							key = "PackageManifestImporter";
							data.importer = new PackageManifestImporter;
							stream[key.c_str()] >> *(data.importer);
							//((MonoImporter*)importer)->print("");
						}
						else
						{
							std::cout << "error: unknown importer type for file " << filepath << std::endl;
						}

						if (stats.find(key) != stats.end())
							stats[key]++;
						else
							stats.insert(std::make_pair(key, 1));

						metaData.insert(std::pair(data.guid, data));
					}
				}
			}

			std::cout << "---------" << std::endl;
			std::cout << "Statistic" << std::endl;
			std::cout << "---------" << std::endl;
			for (auto [key, num] : stats)
				std::cout << key << ": " << num << std::endl;
		}

		bool readSceneFile(const std::string& sceneFullpath, std::map<int64, Object*>& objects)
		{
			std::ifstream is(sceneFullpath);
			if (!is.is_open())
			{
				std::cout << "error: cannot open scene file: " << sceneFullpath << std::endl;
				return false;
			}

			struct UnityObject
			{
				int64 id = 0;
				uint32 type = 0;
				std::string key = "";
				std::string content = "";
				bool stripped = false;
			};

			std::map<int64, UnityObject> unityObjects;
			std::string line;
			std::stringstream ss;
			uint64 currentID = 0;
			ss << is.rdbuf();
			while (std::getline(ss, line))
			{
				if (line.empty() || line[0] == '%') // skip empty lines and commets
					continue;
				if (line.substr(0, 3).compare("---") == 0) // start of a YAML document
				{
					std::string seperator, type, id, tag;
					std::stringstream lineSS(line);
					lineSS >> seperator >> type >> id >> tag;

					std::string keyLine;
					std::getline(ss, keyLine);

					UnityObject object;
					object.id = std::stoull(id.substr(1, id.length() - 1));
					object.type = std::stoul(type.substr(3, type.length() - 3));
					object.key = keyLine.substr(0, keyLine.length() - 1); // remove ':'
					object.content = keyLine + '\n';
					object.stripped = tag.compare("stripped") == 0 ? true : false;
					unityObjects.insert(std::make_pair(object.id, object));

					currentID = object.id;
				}
				else
				{
					unityObjects[currentID].content += line + '\n';
				}
			}

			std::map<std::string, uint32> stats;
			for (auto&& [id, object] : unityObjects)
			{
				if (stats.find(object.key) != stats.end())
					stats[object.key]++;
				else
					stats.insert(std::make_pair(object.key, 1));
			}

			//std::cout << "---------" << std::endl;
			//std::cout << "Statistic" << std::endl;
			//std::cout << "---------" << std::endl;
			//for (auto [key, num] : stats)
			//	std::cout << key << ": " << num << std::endl;
			//std::cout << "Total Unity objects: " << unityObjects.size() << std::endl;

			for (auto&& [id, object] : unityObjects)
			{
				ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(object.content));
				Object* obj = nullptr;
				switch (object.type)
				{
					case 1: obj = new GameObject(object.id, object.type, object.stripped); break;
					case 4: obj = new Transform(object.id, object.type, object.stripped); break;
					case 20: obj = new Camera(object.id, object.type); break;
					case 23: obj = new MeshRenderer(object.id, object.type, object.stripped); break;
					case 29: obj = new OcclusionCullingSettings(object.id, object.type); break;
					case 33: obj = new MeshFilter(object.id, object.type); break;
					case 64: obj = new MeshCollider(object.id, object.type); break;
					case 65: obj = new BoxCollider(object.id, object.type); break;
					case 81: obj = new AudioListener(object.id, object.type); break;
					case 82: obj = new AudioSource(object.id, object.type); break;
					case 95: obj = new Animator(object.id, object.type); break;
					case 104: obj = new RenderSettings(object.id, object.type); break;
					case 108: obj = new Light(object.id, object.type); break;
					case 114: obj = new MonoBehaviour(object.id, object.type); break;
					case 136: obj = new CapsuleCollider(object.id, object.type); break;
					case 143: obj = new CharacterController(object.id, object.type); break;
					case 157: obj = new LightmapSettings(object.id, object.type); break;
					case 196: obj = new NavMeshSettings(object.id, object.type); break;
					case 198: obj = new ParticleSystem(object.id, object.type); break;
					case 199: obj = new ParticleSystemRenderer(object.id, object.type); break;
					case 215: obj = new ReflectionProbe(object.id, object.type); break;
					case 220: obj = new LightProbeGroup(object.id, object.type); break;
					case 222: obj = new CanvasRenderer(object.id, object.type); break;
					case 223: obj = new Canvas(object.id, object.type); break;
					case 224: obj = new RectTransform(object.id, object.type); break;
					case 1001: 
					{ 
						if (object.key.compare("Prefab") == 0)
							obj = new Prefab(object.id, object.type); // root object in a prefab file
						else
							obj = new PrefabInstance(object.id, object.type); // prefab instance in a scene file
						break;
					}						
					case 850595691: obj = new LightingSettings(object.id, object.type); break;
					default:
					{
						std::cout << "error: component " << object.key << " not supported!" << std::endl;
					}
				}

				if (obj != nullptr)
				{
					tree[object.key.c_str()] >> *obj;
					objects.insert(std::make_pair(object.id, obj));
				}
			}

			return true;
		}
	}
}