#include "GLTFImporter.h"
#include <filesystem>
#include <fstream>
#include <sstream>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fs = std::filesystem;

namespace IO
{
	namespace glTF
	{
		Importer::Importer()
		{
			supportedExtensions.insert("KHR_animation_pointer");
			supportedExtensions.insert("KHR_lights_punctual");
			supportedExtensions.insert("KHR_materials_clearcoat");
			supportedExtensions.insert("KHR_materials_emissive_strength");
			supportedExtensions.insert("KHR_materials_ior");
			supportedExtensions.insert("KHR_materials_iridescence");
			supportedExtensions.insert("KHR_materials_pbrSpecularGlossiness");
			supportedExtensions.insert("KHR_materials_sheen");
			supportedExtensions.insert("KHR_materials_specular");
			supportedExtensions.insert("KHR_materials_transmission");
			supportedExtensions.insert("KHR_materials_diffuse_transmission");
			supportedExtensions.insert("KHR_materials_unlit");
			supportedExtensions.insert("KHR_materials_variants");
			supportedExtensions.insert("KHR_materials_volume");
			supportedExtensions.insert("KHR_mesh_quantization");
			supportedExtensions.insert("KHR_texture_transform");

#ifdef WITH_DRACO
			supportedExtensions.insert("KHR_draco_mesh_compression");
#endif // WITH_DRACO

#ifdef WITH_KTX
			supportedExtensions.insert("KHR_texture_basisu");
#endif // WITH_KTX
		}

		Entity::Ptr Importer::importModel(const std::string& filepath)
		{
			auto p = fs::path(filepath);
			auto path = p.parent_path().string();
			auto filename = p.filename().string();
			auto extension = p.extension().string();
			int lastDot = filename.find_last_of('.');
			int extIndex = lastDot + 1;
			std::string name = filename.substr(0, lastDot);

			json::Document doc;
			if (extension.compare(".gltf") == 0)
			{
				std::ifstream file(filepath);
				if (!file.is_open())
				{
					std::cout << "error opening file " << filepath << std::endl;
					return nullptr;
				}

				std::stringstream ss;
				ss << file.rdbuf();
				std::string content = ss.str();
				doc.Parse(content.c_str());

				binData.loadBufferFromJson(doc, path);
			}
			else if (extension.compare(".glb") == 0)
			{
				std::string content = loadGLB(filepath);
				doc.Parse(content.c_str());
			}
			else
			{
				std::cout << "unknown file extension " << extension << std::endl;
				return nullptr;
			}

			if (!checkExtensions(doc))
			{
				std::cout << "required extension not supported, model cannot be loaded!" << std::endl;
				return nullptr;
			}

			loadExtensionData(doc);

			binData.loadBufferViews(doc);
			binData.loadAccessors(doc);
			imgData.loadData(doc, path, binData);
			
			if (doc.HasMember("materials"))
			{
				for (auto& materialNode : doc["materials"].GetArray())
				{
					auto mat = loadMaterial(imgData, materialNode);
					materials.push_back(mat);
				}
			}

			if (doc.HasMember("meshes"))
			{
				for (auto& meshNode : doc["meshes"].GetArray())
				{
					auto mesh = loadMesh(meshNode, binData, materials);
					for (auto v : variants)
						mesh->addVariant(v);
					meshes.push_back(mesh);
				}
			}

			if (doc.HasMember("skins"))
			{
				for (auto& skinNode : doc["skins"].GetArray())
				{
					auto skin = loadSkin(skinNode, binData);
					skins.push_back(skin);
				}
			}

			if (doc.HasMember("animations"))
			{
				for (auto& animNode : doc["animations"].GetArray())
				{
					auto anim = loadAnimation(animNode, binData);
					animations.push_back(anim);
				}
			}

			loadCameras(doc);

			auto root = loadScene(name, doc);

			root->setURI(filepath);

			return root;
		}

		unsigned int getUInt32FromBuffer(unsigned char* buf, int index)
		{
			unsigned int value = 0;
			int shiftValue = 0;
			for (int i = 0; i < 4; i++)
			{
				value |= (int)buf[index + i] << shiftValue;
				shiftValue += 8;
			}
			return value;
		}

		std::string Importer::loadGLB(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::binary | std::ios::ate);
			int size = file.tellg();
			file.seekg(0, std::ios::beg);
			std::vector<unsigned char> buffer(size);
			file.read((char*)buffer.data(), buffer.size());

			// header
			int baseIndex = 0;
			std::string magic(5, '\0');
			for (int i = 0; i < 4; i++)
				magic[i] = buffer[baseIndex + i];
			unsigned int version = getUInt32FromBuffer(buffer.data(), baseIndex + 4);
			unsigned int length = getUInt32FromBuffer(buffer.data(), baseIndex + 8);
			baseIndex += 12;

