#include "UnityImporter.h"


#include <Importer/ModelImporter.h>
#include <FileIO/AssimpImporter.h>
#include <fstream>
#include <sstream>

namespace Unity
{
	Importer::Importer() {

	}
	Importer::~Importer() {
		for (auto&& [guid, data] : metadataDB)
			delete data.importer;
		metadataDB.clear();
	}

	Material::Ptr Importer::loadMaterial(const std::string& guid)
	{
		if (metadataDB.find(guid) == metadataDB.end())
		{
			std::cout << "error: no material with GUID: " << guid << std::endl;
			return nullptr;
		}

		auto metaData = metadataDB[guid];

		std::cout << "loading material " << metaData.filepath << std::endl;

		Material::Ptr material = nullptr;
		if (metaData.extension.compare(".mat") == 0)
		{
			uint32 matIndex = 0;
			std::string content = YAML::loadTxtFile(metaData.filepath);
			ryml::Tree root = ryml::parse_in_arena(ryml::to_csubstr(content));
			for (int i = 0; i < root.rootref().num_children(); i++)
				if (root.docref(i).has_child("Material"))
					matIndex = i;
			auto stream = root.docref(matIndex);
			YAML::Object* object = new YAML::Material(0, 0);
			stream["Material"] >> *object;
			//((YAML::Material*)object)->print("");

			YAML::Material* yamlMat = (YAML::Material*)object;

			std::string shaderFile = "";
			std::string shaderGUID = yamlMat->m_Shader.value.guid;
			int64 shaderFileID = yamlMat->m_Shader.value.fileID;
			
			bool internal = shaderGUID.compare("0000000000000000f000000000000000") == 0;

			if (internal)
			{
				if (shaderFileID == 45)
					shaderFile = "UnitySpecGloss";
				else if (shaderFileID == 46)
					shaderFile = "UnityDefault";
			}
			else
			{
				if (metadataDB.find(shaderGUID) != metadataDB.end())
				{
					shaderFile = metadataDB[shaderGUID].filepath;
				}					
				else
				{
					if (shaderGUID.compare("933532a4fcc9baf4fa0491de14d08ed7") == 0 ||  // Lit
						shaderGUID.compare("0ca6dca7396eb48e5849247ffd444914") == 0)	// BakedLit
						shaderFile = "UnitySpecGloss";
					else
						std::cout << "error: shader with GUID: " << shaderGUID << " not found!" << std::endl;
				}
			}

			material = Material::create(yamlMat->m_Name, shaderFile, internal);
			if (!yamlMat->m_ShaderKeywords.value.empty())
			{
				std::string keywords = yamlMat->m_ShaderKeywords.value;
				std::stringstream ss(keywords);
				std::string kw;
				while (ss >> kw)
					material->addKeyword(kw);
			}
			
			for (auto floatProp : yamlMat->m_Floats.value)
				material->addProperty(floatProp.name, floatProp.value);
			for (auto colorProp : yamlMat->m_Colors.value)
				material->addProperty(colorProp.name, colorProp.value);
			for (auto texProp : yamlMat->m_TexEnvs.value)
			{
				int fileID = texProp.m_Texture.value.fileID;
				if (fileID > 0)
				{
					std::string name = texProp.name;
					std::string guid = texProp.m_Texture.value.guid;
					glm::vec2 offset = texProp.m_Offset;
					glm::vec2 scale = texProp.m_Scale;
					auto texProp = TextureProperty::create(name, guid, offset, scale);
					material->addTexture(name, texProp);
				}
			}				
		}

		return material;
	}

	void Importer::loadPrefab(const std::string& guid)
	{
		if (metadataDB.find(guid) == metadataDB.end())
		{
			std::cout << "error: could not find prefab with GUID: " << guid << std::endl;
			return;
		}

		std::cout << "loading prefab " << metadataDB[guid].filepath << std::endl;

		auto metaData = metadataDB[guid];
		auto prefabFn = metaData.filepath;

		//std::map<int64, YAML::Object*> prefabObjects;
		Database db;
		YAML::readSceneFile(prefabFn, db.unityObjects);

		int64 rootFileID = -1;
		for (auto [id, obj] : db.unityObjects)
		{
			if (obj->type == 1001)
			{
				auto prefab = (YAML::Prefab*)obj;
				rootFileID = prefab->m_RootGameObject.value.fileID;
			}
		}

		if (rootFileID < 0)
		{
			for (auto [id, obj] : db.unityObjects)
			{
				if (obj->type == 4)
				{
					auto transform = (YAML::Transform*)obj;
					if (transform->m_Father.value.fileID == 0) // this is a root node
						rootFileID = transform->m_GameObject.value.fileID;
				}
			}
		}

		PrefabeCache cache;
		cache.root = traverse(rootFileID, nullptr, db);
		cache.fileIDMap = db.fileIDMap;
		prefabCache.insert(std::make_pair(guid, cache));
	}

