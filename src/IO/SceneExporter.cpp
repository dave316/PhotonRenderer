#include "SceneExporter.h"
#include "AssimpImporter.h"
#include "GLTFImporter.h"

#include <Graphics/MeshPrimitives.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace json = rapidjson;
namespace fs = std::filesystem;

namespace IO
{
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

	void saveScene(const std::string& filename, Scene::Ptr scene)
	{
		json::Document doc;
		doc.SetObject();
		auto& allocator = doc.GetAllocator();
		auto rootEntities = scene->getRootEntities();
		json::Value entitiesList;
		entitiesList.SetArray();
		for (auto [entityName, entity] : rootEntities)
		{
			json::Value entityNode;
			entityNode.SetObject();

			json::Value name(entityName.c_str(), allocator);
			json::Value uri(entity->getUri().c_str(), allocator);
			entityNode.AddMember("name", name, allocator);
			entityNode.AddMember("filename", uri, allocator);

			auto t = entity->getComponent<Transform>();
			glm::vec3 pos = t->getLocalPosition();
			json::Value posList;
			posList.SetArray();
			posList.PushBack(pos.x, allocator);
			posList.PushBack(pos.y, allocator);
			posList.PushBack(pos.z, allocator);
			entityNode.AddMember("position", posList, allocator);

			glm::quat rot = t->getLocalRotation();
			json::Value rotList;
			rotList.SetArray();
			rotList.PushBack(rot.x, allocator);
			rotList.PushBack(rot.y, allocator);
			rotList.PushBack(rot.z, allocator);
			rotList.PushBack(rot.w, allocator);
			entityNode.AddMember("rotation", rotList, allocator);

			glm::vec3 scale = t->getLocalScale();
			json::Value scaleList;
			scaleList.SetArray();
			scaleList.PushBack(scale.x, allocator);
			scaleList.PushBack(scale.y, allocator);
			scaleList.PushBack(scale.z, allocator);
			entityNode.AddMember("scale", scaleList, allocator);

			entitiesList.PushBack(entityNode, allocator);
		}

		doc.AddMember("entities", entitiesList, allocator);
		json::StringBuffer buffer;
		json::PrettyWriter<json::StringBuffer> writer(buffer);
		writer.SetIndent(' ', 2);
		doc.Accept(writer);
		buffer.GetString();

		std::ofstream file(filename);
		file << buffer.GetString();
	}

	Scene::Ptr loadScene(const std::string& filename)
	{
		json::Document doc;

		std::ifstream file(filename);
		std::stringstream ss;
		ss << file.rdbuf();
		std::string content = ss.str();
		doc.Parse(content.c_str());

		Scene::Ptr scene = Scene::create("");

		if(doc.HasMember("entities"))
		{
			for (auto& entityNode : doc["entities"].GetArray())
			{
				std::string name = entityNode["name"].GetString();
				std::string fn = entityNode["filename"].GetString();
				glm::vec3 pos = toVec3(entityNode["position"]);
				glm::quat rot = toQuat(entityNode["rotation"]);
				glm::vec3 scale = toVec3(entityNode["scale"]);

				auto p = fs::path(fn);
				auto ext = p.extension().string();

				Entity::Ptr entity = nullptr;
				if (!fn.empty())
				{
					if (ext.compare(".gltf") == 0 || ext.compare(".glb") == 0)
					{
						IO::glTF::Importer importer;
						entity = importer.importModel(fn);
					}
					else
					{
#ifdef WITH_ASSIMP
						IO::AssimpImporter importer;
						entity = importer.importModel(fn);
#else
						std::cout << "not supported file extension: " << ext << std::endl;
#endif
					}
				}
				else
				{
					// TODO: check if light type
					glm::vec3 lightColor = glm::vec3(1.0f);

					entity = Entity::create(name, nullptr);
					entity->addComponent(Light::create(LightType::POINT, lightColor, 10.0f, 10.0f));
					//entity->getComponent<Light>()->setConeAngles(glm::pi<float>() / 16.0f, glm::pi<float>() / 8.0f);

					auto r = Renderable::create();
					float radius = 0.05f;
					auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
					prim->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
					auto mat = getDefaultMaterial();
					mat->addProperty("material.baseColorFactor", glm::vec4(lightColor, 1.0));
					mat->addProperty("material.unlit", true);
					prim->setMaterial(mat);
					auto mesh = Mesh::create("Sphere");
					mesh->addPrimitive(prim);
					r->setMesh(mesh);
					entity->addComponent(r);
				}

				if (entity)
				{
					auto t = entity->getComponent<Transform>();
					t->setLocalPosition(pos);
					t->setLocalRotation(rot);
					t->setLocalScale(scale);
					scene->addEntity(name, entity);
				}
			}
		}
		return scene;
	}
}