			// chunks
			unsigned int chunkLen = getUInt32FromBuffer(buffer.data(), baseIndex);
			baseIndex += 4;
			std::string type(5, '\0');
			for (int i = 0; i < 4; i++)
				type[i] = buffer[baseIndex + i];
			baseIndex += 4;
			std::string content(buffer.begin() + baseIndex, buffer.begin() + baseIndex + chunkLen);
			baseIndex += chunkLen;

			// TODO: check first if bin chunk exists...
			chunkLen = getUInt32FromBuffer(buffer.data(), baseIndex);
			baseIndex += 4;
			std::string type2(5, '\0');
			for (int i = 0; i < 4; i++)
				type2[i] = buffer[baseIndex + i];
			baseIndex += 4;

			Buffer binChunk;
			binChunk.data.resize(chunkLen);
			std::copy(buffer.begin() + baseIndex, buffer.begin() + baseIndex + chunkLen, binChunk.data.begin());
			binData.loadBufferFromBinary(binChunk);

			return content;
		}

		bool Importer::checkExtensions(const json::Document& doc)
		{
			// TODO: check required extensions
			if (doc.HasMember("extensionsUsed"))
			{
				auto extensionsNode = doc.FindMember("extensionsUsed");
				for (auto& extNode : extensionsNode->value.GetArray())
				{
					std::string extension(extNode.GetString());
					if (supportedExtensions.find(extension) == supportedExtensions.end())
						std::cout << "extension " << extension << " not supported" << std::endl;
				}
			}

			if (doc.HasMember("extensionsRequired"))
			{
				auto extensionsNode = doc.FindMember("extensionsRequired");
				for (auto& extNode : extensionsNode->value.GetArray())
				{
					std::string extension(extNode.GetString());
					if (supportedExtensions.find(extension) == supportedExtensions.end())
					{
						std::cout << "extension " << extension << " not supported" << std::endl;
						return false;
					}
				}
			}
			return true;
		}

		void Importer::loadExtensionData(const json::Document& doc)
		{
			if (doc.HasMember("extensions"))
			{
				const auto& extensionsNode = doc["extensions"];
				if (extensionsNode.HasMember("KHR_lights_punctual"))
				{
					auto& lightsPunctualNode = extensionsNode["KHR_lights_punctual"];
					if (lightsPunctualNode.HasMember("lights"))
					{
						for (auto& lightNode : lightsPunctualNode["lights"].GetArray())
						{
							glm::vec3 pos(0);
							glm::vec3 color(1.0);
							float intensity = 1.0f;
							float range = -1.0f; // -1 means infinity
							float inner = 0.0f;
							float outer = glm::pi<float>() / 4.0f;
							int type = 0;

							if (lightNode.HasMember("color"))
							{
								auto array = lightNode["color"].GetArray();
								color.r = array[0].GetFloat();
								color.g = array[1].GetFloat();
								color.b = array[2].GetFloat();
							}

							if (lightNode.HasMember("intensity"))
								intensity = lightNode["intensity"].GetFloat();

							if (lightNode.HasMember("range"))
								range = lightNode["range"].GetFloat();

							if (lightNode.HasMember("type"))
							{
								std::string typeStr = lightNode["type"].GetString();
								if (typeStr.compare("directional") == 0)
									type = 0;
								else if (typeStr.compare("point") == 0)
									type = 1;
								else if (typeStr.compare("spot") == 0)
									type = 2;
							}

							auto light = Light::create(type, color, intensity, range);

							if (lightNode.HasMember("spot"))
							{
								inner = lightNode["spot"]["innerConeAngle"].GetFloat();
								outer = lightNode["spot"]["outerConeAngle"].GetFloat();
								light->setConeAngles(inner, outer);
							}

							lights.push_back(light);
						}
					}
				}

				if (extensionsNode.HasMember("KHR_materials_variants"))
				{
					int index = 0;
					auto& variantsNode = extensionsNode["KHR_materials_variants"];
					for (auto& variantNode : variantsNode["variants"].GetArray())
					{
						if (variantNode.HasMember("name"))
							variants.push_back(variantNode["name"].GetString());
						else
							variants.push_back("variant_" + std::to_string(index));
						index++;
					}
				}

				//if (extensionsNode.HasMember("KHR_displaymapping_pq"))
				//{
				//	needsPQ = true;
				//}
			}
		}

