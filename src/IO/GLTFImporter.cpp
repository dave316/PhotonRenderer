#include "GLTFImporter.h"

#include <IO/ImageLoader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace IO
{
	std::vector<unsigned char> readBinaryFile(const std::string& filename, unsigned int byteLength)
	{
		std::vector<unsigned char> buffer;
		std::ifstream file(filename, std::ios::binary);
		if (file.is_open())
		{
			buffer.resize(byteLength);
			file.read((char*)buffer.data(), byteLength);
		}
		else
		{
			std::cout << "error opening file " << filename << std::endl;
		}
		return buffer;
	}

	Entity::Ptr GLTFImporter::importModel(const std::string& filename)
	{
		std::string path = filename.substr(0, filename.find_last_of('/'));

		std::ifstream file(filename);
		if(!file.is_open())
		{ 
			std::cout << "error opening file " << filename << std::endl;
			return nullptr;
		}

		std::stringstream ss;
		ss << file.rdbuf();
		std::string content = ss.str();

		json::Document doc;
		doc.Parse(content.c_str());

		loadBuffers(doc, path);
		loadTextures(doc, path);
		loadMaterials(doc);
		loadMeshes(doc);
		if(doc.HasMember("animations"))
			loadAnimations(doc);
		auto root = loadScene(doc);

		return root;
	}

	void GLTFImporter::loadBuffers(const json::Document& doc, const std::string& path)
	{
		// TODO check if members exist....
		auto buffersNode = doc.FindMember("buffers");
		for (auto& bufferNode : buffersNode->value.GetArray())
		{
			Buffer buffer;
			std::string uri(bufferNode.FindMember("uri")->value.GetString());
			unsigned int byteLength = bufferNode.FindMember("byteLength")->value.GetInt();
			buffer.data = readBinaryFile(path + "/" + uri, byteLength);
			buffers.push_back(buffer);
		}

		auto bufferViewsNode = doc.FindMember("bufferViews");
		for (auto& bufferViewNode : bufferViewsNode->value.GetArray())
		{
			BufferView bufferView;
			bufferView.buffer = bufferViewNode.FindMember("buffer")->value.GetInt();
			bufferView.byteOffset = bufferViewNode.FindMember("byteOffset")->value.GetInt();
			bufferView.byteLength = bufferViewNode.FindMember("byteLength")->value.GetInt();
			//bufferView.target = bufferViewNode.FindMember("target")->value.GetInt(); 
			bufferViews.push_back(bufferView);
		}

		auto accessorsNode = doc.FindMember("accessors");
		for (auto& accessorNode : accessorsNode->value.GetArray())
		{
			Accessor accessor;
			if(accessorNode.HasMember("bufferView"))
				accessor.bufferView = accessorNode["bufferView"].GetInt();
			if (accessorNode.HasMember("byteOffset"))
				accessor.byteOffset = accessorNode["byteOffset"].GetInt();
			accessor.componentType = accessorNode.FindMember("componentType")->value.GetInt();
			accessor.count = accessorNode.FindMember("count")->value.GetInt();
			accessor.type = accessorNode.FindMember("type")->value.GetString();
			accessors.push_back(accessor);
		}

		std::cout << "GLTF loadBuffers" << std::endl;
		std::cout << "----------------" << std::endl;
		std::cout << "Binary buffers: " << buffers.size() << std::endl;
		for (int i = 0; i < buffers.size(); i++)
			std::cout << "Buffer " << i << " " << buffers[i].data.size() << " bytes" << std::endl;
		std::cout << "BufferViews: " << bufferViews.size() << std::endl;
		std::cout << "Accesssors: " << accessors.size() << std::endl;
	}

	void GLTFImporter::loadAnimations(const json::Document& doc)
	{
		auto animationsNode = doc.FindMember("animations");
		for (auto& animationNode : animationsNode->value.GetArray())
		{
			std::vector<Sampler> samplers;
			auto samplersNode = animationNode.FindMember("samplers");
			for (auto& samplerNode : samplersNode->value.GetArray())
			{
				Sampler sampler;
				sampler.input = samplerNode.FindMember("input")->value.GetInt();
				sampler.output = samplerNode.FindMember("output")->value.GetInt();
				sampler.interpolation = samplerNode.FindMember("interpolation")->value.GetString();
				samplers.push_back(sampler);
			}

			std::vector<std::pair<float, glm::vec3>> positionKeys;
			std::vector<std::pair<float, glm::quat>> rotationKeys;
			std::vector<std::pair<float, glm::vec3>> scaleKeys;
			std::vector<std::pair<float, std::pair<float, float>>> timeWeights;
			bool isMorphAnim = false;
			float maxTime = 0.0f;
			auto channelsNode = animationNode.FindMember("channels");
			for (auto& channelNode : channelsNode->value.GetArray())
			{
				int samplerIndex = channelNode.FindMember("sampler")->value.GetInt();
				auto targetNode = channelNode.FindMember("target")->value.GetObject();
				int tartetNodeIndex = targetNode.FindMember("node")->value.GetInt();
				std::string targetPath = targetNode.FindMember("path")->value.GetString();

				int input = samplers[samplerIndex].input;
				int output = samplers[samplerIndex].output;

				if (targetPath.compare("translation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> translations;
					loadData(input, times);
					loadData(output, translations);

					for (int i = 0; i < times.size(); i++)
						positionKeys.push_back(std::make_pair(times[i], translations[i]));

				}
				else if (targetPath.compare("rotation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::quat> rotations;
					loadData(input, times);
					loadData(output, rotations);

					for (int i = 0; i < times.size(); i++)
						rotationKeys.push_back(std::make_pair(times[i], rotations[i]));

				}
				else if (targetPath.compare("scale") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> scales;
					loadData(input, times);
					loadData(output, scales);

					for (int i = 0; i < times.size(); i++)
						scaleKeys.push_back(std::make_pair(times[i], scales[i]));

				}
				else if (targetPath.compare("weights") == 0)
				{
					isMorphAnim = true;

					std::vector<float> times;
					std::vector<float> weights;
					loadData(input, times);
					loadData(output, weights);

					std::cout << "times: " << times.size() << std::endl;
					std::cout << "weights: " << weights.size() << std::endl;

					for (int i = 0; i < times.size(); i++)
					{
						maxTime = std::max(maxTime, times[i]);
						auto weight = std::make_pair(weights[i * 2], weights[i * 2 + 1]);
						timeWeights.push_back(std::make_pair(times[i], weight));
					}
				}
				else
				{
					std::cout << "ERROR: animation path " << targetPath << " not implemented!!!" << std::endl;
				}
			}

			if (isMorphAnim)
			{
				MorphAnimation::Ptr anim(new MorphAnimation("morph", 0.0f, maxTime, timeWeights));
				morphAnims.push_back(anim);
			}
			else
			{
				Animation::Ptr anim(new Animation("blub", 0, 1.0f));
				anim->setPositions(positionKeys);
				anim->setRotations(rotationKeys);
				anim->setPositions(scaleKeys);
				animations.push_back(anim);
			}
		}
	}

	void GLTFImporter::loadMeshes(const json::Document& doc)
	{
		int primitivNum = 0;

		auto meshesNode = doc.FindMember("meshes");
		for (auto& meshNode : meshesNode->value.GetArray())
		{
			auto renderable = Renderable::create();
			auto nameNode = meshNode.FindMember("name");
			std::string name = "";
			if(meshNode.HasMember("name"))
				name = nameNode->value.GetString();
			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				TriangleSurface surface;
				unsigned int materialIndex = 0;

				primitivNum++;
				
				std::vector<glm::vec3> positions;
				std::vector<glm::vec4> colors;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texCoords;
				std::vector<GLushort> indices; // TODO check other index types!

				auto attributesNode = primitiveNode.FindMember("attributes");
				if (attributesNode->value.HasMember("POSITION"))
				{
					auto posNode = attributesNode->value.FindMember("POSITION");
					int accIndex = posNode->value.GetInt();
					loadData(accIndex, positions);
				}

				if (attributesNode->value.HasMember("COLOR_0"))
				{
					auto colorNode = attributesNode->value.FindMember("COLOR_0");
					int accIndex = colorNode->value.GetInt();
					loadData(accIndex, colors);
				}

				if (attributesNode->value.HasMember("NORMAL"))
				{
					auto normalNode = attributesNode->value.FindMember("NORMAL");
					int accIndex = normalNode->value.GetInt();
					loadData(accIndex, normals);
				}
				
				if (attributesNode->value.HasMember("TEXCOORD_0"))
				{
					auto texCoordNode = attributesNode->value.FindMember("TEXCOORD_0");
					int accIndex = texCoordNode->value.GetInt();
					loadData(accIndex, texCoords);
				}

				if (primitiveNode.HasMember("indices"))
				{
					auto indicesNode = primitiveNode.FindMember("indices");
					int accIndex = indicesNode->value.GetInt();
					loadData(accIndex, indices);
				}

				if (primitiveNode.HasMember("material"))
				{
					materialIndex = primitiveNode["material"].GetInt();
				}

				struct Target
				{
					std::vector<glm::vec3> positions;
					std::vector<glm::vec3> normals;
					std::vector<glm::vec3> tangents;
				};
				std::vector<Target> morphTargets;
				if (primitiveNode.HasMember("targets"))
				{
					for (auto& targetNode : primitiveNode["targets"].GetArray())
					{
						Target t;
						if (targetNode.HasMember("POSITION"))
						{
							auto posNode = targetNode.FindMember("POSITION");
							int accIndex = posNode->value.GetInt();
							loadData(accIndex, t.positions);
						}
						if (targetNode.HasMember("NORMAL"))
						{
							auto normalNode = targetNode.FindMember("NORMAL");
							int accIndex = normalNode->value.GetInt();
							loadData(accIndex, t.normals);
						}
						if (targetNode.HasMember("TANGENT"))
						{
							auto tangentNode = targetNode.FindMember("TANGENT");
							int accIndex = tangentNode->value.GetInt();
							loadData(accIndex, t.tangents);
						}	
						morphTargets.push_back(t);
					}
				}

				for (int i = 0; i < positions.size(); i++)
				{
					Vertex v;
					v.position = positions[i];
					if (i < colors.size())
						v.color = colors[i];
					if (i < normals.size())
						v.normal = normals[i];
					if (i < texCoords.size())
						v.texCoord = texCoords[i];

					if (morphTargets.size() == 2) // TODO: add support for more thant 2...
					{
						v.targetPosition0 = morphTargets[0].positions[i];
						v.targetPosition1 = morphTargets[1].positions[i];
						if (i < morphTargets[0].normals.size() &&
							i < morphTargets[1].normals.size())
						{
							v.targetNormal0 = morphTargets[0].normals[i];
							v.targetNormal1 = morphTargets[1].normals[i];
						}
						if (i < morphTargets[0].tangents.size() &&
							i < morphTargets[1].tangents.size())
						{
							v.targetTangent0 = morphTargets[0].tangents[i];
							v.targetTangent1 = morphTargets[1].tangents[i];
						}
					}

					surface.addVertex(v);
				}

				for (int i = 0; i < indices.size(); i += 3)
				{
					Triangle t(indices[i], indices[i + 1], indices[i + 2]);
					surface.addTriangle(t);
				}

				auto defaultMaterial = Material::create();
				defaultMaterial->setColor(glm::vec3(0.5f));

				Material::Ptr material = defaultMaterial;
				if (materialIndex < materials.size())
					material = materials[materialIndex];

				auto mesh = Mesh::create("blub", surface, 0);
				renderable->addMesh(mesh, material);

				std::cout << "added primitives to mesh " << name << " with material index " << materialIndex << std::endl;
			}
			renderables.push_back(renderable);
		}

		std::cout << "loaded " << renderables.size() << " meshes" << std::endl;
		//std::cout << "loaded " << primitivNum << " primitives" << std::endl;
	}

	void GLTFImporter::loadMaterials(const json::Document& doc)
	{
		if (!doc.HasMember("materials"))
			return;

		for (auto& materialNode : doc["materials"].GetArray())
		{
			auto material = Material::create();
			if (materialNode.HasMember("pbrMetallicRoughness"))
			{
				const auto& pbrNode = materialNode["pbrMetallicRoughness"];

				if (pbrNode.HasMember("baseColorFactor"))
				{
					const auto& baseColorNode = pbrNode["baseColorFactor"];
					auto array = baseColorNode.GetArray();
					glm::vec3 color;
					color.r = array[0].GetFloat();
					color.g = array[1].GetFloat();
					color.b = array[2].GetFloat();
					material->setColor(color);
				}
				else if(pbrNode.HasMember("baseColorTexture"))
				{
					unsigned int texIndex = pbrNode["baseColorTexture"]["index"].GetInt();
					if (texIndex < textures.size())
						material->addTexture(textures[texIndex]);
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
				}
			}
			materials.push_back(material);
		}
	}

	void GLTFImporter::loadTextures(const json::Document& doc, const std::string& path)
	{
		if (!doc.HasMember("images"))
			return;

		for (auto& imagesNode : doc["images"].GetArray())
		{
			if (imagesNode.HasMember("uri"))
			{
				std::string filename = imagesNode["uri"].GetString();
				std::cout << "loading texture " << filename << std::endl;
				auto tex = IO::loadTexture(path + "/" + filename);
				textures.push_back(tex);
			}
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
			std::cout << "error parsing json array to vec3" << std::endl;
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
			std::cout << "error parsing json array to vec3" << std::endl;
		}
		return m;
	}

	Entity::Ptr GLTFImporter::traverse(int nodeIndex, glm::mat4 parentTransform)
	{
		auto node = nodes[nodeIndex];
		//glm::mat4 M = parentTransform * glm::translate(glm::mat4(1.0f), node.translation);
		glm::mat4 M = parentTransform * node.transform;

		auto entity = Entity::create(node.name, M);
		auto t = entity->getComponent<Transform>();

		if (node.meshIndex >= 0)
		{
			auto r = renderables[node.meshIndex];

			//auto defaultMaterial = Material::create();
			//defaultMaterial->setColor(glm::vec3(0.5f));

			//Material::Ptr material = defaultMaterial;
			//unsigned matIndex = mesh->getMaterialIndex();
			//if (matIndex < materials.size())
			//	material = materials[matIndex];

			//auto r = Renderable::create();
			//r->addMesh(mesh, material);
			entity->addComponent(r);
		}
		
		if (node.name.empty())
			node.name = "node_" + std::to_string(nodeIndex);

		for (auto& index : node.children)
		{
			auto childEntity = traverse(index, M);
			t->addChild(childEntity->getComponent<Transform>());
		}

		entities.push_back(entity);

		return entity;
	}

	Entity::Ptr GLTFImporter::loadScene(const json::Document& doc)
	{
		if (doc.HasMember("nodes")) // TODO: maybe check the libraries first before parsing the json at all
		{
			for (auto& node : doc["nodes"].GetArray())
			{
				GLTFNode gltfNode;
				if (node.HasMember("mesh"))
					gltfNode.meshIndex = node["mesh"].GetInt();
				glm::vec3 t(0.0f);
				glm::vec3 s(1.0f);
				glm::quat r(1.0f, 0.0f, 0.0f, 0.0f);

				if (node.HasMember("translation"))
					t = toVec3(node["translation"]);
				if (node.HasMember("rotation"))
					r = toQuat(node["rotation"]);
				if (node.HasMember("scale"))
					s = toVec3(node["scale"]);
				
				glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
				glm::mat4 R = glm::mat4_cast(r);
				glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
				gltfNode.transform = S * R * T;

				//for (int row = 0; row < 4; row++)
				//{
				//	for (int col = 0; col < 4; col++)
				//	{
				//		std::cout << R[row][col] << " ";
				//	}
				//	std::cout << std::endl;
				//}

				if (node.HasMember("matrix"))
					gltfNode.transform = toMat4(node["matrix"]);
				if (node.HasMember("children"))
				{
					for (auto& nodeIndex : node["children"].GetArray())
					{
						gltfNode.children.push_back(nodeIndex.GetInt());
					}
				}
				nodes.push_back(gltfNode);
			}
		}

		glm::mat4 M(1.0f);
		auto root = Entity::create("root", M);
		auto rootTransform = root->getComponent<Transform>();
		if (doc.HasMember("scenes") && doc["scenes"][0].HasMember("nodes"))
		{
			for (auto& nodeIndex : doc["scenes"][0]["nodes"].GetArray())
			{
				auto childEntity = traverse(nodeIndex.GetInt(), M);
				rootTransform->addChild(childEntity->getComponent<Transform>());
			}
		}

		return root;
	}

	std::vector<Entity::Ptr> GLTFImporter::getEntities()
	{
		return entities;
	}
}