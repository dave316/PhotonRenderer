#include "GLTFImporter.h"

#include <IO/ImageLoader.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <set>
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
		loadMaterials(doc, path);
		loadMeshes(doc);
		loadSkins(doc);
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
			if (bufferViewNode.HasMember("byteOffset"))
				bufferView.byteOffset = bufferViewNode.FindMember("byteOffset")->value.GetInt();
			if (bufferViewNode.HasMember("byteStride"))
			{
				bufferView.byteStride = bufferViewNode.FindMember("byteStride")->value.GetInt();
				//std::cout << "Byte stride: " << bufferView.byteStride << std::endl;
			}
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

		//std::cout << "GLTF loadBuffers" << std::endl;
		//std::cout << "----------------" << std::endl;
		//std::cout << "Binary buffers: " << buffers.size() << std::endl;
		//for (int i = 0; i < buffers.size(); i++)
		//	std::cout << "Buffer " << i << " " << buffers[i].data.size() << " bytes" << std::endl;
		//std::cout << "BufferViews: " << bufferViews.size() << std::endl;
		//std::cout << "Accesssors: " << accessors.size() << std::endl;
	}

	//void GLTFImporter::loadAnimations(const json::Document& doc)
	//{
	//	// TODO there has to be a sepperation of node animations and skin animations
	//	if (!doc.HasMember("animations"))
	//		return;

	//	for (auto& animationNode : doc["animations"].GetArray())
	//	{
	//		std::vector<AnimationSampler> samplers;
	//		auto samplersNode = animationNode.FindMember("samplers");
	//		for (auto& samplerNode : samplersNode->value.GetArray())
	//		{
	//			AnimationSampler sampler;
	//			sampler.input = samplerNode.FindMember("input")->value.GetInt();
	//			sampler.output = samplerNode.FindMember("output")->value.GetInt();
	//			if(samplerNode.HasMember("interpolation"))
	//				sampler.interpolation = samplerNode.FindMember("interpolation")->value.GetString();
	//			samplers.push_back(sampler);
	//		}

	//		// TODO: this needs to be changed since a node can be target of multiple channels (translation, rotation,...)
	//		auto channelsNode = animationNode.FindMember("channels");
	//		for (auto& channelNode : channelsNode->value.GetArray())
	//		{
	//			//std::vector<std::pair<float, glm::vec3>> positionKeys;
	//			//std::vector<std::pair<float, glm::quat>> rotationKeys;
	//			//std::vector<std::pair<float, glm::vec3>> scaleKeys;
	//			std::vector<std::pair<float, std::pair<float, float>>> timeWeights;
	//			bool isMorphAnim = false;
	//			float minTime = 1000.0f;
	//			float maxTime = 0.0f;
	//			int tartetNodeIndex = -1;

	//			int samplerIndex = channelNode.FindMember("sampler")->value.GetInt();
	//			auto targetNode = channelNode.FindMember("target")->value.GetObject();
	//			tartetNodeIndex = targetNode.FindMember("node")->value.GetInt();
	//			std::string targetPath = targetNode.FindMember("path")->value.GetString();

	//			int input = samplers[samplerIndex].input;
	//			int output = samplers[samplerIndex].output;
	//			std::string interpolation = samplers[samplerIndex].interpolation;

	//			NodeAnimation::Interpolation interp;
	//			if (interpolation.compare("STEP") == 0)
	//				interp = NodeAnimation::Interpolation::STEP;
	//			else if (interpolation.compare("LINEAR") == 0)
	//				interp = NodeAnimation::Interpolation::LINEAR;
	//			else if (interpolation.compare("CUBICSPLINE") == 0)
	//				interp = NodeAnimation::Interpolation::CUBIC;
	//			else
	//				interp = NodeAnimation::Interpolation::LINEAR;

	//			std::vector<float> times;
	//			std::vector<glm::vec3> translations;
	//			std::vector<glm::quat> rotations;
	//			std::vector<glm::vec3> scales;

	//			if (targetPath.compare("translation") == 0)
	//			{
	//				//std::vector<float> times;
	//				//std::vector<glm::vec3> translations;
	//				times.clear();
	//				loadData(input, times);
	//				loadData(output, translations);

	//				for (int i = 0; i < times.size(); i++)
	//				{
	//					minTime = std::min(minTime, times[i]);
	//					maxTime = std::max(maxTime, times[i]);
	//					//positionKeys.push_back(std::make_pair(times[i], translations[i]));
	//				}
	//			}
	//			else if (targetPath.compare("rotation") == 0)
	//			{
	//				//std::vector<float> times;
	//				//std::vector<glm::quat> rotations;
	//				times.clear();
	//				loadData(input, times);
	//				loadData(output, rotations);

	//				for (int i = 0; i < times.size(); i++)
	//				{
	//					minTime = std::min(minTime, times[i]);
	//					maxTime = std::max(maxTime, times[i]);
	//					//rotationKeys.push_back(std::make_pair(times[i], rotations[i]));
	//				}
	//			}
	//			else if (targetPath.compare("scale") == 0)
	//			{
	//				//std::vector<float> times;
	//				//std::vector<glm::vec3> scales;
	//				times.clear();
	//				loadData(input, times);
	//				loadData(output, scales);

	//				//std::cout << "times: " << times.size() << " scales: " << scales.size() << std::endl;

	//				for (int i = 0; i < times.size(); i++)
	//				{
	//					minTime = std::min(minTime, times[i]);
	//					maxTime = std::max(maxTime, times[i]);
	//					//scaleKeys.push_back(std::make_pair(times[i], scales[i]));
	//				}
	//			}
	//			else if (targetPath.compare("weights") == 0)
	//			{
	//				isMorphAnim = true;

	//				std::vector<float> times;
	//				std::vector<float> weights;
	//				loadData(input, times);
	//				loadData(output, weights);

	//				//std::cout << "times: " << times.size() << std::endl;
	//				//std::cout << "weights: " << weights.size() << std::endl;

	//				for (int i = 0; i < times.size(); i++)
	//				{
	//					minTime = std::min(minTime, times[i]);
	//					maxTime = std::max(maxTime, times[i]);
	//					auto weight = std::make_pair(weights[i * 2], weights[i * 2 + 1]);
	//					timeWeights.push_back(std::make_pair(times[i], weight));
	//				}
	//			}
	//			else
	//			{
	//				std::cout << "ERROR: animation path " << targetPath << " not implemented!!!" << std::endl;
	//			}

	//			if (isMorphAnim)
	//			{
	//				MorphAnimation::Ptr anim(new MorphAnimation("morph", 0.0f, maxTime, tartetNodeIndex, timeWeights));
	//				morphAnims.push_back(anim);
	//			}
	//			else
	//			{
	//				NodeAnimation::Ptr anim;
	//				if (nodeAnims.find(tartetNodeIndex) == nodeAnims.end())
	//				{
	//					anim = NodeAnimation::Ptr(new NodeAnimation("blub", interp, 0, minTime, maxTime, maxTime, tartetNodeIndex));
	//					nodeAnims.insert(std::make_pair(tartetNodeIndex, anim));
	//				}
	//				anim = nodeAnims[tartetNodeIndex];
	//				anim->setTimes(times);
	//				if (!translations.empty())
	//					anim->setPositions(translations);
	//				if (!rotations.empty())
	//					anim->setRotations(rotations);
	//				if (!scales.empty())
	//					anim->setScales(scales);
	//			}
	//		}
	//	}
	//}

	void GLTFImporter::loadAnimations(const json::Document& doc)
	{
		// TODO: add interpolation methods here

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
				if (samplerNode.HasMember("interpolation"))
					sampler.interpolation = samplerNode.FindMember("interpolation")->value.GetString();
				else
					sampler.interpolation = "LINEAR";
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
				
				int input = samplers[samplerIndex].input;
				int output = samplers[samplerIndex].output;
				std::string interpolation = samplers[samplerIndex].interpolation;

				Interpolation interp;
				if (interpolation.compare("STEP") == 0)
					interp = Interpolation::STEP;
				else if (interpolation.compare("LINEAR") == 0)
					interp = Interpolation::LINEAR;
				else if (interpolation.compare("CUBICSPLINE") == 0)
					interp = Interpolation::CUBIC;
				else
					interp = Interpolation::LINEAR;

				// TODO: can different channel paths have different interpolation methods???
				Channel& channel = channels[targetNodeIndex];
				channel.interpolation = interp; 

				if (targetPath.compare("translation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> translations;
					loadData(input, times);
					loadData(output, translations);

					int numValues = translations.size() / times.size();
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);

						std::vector<glm::vec3> translationValues;
						for (int j = 0; j < numValues; j++)
							translationValues.push_back(translations[i * numValues + j]);

						channel.positions.push_back(std::make_pair(times[i], translationValues));
					}
				}
				else if (targetPath.compare("rotation") == 0)
				{
					std::vector<float> times;
					std::vector<glm::quat> rotations;
					loadData(input, times);
					loadData(output, rotations);

					int numValues = rotations.size() / times.size();
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);

						std::vector<glm::quat> rotationValues;
						for (int j = 0; j < numValues; j++)
							rotationValues.push_back(rotations[i * numValues + j]);

						channel.rotations.push_back(std::make_pair(times[i], rotationValues));
					}
				}
				else if (targetPath.compare("scale") == 0)
				{
					std::vector<float> times;
					std::vector<glm::vec3> scales;
					loadData(input, times);
					loadData(output, scales);

					int numValues = scales.size() / times.size();
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);

						std::vector<glm::vec3> scaleValues;
						for (int j = 0; j < numValues; j++)
							scaleValues.push_back(scales[i * numValues + j]);

						channel.scales.push_back(std::make_pair(times[i], scaleValues));
					}
				}
