#include "UnityImporter.h"
#include "UnityYAML.h"

#include <Graphics/MeshPrimitives.h>

#include <IO/AssimpImporter.h>
#include <IO/ShaderLoader.h>
#include <IO/Image/ImageDecoder.h>
#include <IO/Image/Cubemap.h>
#include <Utils/Color.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace IO
{
	namespace Unity
	{
		Importer::Importer(unsigned int maxTexSize) :
			mats(maxTexSize)
		{


		}

		void Importer::loadMetadata(const std::string& assetPath)
		{
			for (const auto& entry : fs::recursive_directory_iterator(assetPath))
			{
				if (entry.is_regular_file())
				{
					auto p = entry.path();
					auto metaPath = p.string();
					auto extension = p.extension().string();

					if (extension.compare(".meta") == 0)
					{
						std::replace(metaPath.begin(), metaPath.end(), '\\', '/');
						std::string filePath = metaPath.substr(0, metaPath.length() - 5); // remove .meta
						std::string content = IO::loadTxtFile(metaPath);
						ryml::Tree root = ryml::parse(ryml::to_csubstr(content));
						auto stream = root.rootref();

						std::string guid;
						stream["guid"] >> guid;

						Metadata data;
						data.filePath = filePath;
						data.root = root;

						metaData.insert(std::pair(guid, data));

						//std::cout << guid << " : " << data.filePath << std::endl;
					}
				}
			}

			mats.setMetadata(metaData);
		}

		void Importer::clearCache()
		{
			for (auto [_, cache] : meshCache)
				cache.root->clearParent();
			meshCache.clear();
		}

		Material::Ptr Importer::loadMaterialGUID(const std::string& guid)
		{
			if (meshCache.find(guid) != meshCache.end())
			{
				std::cout << "warning: material must be loaded form source file! (mesh: " << guid << ")" << std::endl;
				return nullptr;
			}

			// TODO: find a better way to integrate custom materials... 
			//		 maybe parsing the .shader + .mat file to create a custom
			//		 surface shader function that can be plugged into the default shader
			return mats.loadMaterialGUID(guid);
		}

		void Importer::loadMeshGUID(const std::string& guid)
		{
			if (metaData.find(guid) == metaData.end())
			{
				std::cout << "error: could not find mesh with GUID: " << guid << std::endl;
				return;
			}

			Metadata& data = metaData[guid];
			std::cout << "loading model: " << data.filePath << std::endl;

			auto stream = data.root.rootref();
			float scale = 1.0f;
			bool useFileUnits = false;
			bool generateUVs = false;
			std::vector<Material::Ptr> materials;
			if (stream.has_child("ModelImporter"))
			{
				auto& meshNode = stream["ModelImporter"]["meshes"];
				meshNode["globalScale"] >> scale;
				meshNode["useFileUnits"] >> useFileUnits;
				meshNode["generateSecondaryUV"] >> generateUVs;
				if (stream["ModelImporter"].has_child("externalObjects"))
				{
					auto& extObjectsNode = stream["ModelImporter"]["externalObjects"];
					for (auto& entry : extObjectsNode)
					{
						int64_t fileID;
						std::string name, guid;
						
						entry["first"]["name"] >> name;
						entry["second"]["fileID"] >> fileID;
						entry["second"]["guid"] >> guid;

						auto mat = loadMaterialGUID(guid);
						if(mat != nullptr)
							materials.push_back(mat);
					}
				}
			}

			IO::AssimpImporter modelImporter;
			modelImporter.setExternalMaterials(materials);
			auto model = modelImporter.importModel(data.filePath, scale, useFileUnits);

			auto r = model->getComponent<Renderable>();
			std::string rootName = "";
			if (r)
			{
				auto m = r->getMesh()->getSubMeshes();
				if (m.size() > 0)
					rootName = m[0].primitive->getName();
			}

			std::map<int64_t, std::string> fileIDToName;
			if (stream.has_child("ModelImporter"))
			{
				auto& modelNode = stream["ModelImporter"];
				if (modelNode.has_child("fileIDToRecycleName"))
				{
					stream["ModelImporter"]["fileIDToRecycleName"] >> fileIDToName;
					for (auto&& [id, name] : fileIDToName)
					{
						if (name.compare("//RootNode") == 0 && !rootName.empty())
							fileIDToName[id] = rootName;
					}
				}
				else if (modelNode.has_child("internalIDToNameTable"))
				{
					auto& nameTable = modelNode["internalIDToNameTable"];
					for (auto& nameEntry : nameTable)
					{
						int64_t id;
						std::string name;
						auto& idNode = nameEntry["first"].child(0);
						idNode >> id;
						nameEntry["second"] >> name;

						if (name.compare("//RootNode") == 0 && !rootName.empty())
							fileIDToName[id] = rootName;
						else
							fileIDToName.insert(std::make_pair(id, name));
					}
				}
			}

			MeshCache cache;
			cache.root = model;
			cache.fileIDMap = fileIDToName;
			cache.generateUVs = generateUVs;
			meshCache.insert(std::make_pair(guid, cache));
		}

		Mesh::Ptr loadMeshFromTxt(std::string fn)
		{
			std::ifstream file(fn);
			if (!file.is_open())
			{
				//std::cout << "no cache file for mesh " << fn << std::endl;
				return nullptr;
			}

			struct SubMeshInfo
			{
				int baseVertex;
				int firstVertex;
				int vertexCount;
				int indexStart;
				int indexCount;
			};
				
			std::string line;
			std::vector<SubMeshInfo> subMeshInfo;
			std::vector<Vertex> vertices;
			std::vector<GLuint> indices;
			int meshIndex = 0;
			while (std::getline(file, line))
			{
				if (line[0] == 'm')
				{
					std::stringstream ss(line);
					std::string value;
					SubMeshInfo info;

					ss >> value >> info.baseVertex >> info.firstVertex >> info.vertexCount >> info.indexStart >> info.indexCount;
					subMeshInfo.push_back(info);
				}
				if (line[0] == 'v')
				{
					std::stringstream ss(line);
					std::string value;
					Vertex v;
					ss >> value;
					ss >> v.position.x >> v.position.y >> v.position.z;
					ss >> v.normal.x >> v.normal.y >> v.normal.z;
					ss >> v.tangent.x >> v.tangent.y >> v.tangent.z >> v.tangent.w;
					ss >> v.texCoord0.x >> v.texCoord0.y;
					ss >> v.texCoord1.x >> v.texCoord1.y;
					v.position.x = -v.position.x; // WHY??????? :D
					v.normal.x = -v.normal.x; // WHY??????? :D
					v.tangent.x = -v.tangent.x; // WHY??????? :D
					//v.tangent.w = 1.0;
					v.texCoord0.y = 1.0 - v.texCoord0.y;
					v.texCoord1.y = 1.0 - v.texCoord1.y;
					vertices.push_back(v);
				}
				if (line[0] == 't')
				{
					std::stringstream ss(line);
					std::string value;
					GLuint index;
					ss >> value;
					while (ss >> index)
						indices.push_back(index);
				}
			}

			auto mesh = Mesh::create("temp");
			for (auto& subInfo : subMeshInfo)
			{
				Box boundingBox;
				TriangleSurface s;
				for (int i = 0; i < subInfo.vertexCount; i++)
				{
					int idx = subInfo.firstVertex + i;
					s.addVertex(vertices[idx]);
					boundingBox.expand(vertices[idx].position);
				}
					
				for (int i = 0; i < subInfo.indexCount; i += 3)
				{
					int idx = subInfo.indexStart + i;
					unsigned int i0 = indices[idx] - subInfo.firstVertex;
					unsigned int i1 = indices[idx + 1] - subInfo.firstVertex;
					unsigned int i2 = indices[idx + 2] - subInfo.firstVertex;
					s.addTriangle(TriangleIndices(i0, i1, i2));
				}
				
				SubMesh subMesh;
				subMesh.primitive = Primitive::create("temp", s, 4);
				subMesh.primitive->setBoundingBox(boundingBox.getMinPoint(), boundingBox.getMaxPoint());

				mesh->addSubMesh(subMesh);
			}

			return mesh;
		}

		Entity::Ptr Importer::loadPrefabInstance(Database& db, PrefabInstance& prefabInstance, int64_t prefabID, const std::string& guid)
		{
			if (meshCache.find(guid) == meshCache.end())
				loadMeshGUID(guid);

			auto model = meshCache[guid].root;
			auto fileIDToName = meshCache[guid].fileIDMap;
			bool generateUVs = meshCache[guid].generateUVs;

			std::map<int64_t, Entity::Ptr> fileIDTransformInstance;
			auto modelCopy = copy(nullptr, model);
			modelCopy->update(glm::mat4(1.0f));

			if (generateUVs)
			{
				auto renderEntities = modelCopy->getChildrenWithComponent<Renderable>();
				for (auto e : renderEntities)
				{
					auto r = e->getComponent<Renderable>();
					if (!r->isEnabled())
						continue;

					auto subs = r->getMesh()->getSubMeshes();
					for (auto s : subs)
					{
						auto name = s.primitive->getName();
						auto fn = lightmapUVPath + "/" + name + ".txt";
						auto mesh = loadMeshFromTxt(fn);
						
						if (mesh != nullptr)
						{
							// TODO: this is ugly and can cause problems, fix it!
							auto subsNew = mesh->getSubMeshes();
							if (subsNew.size() > 1)
							{
								for (int i = 0; i < subsNew.size(); i++)
								{
									auto prim = subsNew[i].primitive;
									auto surf = prim->getSurface();
									auto bbox = prim->getBoundingBox();
									subs[i].primitive->updatGeometry(surf);
									subs[i].primitive->setBoundingBox(bbox);
									subs[i].primitive->flipWindingOrder();
								}
							}
							else
							{
								auto prim = subsNew[0].primitive;
								auto surf = prim->getSurface();
								auto bbox = prim->getBoundingBox();
								s.primitive->updatGeometry(surf);
								s.primitive->setBoundingBox(bbox);
								s.primitive->flipWindingOrder();
							}
						}
						else
						{
							meshes.insert(s.primitive->getName());
						}
					}
				}
			}

			std::map<std::string, int> names;
			std::map<int64_t, std::string> fileIDsEntity;
			std::map<int64_t, std::string> fileIDsTransform;
			std::map<int64_t, std::string> fileIDsMeshRenderer;
			for (auto [id, name] : fileIDToName)
			{
				if (id / 100000 == 1)
				{
					if (name.compare("//RootNode") == 0)
						name = "RootNode";
					fileIDsEntity.insert(std::make_pair(id, name));
				}
				if (id / 100000 == 4)
				{
					if (name.compare("//RootNode") == 0)
						name = "RootNode";
					fileIDsTransform.insert(std::make_pair(id, name));
				}
				if (id / 100000 == 23)
					fileIDsMeshRenderer.insert(std::make_pair(id, name));
			}

			std::map<int64_t, Entity::Ptr> fileID2Entity;
			std::map<int64_t, Transform::Ptr> fileID2Transform;
			std::map<int64_t, Renderable::Ptr> fileID2Renderable;
			for (auto entity : modelCopy->getChildrenWithComponent<Transform>())
			{
				auto t = entity->getComponent<Transform>();
				for (auto [id, name] : fileIDsTransform)
				{
					if (name.compare(entity->getName()) == 0)
						fileID2Transform.insert(std::make_pair(id, t));
				}

				for (auto [id, name] : fileIDsEntity)
				{
					if (name.compare(entity->getName()) == 0)
						fileID2Entity.insert(std::make_pair(id, entity));
				}

				auto r = entity->getComponent<Renderable>();
				if (r)
				{
					for (auto [id, name] : fileIDsMeshRenderer)
					{
						if (name.compare(entity->getName()) == 0)
							fileID2Renderable.insert(std::make_pair(id, r));
					}
				}
			}

			//if (generateUVs) // TODO: unity can generate secondary UV maps on load but they are not stored in the scene...
			//{
			//	std::cout << "generated UVs from Unity not available!" << std::endl;
			//	auto rendEnts = modelCopy->getChildrenWithComponent<Renderable>();
			//	for(auto e : rendEnts)
			//	{
			//		std::cout << "Entity " << e->getName() << std::endl;
			//		auto r = e->getComponent<Renderable>();
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
					for (auto [id, r] : fileID2Renderable)
					{
						if (prefabLM.find(id) != prefabLM.end())
						{
							auto ld = prefabLM[id];
							r->setDiffuseMode(2);
							r->setLightMapIndex(ld.index);
							r->setLightMapST(ld.offset, ld.scale);
						}
					}
				}
			}

			std::map<int64_t, std::map<std::string, std::string>> propMap;
			for (auto& mod : prefabInstance.modifications)
			{
				int64_t id = mod.target.fileID;
				std::string key = mod.propertyPath;
				std::string value = mod.value;

				if (key.compare("m_ProbeAnchor") == 0)
					value = std::to_string(mod.objectReference.fileID);
				else if (value.empty())
					value = mod.objectReference.guid;

				if (propMap.find(id) == propMap.end())
					propMap.insert(std::make_pair(id, std::map<std::string, std::string>()));
				propMap[id].insert(std::make_pair(key, value));
			}

			for (auto [id, m] : propMap)
			{
				int type = id / 100000;
				switch (type)
				{
				case 1: // GameObject
					if (fileID2Entity.find(id) != fileID2Entity.end())
					{
						auto entity = fileID2Entity[id];
						for (auto [k, v] : m)
						{
							if (k.compare("m_IsActive") == 0)
								entity->setActive(std::stoi(v));

							//if (k.compare("m_Name") == 0)
							//	entity->setName(v);
						}
					}
					break;
				case 4: // Transform
					if (fileID2Transform.find(id) != fileID2Transform.end())
					{
						auto t = fileID2Transform[id];
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
				case 23: // MeshRenderer
					if (fileID2Renderable.find(id) != fileID2Renderable.end())
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
									auto mat = loadMaterialGUID(matGUID);
									std::map<int64_t, Material::Ptr> matMap; // TODO: check if fileID is correct
									matMap.insert(std::make_pair(fileID, mat));
									materialCache.insert(std::make_pair(matGUID, matMap));
								}

								auto matMap = materialCache[matGUID];
								if (matMap.find(fileID) == matMap.end())
								{
									auto mat = loadMaterialGUID(matGUID);
									matMap.insert(std::make_pair(fileID, mat));
								}

								auto material = materialCache[matGUID][fileID];
								if (material)
								{
									auto r = fileID2Renderable[id];
									auto mesh = r->getMesh();
									mesh->setMaterial(matIndex, material);
									if (material->useBlending() || material->isTransmissive())
										r->setType(RenderType::TRANSPARENT);
								}
							}
							else if (k.compare("m_Enabled") == 0)
							{
								bool enabled = std::stoi(v);
								auto r = fileID2Renderable[id];
								r->setEnabled(enabled);
							}
							else if (k.compare("m_ProbeAnchor") == 0)
							{
								int64_t transformID = std::stoll(v);
								auto transform = db.transforms[transformID];
								auto name = db.gameObjects[transform.gameObject.id].name;

								//std::cout << "Reflection probe : " << transformID << " name : " << name << std::endl;

								if (transformID > 0)
								{
									auto r = fileID2Renderable[id];
									r->setReflectionProbe(name, -1);
								}
							}
							else if (k.compare("m_ReceiveGI") == 0)
							{
								int receiveGI = std::stoi(v);
								if (receiveGI == 2)
								{
									auto r = fileID2Renderable[id];
									auto mesh = r->getMesh();

									if (m.find("m_LightProbeUsage") != m.end())
									{
										int probeUsage = std::stoi(m["m_LightProbeUsage"]);
										if (probeUsage == 0)
											r->setDiffuseMode(2);
										else
											r->setDiffuseMode(0);
									}
									else
										r->setDiffuseMode(0);
								}
							}
						}
					}
				}
			}

			return modelCopy;
		}

		Entity::Ptr Importer::traverse(int64_t objectID, Entity::Ptr parent, Database& db)
		{
			auto go = db.gameObjects[objectID];
			int64_t transformID = -1;
			int64_t meshRendererID = -1;
			int64_t meshFilterID = -1;
			int64_t reflectionProbeID = -1;
			int64_t lightProbeGroupID = -1;
			int64_t lightID = -1;
			int64_t lodGroupID = -1;
			int64_t volumeID = -1;
			int64_t boxColliderID = -1;
			for (auto c : go.components)
			{
				auto id = c.fileID.id;
				if (db.transforms.find(id) != db.transforms.end())
					transformID = id;
				if (db.meshRenderers.find(id) != db.meshRenderers.end())
					meshRendererID = id;
				if (db.meshFilters.find(id) != db.meshFilters.end())
					meshFilterID = id;
				if (db.relfectionProbes.find(id) != db.relfectionProbes.end())
					reflectionProbeID = id;
				if (db.lightProbeGroups.find(id) != db.lightProbeGroups.end())
					lightProbeGroupID = id;
				if (db.lights.find(id) != db.lights.end())
					lightID = id;
				if (db.lodGroups.find(id) != db.lodGroups.end())
					lodGroupID = id;
				if (db.volumes.find(id) != db.volumes.end())
					volumeID = id;
				if (db.boxColliders.find(id) != db.boxColliders.end())
					boxColliderID = id;
			}

			auto node = Entity::create(go.name, parent);
			node->setActive(go.isActive);

			if (transformID >= 0)
			{
				auto& unityTransform = db.transforms[transformID];
				unityTransform.localPosition.x = -unityTransform.localPosition.x;
				unityTransform.localRotation.y = -unityTransform.localRotation.y;
				unityTransform.localRotation.z = -unityTransform.localRotation.z;

				auto t = node->getComponent<Transform>();
				t->setLocalPosition(unityTransform.localPosition);
				t->setLocalRotation(unityTransform.localRotation);
				t->setLocalScale(unityTransform.localScale);

				db.fileIDMap.insert(std::make_pair(objectID, go.name));
				db.fileIDMap.insert(std::make_pair(transformID, go.name));

				//db.fileID2Transform.insert(std::make_pair(transformID, t));
			}

			if (meshRendererID >= 0 && meshFilterID >= 0) // mesh filter and renderer have to be present to add a mesh
			{
				auto& unityMeshRenderer = db.meshRenderers[meshRendererID];
				auto& unityMeshFilter = db.meshFilters[meshFilterID];

				db.fileIDMap.insert(std::make_pair(meshRendererID, go.name));
				db.fileIDMap.insert(std::make_pair(meshFilterID, go.name));
				db.fileIDMap.insert(std::make_pair(unityMeshFilter.mesh.fileID, go.name));

				std::vector<Material::Ptr> materials;
				for (auto& unityMaterial : unityMeshRenderer.materials)
				{
					if (materialCache.find(unityMaterial.guid) == materialCache.end())
					{
						auto mat = loadMaterialGUID(unityMaterial.guid);
						std::map<int64_t, Material::Ptr> matMap; // TODO: check if fileID is correct
						matMap.insert(std::make_pair(unityMaterial.fileID, mat));
						materialCache.insert(std::make_pair(unityMaterial.guid, matMap));
					}

					auto matMap = materialCache[unityMaterial.guid];
					if (matMap.find(unityMaterial.fileID) == matMap.end())
					{
						auto mat = loadMaterialGUID(unityMaterial.guid);
						matMap.insert(std::make_pair(unityMaterial.fileID, mat));
					}

					auto material = materialCache[unityMaterial.guid][unityMaterial.fileID];
					materials.push_back(material);
				}

				auto guid = unityMeshFilter.mesh.guid;
				int64_t fileID = unityMeshFilter.mesh.fileID;

				if (guid.compare("0000000000000000e000000000000000") == 0) // buildin unity primitives
				{
					if (fileID == 10209) // plane
					{
						// TODO: check other primitive types
						SubMesh m;
						m.primitive = MeshPrimitives::createPlane(glm::vec3(0), 10.0f);
						m.primitive->setBoundingBox(glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(5.0f, 0.0f, 5.0f));
						if (materials.empty())
							m.material = getDefaultMaterial();
						else
							m.material = materials[0];

						auto mesh = Mesh::create(m.primitive->getName());
						mesh->addSubMesh(m);

						auto r = Renderable::create();
						r->setMesh(mesh);

						node->addComponent(r);
					}
					else
					{
						std::cout << "standard primitive with fileID: " << fileID << " is not supported!" << std::endl;
					}
				}
				else
				{
					if (meshCache.find(guid) == meshCache.end())
						loadMeshGUID(guid);

					if (meshCache.find(guid) != meshCache.end())
					{
						auto model = meshCache[guid].root;
						auto fileIDToName = meshCache[guid].fileIDMap;
						auto generateUVs = meshCache[guid].generateUVs;

						//if (generateUVs) // TODO: unity can generate secondary UV maps on load but they are not stored in the scene...
						//{
						//	std::cout << "generated UVs from Unity not available!" << std::endl;
						//	auto rendEnts = model->getChildrenWithComponent<Renderable>();
						//	for (auto e : rendEnts)
						//	{
						//		std::cout << "Entity " << e->getName() << std::endl;
						//		auto r = e->getComponent<Renderable>();
						//		auto subMeshes = r->getMesh()->getSubMeshes();
						//		for (auto s : subMeshes)
						//		{
						//			//std::cout << "Submesh " << s.primitive->getName() << std::endl;
						//			meshes.insert(s.primitive->getName());
						//		}								
						//	}
						//}

						if (generateUVs)
						{
							auto renderEntities = model->getChildrenWithComponent<Renderable>();
							for (auto e : renderEntities)
							{
								auto r = e->getComponent<Renderable>();
								if (!r->isEnabled())
									continue;

								auto subs = r->getMesh()->getSubMeshes();
								for (auto s : subs)
								{
									auto name = s.primitive->getName();
									auto fn = lightmapUVPath + "/" + name + ".txt";
									auto mesh = loadMeshFromTxt(fn);
									if (mesh != nullptr)
									{
										// TODO: this is ugly and can cause problems, fix it!
										auto subsNew = mesh->getSubMeshes();
										if (subsNew.size() > 1)
										{
											for (int i = 0; i < subsNew.size(); i++)
											{
												auto prim = subsNew[i].primitive;
												auto surf = prim->getSurface();
												auto bbox = prim->getBoundingBox();
												subs[i].primitive->updatGeometry(surf);
												subs[i].primitive->setBoundingBox(bbox);
												subs[i].primitive->flipWindingOrder();
											}
										}
										else
										{
											auto prim = subsNew[0].primitive;
											auto surf = prim->getSurface();
											auto bbox = prim->getBoundingBox();
											s.primitive->updatGeometry(surf);
											s.primitive->setBoundingBox(bbox);
											s.primitive->flipWindingOrder();
										}
									}
									else
									{
										meshes.insert(s.primitive->getName());
									}
								}
							}
						}

						std::map<int64_t, std::string> fileIDsMeshes;
						std::map<int64_t, std::string> fileIDsMeshRenderer;
						std::map<std::string, int> names;
						for (auto [id, name] : fileIDToName)
						{
							if (id / 100000 == 43)
							{
								std::string meshName = name;
								if (names.find(name) != names.end())
								{
									names[name]++;
									meshName = name + " " + std::to_string(names[name]);
								}
								else
								{
									names.insert(std::pair(name, 0));
								}
								fileIDsMeshes.insert(std::make_pair(id, meshName));
							}

							if (id / 100000 == 23)
							{
								fileIDsMeshRenderer.insert(std::make_pair(id, name));
							}
						}

						std::map<int64_t, Renderable::Ptr> fileID2Renderable;
						auto rends = model->getChildrenWithComponent<Renderable>();
						for (auto entity : rends)
						{
							auto r = entity->getComponent<Renderable>();
							for (auto [id, name] : fileIDsMeshes)
							{
								if (name.compare(entity->getName()) == 0)
									fileID2Renderable.insert(std::make_pair(id, r));
							}

							for (auto [id, name] : fileIDsMeshRenderer)
							{
								if (name.compare(entity->getName()) == 0)
									fileID2Renderable.insert(std::make_pair(id, r));
							}
						}

						if (fileID2Renderable.find(fileID) != fileID2Renderable.end())
						{
							auto r = fileID2Renderable[fileID];
							auto mesh = r->getMesh();

							auto newRend = Renderable::create();
							auto newMesh = Mesh::create(mesh->getName());

							auto subMeshes = mesh->getSubMeshes();
							for (int i = 0; i < subMeshes.size(); i++)
							{
								SubMesh s;
								s.primitive = subMeshes[i].primitive;
								s.material = subMeshes[i].material;
								if (i < materials.size() && materials[i] != nullptr) // TODO: check if mesh/mat indices match up!
									s.material = materials[i];

								if (s.material == nullptr)
									std::cout << "error: submesh has no material!" << std::endl;

								newMesh->addSubMesh(s);
							}

							if (!db.isPrefab)
							{
								auto& olmd = lightData.objectLightMapData;
								if (olmd.find(meshRendererID) != olmd.end())
								{
									auto ld = olmd[meshRendererID];
									newRend->setDiffuseMode(2);
									newRend->setLightMapIndex(ld.index);
									newRend->setLightMapST(ld.offset, ld.scale);
								}
							}

							if (unityMeshRenderer.receiveGI == 1)
								newRend->setDiffuseMode(2);

							newRend->setMesh(newMesh);
							node->addComponent(newRend);
						}
						else
						{
							std::cout << "error: missing mesh with fileID " << fileID << std::endl; // TODO: check why this can happen
						}
					}
				}
			}

			if (reflectionProbeID >= 0)
			{
				auto reflectionProbe = db.relfectionProbes[reflectionProbeID];
				auto size = reflectionProbe.boxSize;
				auto offset = reflectionProbe.boxOffset;

				offset.x = -offset.x;

				glm::vec3 boxMin = offset - size * 0.5f;
				glm::vec3 boxMax = offset + size * 0.5f;
				Box box(boxMin, boxMax);

				if (lightData.reflectionProbes.find(reflectionProbeID) != lightData.reflectionProbes.end())
				{
					auto guid = lightData.reflectionProbes[reflectionProbeID];
					auto fn = metaData[guid].filePath;

					std::cout << "loading reflection probe " << fn << std::endl;
										
					auto facesImg = IO::decodeEXRFromFile(fn);
					int w = facesImg->getWidth();
					int h = facesImg->getHeight();
					int c = facesImg->getChannels();

					auto cubemap = IO::CubemapF32::create(h, 3);
					auto cubemapM = IO::CubemapF32::create(h, 3);
					cubemap->setFromFaces(facesImg);
					cubemapM->mirror(cubemap);

					auto tex = cubemapM->upload();

					auto probe = LightProbe::create(tex, box);

					node->addComponent(probe);
				}
			}
			if (lightProbeGroupID >= 0)
			{
				auto lightProbeGroup = db.lightProbeGroups[lightProbeGroupID];
				//std::cout << go.name << " has " << lightProbeGroup.sourcePositions.size() << " probes" << std::endl;
			}
			if (lightID >= 0)
			{
				auto l = db.lights[lightID];
				switch (l.type)
				{
				case 1:
				{
					auto color = Color::sRGBToLinear(l.color);
					auto light = Light::create(LightType::DIRECTIONAL, color, l.intensity, l.range);
					node->addComponent(light);

					// mesh for debugging
					float radius = 1.0f;
					auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
					prim->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));

					auto mat = getDefaultMaterial();
					mat->addProperty("material.baseColorFactor", color);
					mat->addProperty("material.unlit", true);

					SubMesh s;
					s.primitive = prim;
					s.material = mat;

					auto mesh = Mesh::create("Sphere");
					mesh->addSubMesh(s);

					auto r = node->addComponent<Renderable>();
					r->setMesh(mesh);

					break;
				}
				default:
					std::cout << "error: Light type: " << l.type << " not supported!" << std::endl;
				}				
			}
			if (volumeID >= 0)
			{
				UVolume& v = db.volumes[volumeID];
				PostProcessParameters ppp;
				loadCameraProfileGUID(v.sharedProfile.guid, ppp);
				auto volume = Volume::create(v.isGlobal, v.blendDistance);
				volume->setPPP(ppp);
				node->addComponent(volume);
			}
			if (boxColliderID >= 0)
			{
				UBoxCollider& collider = db.boxColliders[boxColliderID];
				glm::vec3 minPoint = collider.center - collider.size * 0.5f;
				glm::vec3 maxPoint = collider.center + collider.size * 0.5f;
				Box boundingBox(minPoint, maxPoint);
				auto boxCollider = BoxCollider::Ptr(new BoxCollider(boundingBox));
				node->addComponent(boxCollider);
			}

			auto& unityTransform = db.transforms[transformID];
			for (auto c : unityTransform.children)
			{
				if (db.transforms.find(c.id) == db.transforms.end())
				{
					std::cout << "error: RectTransform not supported. Skipping!" << std::endl;
					continue;
				}					

				UTransform& transform = db.transforms[c.id];
				int64_t childID = transform.gameObject.id;
				//std::cout << "child: " << transform.prefabInstance.id << std::endl;
				if (childID > 0)
				{
					// if child is in the same file continue with child id
					auto child = traverse(childID, node, db);
					node->addChild(child);
				}
				else
				{
					// TODO: instantiatePrefab() and loadPrefabInstance() do pretty much the same thing! So combine them

					// child comes from an external file (prefab)
					std::string guid = transform.correspondingSourceObject.guid;
					Metadata& data = metaData[guid];
					int idx = data.filePath.find_last_of('.') + 1;
					int len = data.filePath.length() - idx;
					auto ext = data.filePath.substr(idx, len);
					if (ext.compare("prefab") == 0)
					{
						// child is instantiated from a .prefab file
						if (prefabeCache.find(guid) == prefabeCache.end())
							loadPrefabGUID(guid);

						int64_t prefabInstanceID = transform.prefabInstance.id;
						PrefabInstance prefabInstance = db.prefabInstances[prefabInstanceID];

						auto prefabCopyRoot = instantiatePrefab(db, prefabInstance, prefabeCache[guid], prefabInstanceID);
						node->addChild(prefabCopyRoot);
					}
					else
					{
						// child is instantiated from a 3D model file (.fbx)
						int64_t prefabInstanceID = transform.prefabInstance.id;
						PrefabInstance prefabInstance = db.prefabInstances[prefabInstanceID];
						auto model = loadPrefabInstance(db, prefabInstance, prefabInstanceID, guid);

						for (auto [id, t] : db.transforms)
						{
							if (t.father.id == c.id)
							{
								auto child = traverse(t.gameObject.id, model, db);
								model->addChild(child);
							}
						}
						node->addChild(model);
					}
				}
			}

			if (lodGroupID >= 0 && db.lodGroups[lodGroupID].lods.size() > 1) // atleast 2 LODs
			{
				// TODO: can lod groups reference meshes outside of the current hierarchy??
				auto rends = node->getComponentsInChildren<Renderable>();
				auto lodGroup = db.lodGroups[lodGroupID];
				float size = lodGroup.size;
				glm::vec3 refPoint = lodGroup.localReferencePoint;
				
				auto lod = Lod::create(size, refPoint);
				for (int i = 0; i < lodGroup.lods.size(); i++)
				{
					auto lodLevel = lodGroup.lods[i];
					auto id = lodLevel.renderers[0].fileID.id;
					auto mr = db.meshRenderers[id];
					auto name = db.gameObjects[mr.gameObject.id].name;
					float screenHeight = lodLevel.screenRelativeHeight;
					for (auto r : rends)
					{
						for (auto s : r->getMesh()->getSubMeshes())
						{
							if (name.compare(s.primitive->getName()) == 0)
								lod->addLevel(screenHeight, r);
						}				
					}	
				}
				node->addComponent(lod);
			}

			return node;
		}

		Entity::Ptr Importer::copy(Entity::Ptr dst, Entity::Ptr src)
		{
			Entity::Ptr node = Entity::create(src->getName(), dst);
			node->setActive(src->isActive());
			auto srcTransform = src->getComponent<Transform>();
			auto dstTransform = node->getComponent<Transform>();
			dstTransform->setLocalPosition(srcTransform->getLocalPosition());
			dstTransform->setLocalRotation(srcTransform->getLocalRotation());
			dstTransform->setLocalScale(srcTransform->getLocalScale());

			auto r = src->getComponent<Renderable>();
			if (r)
			{
				auto mesh = r->getMesh();
				auto newRend = Renderable::create();
				auto newMesh = Mesh::create(mesh->getName());

				newRend->setLightMapST(r->getLMOffset(), r->getLMScale());
				newRend->setLightMapIndex(r->getLMIndex());
				newRend->setReflectionProbe(r->getReflName(), r->getRPIndex());
				newRend->setDiffuseMode(r->getDiffuseMode());

				for (auto s : mesh->getSubMeshes())
					newMesh->addSubMesh(s);
				newRend->setMesh(newMesh);

				node->addComponent(newRend);
			}		

			for (int i = 0; i < src->numChildren(); i++)
			{
				auto srcChild = src->getChild(i);
				auto dstChild = copy(node, srcChild);
				node->addChild(dstChild);
			}

			auto lod = src->getComponent<Lod>();
			if (lod)
			{
				auto levels = lod->getLevels();
				auto dstRends = node->getComponentsInChildren<Renderable>();
				auto dstLod = Lod::create(lod->getSize(), lod->getReferencePoint());
				for (auto [height, srcRend] : levels)
				{
					Renderable::Ptr targetRend = nullptr;
					for (auto& srcSub : srcRend->getMesh()->getSubMeshes())
					{
						auto srcName = srcSub.primitive->getName();
						for (auto dstRend : dstRends)
						{
							for (auto& dstSub : dstRend->getMesh()->getSubMeshes())
							{
								auto dstName = dstSub.primitive->getName();
								if (srcName.compare(dstName) == 0)
									targetRend = dstRend;
							}
						}
					}
					dstLod->addLevel(height, targetRend);
				}
				node->addComponent(dstLod);
			}

			return node;
		}

		Entity::Ptr Importer::instantiatePrefab(Database& db, PrefabInstance& instance, PrefabCache& cache, int64_t prefabID)
		{
			auto model = cache.root;
			auto fileIDToName = cache.fileIDMap;

			std::map<int64_t, Entity::Ptr> fileIDTransformInstance;
			auto modelCopy = copy(nullptr, model);
			modelCopy->update(glm::mat4(1.0f));

			//if (generateUVs)
			//{
			//	std::string path = "C:/workspace/code/Archviz/lightmap_uv";

			//	auto renderEntities = modelCopy->getChildrenWithComponent<Renderable>();
			//	for (auto e : renderEntities)
			//	{
			//		auto r = e->getComponent<Renderable>();
			//		auto subs = r->getMesh()->getSubMeshes();
			//		for (auto s : subs)
			//		{
			//			auto name = s.primitive->getName();
			//			auto fn = path + "/" + name + ".txt";
			//			auto mesh = loadMeshFromTxt(fn);

			//			if (mesh != nullptr)
			//			{
			//				// TODO: this is ugly and can cause problems, fix it!
			//				auto subsNew = mesh->getSubMeshes();
			//				if (subsNew.size() > 1)
			//				{
			//					for (int i = 0; i < subsNew.size(); i++)
			//					{
			//						auto prim = subsNew[i].primitive;
			//						auto surf = prim->getSurface();
			//						auto bbox = prim->getBoundingBox();
			//						subs[i].primitive->updatGeometry(surf);
			//						subs[i].primitive->setBoundingBox(bbox);
			//						subs[i].primitive->flipWindingOrder();
			//					}
			//				}
			//				else
			//				{
			//					auto prim = subsNew[0].primitive;
			//					auto surf = prim->getSurface();
			//					auto bbox = prim->getBoundingBox();
			//					s.primitive->updatGeometry(surf);
			//					s.primitive->setBoundingBox(bbox);
			//					s.primitive->flipWindingOrder();
			//				}
			//			}
			//		}
			//	}
			//}

			std::map<std::string, int> names;
			std::map<int64_t, std::string> fileIDsEntity;
			std::map<int64_t, std::string> fileIDsTransform;
			std::map<int64_t, std::string> fileIDsMeshRenderer;
			for (auto [id, name] : fileIDToName)
			{
				int64_t type = id / 100000;
				if (id > 10000000ll)
					type = id / 1000000000000000ll;

				if (type == 1)
				{
					if (name.compare("//RootNode") == 0)
						name = "RootNode";
					fileIDsEntity.insert(std::make_pair(id, name));
				}
				if (type == 4)
				{
					if (name.compare("//RootNode") == 0)
						name = "RootNode";
					fileIDsTransform.insert(std::make_pair(id, name));
				}
				if (type == 23)
					fileIDsMeshRenderer.insert(std::make_pair(id, name));
			}

			std::map<int64_t, Entity::Ptr> fileID2Entity;
			std::map<int64_t, Transform::Ptr> fileID2Transform;
			std::map<int64_t, Renderable::Ptr> fileID2Renderable;
			for (auto entity : modelCopy->getChildrenWithComponent<Transform>())
			{
				auto t = entity->getComponent<Transform>();
				for (auto [id, name] : fileIDsTransform)
				{
					if (name.compare(entity->getName()) == 0)
						fileID2Transform.insert(std::make_pair(id, t));
				}

				for (auto [id, name] : fileIDsEntity)
				{
					if (name.compare(entity->getName()) == 0)
						fileID2Entity.insert(std::make_pair(id, entity));
				}

				auto r = entity->getComponent<Renderable>();
				if (r)
				{
					for (auto [id, name] : fileIDsMeshRenderer)
					{
						if (name.compare(entity->getName()) == 0)
							fileID2Renderable.insert(std::make_pair(id, r));
					}
				}
			}

			//if (generateUVs) // TODO: unity can generate secondary UV maps on load but they are not stored in the scene...
			//{
			//	std::cout << "generated UVs from Unity not available!" << std::endl;
			//	auto rendEnts = modelCopy->getChildrenWithComponent<Renderable>();
			//	for(auto e : rendEnts)
			//	{
			//		std::cout << "Entity " << e->getName() << std::endl;
			//		auto r = e->getComponent<Renderable>();
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
					for (auto [id, r] : fileID2Renderable)
					{
						if (prefabLM.find(id) != prefabLM.end())
						{
							auto ld = prefabLM[id];
							auto mesh = r->getMesh();
							if (r->getDiffuseMode() == 2)
							{
								r->setLightMapIndex(ld.index);
								r->setLightMapST(ld.offset, ld.scale);
							}
						}
					}
				}
				else
				{
					for (auto [id, r] : fileID2Renderable)
						r->setDiffuseMode(0);
				}
			}

			std::map<int64_t, std::map<std::string, std::string>> propMap;
			for (auto& mod : instance.modifications)
			{
				int64_t id = mod.target.fileID;
				std::string key = mod.propertyPath;
				std::string value = mod.value;

				if (key.compare("m_ProbeAnchor") == 0)
					value = std::to_string(mod.objectReference.fileID);
				else if (value.empty())
					value = mod.objectReference.guid;

				if (propMap.find(id) == propMap.end())
					propMap.insert(std::make_pair(id, std::map<std::string, std::string>()));
				propMap[id].insert(std::make_pair(key, value));
			}

			for (auto [id, m] : propMap)
			{
				int64_t type = id / 100000;
				if (id > 10000000ll)
					type = id / 1000000000000000ll;
				
				switch (type)
				{
				case 1: // GameObject
					if (fileID2Entity.find(id) != fileID2Entity.end())
					{
						auto entity = fileID2Entity[id];
						for (auto [k, v] : m)
						{
							if (k.compare("m_IsActive") == 0)
								entity->setActive(std::stoi(v));

							//if (k.compare("m_Name") == 0)
							//	entity->setName(v);
						}
					}
					break;
				case 4: // Transform
					if (fileID2Transform.find(id) != fileID2Transform.end())
					{
						auto t = fileID2Transform[id];
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
				case 23: // MeshRenderer
					if (fileID2Renderable.find(id) != fileID2Renderable.end())
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
									auto mat = loadMaterialGUID(matGUID);
									std::map<int64_t, Material::Ptr> matMap; // TODO: check if fileID is correct
									matMap.insert(std::make_pair(fileID, mat));
									materialCache.insert(std::make_pair(matGUID, matMap));
								}

								auto matMap = materialCache[matGUID];
								if (matMap.find(fileID) == matMap.end())
								{
									auto mat = loadMaterialGUID(matGUID);
									matMap.insert(std::make_pair(fileID, mat));
								}

								auto material = materialCache[matGUID][fileID];

								if (material)
								{
									auto r = fileID2Renderable[id];
									auto mesh = r->getMesh();
									mesh->setMaterial(matIndex, material);
									if (material->useBlending() || material->isTransmissive())
										r->setType(RenderType::TRANSPARENT);
								}
							}
							else if (k.compare("m_Enabled") == 0)
							{
								bool enabled = std::stoi(v);
								auto r = fileID2Renderable[id];
								r->setEnabled(enabled);
							}
							else if (k.compare("m_ProbeAnchor") == 0)
							{
								int64_t transformID = std::stoll(v);
								auto transform = db.transforms[transformID];
								auto name = db.gameObjects[transform.gameObject.id].name;

								//std::cout << "Reflection probe : " << transformID << " name : " << name << std::endl;

								if (transformID > 0)
								{
									auto r = fileID2Renderable[id];
									r->setReflectionProbe(name, -1);
								}
							}
							else if (k.compare("m_ReceiveGI") == 0)
							{
								int receiveGI = std::stoi(v);
								if (receiveGI == 2)
								{
									auto r = fileID2Renderable[id];
									auto mesh = r->getMesh();

									if (m.find("m_LightProbeUsage") != m.end())
									{
										int probeUsage = std::stoi(m["m_LightProbeUsage"]);
										if (probeUsage == 0)
											r->setDiffuseMode(2);
										else
											r->setDiffuseMode(0);
									}
									else
										r->setDiffuseMode(0);
								}
							}
							else if (k.compare("m_LightProbeUsage") == 0)
							{
								int probeUsage = std::stoi(v);
								if (probeUsage > 0)
								{
									auto r = fileID2Renderable[id];
									auto mesh = r->getMesh();
									r->setDiffuseMode(0);
								}
							}
						}
					}
				}
			}

			return modelCopy;
		}

		void Importer::loadPrefabGUID(const std::string& guid)
		{
			if (metaData.find(guid) == metaData.end())
			{
				std::cout << "error: could not find prefab with GUID: " << guid << std::endl;
				return;
			}

			Metadata& data = metaData[guid];
			auto prefabFile = data.filePath;

			std::cout << "loading prefab " << prefabFile << std::endl;

			std::ifstream file(prefabFile);
			std::stringstream ss;
			if (file.is_open())
				ss << file.rdbuf();
			else
				std::cout << "could not open file " << prefabFile << std::endl;
			std::string content = ss.str();

			std::string line;
			uint64_t currentID;
			std::map<int64_t, UnityObject> prefabObjects;
			while (std::getline(ss, line))
			{
				if (line.empty())
					continue;
				if (line[0] == '%') // comment
					continue;
				if (line.length() > 2 && line.substr(0, 3).compare("---") == 0)
				{
					std::string empty, type, id;
					std::stringstream lineSS(line);
					lineSS >> empty >> type >> id;

					std::stringstream ssID(id.substr(1, id.length() - 1));
					uint32_t objectType = std::stoi(type.substr(3, type.length() - 3));
					uint64_t objectID;
					ssID >> objectID;

					UnityObject object;
					object.id = objectID;
					object.type = objectType;

					std::getline(ss, line);
					object.key = line.substr(0, line.length() - 1); // remove the ':'

					currentID = objectID;
					prefabObjects.insert(std::make_pair(objectID, object));
					prefabObjects[currentID].content = line + '\n';
				}
				else
				{
					prefabObjects[currentID].content += line + '\n';
				}
			}

			int64_t rootID = -1;
			Database db;
			std::map<std::string, int> stats;
			for (auto&& [id, obj] : prefabObjects)
			{
				if (stats.find(obj.key) != stats.end())
					stats[obj.key]++;
				else
					stats.insert(std::make_pair(obj.key, 1));

				if (obj.key.compare("GameObject") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					GameObject go;
					tree["GameObject"] >> go;
					db.gameObjects.insert(std::make_pair(id, go));
				}
				else if (obj.key.compare("Transform") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					UTransform t;
					tree["Transform"] >> t;
					db.transforms.insert(std::make_pair(id, t));
				}
				else if (obj.key.compare("MeshRenderer") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshRenderer mr;
					tree["MeshRenderer"] >> mr;
					db.meshRenderers.insert(std::make_pair(id, mr));
				}
				else if (obj.key.compare("MeshFilter") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshFilter mf;
					tree["MeshFilter"] >> mf;
					db.meshFilters.insert(std::make_pair(id, mf));
				}
				else if (obj.key.compare("Prefab") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));
					tree["Prefab"]["m_RootGameObject"]["fileID"] >> rootID;
					//std::cout << "root id: " << rootID << std::endl;
				}
				else if (obj.key.compare("LODGroup") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					LODGroup lod;
					tree["LODGroup"] >> lod;
					db.lodGroups.insert(std::make_pair(id, lod));
				}
			}

			//std::cout << "Statistic" << std::endl;
			//std::cout << "---------" << std::endl;
			//for (auto [key, num] : stats)
			//	std::cout << key << ": " << num << std::endl;

			if (rootID < 0) // TODO: check if theres a valid root in the prefab
			{
				for (auto [id, t] : db.transforms)
				{
					if (t.father.id == 0)
						rootID = t.gameObject.id;
				}
			}

			db.isPrefab = true;
			auto prefab = traverse(rootID, nullptr, db);

			PrefabCache cache;
			cache.root = prefab;
			cache.fileIDMap = db.fileIDMap;
			prefabeCache.insert(std::make_pair(guid, cache));
		}

		Entity::Ptr Importer::loadPrefab(const std::string& prefabFile)
		{
			std::ifstream file(prefabFile);
			std::stringstream ss;
			if (file.is_open())
				ss << file.rdbuf();
			else
				std::cout << "could not open file " << prefabFile << std::endl;
			std::string content = ss.str();

			std::string line;
			uint64_t currentID;
			std::map<int64_t, UnityObject> prefabObjects;
			while (std::getline(ss, line))
			{
				if (line.empty())
					continue;
				if (line[0] == '%') // comment
					continue;
				if (line.length() > 2 && line.substr(0, 3).compare("---") == 0)
				{
					std::string empty, type, id;
					std::stringstream lineSS(line);
					lineSS >> empty >> type >> id;

					std::stringstream ssID(id.substr(1, id.length() - 1));
					uint32_t objectType = std::stoi(type.substr(3, type.length() - 3));
					uint64_t objectID;
					ssID >> objectID;

					UnityObject object;
					object.id = objectID;
					object.type = objectType;

					std::getline(ss, line);
					object.key = line.substr(0, line.length() - 1); // remove the ':'

					currentID = objectID;
					prefabObjects.insert(std::make_pair(objectID, object));
					prefabObjects[currentID].content = line + '\n';
				}
				else
				{
					prefabObjects[currentID].content += line + '\n';
				}
			}

			int64_t rootID = -1;
			Database db;
			std::map<std::string, int> stats;
			for (auto&& [id, obj] : prefabObjects)
			{
				if (stats.find(obj.key) != stats.end())
					stats[obj.key]++;
				else
					stats.insert(std::make_pair(obj.key, 1));

				if (obj.key.compare("GameObject") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					GameObject go;
					tree["GameObject"] >> go;
					db.gameObjects.insert(std::make_pair(id, go));
				}
				else if (obj.key.compare("Transform") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					UTransform t;
					tree["Transform"] >> t;
					db.transforms.insert(std::make_pair(id, t));
				}
				else if (obj.key.compare("MeshRenderer") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshRenderer mr;
					tree["MeshRenderer"] >> mr;
					db.meshRenderers.insert(std::make_pair(id, mr));
				}
				else if (obj.key.compare("MeshFilter") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshFilter mf;
					tree["MeshFilter"] >> mf;
					db.meshFilters.insert(std::make_pair(id, mf));
				}
				else if (obj.key.compare("Prefab") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));
					tree["Prefab"]["m_RootGameObject"]["fileID"] >> rootID;
					//std::cout << "root id: " << rootID << std::endl;
				}
				else if (obj.key.compare("LODGroup") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					LODGroup lod;
					tree["LODGroup"] >> lod;
					db.lodGroups.insert(std::make_pair(id, lod));
				}
			}

			//std::cout << "Statistic" << std::endl;
			//std::cout << "---------" << std::endl;
			//for (auto [key, num] : stats)
			//	std::cout << key << ": " << num << std::endl;

			if (rootID < 0) // TODO: check if theres a valid root in the prefab
			{
				for (auto [id, t] : db.transforms)
				{
					if (t.father.id == 0)
						rootID = t.gameObject.id;
				}
			}

			db.isPrefab = true;
			auto prefab = traverse(rootID, nullptr, db);

			std::cout << "prefab fileID map" << std::endl;
			for (auto&& [id, name] : db.fileIDMap)
				std::cout << id << " : " << name << std::endl;

			return prefab;
		}

		void Importer::loadCameraProfileGUID(const std::string& guid, PostProcessParameters& ppp)
		{
			if (metaData.find(guid) == metaData.end())
			{
				std::cout << "error: could not find camera profile!" << std::endl;
			}

			Metadata& data = metaData[guid];

			std::string content = IO::loadTxtFile(data.filePath);
			ryml::Tree root = ryml::parse(ryml::to_csubstr(content));
			int matIndex = 0;
			for (int i = 0; i < root.rootref().num_children(); i++) // There can be other objects in a .mat file...
			{
				ryml::NodeRef stream = root.docref(i);
				if (stream.has_child("MonoBehaviour"))
				{
					auto& monoNode = stream["MonoBehaviour"];

					std::string name;
					monoNode["m_Name"] >> name;
					std::cout << name << std::endl;
					if (name.compare("ColorAdjustments") == 0)
					{
						bool overrideState = false;
						monoNode["postExposure"]["m_OverrideState"] >> overrideState;
						if (overrideState)
							monoNode["postExposure"]["m_Value"] >> ppp.postExposure;
					}
					else if (name.compare("Bloom") == 0)
					{
						bool overrideState = false;
						monoNode["threshold"]["m_OverrideState"] >> overrideState;
						if(overrideState)
							monoNode["threshold"]["m_Value"] >> ppp.bloomThreshold;

						monoNode["intensity"]["m_OverrideState"] >> overrideState;
						if (overrideState)
						{
							monoNode["intensity"]["m_Value"] >> ppp.bloomIntensity;
							ppp.bloomIntensity *= 0.5f;
						}					

						monoNode["tint"]["m_OverrideState"] >> overrideState;
						if (overrideState)
						{
							glm::vec4 color;
							read(monoNode["tint"]["m_Value"], color);
							ppp.bloomTint = color;
						}
					}				
				}
			}
			ryml::NodeRef stream = root.docref(matIndex);
		}

		void Importer::loadObjects(const std::string& sceneFile)
		{
			std::ifstream file(sceneFile);
			std::stringstream ss;
			if (file.is_open())
				ss << file.rdbuf();
			else
				std::cout << "could not open file " << sceneFile << std::endl;
			std::string content = ss.str();

			std::vector<std::string> lines;
			std::string line;
			unsigned int numObjects = 0;
			uint64_t currentID;
			while (std::getline(ss, line))
			{
				if (line.empty())
					continue;
				if (line[0] == '%') // comment
					continue;
				if (line.length() > 2 && line.substr(0, 3).compare("---") == 0)
				{
					std::string empty, type, id;
					std::stringstream lineSS(line);
					lineSS >> empty >> type >> id;

					std::stringstream ssID(id.substr(1, id.length() - 1));
					uint32_t objectType = std::stoi(type.substr(3, type.length() - 3));
					uint64_t objectID;
					ssID >> objectID;

					UnityObject object;
					object.id = objectID;
					object.type = objectType;

					std::getline(ss, line);
					object.key = line.substr(0, line.length() - 1); // remove the ':'

					currentID = objectID;
					unityObjects.insert(std::make_pair(objectID, object));
					unityObjects[currentID].content = line + '\n';
				}
				else
				{
					unityObjects[currentID].content += line + '\n';
				}
			}
		}

		Scene::Ptr Importer::loadScene(const std::string& assetPath, const std::string& sceneFile)
		{
			int len = assetPath.find_last_of('/');
			std::string prefix = assetPath.substr(0, len);
			this->lightmapUVPath = prefix + "/lightmap_uv";

			// load all unity objects as yaml docs
			loadObjects(assetPath + "/" + sceneFile);

			// parse all supported class IDs and build database
			Database db;
			std::map<std::string, int> stats;
			for (auto&& [id, obj] : unityObjects)
			{
				if (stats.find(obj.key) != stats.end())
					stats[obj.key]++;
				else
					stats.insert(std::make_pair(obj.key, 1));

				if (obj.key.compare("GameObject") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					GameObject go;
					tree["GameObject"] >> go;
					db.gameObjects.insert(std::make_pair(id, go));
				}
				else if (obj.key.compare("Transform") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					UTransform t;
					tree["Transform"] >> t;
					db.transforms.insert(std::make_pair(id, t));
				}
				else if (obj.key.compare("MeshRenderer") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshRenderer mr;
					tree["MeshRenderer"] >> mr;
					db.meshRenderers.insert(std::make_pair(id, mr));
				}
				else if (obj.key.compare("MeshFilter") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					MeshFilter mf;
					tree["MeshFilter"] >> mf;
					db.meshFilters.insert(std::make_pair(id, mf));
				}
				else if (obj.key.compare("PrefabInstance") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					PrefabInstance prefab;
					tree["PrefabInstance"] >> prefab;
					db.prefabInstances.insert(std::make_pair(id, prefab));
				}
				else if (obj.key.compare("ReflectionProbe") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					ReflectionProbe rb;
					tree["ReflectionProbe"] >> rb;
					db.relfectionProbes.insert(std::make_pair(id, rb));
				}
				else if (obj.key.compare("LightProbeGroup") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					LightProbeGroup lpg;
					tree["LightProbeGroup"] >> lpg;
					db.lightProbeGroups.insert(std::make_pair(id, lpg));
				}
				else if (obj.key.compare("Light") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					ULight l;
					tree["Light"] >> l;
					db.lights.insert(std::make_pair(id, l));
				}
				else if (obj.key.compare("RenderSettings") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));
					tree["RenderSettings"] >> db.renderSettings;
				}
				else if (obj.key.compare("LightmapSettings") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));
					tree["LightmapSettings"] >> db.lightmapSettings;
				}
				//else if (obj.key.compare("MonoBehaviour") == 0)
				//{
				//	ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));
				//	auto& monoNode = tree["MonoBehaviour"];
				//	if (monoNode.has_child("sharedProfile")) // Postprocess Volumes
				//	{
				//		UVolume volume;
				//		monoNode["m_GameObject"] >> volume.gameObject;
				//		monoNode["m_IsGlobal"] >> volume.isGlobal;
				//		monoNode["blendDistance"] >> volume.blendDistance;
				//		monoNode["sharedProfile"] >> volume.sharedProfile;
				//		db.volumes.insert(std::make_pair(id, volume));
				//	}					
				//}
				else if (obj.key.compare("BoxCollider") == 0)
				{
					ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(obj.content));

					UBoxCollider boxCollider;
					tree["BoxCollider"] >> boxCollider;
					db.boxColliders.insert(std::make_pair(id, boxCollider));
				}
			}

			std::cout << "---------" << std::endl;
			std::cout << "Statistic" << std::endl;
			std::cout << "---------" << std::endl;
			for (auto [key, num] : stats)
				std::cout << key << ": " << num << std::endl;
			std::cout << "Total Unity objects: " << unityObjects.size() << std::endl;

			std::cout << "----------" << std::endl;
			std::cout << "Root nodes" << std::endl;
			std::cout << "----------" << std::endl;

			// find root nodes in the scene
			std::map<std::string, int64_t> rootNodes;
			for (auto [id,t] : db.transforms)
			{
				if (t.father.id == 0)
				{
					std::string name = db.gameObjects[t.gameObject.id].name;
					int64_t id = t.gameObject.id;
					rootNodes.insert(std::make_pair(name, id));
				}
			}
			for (auto&& [name, id] : rootNodes)
				std::cout << id << " " << name << std::endl;

			// load all .meta files
			loadMetadata(assetPath);

			std::cout << "----------" << std::endl;
			std::cout << "Loading..." << std::endl;
			std::cout << "----------" << std::endl;

			// load lightdata asset
			auto lightDataGUID = db.lightmapSettings.lightingDataAsset.guid;
			if (metaData.find(lightDataGUID) != metaData.end())
			{
				// TODO: check if txt file exists, deactivate lightmaps/probes otherwise
				auto fn = metaData[lightDataGUID].filePath + ".txt";
				lightData.loadAsset(fn);
			}
			
			// parse scene hierarchy using DFS
			auto scene = Scene::create("UnityScene");
			//scene->addRootEntity(traverse(1308100271, nullptr, db));
			//scene->addRootEntity(traverse(810077401, nullptr, db));
			scene->addRootEntity(traverse(1308100271, nullptr, db));
			scene->addRootEntity(traverse(2075706406, nullptr, db));

			//for (auto&& [name, id] : rootNodes)
			//{
			//	auto root = traverse(id, nullptr, db);
			//	scene->addRootEntity(name, root);
			//}

			// flip faces that are CW
			scene->checkWindingOrder();

			// load GI settings, light maps/probes
			loadSettings(db.renderSettings, scene);
			loadLightMaps(scene);

			SHLightProbes probes;
			probes.tetrahedras = lightData.tetrahedras;
			probes.coeffs = lightData.probeCoeffs;
			probes.positions = lightData.probePositions;
			scene->setLightProbes(probes);

			if (!meshes.empty())
			{
				std::ofstream file(prefix + "/meshes_missing_lightmap_uv.txt");
				for (auto m : meshes)
					file << m << std::endl;
			}

			return scene;
		}

		void Importer::loadLightMaps(Scene::Ptr scene)
		{
			// TODO: this should be done in renderer.prepare(). Just load the maps and store in scene here...
			{
				std::vector<ImageF32::Ptr> lightMapImages;
				for (auto guid : lightData.lightMaps)
				{
					if (metaData.find(guid) != metaData.end())
					{
						std::cout << "loading lightmap " << metaData[guid].filePath << std::endl;
						lightMapImages.push_back(IO::decodeEXRFromFile(metaData[guid].filePath));
					}
				}

				uint32 size = 2048;
				uint32 numImages = lightMapImages.size();
				uint32 imageSize = size * size * 4;
				float* buffer = new float[numImages * imageSize];
				for (int i = 0; i < numImages; i++)
				{
					int offset = i * imageSize;
					auto img = IO::resizeImage(lightMapImages[i], size, size);
					std::memcpy(buffer + offset, img->getRawPtr(), imageSize * sizeof(float));
					//auto tex = img->upload(false);
					//tex->generateMipmaps();
					//tex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
					//scene->addLightMap(tex);
				}

				auto lightMaps = Texture2DArray::create(size, size, lightMapImages.size(), GL::RGBA32F);
				lightMaps->upload(buffer);
				//lightMaps->generateMipmaps();
				//lightMaps->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

				delete[] buffer;

				scene->addLightMaps(lightMaps);
			}

			{
				std::vector<ImageUI8::Ptr> dirMapImages;
				for (auto guid : lightData.directionMaps)
				{
					if (metaData.find(guid) != metaData.end())
					{
						std::cout << "loading direction map " << metaData[guid].filePath << std::endl;
						dirMapImages.push_back(IO::decodeFromFile(metaData[guid].filePath));
					}
				}

				uint32 size = 2048;
				uint32 numImages = dirMapImages.size();
				uint32 imageSize = size * size * 4;
				uint8* buffer = new uint8[numImages * imageSize];
				for (int i = 0; i < numImages; i++)
				{
					int offset = i * imageSize;
					auto img = IO::resizeImage(dirMapImages[i], size, size);
					std::memcpy(buffer + offset, img->getRawPtr(), imageSize);
				}

				auto dirMaps = Texture2DArray::create(size, size, numImages, GL::RGBA8);
				dirMaps->upload(buffer);

				delete[] buffer;

				scene->addDirectionMaps(dirMaps);
			}
		}

		void Importer::loadSettings(RenderSettings& settings, Scene::Ptr scene)
		{
			if (metaData.find(settings.skyboxMaterial.guid) != metaData.end())
			{
				Metadata& data = metaData[settings.skyboxMaterial.guid];
				std::string content = IO::loadTxtFile(data.filePath);
				ryml::Tree root = ryml::parse(ryml::to_csubstr(content));
				int matIndex = 0;
				for (int i = 0; i < root.rootref().num_children(); i++)
				{
					if (root.docref(i).has_child("Material"))
						matIndex = i;
				}
				ryml::NodeRef stream = root.docref(matIndex);

				UnityMaterial unityMaterial;
				stream["Material"] >> unityMaterial;

				auto texMap = unityMaterial.texEnvs;
				auto floatMap = unityMaterial.floats;
				auto colorMap = unityMaterial.colors;

				Skybox skybox;

				if (texMap.find("_Tex") != texMap.end())
				{
					auto texGUID = texMap["_Tex"].texture.guid;

					if (metaData.find(texGUID) != metaData.end())
					{
						auto fn = metaData[texGUID].filePath;

						int idx = fn.find_last_of('.') + 1;
						int len = fn.length() - idx;
						std::string ext = fn.substr(idx, len);
						IO::ImageF32::Ptr panoImg;
						if (ext.compare("hdr") == 0)
							panoImg = IO::decodeHDRFromFile(fn, true);
						else if (ext.compare("exr") == 0)
							panoImg = IO::decodeEXRFromFile(fn);
						skybox.texture = panoImg->upload(false);
					}
					else
					{
						std::cout << "error: could not find skybox texture!" << std::endl;
					}
				}
				else
				{
					std::cout << "error: no skybox texture defined!" << std::endl;
				}

				if (floatMap.find("_Exposure") != floatMap.end())
					skybox.exposure = floatMap["_Exposure"];
				if (floatMap.find("_Rotation") != floatMap.end())
					skybox.rotation = 360.0f - floatMap["_Rotation"]; // convert from left to right handed
				if (colorMap.find("_Color") != colorMap.end())
					skybox.color = colorMap["_Color"];
				if (colorMap.find("_Tint") != colorMap.end())
					skybox.tint = colorMap["_Tint"];

				scene->setSkybox(skybox);
			}
			else
			{
				// TODO: fallback to default skybox
				std::cout << "error: could not find skybox material!" << std::endl;
			}
		}
	}
}
