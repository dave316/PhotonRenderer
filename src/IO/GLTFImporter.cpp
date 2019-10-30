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
		loadSkins(doc);

		if (skin.boneMapping.empty())
			loadAnimations(doc);
		else
			loadRiggedAnimations(doc);

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
			std::vector<AnimationSampler> samplers;
			auto samplersNode = animationNode.FindMember("samplers");
			for (auto& samplerNode : samplersNode->value.GetArray())
			{
				AnimationSampler sampler;
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

				NodeAnimation::Interpolation interp;
				if (interpolation.compare("STEP") == 0)
					interp = NodeAnimation::Interpolation::STEP;
				else if (interpolation.compare("LINEAR") == 0)
					interp = NodeAnimation::Interpolation::LINEAR;
				else if (interpolation.compare("CUBICSPLINE") == 0)
					interp = NodeAnimation::Interpolation::CUBIC;
				else
					interp = NodeAnimation::Interpolation::LINEAR;

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
					NodeAnimation::Ptr anim(new NodeAnimation("blub", interp, 0, minTime, maxTime, maxTime, tartetNodeIndex));
					anim->setTimes(times);
					anim->setPositions(translations);
					anim->setRotations(rotations);
					anim->setScales(scales);
					nodeAnims.push_back(anim);
				}
			}
		}
	}

	void GLTFImporter::loadRiggedAnimations(const json::Document& doc)
	{
		if (!doc.HasMember("animations"))
			return;

		for (auto& animationNode : doc["animations"].GetArray())
		{
			std::vector<AnimationSampler> samplers;
			auto samplersNode = animationNode.FindMember("samplers");
			for (auto& samplerNode : samplersNode->value.GetArray())
			{
				AnimationSampler sampler;
				sampler.input = samplerNode.FindMember("input")->value.GetInt();
				sampler.output = samplerNode.FindMember("output")->value.GetInt();
				sampler.interpolation = samplerNode.FindMember("interpolation")->value.GetString();
				samplers.push_back(sampler);
			}

			std::map<int, Channel> channels;
			float minTime = 1000.0f;
			float maxTime = 0.0f;
			int channelIndex = 0;
			auto channelsNode = animationNode.FindMember("channels");
			for (auto& channelNode : channelsNode->value.GetArray())
			{
				int samplerIndex = channelNode["sampler"].GetInt();
				auto targetNode = channelNode["target"].GetObject();
				int targetNodeIndex = targetNode["node"].GetInt();
				std::string targetPath = targetNode["path"].GetString();

				if (channels.find(targetNodeIndex) == channels.end())
					channels.insert(std::make_pair(targetNodeIndex, Channel()));

				//std::cout << "channel: " << channelIndex << " sampler: " << samplerIndex 
				//	<< " target node: " << targetNodeIndex << " path: " << targetPath 
				//	<< std::endl;
				
				int input = samplers[samplerIndex].input;
				int output = samplers[samplerIndex].output;
				std::string interpolation = samplers[samplerIndex].interpolation;

				if (targetPath.compare("translation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> translations;
					loadData(input, times);
					loadData(output, translations);

					Channel& channel = channels[targetNodeIndex];
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						channel.positions.push_back(std::make_pair(times[i], translations[i]));
					}
				}
				else if (targetPath.compare("rotation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::quat> rotations;
					loadData(input, times);
					loadData(output, rotations);

					Channel& channel = channels[targetNodeIndex];
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						channel.rotations.push_back(std::make_pair(times[i], rotations[i]));
					}
				}
				else if (targetPath.compare("scale") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> scales;
					loadData(input, times);
					loadData(output, scales);
					Channel& channel = channels[targetNodeIndex];
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);
						channel.scales.push_back(std::make_pair(times[i], scales[i]));
					}
				}
			}

			std::cout << "minTime: " << minTime << " maxTime: " << maxTime << std::endl;

			auto animation = Animation::create("blub", maxTime, 2, skin.jointMapping.size());
			for (auto it : channels)
			{
				animation->addChannel(it.first, it.second);
			}
			animations.push_back(animation);
		}
	}

	void GLTFImporter::loadSkins(const json::Document& doc)
	{
		// TODO: retrive ordering of joints from joint array, since this is important for blending the correct bones.

		if (!doc.HasMember("skins"))
			return;

		for (auto& skinNode : doc["skins"].GetArray()) // can there be multiple skins????
		{
			std::vector<int> boneJoints;
			std::vector<glm::mat4> boneMatrices;
			if (skinNode.HasMember("inverseBindMatrices"))
			{
				int accIndex = skinNode["inverseBindMatrices"].GetInt();
				loadData(accIndex, boneMatrices);
			}
			if (skinNode.HasMember("joints"))
			{
				for (auto& jointNode : skinNode["joints"].GetArray())
				{
					boneJoints.push_back(jointNode.GetInt());
				}
			}
			if (skinNode.HasMember("skeleton"))
				skin.rootNode = skinNode["skeleton"].GetInt();
			if (skinNode.HasMember("name"))
				skin.name = skinNode["name"].GetString();

			//for (int i = 0; i < boneJoints.size(); i++)
			//{
			//	std::cout << "joint: " << i << " node: " << boneJoints[i] << std::endl;
			//}

			for (int i = 0; i < boneJoints.size(); i++)
			{
				skin.boneMapping.insert(std::make_pair(boneJoints[i], boneMatrices[i]));
				skin.jointMapping.insert(std::make_pair(boneJoints[i], i));
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
				std::vector<GLuint> indices;
				std::vector<glm::u16vec4> boneIndices; // TODO check index type!
				//std::vector<GLushort> boneIndices;
				std::vector<glm::vec4> boneWeights;
				bool calcNormals = true;
				bool calcTangentSpace = true;
				auto& attributesNode = primitiveNode["attributes"];
				if (attributesNode.HasMember("POSITION"))
				{
					int accIndex = attributesNode["POSITION"].GetInt();
					loadData(accIndex, positions);
				}

				if (attributesNode.HasMember("COLOR_0"))
				{
					int accIndex = attributesNode["COLOR_0"].GetInt();
					loadData(accIndex, colors);
				}

				if (attributesNode.HasMember("NORMAL"))
				{
					int accIndex = attributesNode["NORMAL"].GetInt();
					loadData(accIndex, normals);
					calcNormals = false;
				}
				
				if (attributesNode.HasMember("TEXCOORD_0"))
				{
					int accIndex = attributesNode["TEXCOORD_0"].GetInt();
					loadData(accIndex, texCoords);
				}

				if (attributesNode.HasMember("TANGENT"))
				{
					int accIndex = attributesNode["TANGENT"].GetInt();
					loadData(accIndex, tangets);
					calcTangentSpace = false;
				}

				if (attributesNode.HasMember("JOINTS_0"))
				{
					int accIndex = attributesNode["JOINTS_0"].GetInt();
					loadData(accIndex, boneIndices);
				}

				if (attributesNode.HasMember("WEIGHTS_0"))
				{
					int accIndex = attributesNode["WEIGHTS_0"].GetInt();
					loadData(accIndex, boneWeights);
				}

				//std::cout << "bones: " << boneIndices.size() << " indices and " << boneWeights.size() << " weights " << std::endl;
				//for (int i = 0; i < boneIndices.size(); i++)
				//{
				//	float weightSum = boneWeights[i].x + boneWeights[i].y + boneWeights[i].z + boneWeights[i].w;
				//	std::cout << i << " "
				//		<< boneIndices[i].x << " " << boneIndices[i].y << " " << boneIndices[i].z << " " << boneIndices[i].w << " "
				//		<< boneWeights[i].x << " " << boneWeights[i].y << " " << boneWeights[i].z << " " << boneWeights[i].w << " "
				//		<< "sum: " << weightSum << std::endl;
				//}

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
					if (i < boneIndices.size())
					{						
						v.boneIDs = glm::vec4(boneIndices[i].x, boneIndices[i].y, boneIndices[i].z, boneIndices[i].w);
					}
					if (i < boneWeights.size())
					{
						float weightSum = boneWeights[i].x + boneWeights[i].y + boneWeights[i].z + boneWeights[i].w;
						v.boneWeights = boneWeights[i] / weightSum;
					}
					
					//if (morphTargets.size() == 2) // TODO: add support for more thant 2...
					//{
					//	v.targetPosition0 = morphTargets[0].positions[i];
					//	v.targetPosition1 = morphTargets[1].positions[i];
					//	if (i < morphTargets[0].normals.size() &&
					//		i < morphTargets[1].normals.size())
					//	{
					//		v.targetNormal0 = morphTargets[0].normals[i];
					//		v.targetNormal1 = morphTargets[1].normals[i];
					//	}
					//	if (i < morphTargets[0].tangents.size() &&
					//		i < morphTargets[1].tangents.size())
					//	{
					//		v.targetTangent0 = morphTargets[0].tangents[i];
					//		v.targetTangent1 = morphTargets[1].tangents[i];
					//	}
					//}

					surface.addVertex(v);
				}

				for (int i = 0; i < indices.size(); i += 3)
				{
					Triangle t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
					surface.addTriangle(t);
				}

				if (calcNormals)
					surface.calcNormals();

				if(calcTangentSpace)
					surface.calcTangentSpace();

				auto defaultMaterial = Material::create();
				defaultMaterial->addProperty("material.baseColorFactor", glm::vec4(1.0));
				defaultMaterial->addProperty("material.roughnessFactor", 1.0f);
				defaultMaterial->addProperty("material.metallicFactor", 0.0f);
				defaultMaterial->addProperty("material.occlusionFactor", 0.0f);
				defaultMaterial->addProperty("material.emissiveFactor", glm::vec3(0.0));
				defaultMaterial->addProperty("material.useBaseColorTex", false);
				defaultMaterial->addProperty("material.usePbrTex", false);
				defaultMaterial->addProperty("material.useNormalTex", false);
				defaultMaterial->addProperty("material.useEmissiveTex", false);

				Material::Ptr material = defaultMaterial;
				if (materialIndex < materials.size())
					material = materials[materialIndex];

				auto mesh = Mesh::create("blub", surface, 0);
				renderable->addMesh(mesh, material);

				//std::cout << "added primitives to mesh " << name << " with material index " << materialIndex << std::endl;
			}
			renderables.push_back(renderable);
		}

		std::cout << "loaded " << renderables.size() << " meshes" << std::endl;
	}

	void GLTFImporter::loadMaterials(const json::Document& doc)
	{
		if (!doc.HasMember("materials"))
			return;

		for (auto& materialNode : doc["materials"].GetArray())
		{
			auto material = Material::create();
			if (materialNode.HasMember("name"))
			{
				std::string name = materialNode["name"].GetString();
				//std::cout << "loading material " << name << std::endl;
			}
			std::string alphaMode = "OPAQUE";
			int alphaModeEnum = 0;
			if (materialNode.HasMember("alphaMode"))
			{
				alphaMode = materialNode["alphaMode"].GetString();
				//std::cout << "alpha mode: " << alphaMode << std::endl;
			}

			float cutOff = 0.0f;
			if (alphaMode.compare("MASK") == 0)
			{
				alphaModeEnum = 1;
				if (materialNode.HasMember("alphaCutoff"))
				{
					cutOff = materialNode["alphaCutoff"].GetFloat();
					//std::cout << "alpha cut off: " << cutOff << std::endl;
				}
				else
				{
					cutOff = 0.5f;
				}
			}

			if (alphaMode.compare("BLEND") == 0)
			{
				alphaModeEnum = 2;
				material->setBlending(true);
			}

			material->addProperty("material.alphaMode", alphaModeEnum);
			material->addProperty("material.alphaCutOff", cutOff);
				
			if (materialNode.HasMember("doubleSided"))
			{
				bool doubleSided = materialNode["doubleSided"].GetBool();
				std::cout << "doubleSided: " << doubleSided << std::endl;
			}
			if (materialNode.HasMember("pbrMetallicRoughness"))
			{
				const auto& pbrNode = materialNode["pbrMetallicRoughness"];
				material->addProperty("material.baseColorFactor", glm::vec4(1.0f));
				if (pbrNode.HasMember("baseColorFactor"))
				{
					const auto& baseColorNode = pbrNode["baseColorFactor"];
					auto array = baseColorNode.GetArray();
					glm::vec4 color;
					color.r = array[0].GetFloat();
					color.g = array[1].GetFloat();
					color.b = array[2].GetFloat();
					color.a = array[3].GetFloat();
					material->addProperty("material.baseColorFactor", color);
					material->addProperty("material.useBaseColorTex", false);
				}
				if (pbrNode.HasMember("baseColorTexture"))
				{
					unsigned int texIndex = pbrNode["baseColorTexture"]["index"].GetInt();
					if (texIndex < textures.size())
					{
						std::cout << "added baseColorTexture texture index " << texIndex << std::endl;
						material->addTexture("material.baseColorTex", textures[texIndex]);
						material->addProperty("material.useBaseColorTex", true);
					}
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
				}
	
				if (pbrNode.HasMember("metallicRoughnessTexture"))
				{
					unsigned int texIndex = pbrNode["metallicRoughnessTexture"]["index"].GetInt();
					if (texIndex < textures.size())
					{
						//std::cout << "added metallicRoughnessTexture texture index " << texIndex << std::endl;
						material->addTexture("material.pbrTex", textures[texIndex]);
						material->addProperty("material.usePbrTex", true);
					}						
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
					//std::cout << "implement loading of metallicRoughnessTexture" << std::endl;
				}
				else
				{
					// TODO: read these values from GLTF
					material->addProperty("material.roughnessFactor", 1.0f);
					material->addProperty("material.metallicFactor", 0.0f);
					material->addProperty("material.usePbrTex", false);
				}
			}

			if (materialNode.HasMember("normalTexture"))
			{
				unsigned int texIndex = materialNode["normalTexture"]["index"].GetInt();
				if (texIndex < textures.size())
				{
					//std::cout << "added normalTexture texture index " << texIndex << std::endl;
					material->addTexture("material.normalTex", textures[texIndex]);
					material->addProperty("material.useNormalTex", true);
				}					
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
			else
			{
				material->addProperty("material.useNormalTex", false);
			}

			// TODO: check for occlusion texture (can be packed with metalRough tex)
			if (materialNode.HasMember("occlusionTexture"))
			{
				unsigned int texIndex = materialNode["occlusionTexture"]["index"].GetInt();
				if (texIndex < textures.size())
				{
					material->addTexture("material.occlusionTex", textures[texIndex]);
					material->addProperty("material.useOcclusionTex", true);
				}					
				else
				{
					std::cout << "texture index " << texIndex << " not found" << std::endl;
				}
			}
			else
			{
				material->addProperty("material.useOcclusionTex", false);
			}

			if (materialNode.HasMember("emissiveTexture"))
			{
				unsigned int texIndex = materialNode["emissiveTexture"]["index"].GetInt();
				if (texIndex < textures.size())
				{
					//std::cout << "added emissiveTexture texture index " << texIndex << std::endl;
					material->addTexture("material.emissiveTex", textures[texIndex]);
					material->addProperty("material.useEmissiveTex", true);
				}
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
			else
			{
				material->addProperty("material.useEmissiveTex", false);
			}

			if (materialNode.HasMember("extensions"))
			{
				material->addProperty("useSpecGlossMat", true);
				const auto& extensionNode = materialNode["extensions"];
				{
					if (extensionNode.HasMember("KHR_materials_pbrSpecularGlossiness"))
					{
						const auto& pbrSpecGlossNode = extensionNode["KHR_materials_pbrSpecularGlossiness"];
						if (pbrSpecGlossNode.HasMember("diffuseFactor"))
						{
							const auto& baseColorNode = pbrSpecGlossNode["diffuseFactor"];
							auto array = baseColorNode.GetArray();
							glm::vec4 color;
							color.r = array[0].GetFloat();
							color.g = array[1].GetFloat();
							color.b = array[2].GetFloat();
							color.a = array[3].GetFloat();
							material->addProperty("material2.diffuseFactor", color);
							material->addProperty("material2.useDiffuseTex", false);
						}
						if (pbrSpecGlossNode.HasMember("diffuseTexture")) 
						{
							unsigned int texIndex = pbrSpecGlossNode["diffuseTexture"]["index"].GetInt();
							if (texIndex < textures.size())
							{
								//std::cout << "added normalTexture texture index " << texIndex << std::endl;
								material->addTexture("material2.diffuseTex", textures[texIndex]);
								material->addProperty("material2.useDiffuseTex", true);
							}
							else
								std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
						if (pbrSpecGlossNode.HasMember("specularFactor"))
						{
							const auto& baseColorNode = pbrSpecGlossNode["specularFactor"];
							auto array = baseColorNode.GetArray();
							glm::vec3 color;
							color.r = array[0].GetFloat();
							color.g = array[1].GetFloat();
							color.b = array[2].GetFloat();
							material->addProperty("material2.specularFactor", color);
							material->addProperty("material2.useSpecularTex", false);
						}
						if (pbrSpecGlossNode.HasMember("glossinessFactor"))
						{
							float glossiness = pbrSpecGlossNode["glossinessFactor"].GetFloat();
							material->addProperty("material2.glossFactor", glossiness);
							material->addProperty("material2.useSpecularTex", false);
						}
						if (pbrSpecGlossNode.HasMember("specularGlossinessTexture"))
						{
							unsigned int texIndex = pbrSpecGlossNode["specularGlossinessTexture"]["index"].GetInt();
							if (texIndex < textures.size())
							{
								//std::cout << "added normalTexture texture index " << texIndex << std::endl;
								material->addTexture("material2.specGlossTex", textures[texIndex]);
								material->addProperty("material2.useSpecularTex", true);
							}
							else
								std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
					else
					{
						std::cout << "extension not supported!" << std::endl;
					}
				}
			}
			else
				material->addProperty("useSpecGlossMat", false);

			materials.push_back(material);
		}
	}

	void GLTFImporter::loadTextures(const json::Document& doc, const std::string& path)
	{
		if (!doc.HasMember("images"))
			return;

		std::vector<std::string> imageFiles;
		for (auto& imagesNode : doc["images"].GetArray())
		{
			if (imagesNode.HasMember("uri"))
			{
				std::string filename = imagesNode["uri"].GetString();
				imageFiles.push_back(filename);
			}
			else
			{
				std::cout << "loading images from binary buffer not supported!" << std::endl;
			}
		}

		struct Sampler
		{
			int minFilter;
			int magFilter;
			int wrapS;
			int wrapT;
		};
		std::vector<Sampler> samplers;
		if (doc.HasMember("samplers")) 
		{
			for (auto& samplerNode : doc["samplers"].GetArray())
			{
				if (samplerNode.HasMember("minFilter")) // TODO: check this properly
				{
					Sampler s;
					s.minFilter = samplerNode["minFilter"].GetInt();
					s.magFilter = samplerNode["magFilter"].GetInt();
					s.wrapS = samplerNode["wrapS"].GetInt();
					s.wrapT = samplerNode["wrapT"].GetInt();
					samplers.push_back(s);
				}
			}
		}

		if (doc.HasMember("textures"))
		{
			for (auto& textureNode : doc["textures"].GetArray())
			{
				int imageIndex = textureNode["source"].GetInt();

				std::string filename = imageFiles[imageIndex];
				std::cout << "loading texture " << filename << std::endl;

				int i0 = filename.find_last_of("_") + 1;
				int i1 = filename.find_last_of(".");
				int len = i1 - i0;

				std::string mapType = filename.substr(i0, len);

				bool sRGB = false;
				if (mapType.substr(0, 9).compare("baseColor") == 0 || 
					mapType.compare("albedo") == 0 || 
					mapType.compare("emissive") == 0 ||
					mapType.compare("diffuse") == 0 ||
					mapType.compare("specularGlossiness") == 0 ||
					mapType.compare("a") == 0 ||
					mapType.compare("sg") == 0)
				{
					std::cout << "SRGB: true" << std::endl;
					sRGB = true;
				}				

				auto tex = IO::loadTexture(path + "/" + filename, sRGB);
				//tex->generateMipmaps();
				//tex->setFilter(GL::LINEAR_MIPMAP_LINEAR);
				if (textureNode.HasMember("sampler"))
				{
					int samplerIndex = textureNode["sampler"].GetInt();
					if (samplerIndex < samplers.size())
					{
						Sampler& sampler = samplers[samplerIndex];
						if (sampler.minFilter >= 9984 && sampler.minFilter <= 9987)
							tex->generateMipmaps();
						tex->setFilter(GL::TextureFilter(sampler.minFilter));
						tex->setWrap(GL::TextureWrap(sampler.wrapS));
					}
				}
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

			if (!skin.boneTree.children.empty())
				anim->setSkin(skin.boneTree);

			if (node.animIndex < animations.size())
				anim->addAnimation(animations[node.animIndex]);
			else if (node.animIndex < nodeAnims.size())
				anim->addNodeAnim(nodeAnims[node.animIndex]);
			else if (node.animIndex < morphAnims.size())
				anim->addMorphAnim(morphAnims[node.animIndex]);
			anim->setSkin(skin.boneTree);
			entity->addComponent(anim);
		}

		for (auto& index : node.children)
		{
			auto childEntity = traverse(index);
			t->addChild(childEntity->getComponent<Transform>());
		}

		entities.push_back(entity);

		return entity;
	}

	void GLTFImporter::buildBoneTree(int childIndex, BoneNode& parentNode)
	{
		GLTFNode& sceneNode = nodes[childIndex];
		glm::mat4 T = glm::translate(glm::mat4(1.0f), sceneNode.translation);
		glm::mat4 R = glm::mat4_cast(sceneNode.rotation);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), sceneNode.scale);

		BoneNode boneNode;
		boneNode.jointIndex = skin.jointMapping[childIndex];
		boneNode.boneIndex = childIndex;
		boneNode.boneTransform = skin.boneMapping[childIndex];
		boneNode.nodeTransform = T * R * S;

		parentNode.children.push_back(boneNode);

		for (int noneIndex : sceneNode.children)
		{
			buildBoneTree(noneIndex, parentNode.children[parentNode.children.size() - 1]);
		}
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

				if (node.HasMember("translation"))
					gltfNode.translation = toVec3(node["translation"]);
				if (node.HasMember("rotation"))
					gltfNode.rotation = toQuat(node["rotation"]);
				if (node.HasMember("scale"))
					gltfNode.scale = toVec3(node["scale"]);

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

		for (int i = 0; i < nodeAnims.size(); i++)
		{
			unsigned int nodeIndex = nodeAnims[i]->getNodeIndex();
			nodes[nodeIndex].animIndex = i;
		}

		for (int i = 0; i < morphAnims.size(); i++)
		{
			unsigned int nodeIndex = morphAnims[i]->getNodeIndex();
			nodes[nodeIndex].animIndex = i;
		}

		if (!skin.boneMapping.empty())
		{
			GLTFNode& sceneRoot = nodes[skin.rootNode];
			glm::mat4 T = glm::translate(glm::mat4(1.0f), sceneRoot.translation);
			glm::mat4 R = glm::mat4_cast(sceneRoot.rotation);
			glm::mat4 S = glm::scale(glm::mat4(1.0f), sceneRoot.scale);

			//for (auto it : skin.jointMapping)
			//{
			//	std::cout << it.first << " " << it.second << std::endl;
			//}

			BoneNode boneRoot;
			boneRoot.jointIndex = skin.jointMapping[skin.rootNode];
			boneRoot.boneIndex = skin.rootNode;
			boneRoot.boneTransform = skin.boneMapping[skin.rootNode];
			boneRoot.nodeTransform = T * R * S;
			boneRoot.name = sceneRoot.name;

			for (int nodeIndex : sceneRoot.children)
			{
				buildBoneTree(nodeIndex, boneRoot);
			}

			skin.boneTree = boneRoot;
		}

		//skin.boneTree.print();

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

	void GLTFImporter::clear()
	{
		buffers.clear();
		bufferViews.clear();
		accessors.clear();
		nodes.clear();
		materials.clear();
		renderables.clear();
		animators.clear();
		textures.clear();
		nodeAnims.clear();
		morphAnims.clear();
		entities.clear();
	}
}