	void Importer::loadMesh(const std::string& guid)
	{
		if (metadataDB.find(guid) == metadataDB.end())
		{
			std::cout << "error: no mesh with GUID: " << guid << std::endl;
			return;
		}
		auto metaData = metadataDB[guid];

		//std::cout << "-----------------------------------" << std::endl;
		std::cout << "loading model " << metaData.filepath << std::endl;
		
		YAML::ModelImporter* modelImporter = (YAML::ModelImporter*)metaData.importer;
		YAML::Meshes meshes = modelImporter->meshes;
		float scale = meshes.globalScale;
		bool useFileUnits = meshes.useFileUnits;
		bool generateUVs = meshes.generateSecondaryUV;

		std::vector<Unity::Material::Ptr> materials;
		for (auto extObj : modelImporter->externalObjects.value)
		{
			std::string guid = extObj.reference.value.guid;
			auto mat = loadMaterial(guid);
			if (mat != nullptr)
				materials.push_back(mat);
		}

		FileIO::AssimpImporter importer;
		importer.setExternalMaterials(materials);
		auto rootGO = importer.importModel(metaData.filepath, scale, useFileUnits);
		auto rootTransform = rootGO->getComponent<Transform>();

		auto mf = rootGO->getComponent<Unity::MeshFilter>();
		std::string rootName = "";
		if (mf)
			rootName = mf->getMesh()->getName();

		auto fileIDMap = modelImporter->fileIDToRecycleName.value;
		for (auto&& [id, name] : fileIDMap)
		{
			if (name.compare("//RootNode") == 0 && !rootName.empty())
				fileIDMap[id] = rootName;
		}

		//std::cout << "-------------" << std::endl;
		//std::cout << "loaded meshes" << std::endl;
		//std::cout << "-------------" << std::endl;
		std::map<std::string, GameObject::Ptr> nameToGameObject;
		std::map<std::string, Mesh::Ptr> nameToMesh;
		std::map<std::string, MeshRenderer::Ptr> nameToMeshRenderer;
		std::map<std::string, MeshFilter::Ptr> nameToMeshFilter;
		std::map<std::string, Transform::Ptr> nameToTransform;

		for (auto go : rootTransform->getChildrenWithComponent<MeshFilter>())
		{
			auto t = go->getComponent<Transform>();
			std::string goName = go->getName();
			nameToGameObject.insert(std::make_pair(goName, go));
			nameToTransform.insert(std::make_pair(goName, t));
						
			auto mr = go->getComponent<MeshRenderer>();
			auto mf = go->getComponent<MeshFilter>();
			auto mesh = mf->getMesh();
			std::string meshName = mesh->getName();

			nameToMesh.insert(std::make_pair(meshName, mesh));
			nameToMeshRenderer.insert(std::make_pair(meshName, mr));
			nameToMeshFilter.insert(std::make_pair(meshName, mf));
		}

		//std::cout << "---------" << std::endl;
		//std::cout << "fileIDMap" << std::endl;
		//std::cout << "---------" << std::endl;

		std::map<std::string, int> meshNames;

		MeshCache cache;
		cache.root = rootGO;
		cache.fileIDMap = fileIDMap;
		cache.generateUVs = generateUVs;
		for (auto&& [id, name] : fileIDMap)
		{
			if (id / 100000 == 1)
			{
				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToGameObject.find(goName) != nameToGameObject.end())
					cache.fileIDToGameObject.insert(std::make_pair(id, nameToGameObject[goName]));
			}
			if (id / 100000 == 4)
			{
				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToTransform.find(goName) != nameToTransform.end())
					cache.fileIDToTransform.insert(std::make_pair(id, nameToTransform[goName]));
			}
			if (id / 100000 == 23)
			{
				if (nameToMeshRenderer.find(name) != nameToMeshRenderer.end())
					cache.fileIDToMeshRenderer.insert(std::make_pair(id, nameToMeshRenderer[name]));
			}
			if (id / 100000 == 33)
			{
				if (nameToMeshFilter.find(name) != nameToMeshFilter.end())
					cache.fileIDToMeshFilter.insert(std::make_pair(id, nameToMeshFilter[name]));
			}
			if (id / 100000 == 43)
			{
				std::string meshName = name;
				if (meshNames.find(name) != meshNames.end())
				{
					meshNames[name]++;
					meshName = name + " " + std::to_string(meshNames[name]);
				}
				else
				{
					meshNames.insert(std::make_pair(name, 0));
				}

				if (nameToMesh.find(meshName) != nameToMesh.end())
					cache.fileIDToMesh.insert(std::make_pair(id, nameToMesh[meshName]));
			}
		}

		//std::cout << "File ID Map" << std::endl;
		//for (auto&& [id, name] : fileIDMap)
		//	if (id / 100000 == 43)
		//		std::cout << id << " - " << name << std::endl;
		//std::cout << "File ID to Mesh" << std::endl;
		//for (auto&& [id, mesh] : cache.fileIDToMesh)
		//	std::cout << id << " - " << mesh->getName() << std::endl;
		//for (auto&& [name, mesh] : nameToMesh)
		//	std::cout << name << " - " << mesh->getName() << std::endl;


		meshCache.insert(std::make_pair(guid, cache));
	}

	Mesh::Ptr loadMeshFromTxt(std::string name, std::string fn)
	{
		std::ifstream file(fn);
		if (!file.is_open())
		{
			//std::cout << "no cache file for mesh " << fn << std::endl;
			return nullptr;
		}

		auto mesh = Mesh::create(name);
		std::string line;
		while (std::getline(file, line))
		{
			if (line[0] == 'm')
			{
				std::stringstream ss(line);
				std::string value;
				SubMeshInfo info;
				ss >> value >> info.baseVertex >> info.firstVertex >> info.vertexCount >> info.indexStart >> info.indexCount;
				mesh->subMeshes.push_back(info);
			}
			if (line[0] == 'v')
			{
				std::stringstream ss(line);
				std::string value;
				glm::vec3 position;
				glm::vec3 normal;
				glm::vec4 tangent;
				glm::vec2 texCoord0;
				glm::vec2 texCoord1;
				ss >> value;
				ss >> position.x >> position.y >> position.z;
				ss >> normal.x >> normal.y >> normal.z;
				ss >> tangent.x >> tangent.y >> tangent.z >> tangent.w;
				ss >> texCoord0.x >> texCoord0.y;
				ss >> texCoord1.x >> texCoord1.y;
				position.x = -position.x;
				normal.x = -normal.x;
				tangent.x = -tangent.x;
				texCoord0.y = 1.0f - texCoord0.y;
				texCoord1.y = 1.0f - texCoord1.y;
				mesh->vertices.push_back(position);
				mesh->normals.push_back(normal);
				mesh->tangents.push_back(tangent);
				mesh->uv1.push_back(texCoord0);
				mesh->uv2.push_back(texCoord1);
			}
			if (line[0] == 't')
			{
				std::stringstream ss(line);
				std::string value;
				uint32 index;
				ss >> value;
				while (ss >> index)
					mesh->triangles.push_back(index);
			}
		}

		for (int i = 0; i < mesh->triangles.size(); i += 3)
			std::swap(mesh->triangles[i], mesh->triangles[i + 2]);

		return mesh;
	}

	GameObject::Ptr Importer::instantiatePrefab(Database& db, int64 prefabID, const std::string& guid)
	{
		auto originalGO = prefabCache[guid].root;
		auto fileIDMap = prefabCache[guid].fileIDMap;
		//auto generateUVs = meshCache[guid].generateUVs;
		auto rootGO = copy(nullptr, originalGO);
		auto rootTransform = rootGO->getComponent<Transform>();

		std::map<std::string, GameObject::Ptr> nameToGameObject;
		std::map<std::string, Mesh::Ptr> nameToMesh;
		std::map<std::string, MeshRenderer::Ptr> nameToMeshRenderer;
		std::map<std::string, MeshFilter::Ptr> nameToMeshFilter;
		std::map<std::string, Transform::Ptr> nameToTransform;

		for (auto go : rootTransform->getChildrenWithComponent<Transform>())
		{
			auto t = go->getComponent<Transform>();
			std::string goName = go->getName();
			nameToGameObject.insert(std::make_pair(goName, go));
			nameToTransform.insert(std::make_pair(goName, t));

			if (go->getComponent<MeshRenderer>())
			{
				auto mr = go->getComponent<MeshRenderer>();
				auto mf = go->getComponent<MeshFilter>();
				auto mesh = mf->getMesh();

				if (mesh)
				{
					std::string meshName = mesh->getName();
					nameToMesh.insert(std::make_pair(goName, mesh));
					nameToMeshRenderer.insert(std::make_pair(goName, mr));
					nameToMeshFilter.insert(std::make_pair(goName, mf));
				}
			}
		}

		std::map<int64, GameObject::Ptr> fileIDToGameObject;
		std::map<int64, Mesh::Ptr> fileIDToMesh;
		std::map<int64, MeshRenderer::Ptr> fileIDToMeshRenderer;
		std::map<int64, MeshFilter::Ptr> fileIDToMeshFilter;
		std::map<int64, Transform::Ptr> fileIDToTransform;
		for (auto&& [id, name] : fileIDMap)
		{
			int64_t type = id / 100000;
			if (id > 10000000ll)
				type = id / 1000000000000000ll;

			if (type == 1)
			{
				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToGameObject.find(goName) != nameToGameObject.end())
					fileIDToGameObject.insert(std::make_pair(id, nameToGameObject[goName]));
			}
			if (type == 4)
			{
				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToTransform.find(goName) != nameToTransform.end())
					fileIDToTransform.insert(std::make_pair(id, nameToTransform[goName]));
			}
			if (type == 23)
			{
				if (nameToMeshRenderer.find(name) != nameToMeshRenderer.end())
					fileIDToMeshRenderer.insert(std::make_pair(id, nameToMeshRenderer[name]));
			}
			if (type == 33)
			{
				if (nameToMeshFilter.find(name) != nameToMeshFilter.end())
					fileIDToMeshFilter.insert(std::make_pair(id, nameToMeshFilter[name]));
			}
			if (type == 43)
			{
				if (nameToMesh.find(name) != nameToMesh.end())
					fileIDToMesh.insert(std::make_pair(id, nameToMesh[name]));
			}
		}

		//if (generateUVs) // TODO: unity can generate secondary UV maps on load but they are not stored in the scene...
		//{
		//	std::cout << "generated UVs from Unity not available!" << std::endl;
		//	auto rendEnts = modelCopy->getChildrenWithComponent<pr::Renderable>();
		//	for(auto e : rendEnts)
		//	{
		//		std::cout << "Entity " << e->getName() << std::endl;
		//		auto r = e->getComponent<pr::Renderable>();
		//		auto subMeshes = r->getMesh()->getSubMeshes();
		//		for (auto s : subMeshes)
		//			meshes.insert(s.primitive->getName());
		//	}
		//}
		//else
		{
			auto& plmd = lightData.prefabLightMapData;
			if (plmd.find(prefabID) != plmd.end())
			{
				auto prefabLM = plmd[prefabID];
				for (auto [id, mr] : fileIDToMeshRenderer)
				{
					if (prefabLM.find(id) != prefabLM.end())
					{
						auto ld = prefabLM[id];
						if (mr->getDiffuseMode() == 2)
						{
							mr->setLightMapIndex(ld.index);
							mr->setLightMapST(ld.offset, ld.scale);
						}
					}
				}
			}
			else
			{
				for (auto [id, r] : fileIDToMeshRenderer)
					r->setDiffuseMode(0);
			}
		}

		std::map<int64, std::map<std::string, std::string>> propMap;
		YAML::PrefabInstance* prefabInstance = (YAML::PrefabInstance*)db.unityObjects[prefabID];
		for (auto mod : prefabInstance->m_Modification.value.m_Modifications.value)
		{
			int64 id = mod.target.value.fileID;
			std::string key = mod.propertyPath;
			std::string value = mod.value;

			if (key.compare("m_ProbeAnchor") == 0)
				value = std::to_string(mod.objectReference.value.fileID);
			else if (value.empty())
				value = mod.objectReference.value.guid;

			if (propMap.find(id) == propMap.end())
				propMap.insert(std::make_pair(id, std::map<std::string, std::string>()));
			propMap[id].insert(std::make_pair(key, value));
		}

		for (auto&& [id, m] : propMap)
		{
			int type = (int)id / 100000;
			if (id > 10000000ll)
				type = id / 1000000000000000ll;

			switch (type)
			{
				case 1:
				{
					if (fileIDToGameObject.find(id) != fileIDToGameObject.end())
					{
						auto go = fileIDToGameObject[id];
						for (auto&& [k, v] : m)
						{
							if (k.compare("m_IsActive") == 0)
								go->setActive(std::stoi(v));
						}
					}
					break;
				}
				case 4:
				{
					if (fileIDToTransform.find(id) != fileIDToTransform.end())
					{
						auto t = fileIDToTransform[id];
						glm::vec3 localPosition = t->getLocalPosition();
						glm::quat localRotation = t->getLocalRotation();
						glm::vec3 localScale = t->getLocalScale();

						for (auto [k, v] : m)
						{
							if (k.compare("m_LocalPosition.x") == 0)
								localPosition.x = -std::stof(v);
							else if (k.compare("m_LocalPosition.y") == 0)
								localPosition.y = std::stof(v);
							else if (k.compare("m_LocalPosition.z") == 0)
								localPosition.z = std::stof(v);
							else if (k.compare("m_LocalRotation.x") == 0)
								localRotation.x = std::stof(v);
							else if (k.compare("m_LocalRotation.y") == 0)
								localRotation.y = -std::stof(v);
							else if (k.compare("m_LocalRotation.z") == 0)
								localRotation.z = -std::stof(v);
							else if (k.compare("m_LocalRotation.w") == 0)
								localRotation.w = std::stof(v);
							else if (k.compare("m_LocalScale.x") == 0)
								localScale.x = std::stof(v);
							else if (k.compare("m_LocalScale.y") == 0)
								localScale.y = std::stof(v);
							else if (k.compare("m_LocalScale.z") == 0)
								localScale.z = std::stof(v);
						}

						t->setLocalPosition(localPosition);
						t->setLocalRotation(localRotation);
						t->setLocalScale(localScale);
					}
					break;
				}
				case 23: // MeshRenderer
				{
					if (fileIDToMeshRenderer.find(id) != fileIDToMeshRenderer.end())
					{
						for (auto [k, v] : m)
						{
							if (k.length() > 22 && k.substr(0, 22).compare("m_Materials.Array.data") == 0)
							{
								int matIndex = std::stoi(k.substr(23, 1));

								// TODO: use instance fileID to reference material
								std::string matGUID = v;
								int64_t fileID = 2100000;
								if (materialCache.find(matGUID) == materialCache.end())
								{
									auto mat = loadMaterial(matGUID);
									MaterialCache matCache; // TODO: check if fileID is correct
									matCache.fileIDMap.insert(std::make_pair(fileID, mat));
									materialCache.insert(std::make_pair(matGUID, matCache));
								}

								auto matCache = materialCache[matGUID];
								if (matCache.fileIDMap.find(fileID) == matCache.fileIDMap.end())
								{
									auto mat = loadMaterial(matGUID);
									matCache.fileIDMap.insert(std::make_pair(fileID, mat));
								}

								auto material = materialCache[matGUID].fileIDMap[fileID];
								if (material)
								{
									auto mr = fileIDToMeshRenderer[id];
									mr->setMaterial(matIndex, material);
									//if (material->isTransparent())
									//	r->setType(pr::RenderType::Transparent);
								}
							}
							if (k.compare("m_Enabled") == 0)
							{
								bool enabled = std::stoi(v);
								auto mr = fileIDToMeshRenderer[id];
								mr->setEnabled(enabled);
							}
							else if (k.compare("m_ProbeAnchor") == 0)
							{
								int64_t transformID = std::stoll(v);
								if (transformID > 0)
								{
									YAML::Transform* probeTransform = (YAML::Transform*)db.unityObjects[transformID];
									YAML::GameObject* probeGO = (YAML::GameObject*)db.unityObjects[probeTransform->m_GameObject.value.fileID];
									auto name = probeGO->m_Name.value;

									//std::cout << "Reflection probe : " << transformID << " name : " << name << std::endl;

									auto r = fileIDToMeshRenderer[id];
									r->setReflectionProbe(name, -1);
								}
							}
							else if (k.compare("m_ReceiveGI") == 0)
							{
								int receiveGI = std::stoi(v);
								if (receiveGI == 2)
								{
									auto mr = fileIDToMeshRenderer[id];
									if (m.find("m_LightProbeUsage") != m.end())
									{
										int probeUsage = std::stoi(m["m_LightProbeUsage"]);
										if (probeUsage == 0)
											mr->setDiffuseMode(2);
										else
											mr->setDiffuseMode(0);
									}
									else
										mr->setDiffuseMode(0);
								}
							}
							else if (k.compare("m_LightProbeUsage") == 0)
							{
								int probeUsage = std::stoi(v);
								if (probeUsage > 0)
								{
									auto mr = fileIDToMeshRenderer[id];
									mr->setDiffuseMode(0);
								}
							}
						}
					}
				}
			}
		}

		return rootGO;
	}

	GameObject::Ptr Importer::loadPrefabInstance(Database& db, int64 prefabID, const std::string& guid)
	{
		if (meshCache.find(guid) == meshCache.end())
			loadMesh(guid);

		auto originalGO = meshCache[guid].root;
		auto fileIDMap = meshCache[guid].fileIDMap;
		auto generateUVs = meshCache[guid].generateUVs;
		auto rootGO = copy(nullptr, originalGO);
		auto rootTransform = rootGO->getComponent<Transform>();

		if (generateUVs)
		{
			for (auto gameObject : rootTransform->getChildrenWithComponent<MeshRenderer>())
			{
				auto mr = gameObject->getComponent<MeshRenderer>();
				auto mf = gameObject->getComponent<MeshFilter>();
				if (!mr->isEnabled())
					continue;
				
				// TODO: replace unity mesh with mesh from text file
				auto mesh = mf->getMesh();
				auto name = mesh->getName();
				//std::cout << "laoding uv1 mesh: " << name << std::endl;
				auto newMesh = loadMeshFromTxt(name, lightmapUVPath + "/" + name + ".txt");
				mf->setMesh(newMesh);
			}
		}

		std::map<std::string, GameObject::Ptr> nameToGameObject;
		std::map<std::string, Mesh::Ptr> nameToMesh;
		std::map<std::string, MeshRenderer::Ptr> nameToMeshRenderer;
		std::map<std::string, MeshFilter::Ptr> nameToMeshFilter;
		std::map<std::string, Transform::Ptr> nameToTransform;

		for (auto go : rootTransform->getChildrenWithComponent<Transform>())
		{
			auto t = go->getComponent<Transform>();
			std::string goName = go->getName();
			nameToGameObject.insert(std::make_pair(goName, go));
			nameToTransform.insert(std::make_pair(goName, t));

			if (go->getComponent<MeshRenderer>())
			{
				auto mr = go->getComponent<MeshRenderer>();
				auto mf = go->getComponent<MeshFilter>();
				auto mesh = mf->getMesh();

				if (mesh)
				{
					std::string meshName = mesh->getName();
					nameToMesh.insert(std::make_pair(goName, mesh));
					nameToMeshRenderer.insert(std::make_pair(goName, mr));
					nameToMeshFilter.insert(std::make_pair(goName, mf));
				}
			}
		}

		std::map<int64, GameObject::Ptr> fileIDToGameObject;
		std::map<int64, Mesh::Ptr> fileIDToMesh;
		std::map<int64, MeshRenderer::Ptr> fileIDToMeshRenderer;
		std::map<int64, MeshFilter::Ptr> fileIDToMeshFilter;
		std::map<int64, Transform::Ptr> fileIDToTransform;
		for (auto&& [id, name] : fileIDMap)
		{
			if (id / 100000 == 1)
			{
				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToGameObject.find(goName) != nameToGameObject.end())
					fileIDToGameObject.insert(std::make_pair(id, nameToGameObject[goName]));
			}
			if (id / 100000 == 4)
			{
 				std::string goName = name;
				if (name.compare("//RootNode") == 0)
					goName = "RootNode";
				if (nameToTransform.find(goName) != nameToTransform.end())
					fileIDToTransform.insert(std::make_pair(id, nameToTransform[goName]));
			}
			if (id / 100000 == 23)
			{
				if (nameToMeshRenderer.find(name) != nameToMeshRenderer.end())
					fileIDToMeshRenderer.insert(std::make_pair(id, nameToMeshRenderer[name]));
			}
			if (id / 100000 == 33)
			{
				if (nameToMeshFilter.find(name) != nameToMeshFilter.end())
					fileIDToMeshFilter.insert(std::make_pair(id, nameToMeshFilter[name]));
			}
			if (id / 100000 == 43)
			{
				if (nameToMesh.find(name) != nameToMesh.end())
					fileIDToMesh.insert(std::make_pair(id, nameToMesh[name]));
			}
		}

		//if (generateUVs) // TODO: unity can generate secondary UV maps on load but they are not stored in the scene...
		//{
		//	std::cout << "generated UVs from Unity not available!" << std::endl;
		//	auto rendEnts = modelCopy->getChildrenWithComponent<pr::Renderable>();
		//	for(auto e : rendEnts)
		//	{
		//		std::cout << "Entity " << e->getName() << std::endl;
		//		auto r = e->getComponent<pr::Renderable>();
		//		auto subMeshes = r->getMesh()->getSubMeshes();
		//		for (auto s : subMeshes)
		//			meshes.insert(s.primitive->getName());
		//	}
		//}
		//else
		{
			auto& plmd = lightData.prefabLightMapData;
			if (plmd.find(prefabID) != plmd.end())
			{
				auto prefabLM = plmd[prefabID];
				for (auto [id, mr] : fileIDToMeshRenderer)
				{
					if (prefabLM.find(id) != prefabLM.end())
					{
						auto ld = prefabLM[id];
						mr->setDiffuseMode(2);
						mr->setLightMapIndex(ld.index);
						mr->setLightMapST(ld.offset, ld.scale);
					}
				}
			}
		}

		std::map<int64, std::map<std::string, std::string>> propMap;
		YAML::PrefabInstance* prefabInstance = (YAML::PrefabInstance*)db.unityObjects[prefabID];
		for (auto mod : prefabInstance->m_Modification.value.m_Modifications.value)
		{
			int64 id = mod.target.value.fileID;
			std::string key = mod.propertyPath;
			std::string value = mod.value;

			if (key.compare("m_ProbeAnchor") == 0)
				value = std::to_string(mod.objectReference.value.fileID);
			else if (value.empty())
				value = mod.objectReference.value.guid;

			if (propMap.find(id) == propMap.end())
				propMap.insert(std::make_pair(id, std::map<std::string, std::string>()));
			propMap[id].insert(std::make_pair(key, value));
		}

		for (auto&& [id, m] : propMap)
		{
			int type = (int)id / 100000;
			switch (type)
			{
				case 1: 
				{
					if (fileIDToGameObject.find(id) != fileIDToGameObject.end())
					{
						auto go = fileIDToGameObject[id];
						for (auto&& [k, v] : m)
						{
							if (k.compare("m_IsActive") == 0)
								go->setActive(std::stoi(v));
						}
					}
					break;
				}
				case 4:
				{
					if (fileIDToTransform.find(id) != fileIDToTransform.end())
					{
						auto t = fileIDToTransform[id];
						glm::vec3 localPosition = t->getLocalPosition();
						glm::quat localRotation = t->getLocalRotation();
						glm::vec3 localScale = t->getLocalScale();

						for (auto [k, v] : m)
						{
							if (k.compare("m_LocalPosition.x") == 0)
								localPosition.x = -std::stof(v);
							else if (k.compare("m_LocalPosition.y") == 0)
								localPosition.y = std::stof(v);
							else if (k.compare("m_LocalPosition.z") == 0)
								localPosition.z = std::stof(v);
							else if (k.compare("m_LocalRotation.x") == 0)
								localRotation.x = std::stof(v);
							else if (k.compare("m_LocalRotation.y") == 0)
								localRotation.y = -std::stof(v);
							else if (k.compare("m_LocalRotation.z") == 0)
								localRotation.z = -std::stof(v);
							else if (k.compare("m_LocalRotation.w") == 0)
								localRotation.w = std::stof(v);
							else if (k.compare("m_LocalScale.x") == 0)
								localScale.x = std::stof(v);
							else if (k.compare("m_LocalScale.y") == 0)
								localScale.y = std::stof(v);
							else if (k.compare("m_LocalScale.z") == 0)
								localScale.z = std::stof(v);
						}

						t->setLocalPosition(localPosition);
						t->setLocalRotation(localRotation);
						t->setLocalScale(localScale);
					}
					break;
				}
				case 23: // MeshRenderer
				{
					if (fileIDToMeshRenderer.find(id) != fileIDToMeshRenderer.end())
					{
						for (auto [k, v] : m)
						{
							if (k.length() > 22 && k.substr(0, 22).compare("m_Materials.Array.data") == 0)
							{
								int matIndex = std::stoi(k.substr(23, 1));

								// TODO: use instance fileID to reference material
								std::string matGUID = v;
								int64_t fileID = 2100000;
								if (materialCache.find(matGUID) == materialCache.end())
								{
									auto mat = loadMaterial(matGUID);
									MaterialCache matCache; // TODO: check if fileID is correct
									matCache.fileIDMap.insert(std::make_pair(fileID, mat));
									materialCache.insert(std::make_pair(matGUID, matCache));
								}

								auto matCache = materialCache[matGUID];
								if (matCache.fileIDMap.find(fileID) == matCache.fileIDMap.end())
								{
									auto mat = loadMaterial(matGUID);
									matCache.fileIDMap.insert(std::make_pair(fileID, mat));
								}

								auto material = materialCache[matGUID].fileIDMap[fileID];
								if (material)
								{
									auto mr = fileIDToMeshRenderer[id];
									mr->setMaterial(matIndex, material);
									//if (material->isTransparent())
									//	r->setType(pr::RenderType::Transparent);
								}
							}
							if (k.compare("m_Enabled") == 0)
							{
								bool enabled = std::stoi(v);
								auto mr = fileIDToMeshRenderer[id];
								mr->setEnabled(enabled);
							}
							else if (k.compare("m_ProbeAnchor") == 0)
							{
								int64_t transformID = std::stoll(v);
								if (transformID > 0)
								{
									YAML::Transform* probeTransform = (YAML::Transform*)db.unityObjects[transformID];
									YAML::GameObject* probeGO = (YAML::GameObject*)db.unityObjects[probeTransform->m_GameObject.value.fileID];
									auto name = probeGO->m_Name.value;

									//std::cout << "Reflection probe : " << transformID << " name : " << name << std::endl;

									auto r = fileIDToMeshRenderer[id];
									r->setReflectionProbe(name, -1);
								}
							}
							else if (k.compare("m_ReceiveGI") == 0)
							{
								int receiveGI = std::stoi(v);
								if (receiveGI == 2)
								{
									auto mr = fileIDToMeshRenderer[id];
									if (m.find("m_LightProbeUsage") != m.end())
									{
										int probeUsage = std::stoi(m["m_LightProbeUsage"]);
										if (probeUsage == 0)
											mr->setDiffuseMode(2);
										else
											mr->setDiffuseMode(0);
									}
									else
										mr->setDiffuseMode(0);
								}
							}
						}
					}
				}
			}
		}

		return rootGO;
	}

	GameObject::Ptr Importer::traverse(int64 fileID, GameObject::Ptr parent, Database& db)
	{
		YAML::GameObject* yamlGO = (YAML::GameObject*)db.unityObjects[fileID];

		auto gameObject = GameObject::create(yamlGO->m_Name);
		gameObject->setActive(yamlGO->m_IsActive.value);
		
		std::vector<YAML::FileID> children;
		for (auto& component : yamlGO->m_Component.value)
		{
			if (db.unityObjects.find(component.fileID) == db.unityObjects.end())
			{
				std::cout << "error: component with fileID: " << component.fileID << " not found!" << std::endl;
				continue;
			}

			YAML::Object* object = db.unityObjects[component.fileID];
			switch (object->type) 
			{
				case 4: // Transform
				{
					auto yamlTransform = (YAML::Transform*)object;
					auto transform = Transform::create();
					transform->setGameObject(gameObject);
					if (parent)
						transform->setParent(parent->getComponent<Transform>());
					else
						transform->setParent(nullptr);

					glm::vec3 pos = yamlTransform->m_LocalPosition;
					glm::quat rot = yamlTransform->m_LocalRotation;
					glm::vec3 scale = yamlTransform->m_LocalScale;

					pos.x = -pos.x;
					rot.y = -rot.y;
					rot.z = -rot.z;

					transform->setLocalPosition(pos);
					transform->setLocalRotation(rot);
					transform->setLocalScale(scale);
					gameObject->addComponent(transform);
					children = yamlTransform->m_Children;

					db.fileIDMap.insert(std::make_pair(fileID, gameObject->getName()));
					db.fileIDMap.insert(std::make_pair(component.fileID, gameObject->getName()));

					break;
				}
				case 23: // MeshRenderer
				{
					auto meshRenderer = MeshRenderer::create();
					auto yamlMeshRenderer = (YAML::MeshRenderer*)object;
					meshRenderer->setEnabled(yamlMeshRenderer->m_Enabled.value);
					for (auto& yamlMat : yamlMeshRenderer->m_Materials.value)
					{
						bool cacheHit = true;
						if (materialCache.find(yamlMat.guid) == materialCache.end())
						{
							auto mat = loadMaterial(yamlMat.guid);
							MaterialCache matCache;
							matCache.fileIDMap.insert(std::make_pair(yamlMat.fileID, mat));
							materialCache.insert(std::make_pair(yamlMat.guid, matCache));
							cacheHit = false;
						}

						auto& fileIDMap = materialCache[yamlMat.guid].fileIDMap;
						if (fileIDMap.find(yamlMat.fileID) == fileIDMap.end())
						{
							auto mat = loadMaterial(yamlMat.guid);
							fileIDMap.insert(std::make_pair(yamlMat.fileID, mat));
							cacheHit = false;
						}

						auto material = materialCache[yamlMat.guid].fileIDMap[yamlMat.fileID];
						meshRenderer->addMaterial(material);
					}

					auto& olmd = lightData.objectLightMapData;
					if (olmd.find(component.fileID) != olmd.end())
					{
						auto ld = olmd[component.fileID];
						meshRenderer->setDiffuseMode(2);
						meshRenderer->setLightMapIndex(ld.index);
						meshRenderer->setLightMapST(ld.offset, ld.scale);
					}

					if (yamlMeshRenderer->m_ReceiveGI.value  == 1)
						meshRenderer->setDiffuseMode(2);

					gameObject->addComponent(meshRenderer);

					db.fileIDMap.insert(std::make_pair(component.fileID, gameObject->getName()));

					break;
				}
				case 33: // MeshFilter
				{
					auto yamlMeshFilter = (YAML::MeshFilter*)object;
					int64 fileID = yamlMeshFilter->m_Mesh.value.fileID;
					std::string guid = yamlMeshFilter->m_Mesh.value.guid;
					
					if (meshCache.find(guid) == meshCache.end())
						loadMesh(guid);

					if (meshCache.find(guid) != meshCache.end())
					{
						bool generateUVs = meshCache[guid].generateUVs;
						auto mesh = meshCache[guid].fileIDToMesh[fileID];
						if (mesh && generateUVs)
						{
							auto name = mesh->getName();
							//std::cout << "loading uv1 mesh: " << name << std::endl;
							auto newMesh = loadMeshFromTxt(name, lightmapUVPath + "/" + name + ".txt");
							auto meshFilter = Unity::MeshFilter::create(newMesh);
							gameObject->addComponent(meshFilter);
						}
						else
						{
							auto meshFilter = Unity::MeshFilter::create(mesh);
							gameObject->addComponent(meshFilter);
						}
					}

					db.fileIDMap.insert(std::make_pair(component.fileID, gameObject->getName()));
					db.fileIDMap.insert(std::make_pair(fileID, gameObject->getName()));

					break;
				}				
				case 108: // Light
				{
					auto yamlLight = (YAML::Light*)object;
					auto light = Unity::Light::create();
					light->fromYAML(yamlLight);
					gameObject->addComponent(light);
					break;
				}
				case 215: // ReflectionProbe
				{
					auto yamlReflectionProbe = (YAML::ReflectionProbe*)object;
					auto reflectionProbe = Unity::ReflectionProbe::create();
					reflectionProbe->setBoxSize(yamlReflectionProbe->m_BoxSize.value);
					reflectionProbe->setBoxOffset(yamlReflectionProbe->m_BoxOffset.value);

					if (lightData.reflectionProbes.find(component.fileID) != lightData.reflectionProbes.end())
					{
						auto guid = lightData.reflectionProbes[component.fileID];
						reflectionProbe->setSourceFile(metadataDB[guid].filepath);
						gameObject->addComponent(reflectionProbe);
					}
					else
					{
						std::cout << "error: could not find baked reflection probe with ID: " << component.fileID << std::endl;
					}
					
					break;
				}				
				default:
				{
					//std::cout << "error: component type " << object->type << " not implemented!" << std::endl;
					break;
				}
			}
		}

		for (auto child : children)
		{
			YAML::Object* object = db.unityObjects[child.fileID];
			if (object->type == 4)
			{
				auto yamlTransform = (YAML::Transform*)db.unityObjects[child.fileID];
				if (yamlTransform->stripped)
				{
					auto guid = yamlTransform->m_CorrespondingSourceObject.value.guid;
					if (metadataDB.find(guid) != metadataDB.end())
					{
						auto ext = metadataDB[guid].extension;
						if (ext.compare(".prefab") == 0)
						{
							if (prefabCache.find(guid) == prefabCache.end())
								loadPrefab(guid);

							auto transform = gameObject->getComponent<Transform>();
							int64 prefabID = yamlTransform->m_PrefabInstance.value.fileID;

							auto childGO = instantiatePrefab(db, prefabID, guid);
							auto childTransform = childGO->getComponent<Transform>();

							transform->addChild(childTransform);
							//std::cout << "error: loading from prefab filed not implemented! (file: " << metadataDB[guid].filename << ")" << std::endl;
						}
						else if (ext.compare(".fbx") == 0 || ext.compare(".FBX") == 0)
						{
							auto transform = gameObject->getComponent<Transform>();
							int64 prefabID = yamlTransform->m_PrefabInstance.value.fileID;

							auto childGO = loadPrefabInstance(db, prefabID, guid);
							auto childTransform = childGO->getComponent<Transform>();

							for (auto [id, obj] : db.unityObjects)
							{
								if (obj->type == 4)
								{
									YAML::Transform* yamlChildTransform = (YAML::Transform*)obj;
									if (child.fileID == yamlChildTransform->m_Father.value.fileID)
									{
										int64 grandChildID = yamlChildTransform->m_GameObject.value.fileID;
										auto grandChild = traverse(grandChildID, childGO, db);
										childTransform->addChild(grandChild->getComponent<Transform>());
									}
								}
							}

							transform->addChild(childTransform);
						}
						else
						{
							std::cout << "error: not supported external file extension: " << ext << std::endl;
						}
					}						
					else
						std::cout << "error: GUID " << guid << " does not exist in asset folder!" << std::endl;

					// TODO: load GameObject from external reference (Prefab, FBX...)
				}
				else
				{
					int64 childID = yamlTransform->m_GameObject.value.fileID;
					auto childGO = traverse(childID, gameObject, db);
					auto transform = gameObject->getComponent<Transform>();
					transform->addChild(childGO->getComponent<Transform>());
				}
			}
			else
			{
				std::cout << "RectTransform not implemented!" << std::endl;
			}
		}

		return gameObject;
	}

	GameObject::Ptr Importer::copy(GameObject::Ptr parentGO, GameObject::Ptr srcGO)
	{
		auto dstGO = GameObject::create(srcGO->getName());
		dstGO->setActive(srcGO->isActive());
		auto dstTransform = dstGO->addComponent<Transform>();
		auto srcTransform = srcGO->getComponent<Transform>();
		Transform::Ptr parentTransform = nullptr;
		if (parentGO)
			parentTransform = parentGO->getComponent<Transform>();
		dstTransform->setParent(parentTransform);
		dstTransform->setGameObject(dstGO);
		dstTransform->setLocalPosition(srcTransform->getLocalPosition());
		dstTransform->setLocalRotation(srcTransform->getLocalRotation());
		dstTransform->setLocalScale(srcTransform->getLocalScale());

		if (srcGO->getComponent<MeshRenderer>())
		{
			auto srcMR = srcGO->getComponent<MeshRenderer>();
			auto dstMR = MeshRenderer::create();
			dstMR->setEnabled(srcMR->isEnabled());
			dstMR->setDiffuseMode(srcMR->getDiffuseMode());
			dstMR->setLightMapIndex(srcMR->getLMIndex());
			dstMR->setLightMapST(srcMR->getLMOffset(), srcMR->getLMScale());
			for (int i = 0; i < srcMR->getNumMaterials(); i++)
				dstMR->addMaterial(srcMR->getMaterial(i));
			dstGO->addComponent(dstMR);
		}

		if (srcGO->getComponent<MeshFilter>())
		{
			auto srcMF = srcGO->getComponent<MeshFilter>();
			auto dstMF = MeshFilter::create(srcMF->getMesh());
			dstGO->addComponent(dstMF);
		}

		for (int i = 0; i < srcTransform->numChildren(); i++)
		{
			auto srcChild = srcTransform->getChild(i)->getGameObject();
			auto dstChild = copy(dstGO, srcChild);
			dstTransform->addChild(dstChild->getComponent<Transform>());
		}

		return dstGO;
	}

	Scene::Ptr Importer::importScene(const std::string& assetPath, const std::string& sceneFile)
	{
		int len = (int)assetPath.find_last_of('/');
		std::string prefix = assetPath.substr(0, len);
		this->lightmapUVPath = prefix + "/lightmap_uv";

		//std::map<int64, YAML::Object*> unityObjects;

		Database db;
		std::string sceneFullpath = assetPath + "/" + sceneFile;
		YAML::loadMetadata(assetPath, metadataDB);
		YAML::readSceneFile(sceneFullpath, db.unityObjects);

		Rendersettings renderSettings;
		std::map<std::string, int64> rootNodes;
		for (auto&& [id, obj] : db.unityObjects)
		{
			if (obj->getType() == 4) // Transform
			{
				YAML::Transform* t = (YAML::Transform*)obj;
				if (t->stripped) // stripped versions have no father since they are defined from a prefab instance
					continue;

				int64 fileID = t->m_Father.value.fileID;
				if (fileID == 0) // we have a root node
				{
					int64 goFileID = t->m_GameObject.value.fileID;
					YAML::GameObject* go = (YAML::GameObject*)db.unityObjects[goFileID];
					std::string name = go->m_Name.value;
					rootNodes.insert(std::make_pair(name, goFileID));
				}
			}
			if (obj->getType() == 104) // RenderSettings
			{
				YAML::RenderSettings* rs = (YAML::RenderSettings*)obj;
				auto skyboxMat = loadMaterial(rs->m_SkyboxMaterial.value.guid);
				renderSettings.setSkyboxMaterial(skyboxMat);
			}
		}

		for (auto&& [name, id] : rootNodes)
			std::cout << id << " " << name << std::endl;

		std::string lightAssetFn = assetPath + "/Viking Village/Scenes/The_Viking_Village/LightingData.asset.txt";
		lightData.loadAsset(lightAssetFn);

		auto scene = Scene::create(renderSettings);
		//scene->addRoot(traverse(1308100271, nullptr));
		//scene->addRoot(traverse(810077401, nullptr));
		 
		//scene->addRoot(traverse(2075706406, nullptr));
		//scene->addRoot(traverse(2103635834, nullptr));
		//scene->addRoot(traverse(380792123, nullptr));

		//scene->addRoot(traverse(1104400804, nullptr, db));
		
		for (auto&& [name, id] : rootNodes)
		{
			auto rootGO = traverse(id, nullptr, db);
			scene->addRoot(rootGO);
		}

		// TODO: parse scene hierarchy

		//for (auto&& [guid, meta] : metadata)
		//{
		//	if (meta.extension.compare(".mat") == 0)
		//	{
		//		//std::cout << guid << " : " << meta.filename << std::endl;

		//		uint32 matIndex = 0;
		//		std::string content = YAML::loadTxtFile(meta.filepath);
		//		ryml::Tree root = ryml::parse_in_arena(ryml::to_csubstr(content));
		//		for (int i = 0; i < root.rootref().num_children(); i++)
		//			if (root.docref(i).has_child("Material"))
		//				matIndex = i;

		//		auto stream = root.docref(matIndex);
		//		
		//		YAML::Object* object = new YAML::Material(0, 0);
		//		stream["Material"] >> *object;
		//	}
		//		
		//	//std::cout << guid << " : " << meta.filepath << std::endl;
		//}			

		//for (auto&& [id, obj] : unityObjects)
		//	std::cout << obj->getID() << " " << obj->getType() << std::endl;

		for (auto&& [id, obj] : db.unityObjects)
			delete obj;
		db.unityObjects.clear();

		return scene;
	}

	void Importer::loadMetadata(const std::string& assetPath)
	{
		YAML::loadMetadata(assetPath, metadataDB);

		int len = (int)assetPath.find_last_of('/');
		std::string prefix = assetPath.substr(0, len);
		this->lightmapUVPath = prefix + "/lightmap_uv";
	}

	GameObject::Ptr Importer::importPrefab(const std::string& filepath)
	{
		//if (metadataDB.find(guid) == metadataDB.end())
		//{
		//	std::cout << "error: could not find prefab with GUID: " << guid << std::endl;
		//	return;
		//}

		std::cout << "loading prefab " << filepath << std::endl;

		//auto metaData = metadataDB[guid];
		//auto prefabFn = metaData.filepath;

		//std::map<int64, YAML::Object*> prefabObjects;
		Database db;
		YAML::readSceneFile(filepath, db.unityObjects);

		int64 rootFileID = -1;
		for (auto [id, obj] : db.unityObjects)
		{
			if (obj->type == 1001)
			{
				auto prefab = (YAML::Prefab*)obj;
				rootFileID = prefab->m_RootGameObject.value.fileID;
			}
		}

		if (rootFileID < 0)
		{
			for (auto [id, obj] : db.unityObjects)
			{
				if (obj->type == 4)
				{
					auto transform = (YAML::Transform*)obj;
					if (transform->m_Father.value.fileID == 0) // this is a root node
						rootFileID = transform->m_GameObject.value.fileID;
				}
			}
		}

		auto prefab = traverse(rootFileID, nullptr, db);
		return prefab;
	}

	YAML::Metadata Importer::getMetadata(std::string guid)
	{
		return metadataDB[guid];
	}

	bool Importer::hasMetadata(std::string guid)
	{
		return metadataDB.find(guid) != metadataDB.end();
	}
}