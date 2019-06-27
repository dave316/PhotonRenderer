#include "GLTFImporter.h"

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

		auto sceneNode = doc.FindMember("scenes");
		//for (auto& scene : sceneNode->value.GetArray())
		//{
		//	for (auto& nodes : scene.FindMember("nodes")->value.GetArray())
		//	{
		//		std::cout <<  "root node: " << nodes.GetInt() << std::endl;
		//	}
		//}

		loadBuffers(doc, path);

		if(doc.HasMember("animations"))
			loadAnimations(doc);

		loadMeshes(doc);

		auto material = Material::create();
		material->setColor(glm::vec3(0.5f));

		// TODO: parse scene graphs 
		// TODO: add meshes/animations to corresponding nodes
		glm::mat4 M = glm::mat4(1.0f);
		auto entity = Entity::create("blub", M);
		auto r = Renderable::create(meshes[0], material);

		// TODO handle multiple animations!
		if(animations.size() > 0)
			r->setAnimation(animations[0]);
		if (morphAnims.size() > 0)
			r->setMorphAnim(morphAnims[0]);
		entity->addComponent(r);
		return entity;
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
		TriangleSurface surface;
		auto meshesNode = doc.FindMember("meshes");
		for (auto& meshNode : meshesNode->value.GetArray())
		{
			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				std::vector<glm::vec3> positions;
				std::vector<glm::vec3> normals;
				std::vector<GLushort> indices; // TODO check other index types!

				auto attributesNode = primitiveNode.FindMember("attributes");
				if (attributesNode->value.HasMember("POSITION"))
				{
					auto posNode = attributesNode->value.FindMember("POSITION");
					int accIndex = posNode->value.GetInt();
					loadData(accIndex, positions);
				}

				if (attributesNode->value.HasMember("NORMAL"))
				{
					auto normalNode = attributesNode->value.FindMember("NORMAL");
					int accIndex = normalNode->value.GetInt();
					loadData(accIndex, normals);
				}

				if (primitiveNode.HasMember("indices"))
				{
					auto indicesNode = primitiveNode.FindMember("indices");
					int accIndex = indicesNode->value.GetInt();
					loadData(accIndex, indices);
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
					if (i < normals.size())
						v.normal = normals[i];

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
			}			
		}

		auto mesh = Mesh::create("blub", surface, 0);
		meshes.push_back(mesh);
	}
}