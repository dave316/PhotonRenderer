#include "SceneLoader.h"
#include "GLTFImporter.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <fstream>

namespace json = rapidjson;

namespace IO
{
	namespace SceneLoader
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

		glm::vec4 toVec4(const json::Value& value)
		{
			glm::vec4 v(0.0f);
			if (value.IsArray() && value.Size() == 4)
			{
				auto array = value.GetArray();
				v.x = array[0].GetFloat();
				v.y = array[1].GetFloat();
				v.z = array[2].GetFloat();
				v.w = array[3].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to vec4" << std::endl;
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

		json::Value toJson(glm::vec3& value, json::MemoryPoolAllocator<json::CrtAllocator>& allocator)
		{
			json::Value valList;
			valList.SetArray();
			valList.PushBack(value.x, allocator);
			valList.PushBack(value.y, allocator);
			valList.PushBack(value.z, allocator);
			return valList;
		}

		json::Value toJson(glm::vec4& value, json::MemoryPoolAllocator<json::CrtAllocator>& allocator)
		{
			json::Value valList;
			valList.SetArray();
			valList.PushBack(value.x, allocator);
			valList.PushBack(value.y, allocator);
			valList.PushBack(value.z, allocator);
			valList.PushBack(value.w, allocator);
			return valList;
		}

		json::Value toJson(glm::quat& value, json::MemoryPoolAllocator<json::CrtAllocator>& allocator)
		{
			json::Value valList;
			valList.SetArray();
			valList.PushBack(value.x, allocator);
			valList.PushBack(value.y, allocator);
			valList.PushBack(value.z, allocator);
			valList.PushBack(value.w, allocator);
			return valList;
		}

		json::Value toJson(pr::Transform::Ptr transform, json::MemoryPoolAllocator<json::CrtAllocator>& allocator)
		{
			json::Value transformNode;
			transformNode.SetObject();

			auto posNode = toJson(transform->getLocalPosition(), allocator);
			auto rotNode = toJson(transform->getLocalRotation(), allocator);
			auto scaleNode = toJson(transform->getLocalScale(), allocator);
			transformNode.AddMember("position", posNode, allocator);
			transformNode.AddMember("rotation", rotNode, allocator);
			transformNode.AddMember("scale", scaleNode, allocator);

			return transformNode;
		}

		void saveScene(std::string filepath, pr::Scene::Ptr scene)
		{
			json::Document doc;
			doc.SetObject();
			auto& allocator = doc.GetAllocator();

			json::Value entitiesList;
			entitiesList.SetArray();

			for (auto root : scene->getRootNodes())
			{
				json::Value name(root->getName().c_str(), allocator);
				json::Value uri(root->getUri().c_str(), allocator);
				json::Value transform = toJson(root->getComponent<pr::Transform>(), allocator);
				
				json::Value entityNode;
				entityNode.SetObject();
				entityNode.AddMember("name", name, allocator);
				entityNode.AddMember("filename", uri, allocator);
				entityNode.AddMember("transform", transform, allocator);

				auto light = root->getComponent<pr::Light>();
				if (light)
				{
					auto type = light->getType();
					auto color = light->getColor();
					auto lumen = light->getLumen();
					auto range = light->getRange();

					json::Value lightNode;
					lightNode.SetObject();
					lightNode.AddMember("type", (int)type, allocator);
					lightNode.AddMember("color", toJson(color, allocator), allocator);
					lightNode.AddMember("lumen", lumen, allocator);
					lightNode.AddMember("range", range, allocator);

					entityNode.AddMember("light", lightNode, allocator);
				}

				entitiesList.PushBack(entityNode, allocator);
			}

			doc.AddMember("entities", entitiesList, allocator);
			json::StringBuffer buffer;
			json::PrettyWriter<json::StringBuffer> writer(buffer);
			doc.Accept(writer);

			std::ofstream file(filepath);
			file << buffer.GetString();
		}

		pr::Scene::Ptr loadScene(AssetManager& assetManager, std::string filepath)
		{
			json::Document doc;
			std::ifstream file(filepath);
			std::stringstream ss;
			ss << file.rdbuf();
			std::string content = ss.str();
			doc.Parse(content.c_str());

			auto scene = pr::Scene::create("Scene");

			for (auto& entityNode : doc["entities"].GetArray())
			{
				std::string name = entityNode["name"].GetString();
				std::string uri = entityNode["filename"].GetString();

				pr::Entity::Ptr root;
				if (uri.empty())
				{
					root = pr::Entity::create(name, nullptr);
				}					
				else
				{
					auto node = assetManager.getNodeFromPath(uri);
					root = assetManager.getEntity(node->entityIndex);
				}

				if (entityNode.HasMember("transform"))
				{
					auto& transformNode = entityNode["transform"];
					glm::vec3 pos = toVec3(transformNode["position"]);
					glm::quat rot = toQuat(transformNode["rotation"]);
					glm::vec3 scale = toVec3(transformNode["scale"]);
					auto t = root->getComponent<pr::Transform>();
					t->setLocalPosition(pos);
					t->setLocalRotation(rot);
					t->setLocalScale(scale);
				}

				if (entityNode.HasMember("light"))
				{
					auto& lightNode = entityNode["light"];
					auto type = (pr::LightType)lightNode["type"].GetInt();
					auto color = toVec3(lightNode["color"]);
					auto lumen = lightNode["lumen"].GetFloat();
					auto range = lightNode["range"].GetFloat();

					auto light = pr::Light::create(type, color, 1.0f, range);
					light->setLuminousPower(lumen);
					root->addComponent(light);
				}

				scene->addRoot(root);
			}

			return scene;
		}
	}
}