		void Importer::loadCameras(const json::Document& doc)
		{
			if (!doc.HasMember("cameras"))
				return;

			auto camerasNode = doc.FindMember("cameras");
			int cameraIndex = 0;
			for (auto& cameraNode : camerasNode->value.GetArray())
			{
				std::string name;
				if (cameraNode.HasMember("name"))
					name = cameraNode["name"].GetString();
				else
					name = "Camera_" + std::to_string(cameraIndex);

				std::string type = cameraNode["type"].GetString();
				if (type.compare("perspective") == 0)
				{
					float aspect = 16.0f / 9.0f;
					float yfov = glm::pi<float>();
					float znear = 0.1f;
					float zfar = 1.0f;

					auto& perspectiveNode = cameraNode["perspective"];
					if (perspectiveNode.HasMember("aspectRatio"))
						aspect = perspectiveNode["aspectRatio"].GetFloat();
					if (perspectiveNode.HasMember("yfov"))
						yfov = perspectiveNode["yfov"].GetFloat();
					if (perspectiveNode.HasMember("znear"))
						znear = perspectiveNode["znear"].GetFloat();
					if (perspectiveNode.HasMember("zfar"))
						zfar = perspectiveNode["zfar"].GetFloat();

					auto camera = Camera::create(Camera::PERSPECTIVE, name);
					camera->setFieldOfView(yfov);
					camera->setAspectRatio(aspect);
					camera->setNearPlane(znear);
					camera->setFarPlane(zfar);
					cameras.push_back(camera);
				}
				else if (type.compare("orthographic") == 0)
				{
					float xmag = 1.0f;
					float ymag = 1.0f;
					float znear = 0.1f;
					float zfar = 1.0f;
					auto& perspectiveNode = cameraNode["orthographic"];
					if (perspectiveNode.HasMember("xmag"))
						xmag = perspectiveNode["xmag"].GetFloat();
					if (perspectiveNode.HasMember("ymag"))
						ymag = perspectiveNode["ymag"].GetFloat();
					if (perspectiveNode.HasMember("znear"))
						znear = perspectiveNode["znear"].GetFloat();
					if (perspectiveNode.HasMember("zfar"))
						zfar = perspectiveNode["zfar"].GetFloat();

					auto camera = Camera::create(Camera::ORTHOGRAPHIC, name);
					camera->setOrthoProjection(xmag, ymag);
					camera->setAspectRatio(ymag / xmag);
					camera->setNearPlane(znear);
					camera->setFarPlane(zfar);
					cameras.push_back(camera);
				}
				else
				{
					std::cout << "unknow camera type " << type << std::endl;
				}

				cameraIndex++;
			}
		}

		glm::vec3 toVec3(const json::Value& value)
		{
			glm::vec3 v(0.0f);
			if (value.IsArray() && value.Size() == 3)
			{
				auto array = value.GetArray();
				v.x = array[0].GetFloat();
				v.y = array[1].GetFloat();
				v.z = array[2].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to vec3" << std::endl;
			}
			return v;
		}

		glm::quat toQuat(const json::Value& value)
		{
			glm::quat q = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			if (value.IsArray() && value.Size() == 4)
			{
				auto array = value.GetArray();
				q.x = array[0].GetFloat();
				q.y = array[1].GetFloat();
				q.z = array[2].GetFloat();
				q.w = array[3].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to quaternion" << std::endl;
			}
			return q;
		}

		glm::mat4 toMat4(const json::Value& value)
		{
			glm::mat4 m(0.0f);
			if (value.IsArray() && value.Size() == 16)
			{
				float* values = glm::value_ptr<float>(m);
				for (int i = 0; i < value.GetArray().Size(); i++)
					values[i] = value[i].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to mat4" << std::endl;
			}
			return m;
		}

		Entity::Ptr Importer::traverse(int nodeIndex, Entity::Ptr parent)
		{
			auto node = nodes[nodeIndex];
			if (node.name.empty())
				node.name = "node_" + std::to_string(nodeIndex);

			auto entity = Entity::create(node.name, parent);
			auto t = entity->getComponent<Transform>();
			t->setLocalPosition(node.translation);
			t->setLocalRotation(node.rotation);
			t->setLocalScale(node.scale);

			if (node.mesh >= 0)
			{
				auto renderable = Renderable::create();
				auto mesh = meshes[node.mesh];
				
				// neg. scale results in wrongly oriented faces!
				if (node.scale.x < 0 || node.scale.y < 0 || node.scale.z < 0)
					mesh->flipWindingOrder();

				renderable->setMesh(mesh);
				entity->addComponent(renderable);
			}

			if (node.skin >= 0)
			{
				auto skin = skins[node.skin];
				skin->setSkeleton(nodeIndex);

				auto r = entity->getComponent<Renderable>();
				if (r)
					r->setSkin(skin);
			}

			if (node.camera >= 0)
				entity->addComponent(cameras[node.camera]);

			if (node.light >= 0)
				entity->addComponent(lights[node.light]);

			for (auto& index : node.children)
			{
				auto childEntity = traverse(index, entity);
				entity->addChild(childEntity);
			}

			entities[nodeIndex] = entity;

			return entity;
		}