#ifdef MORPH_TARGETS
				else if (targetPath.compare("weights") == 0)
				{
					std::vector<float> times;
					std::vector<float> weights;
					loadData(input, times);
					loadData(output, weights);

					int numTargets = weights.size() / times.size();
					const int maxtargets = 2;

					if (numTargets > maxtargets)
						std::cout << "Warning, max. 2 morph targets are supported!" << std::endl;

					//Channel& channel = channels[targetNodeIndex];
					for (int i = 0; i < times.size(); i++)
					{
						minTime = std::min(minTime, times[i]);
						maxTime = std::max(maxTime, times[i]);

						std::vector<float> w;
						for (int j = 0; j < maxtargets; j++)
							w.push_back(weights[i * numTargets + j]);
						channel.weights.push_back(std::make_pair(times[i], w));
					}
				}
#endif // MORPH_TARGETS
			}			

			auto animation = Animation::create("animation", maxTime);
			for (auto it : channels)
				animation->addChannel(it.first, it.second);
			animations.push_back(animation);
		}
	}

	void GLTFImporter::loadSkins(const json::Document& doc)
	{
		if (!doc.HasMember("skins"))
			return;

		for (auto& skinNode : doc["skins"].GetArray())
		{
			Skin skin;

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

			for (int i = 0; i < boneJoints.size(); i++)
				skin.addJoint(boneJoints[i], boneMatrices[i]);

			//if (skinNode.HasMember("skeleton"))
			//	skin.rootNode = skinNode["skeleton"].GetInt();
			//else
			//	skin.rootNode = 0; // TODO: find root node in joints to attach skin to

			//if (skinNode.HasMember("name"))
			//	skin.name = skinNode["name"].GetString();

  			skins.push_back(skin);
		}
	}

	void GLTFImporter::loadMeshes(const json::Document& doc)
	{
		int primitivNum = 0;		
		int numJoints = 0;

		auto meshesNode = doc.FindMember("meshes");
		for (auto& meshNode : meshesNode->value.GetArray())
		{
			GLTFMesh gltfMesh;

			auto nameNode = meshNode.FindMember("name");
			std::string name = "";
			if(meshNode.HasMember("name"))
				name = nameNode->value.GetString();

			std::vector<float> weights;
#ifdef MORPH_TARGETS
			if (meshNode.HasMember("weights"))
			{
				auto weightsNode = meshNode.FindMember("weights");
				for (auto& weightNode : weightsNode->value.GetArray())
				{
					weights.push_back(weightNode.GetFloat());
				}
			}
#endif // MORPH_TARGETS

			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				TriangleSurface surface;
				unsigned int materialIndex = 0;

				primitivNum++;
				
				std::vector<glm::vec3> positions;
				std::vector<glm::u8vec4> colors; // TODO colors can be RGB or RGBA!!!
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texCoords0;
				std::vector<glm::vec2> texCoords1;
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
					loadData(accIndex, texCoords0);
				}

				if (attributesNode.HasMember("TEXCOORD_1"))
				{
					int accIndex = attributesNode["TEXCOORD_1"].GetInt();
					loadData(accIndex, texCoords1);
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
						v.color = glm::vec4(colors[i]) / 255.0f;
					if (i < normals.size())
						v.normal = normals[i];
					if (i < texCoords0.size())
						v.texCoord0 = texCoords0[i];
					if (i < texCoords1.size())
						v.texCoord1 = texCoords1[i];
					if (i < tangets.size())
						v.tangent = tangets[i];
					if (i < boneIndices.size())
						v.boneIDs = glm::vec4(boneIndices[i].x, boneIndices[i].y, boneIndices[i].z, boneIndices[i].w) + glm::vec4(numJoints);
					if (i < boneWeights.size())
					{
						float weightSum = boneWeights[i].x + boneWeights[i].y + boneWeights[i].z + boneWeights[i].w;
						v.boneWeights = boneWeights[i] / weightSum;
					}

#ifdef MORPH_TARGETS
					if (morphTargets.size() > 0)
					{
						v.targetPosition0 = morphTargets[0].positions[i];
						if (i < morphTargets[0].normals.size())
							v.targetNormal0 = morphTargets[0].normals[i];
						if (i < morphTargets[0].tangents.size())
							v.targetTangent0 = morphTargets[0].tangents[i];
					}
					
					if (morphTargets.size() > 1)
					{
						v.targetPosition1 = morphTargets[1].positions[i];
						if (i < morphTargets[1].normals.size())
							v.targetNormal1 = morphTargets[1].normals[i];
						if (i < morphTargets[1].tangents.size())
							v.targetTangent1 = morphTargets[1].tangents[i];
					}
#endif

					surface.addVertex(v);
				}

				for (int i = 0; i < indices.size(); i += 3)
				{
					TriangleIndices t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
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

				Primitive primitive;
				primitive.mesh = Mesh::create(name, surface, materialIndex);
				primitive.material = material;
				gltfMesh.primitives.push_back(primitive);
			}
			
			if (!weights.empty())
				gltfMesh.morphWeights = weights;

			meshes.push_back(gltfMesh);
		}
	}

	Texture2D::Ptr GLTFImporter::loadTexture(TextureInfo& texInfo, const std::string& path, bool sRGB)
	{
		auto tex = IO::loadTexture(path + "/" + texInfo.filename, sRGB);
		Sampler& sampler = texInfo.sampler;
		if (sampler.minFilter >= 9984 && texInfo.sampler.minFilter <= 9987)
			tex->generateMipmaps();
		tex->setFilter(GL::TextureFilter(sampler.minFilter), GL::TextureFilter(sampler.magFilter));
		tex->setWrap(GL::TextureWrap(sampler.wrapS), GL::TextureWrap(sampler.wrapT));
		return tex;
	}

	void GLTFImporter::loadMaterials(const json::Document& doc, const std::string& path)
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
			if (materialNode.HasMember("doubleSided"))
			{
				bool doubleSided = materialNode["doubleSided"].GetBool();
				material->setDoubleSided(doubleSided);

				//std::cout << "material doubleSided: " << doubleSided << std::endl;
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
						//std::cout << "added baseColorTexture texture index " << texIndex << std::endl;
						auto tex = loadTexture(textures[texIndex], path, true);
							
						material->addTexture("material.baseColorTex", tex);
						material->addProperty("material.useBaseColorTex", true);
					}
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
				}

				if (pbrNode.HasMember("roughnessFactor"))
				{
					float roughnessFactor = pbrNode["roughnessFactor"].GetFloat();
					material->addProperty("material.roughnessFactor", roughnessFactor);
					material->addProperty("material.usePbrTex", false);
				}
				else
				{
					material->addProperty("material.roughnessFactor", 1.0f);
				}

				if (pbrNode.HasMember("metallicFactor"))
				{
					float metallicFactor = pbrNode["metallicFactor"].GetFloat();
					material->addProperty("material.metallicFactor", metallicFactor);
					material->addProperty("material.usePbrTex", false);
				}
				else
				{
					material->addProperty("material.metallicFactor", 0.0f);
				}
	
				if (pbrNode.HasMember("metallicRoughnessTexture"))
				{
					unsigned int texIndex = pbrNode["metallicRoughnessTexture"]["index"].GetInt();
					if (texIndex < textures.size())
					{
						auto tex = loadTexture(textures[texIndex], path, false);

						material->addTexture("material.pbrTex", tex);
						material->addProperty("material.usePbrTex", true);
					}
					else
					{
						std::cout << "texture index " << texIndex << " not found" << std::endl;
					}
				}
			}

			if (materialNode.HasMember("normalTexture"))
			{
				unsigned int texIndex = materialNode["normalTexture"]["index"].GetInt();
				if (texIndex < textures.size())
				{
					auto tex = loadTexture(textures[texIndex], path, false);

					material->addTexture("material.normalTex", tex);
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
				const auto& occlTexNode = materialNode["occlusionTexture"];
				unsigned int texIndex = occlTexNode["index"].GetInt();
				if (texIndex < textures.size())
				{
					auto tex = loadTexture(textures[texIndex], path, false);

					material->addTexture("material.occlusionTex", tex);
					material->addProperty("material.useOcclusionTex", true);

					int texCoordIdx = 0;
					if (occlTexNode.HasMember("texCoord"))
						texCoordIdx = occlTexNode["texCoord"].GetInt();
					material->addProperty("material.occlusionUVIndex", texCoordIdx);
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

			material->addProperty("material.emissiveFactor", glm::vec3(0.0f));
			if (materialNode.HasMember("emissiveFactor"))
			{
				const auto& baseColorNode = materialNode["emissiveFactor"];
				auto array = baseColorNode.GetArray();
				glm::vec3 color;
				color.r = array[0].GetFloat();
				color.g = array[1].GetFloat();
				color.b = array[2].GetFloat();
				material->addProperty("material.emissiveFactor", color);
				material->addProperty("material.emissiveTexture", false);
			}

			if (materialNode.HasMember("emissiveTexture"))
			{
				const auto& emissiveTexNode = materialNode["emissiveTexture"];
				unsigned int texIndex = emissiveTexNode["index"].GetInt();
				if (texIndex < textures.size())
				{
					auto tex = loadTexture(textures[texIndex], path, true);

					material->addTexture("material.emissiveTex", tex);
					material->addProperty("material.useEmissiveTex", true);

					int texCoordIdx = 0;
					if (emissiveTexNode.HasMember("texCoord"))
						texCoordIdx = emissiveTexNode["texCoord"].GetInt();
					material->addProperty("material.emissiveUVIndex", texCoordIdx);
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
				const auto& extensionNode = materialNode["extensions"];
				{
					if (extensionNode.HasMember("KHR_materials_pbrSpecularGlossiness"))
					{
						material->addProperty("useSpecGlossMat", true);
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
								auto tex = loadTexture(textures[texIndex], path, true);

								//std::cout << "added normalTexture texture index " << texIndex << std::endl;
								material->addTexture("material2.diffuseTex", tex);
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
								auto tex = loadTexture(textures[texIndex], path, true);

								//std::cout << "added normalTexture texture index " << texIndex << std::endl;
								material->addTexture("material2.specGlossTex", tex);
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

				int index; // TODO: add proper parsing of URIs
				while ((index = filename.find("%20")) >= 0)
					filename.replace(index, 3, " ");

				imageFiles.push_back(filename);
			}
			else
			{
				std::cout << "loading images from binary buffer not supported!" << std::endl;
			}
		}

		std::vector<Sampler> samplers;
		if (doc.HasMember("samplers")) 
		{
			for (auto& samplerNode : doc["samplers"].GetArray())
			{
				Sampler s;
				if (samplerNode.HasMember("minFilter"))
					s.minFilter = samplerNode["minFilter"].GetInt();
				if (samplerNode.HasMember("magFilter"))
					s.magFilter = samplerNode["magFilter"].GetInt();
				if (samplerNode.HasMember("wrapS"))
					s.wrapS = samplerNode["wrapS"].GetInt();
				if (samplerNode.HasMember("wrapT"))
					s.wrapT = samplerNode["wrapT"].GetInt();
				samplers.push_back(s);
			}
		}

		if (doc.HasMember("textures"))
		{
			for (auto& textureNode : doc["textures"].GetArray())
			{
				TextureInfo texInfo;

				int imageIndex = textureNode["source"].GetInt();
				texInfo.filename = imageFiles[imageIndex];

				int samplerIndex = -1;
				if (textureNode.HasMember("sampler"))
				{
					samplerIndex = textureNode["sampler"].GetInt();
					texInfo.sampler = samplers[samplerIndex];
				}

				textures.push_back(texInfo);
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
			auto renderable = Renderable::create();

			auto gltfMesh = meshes[node.meshIndex];
			for(auto p : gltfMesh.primitives)
				renderable->addMesh("", p.mesh, p.material);
			if (!gltfMesh.morphWeights.empty())
				renderable->setMorphWeights(gltfMesh.morphWeights);

			// neg. scale results in wrongly oriented faces!
			if (node.scale.x < 0 || node.scale.y < 0 || node.scale.z < 0)
				renderable->flipWindingOrder();

			entity->addComponent(renderable);
		}

		if (node.skinIndex >= 0)
		{
			if (node.skinIndex < skins.size())
			{
				Skin& skin = skins[node.skinIndex];
				auto joints = skin.getJoints();

				//int commonRootIdx = -1;
				//for (auto i : joints)
				//{
				//	int parentIndex = nodes[i].parentIndex;
				//	bool jointRoot = false;
				//	for (auto j : joints)
				//	{
				//		if (j == parentIndex)
				//		{
				//			jointRoot = true;
				//			break;
				//		}
				//	}
				//	if (!jointRoot)
				//	{
				//		commonRootIdx = parentIndex;
				//		break;
				//	}
				//}

				//if (commonRootIdx < 0)
				//	commonRootIdx = joints[0];

				skin.setSkeleton(nodeIndex);

				auto r = entity->getComponent<Renderable>();
				if (r)
					r->setSkin(skin);
			}
		}

		for (auto& index : node.children)
		{
			auto childEntity = traverse(index);
			entity->addChild(childEntity);
		}

		entities[nodeIndex] = entity;

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
				if (node.HasMember("skin"))
					gltfNode.skinIndex = node["skin"].GetInt();
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
		
		// TODO: dont add an empty root node! use the one from GLTF instead
		auto root = Entity::create("root");
		auto rootTransform = root->getComponent<Transform>();
		auto rootNode = nodes[0];

		entities.resize(nodes.size());
		entities[0] = root;

		// TODO: a GLTF scene can define more than one root node! 
		//	     Now each root not is traversed seperatly resulting in many duplicated nodes!!!
		// 
		// Solution: All those root nodes should be collected under one "root" node.
		//			 From this root the scene hierarchy should be traversed to make sure each node is instanced once

		if (doc.HasMember("scenes") && doc["scenes"][0].HasMember("nodes"))
		{
			std::cout << "number of root nodes: " << doc["scenes"][0]["nodes"].Size() << std::endl;
			for (auto& nodeIndex : doc["scenes"][0]["nodes"].GetArray())
			{
				auto childEntity = traverse(nodeIndex.GetInt());
				root->addChild(childEntity);
			}
		}

		if (!animations.empty())
		{
			auto animator = Animator::create(skins.empty());
			animator->setNodes(entities);
			for (auto a : animations)
				animator->addAnimation(a);
			root->addComponent(animator);
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
		meshes.clear();
		materials.clear();
		animators.clear();
		animations.clear();
		textures.clear();
		entities.clear();
		skins.clear();
	}
}