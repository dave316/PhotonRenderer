#include "GLTFImporter.h"

#include <IO/ImageLoader.h>
#include <glm/gtx/matrix_decompose.hpp>
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
		// TODO there has to be a sepperation of node animations and skin animations
		if (!doc.HasMember("animations"))
			return;

		for (auto& animationNode : doc["animations"].GetArray())
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

			// TODO: this needs to be changed since a node can be target of multiple channels (translation, rotation,...)
			auto channelsNode = animationNode.FindMember("channels");
			for (auto& channelNode : channelsNode->value.GetArray())
			{
				//std::vector<std::pair<float, glm::vec3>> positionKeys;
				//std::vector<std::pair<float, glm::quat>> rotationKeys;
				//std::vector<std::pair<float, glm::vec3>> scaleKeys;
				std::vector<std::pair<float, std::pair<float, float>>> timeWeights;
				bool isMorphAnim = false;
				float minTime = 1000.0f;
				float maxTime = 0.0f;
				int tartetNodeIndex = -1;

				int samplerIndex = channelNode.FindMember("sampler")->value.GetInt();
				auto targetNode = channelNode.FindMember("target")->value.GetObject();
				tartetNodeIndex = targetNode.FindMember("node")->value.GetInt();
				std::string targetPath = targetNode.FindMember("path")->value.GetString();

				int input = samplers[samplerIndex].input;
				int output = samplers[samplerIndex].output;
				std::string interpolation = samplers[samplerIndex].interpolation;

				Animation::Interpolation interp;
				if (interpolation.compare("STEP") == 0)
					interp = Animation::Interpolation::STEP;
				else if (interpolation.compare("LINEAR") == 0)
					interp = Animation::Interpolation::LINEAR;
				else if (interpolation.compare("CUBICSPLINE") == 0)
					interp = Animation::Interpolation::CUBIC;
				else
					interp = Animation::Interpolation::LINEAR;

				std::vector<float> times;
				std::vector<glm::vec3> translations;
				std::vector<glm::quat> rotations;
				std::vector<glm::vec3> scales;

				if (targetPath.compare("translation") == 0)
				{
					//std::vector<float> times;
					//std::vector<glm::vec3> translations;
					times.clear();
					loadData(input, times);
					loadData(output, translations);

					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						//positionKeys.push_back(std::make_pair(times[i], translations[i]));
					}
				}
				else if (targetPath.compare("rotation") == 0)
				{
					//std::vector<float> times;
					//std::vector<glm::quat> rotations;
					times.clear();
					loadData(input, times);
					loadData(output, rotations);

					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						//rotationKeys.push_back(std::make_pair(times[i], rotations[i]));
					}
				}
				else if (targetPath.compare("scale") == 0)
				{
					//std::vector<float> times;
					//std::vector<glm::vec3> scales;
					times.clear();
					loadData(input, times);
					loadData(output, scales);

					//std::cout << "times: " << times.size() << " scales: " << scales.size() << std::endl;

					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						//scaleKeys.push_back(std::make_pair(times[i], scales[i]));
					}
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
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						auto weight = std::make_pair(weights[i * 2], weights[i * 2 + 1]);
						timeWeights.push_back(std::make_pair(times[i], weight));
					}
				}
				else
				{
					std::cout << "ERROR: animation path " << targetPath << " not implemented!!!" << std::endl;
				}

				if (isMorphAnim)
				{
					MorphAnimation::Ptr anim(new MorphAnimation("morph", 0.0f, maxTime, tartetNodeIndex, timeWeights));
					morphAnims.push_back(anim);
				}
				else
				{
					// TODO figure out how to deal with different start/end times
					Animation::Ptr anim(new Animation("blub", interp, 0, minTime, maxTime, maxTime, tartetNodeIndex));
					anim->setTimes(times);
					anim->setPositions(translations);
					anim->setRotations(rotations);
					anim->setScales(scales);
					animations.push_back(anim);
				}
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
				std::vector<glm::vec4> colors; // TODO colors can be RGB or RGBA!!!
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texCoords;
				std::vector<glm::vec4> tangets;
				std::vector<GLuint> indices; // TODO check other index types!

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

				if (attributesNode->value.HasMember("TANGENT"))
				{
					auto tangentNode = attributesNode->value.FindMember("TANGENT");
					int accIndex = tangentNode->value.GetInt();
					loadData(accIndex, tangets);
				}

				if (primitiveNode.HasMember("indices"))
				{
					auto indicesNode = primitiveNode.FindMember("indices");
					int accIndex = indicesNode->value.GetInt();
					int type = accessors[accIndex].componentType;
					switch (type)
					{
					case GL_UNSIGNED_BYTE:
					{
						std::vector<GLubyte> byteIndices;
						loadData(accIndex, byteIndices);
						for (auto i : byteIndices)
							indices.push_back(i);
						break;
					}
					case GL_UNSIGNED_SHORT:
					{
						std::vector<GLushort> shortIndices;
						loadData(accIndex, shortIndices);
						for (auto i : shortIndices)
							indices.push_back(i);
						break;
					}
					case GL_UNSIGNED_INT:
						loadData(accIndex, indices);
						break;
					default:
						std::cout << "index type not supported!!!" << std::endl;
						break;
					}
					
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
					Vertex v(positions[i]);
					if (i < colors.size())
						v.color = colors[i];
					if (i < normals.size())
						v.normal = normals[i];
					if (i < texCoords.size())
						v.texCoord = texCoords[i];
					if (i < tangets.size())
						v.tangent = tangets[i];
					
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
					Triangle t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
					surface.addTriangle(t);
				}

				auto defaultMaterial = Material::create();
				defaultMaterial->setColor(glm::vec4(1.0f));

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
					glm::vec4 color;
					color.r = array[0].GetFloat();
					color.g = array[1].GetFloat();
					color.b = array[2].GetFloat();
					color.a = array[3].GetFloat();
					material->setColor(color);
				}
				else if(pbrNode.HasMember("baseColorTexture"))
				{
					unsigned int texIndex = pbrNode["baseColorTexture"]["index"].GetInt();
					//unsigned int texIndex = 0;
					if (texIndex < textures.size())
						material->addTexture(textures[texIndex]);
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
				}
				else
				{
					material->setColor(glm::vec4(1.0f));
				}

				if (pbrNode.HasMember("metallicRoughnessTexture"))
				{
					unsigned int texIndex = pbrNode["metallicRoughnessTexture"]["index"].GetInt();
					if (texIndex < textures.size())
						material->addTexture(textures[texIndex]);
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
					//std::cout << "implement loading of metallicRoughnessTexture" << std::endl;
				}
			}

			if (materialNode.HasMember("normalTexture"))
			{
				unsigned int texIndex = materialNode["normalTexture"]["index"].GetInt();
				if (texIndex < textures.size())
					material->addTexture(textures[texIndex]);
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}

			//if (materialNode.HasMember("occlusionTexture"))
			//{
			//	unsigned int texIndex = materialNode["occlusionTexture"]["index"].GetInt();
			//	if (texIndex < textures.size())
			//		material->addTexture(textures[texIndex]);
			//	else
			//		std::cout << "texture index " << texIndex << " not found" << std::endl;
			//}

			if (materialNode.HasMember("emissiveTexture"))
			{
				unsigned int texIndex = materialNode["emissiveTexture"]["index"].GetInt();
				if (texIndex < textures.size())
					material->addTexture(textures[texIndex]);
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
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

				int i0 = filename.find_first_of("_") + 1;
				int i1 = filename.find_first_of(".");
				int len = i1 - i0;

				std::string mapType = filename.substr(i0, len);

				bool sRGB = false;
				if (mapType.compare("baseColor") == 0 || mapType.compare("albedo") == 0)
				{
					std::cout << "SRGB: true" << std::endl;
					sRGB = true;
				}				

				auto tex = IO::loadTexture(path + "/" + filename, sRGB);
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

	Entity::Ptr GLTFImporter::traverse(int nodeIndex)
	{
		auto node = nodes[nodeIndex];
		//glm::mat4 M = parentTransform * glm::translate(glm::mat4(1.0f), node.translation);
		//glm::mat4 M = node.transform;

		if (node.name.empty())
			node.name = "node_" + std::to_string(nodeIndex);

		auto entity = Entity::create(node.name);
		auto t = entity->getComponent<Transform>();
		t->setPosition(node.translation);
		t->setRotation(node.rotation);
		t->setScale(node.scale);

		if (node.meshIndex >= 0)
		{
			auto r = renderables[node.meshIndex];
			entity->addComponent(r);
		}

		if (node.animIndex >= 0)
		{
			auto anim = Animator::create();

			if (node.animIndex < animations.size())
				anim->addAnimation(animations[node.animIndex]);
			else if (node.animIndex < morphAnims.size())
				anim->addMorphAnim(morphAnims[node.animIndex]);
			entity->addComponent(anim);
		}

		//std::cout << "node " << node.name << std::endl;
		//std::cout << "transformation:" << std::endl;
		//for (int row = 0; row < 4; row++)
		//{
		//	for (int col = 0; col < 4; col++) 
		//	{
		//		std::cout << M[row][col] << " ";
		//	}
		//	std::cout << std::endl;
		//}

		for (auto& index : node.children)
		{
			auto childEntity = traverse(index);
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
				//glm::vec3 t(0.0f);
				//glm::vec3 s(1.0f);
				//glm::quat r(1.0f, 0.0f, 0.0f, 0.0f);

				if (node.HasMember("translation"))
					gltfNode.translation = toVec3(node["translation"]);
				if (node.HasMember("rotation"))
					gltfNode.rotation = toQuat(node["rotation"]);
				if (node.HasMember("scale"))
					gltfNode.scale = toVec3(node["scale"]);
				
				//glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
				//glm::mat4 R = glm::mat4_cast(r);
				//glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
				//gltfNode.transform = T * R * S;

				if (node.HasMember("matrix"))
				{
					
					glm::mat4 M = toMat4(node["matrix"]);
					glm::vec3 skew;
					glm::vec4 persp;
					glm::decompose(M, gltfNode.scale, gltfNode.rotation, gltfNode.translation, skew, persp);
				}					

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

 		for (int i = 0; i < animations.size(); i++)
		{
			unsigned int nodeIndex = animations[i]->getNodeIndex();
			nodes[nodeIndex].animIndex = i;
		}

		for (int i = 0; i < morphAnims.size(); i++)
		{
			unsigned int nodeIndex = morphAnims[i]->getNodeIndex();
			nodes[nodeIndex].animIndex = i;
		}

		auto root = Entity::create("root");
		auto rootTransform = root->getComponent<Transform>();
		if (doc.HasMember("scenes") && doc["scenes"][0].HasMember("nodes"))
		{
			for (auto& nodeIndex : doc["scenes"][0]["nodes"].GetArray())
			{
				auto childEntity = traverse(nodeIndex.GetInt());
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