		Entity::Ptr Importer::loadScene(std::string name, const json::Document& doc)
		{
			if (doc.HasMember("nodes")) // TODO: maybe check the libraries first before parsing the json at all
			{
				for (auto& jsonNode : doc["nodes"].GetArray())
				{
					Node node;
					if (jsonNode.HasMember("name"))
						node.name = jsonNode["name"].GetString();
					if (jsonNode.HasMember("camera"))
						node.camera = jsonNode["camera"].GetInt();
					if (jsonNode.HasMember("mesh"))
						node.mesh = jsonNode["mesh"].GetInt();
					if (jsonNode.HasMember("skin"))
						node.skin = jsonNode["skin"].GetInt();
					if (jsonNode.HasMember("translation"))
						node.translation = toVec3(jsonNode["translation"]);
					if (jsonNode.HasMember("rotation"))
						node.rotation = toQuat(jsonNode["rotation"]);
					if (jsonNode.HasMember("scale"))
						node.scale = toVec3(jsonNode["scale"]);

					if (jsonNode.HasMember("matrix"))
					{
						glm::mat4 M = toMat4(jsonNode["matrix"]);
						glm::vec3 skew;
						glm::vec4 persp;
						glm::decompose(M, node.scale, node.rotation, node.translation, skew, persp);
					}

					if (jsonNode.HasMember("children"))
					{
						for (auto& nodeIndex : jsonNode["children"].GetArray())
						{
							node.children.push_back(nodeIndex.GetInt());
						}
					}

					if (jsonNode.HasMember("extensions"))
					{
						auto& extensionNode = jsonNode["extensions"];
						if (extensionNode.HasMember("KHR_lights_punctual"))
							node.light = extensionNode["KHR_lights_punctual"]["light"].GetInt();
					}

					nodes.push_back(node);
				}
			}

			// TODO: if one root node is present, use it as root node
			// TODO: if more than one root node is present, create a empty root node and add all nodes as children
			auto rootNode = Entity::create(name, nullptr);
			entities.resize(nodes.size());
			entities[0] = rootNode;

			int rootNodeIndex = 0;

			if (doc.HasMember("scenes"))
			{
				auto& scenesNode = doc["scenes"];
				if (scenesNode.GetArray().Size() > 1)
					std::cout << "error more than one GLTF scenes not supported!" << std::endl;
				if (scenesNode[0].HasMember("nodes"))
				{
					auto& nodesNode = scenesNode[0]["nodes"];
					for (auto& nodeIndex : nodesNode.GetArray())
					{
						rootNodeIndex = nodeIndex.GetInt();
						auto childEntity = traverse(rootNodeIndex, rootNode);
						rootNode->addChild(childEntity);
					}
				}
			}

			if (!animations.empty())
			{
				bool playAll = (rootNode->numChildren() > 1 && skins.empty());
				auto animator = Animator::create(playAll);
				animator->setNodes(entities);
				animator->setCameras(cameras);
				animator->setLights(lights);
				animator->setMaterials(materials);
				for (auto a : animations)
					animator->addAnimation(a);
				rootNode->addComponent(animator);
			}

			return rootNode;
		}

		std::map<std::string, int> Importer::getGeneralStats()
		{
			std::map<std::string, int> info;
			info.insert(std::make_pair("Nodes", nodes.size()));
			info.insert(std::make_pair("Meshes", meshes.size()));
			info.insert(std::make_pair("Materials", materials.size()));
			info.insert(std::make_pair("Cameras", cameras.size()));
			info.insert(std::make_pair("Lights", lights.size()));
			info.insert(std::make_pair("Animations", animations.size()));
			info.insert(std::make_pair("Skins", skins.size()));
			return info;
		}

		std::map<std::string, int> Importer::getRenderingStats()
		{
			int numVertices = 0;
			int numTriangles = 0;
			int numPrimitives = 0;
			for (auto m : meshes)
			{
				numVertices += m->getNumVertices();
				numTriangles += m->getNumTriangles();
				numPrimitives += m->getNumPrimitives();
			}
			std::map<std::string, int> info;
			info.insert(std::make_pair("Primitives (Drawcalls)", numPrimitives));
			info.insert(std::make_pair("Vertices", numVertices));
			info.insert(std::make_pair("Triangles", numTriangles));
			return info;
		}

		void Importer::clear()
		{
			nodes.clear();
			animations.clear();
			meshes.clear();
			materials.clear();
			cameras.clear();
			lights.clear();
			skins.clear();
			entities.clear();
		}
	}
}