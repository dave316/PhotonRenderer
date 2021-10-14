#include "GLTFImporter.h"

#include <IO/ImageLoader.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

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

	GLTFImporter::GLTFImporter()
	{
		supportedExtensions.insert("KHR_materials_sheen");
		supportedExtensions.insert("KHR_materials_clearcoat");
		supportedExtensions.insert("KHR_materials_transmission");
		supportedExtensions.insert("KHR_materials_volume");
		supportedExtensions.insert("KHR_materials_ior");
		supportedExtensions.insert("KHR_materials_specular");
		supportedExtensions.insert("KHR_materials_unlit");
		supportedExtensions.insert("KHR_materials_variants");
		supportedExtensions.insert("KHR_materials_pbrSpecularGlossiness");
		supportedExtensions.insert("KHR_lights_punctual");
		supportedExtensions.insert("KHR_texture_transform");
	}

	Entity::Ptr GLTFImporter::importModel(std::string filename)
	{
		std::replace(filename.begin(), filename.end(), '\\', '/');
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

		checkExtensions(doc);
		loadExtensionData(doc);
		loadBuffers(doc, path);
		loadTextures(doc, path);
		loadMaterials(doc, path);
		loadMeshes(doc);
		loadCameras(doc);
		loadSkins(doc);
		loadAnimations(doc);

		auto root = loadScene(doc);

 		return root;
	}

	void GLTFImporter::checkExtensions(const json::Document& doc)
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
	}

	void GLTFImporter::loadExtensionData(const json::Document& doc)
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
					{
						varaints.push_back(variantNode["name"].GetString());
					}
					else
					{
						varaints.push_back("variant_" + std::to_string(index));
					}
					index++;
				}
			}
		}
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

	void GLTFImporter::loadAnimations(const json::Document& doc)
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
#if defined MORPH_TARGETS_2 || defined MORPH_TARGETS_8
				else if (targetPath.compare("weights") == 0)
				{
					std::vector<float> times;
					std::vector<float> weights;
					loadData(input, times);
					loadData(output, weights);

					int numTargets = weights.size() / times.size();
					const int maxtargets = 8;

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
#endif // MORPH_TARGETS_8
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
#if defined MORPH_TARGETS_2 || defined MORPH_TARGETS_8
			if (meshNode.HasMember("weights"))
			{
				auto weightsNode = meshNode.FindMember("weights");
				for (auto& weightNode : weightsNode->value.GetArray())
				{
					weights.push_back(weightNode.GetFloat());
				}
			}
#endif

			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				TriangleSurface surface;
				std::vector<unsigned int> materialIndices;

				primitivNum++;

				// TODO: check accessor component type and cast data accordingly
				
				std::vector<glm::vec3> positions;
				std::vector<glm::vec4> colors;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texCoords0;
				std::vector<glm::vec2> texCoords1;
				std::vector<glm::vec4> tangets;
				std::vector<GLuint> indices;
				std::vector<glm::u16vec4> boneIndices;
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
					if (accessors[accIndex].componentType == 5121)
					{
						std::vector<glm::u8vec4> colorsUI8;
						loadData(accIndex, colorsUI8);
						colors.resize(colorsUI8.size());
						for (int i = 0; i < colors.size(); i++)
							colors[i] = glm::vec4(colorsUI8[i]) / 255.0f;
					}
					else
					{
						loadData(accIndex, colors);
					}
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
					materialIndices.push_back(primitiveNode["material"].GetInt());
				}

				if (primitiveNode.HasMember("extensions"))
				{
					// TODO: add variant - material mapping
					auto& extensionNode = primitiveNode["extensions"];
					if (extensionNode.HasMember("KHR_materials_variants"))
					{
						materialIndices.clear();
						auto& variantsNode = extensionNode["KHR_materials_variants"];
						if(variantsNode.HasMember("mappings"))
						{
							for (auto& mappintNode : variantsNode["mappings"].GetArray())
							{
								materialIndices.push_back(mappintNode["material"].GetInt());
							}
						}
					}
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

#ifdef MORPH_TARGETS_2
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

#ifdef MORPH_TARGETS_8
					if (morphTargets.size() > 0)
						v.targetPosition0 = morphTargets[0].positions[i];
					if (morphTargets.size() > 1)
						v.targetPosition1 = morphTargets[1].positions[i];
					if (morphTargets.size() > 2)
						v.targetPosition2 = morphTargets[2].positions[i];
					if (morphTargets.size() > 3)
						v.targetPosition3 = morphTargets[3].positions[i];
					if (morphTargets.size() > 4)
						v.targetPosition4 = morphTargets[4].positions[i];
					if (morphTargets.size() > 5)
						v.targetPosition5 = morphTargets[5].positions[i];
					if (morphTargets.size() > 6)
						v.targetPosition6 = morphTargets[6].positions[i];
					if (morphTargets.size() > 7)
						v.targetPosition7 = morphTargets[7].positions[i];
#endif

					surface.addVertex(v);
				}

				for (int i = 0; i < indices.size(); i += 3)
				{
					TriangleIndices t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
					surface.addTriangle(t);
				}

				bool computeFlatNormals = false;
				if (calcNormals)
				{
					if (surface.triangles.empty())
						surface.calcFlatNormals();
					else
						computeFlatNormals = true;
				}

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

				Primitive primitive;
				primitive.mesh = Mesh::create(name, surface, 0);
				for (auto materialIndex : materialIndices)
				{
					if (materialIndex < materials.size())
					{
						auto mat = materials[materialIndex];
						mat->addProperty("material.computeFlatNormals", computeFlatNormals);
						primitive.materials.push_back(mat);
					}
						
				}
				if (primitive.materials.empty())
					primitive.materials.push_back(defaultMaterial);

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
		TextureSampler& sampler = texInfo.sampler;
		if (sampler.minFilter >= 9984 && texInfo.sampler.minFilter <= 9987)
			tex->generateMipmaps();
		tex->setFilter(GL::TextureFilter(sampler.minFilter), GL::TextureFilter(sampler.magFilter));
		tex->setWrap(GL::TextureWrap(sampler.wrapS), GL::TextureWrap(sampler.wrapT));
		return tex;
	}

	void GLTFImporter::loadCameras(const json::Document& doc)
	{
		if (!doc.HasMember("cameras"))
			return;

		auto camerasNode = doc.FindMember("cameras");
		int cameraIndex = 0;
		for (auto& cameraNode : camerasNode->value.GetArray())
		{
			GLTFCamera cam;
			if (cameraNode.HasMember("name"))
			{
				cam.name = cameraNode["name"].GetString();
			}
			else
			{
				cam.name = "Camera_" + std::to_string(cameraIndex);
			}

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

				cam.P = glm::perspective(yfov, aspect, znear, zfar);
				cameras.push_back(cam);
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
				
				cam.P = glm::ortho(-xmag, xmag, -ymag, ymag, znear, zfar);
				cameras.push_back(cam);
			}
			else
			{
				std::cout << "unknow camera type " << type << std::endl;
			}

			cameraIndex++;
		}
	}

	glm::mat3 getTexTransform(const json::Value& texTransNode)
	{
		glm::vec2 offset(0.0f);
		glm::vec2 scale(1.0f);
		float rotation = 0.0f;

		if (texTransNode.HasMember("offset"))
		{
			offset.x = texTransNode["offset"].GetArray()[0].GetFloat();
			offset.y = texTransNode["offset"].GetArray()[1].GetFloat();
		}
		if (texTransNode.HasMember("rotation"))
		{
			rotation = texTransNode["rotation"].GetFloat();
		}
		if (texTransNode.HasMember("scale"))
		{
			scale.x = texTransNode["scale"].GetArray()[0].GetFloat();
			scale.y = texTransNode["scale"].GetArray()[1].GetFloat();
		}

		glm::mat3 T(1.0f);
		T[2][0] = offset.x;
		T[2][1] = offset.y;

		glm::mat3 S(1.0f);
		S[0][0] = scale.x;
		S[1][1] = scale.y;

		glm::mat3 R(1.0f);
		R[0][0] = glm::cos(rotation);
		R[1][0] = glm::sin(rotation);
		R[0][1] = -glm::sin(rotation);
		R[1][1] = glm::cos(rotation);

		return T * R * S;
	}

	void GLTFImporter::loadMaterials(const json::Document& doc, const std::string& path)
	{
		// TODO: abstract texture info and extension loading to optimize code

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
				material->addProperty("material.useBaseColorTex", false);
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
					auto& baseColorTexNode = pbrNode["baseColorTexture"];
					unsigned int texIndex = baseColorTexNode["index"].GetInt();
					if (texIndex < textures.size())
					{
						auto tex = loadTexture(textures[texIndex], path, true);
							
						material->addTexture("material.baseColorTex", tex);
						material->addProperty("material.useBaseColorTex", true);
						material->addProperty("material.hasBaseColorUVTransform", false);

						int texCoordIdx = 0;
						if (baseColorTexNode.HasMember("texCoord"))
							texCoordIdx = baseColorTexNode["texCoord"].GetInt();
						material->addProperty("material.baseColorUVIndex", texCoordIdx);

						if (baseColorTexNode.HasMember("extensions"))
						{
							auto& extNode = baseColorTexNode["extensions"];
							if (extNode.HasMember("KHR_texture_transform"))
							{
								glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

								material->addProperty("material.baseColorUVTransform", texTransform);
								material->addProperty("material.hasBaseColorUVTransform", true);
							}
						}
					}
					else
						std::cout << "texture index " << texIndex << " not found" << std::endl;
				}

				material->addProperty("material.usePbrTex", false);
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
					material->addProperty("material.metallicFactor", 1.0f);
				}
	
				if (pbrNode.HasMember("metallicRoughnessTexture"))
				{
					auto& metallicRoughTexNode = pbrNode["metallicRoughnessTexture"];
					unsigned int texIndex = metallicRoughTexNode["index"].GetInt();
					if (texIndex < textures.size())
					{
						auto tex = loadTexture(textures[texIndex], path, false);

						material->addTexture("material.pbrTex", tex);
						material->addProperty("material.usePbrTex", true);
						material->addProperty("material.hasPbrTexUVTransform", false);

						int texCoordIdx = 0;
						if (metallicRoughTexNode.HasMember("texCoord"))
							texCoordIdx = metallicRoughTexNode["texCoord"].GetInt();
						material->addProperty("material.pbrTexUVIndex", texCoordIdx);

						if (metallicRoughTexNode.HasMember("extensions"))
						{
							auto& extNode = metallicRoughTexNode["extensions"];
							if (extNode.HasMember("KHR_texture_transform"))
							{
								glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

								material->addProperty("material.pbrTexUVTransform", texTransform);
								material->addProperty("material.hasPbrTexUVTransform", true);
							}
						}
					}
					else
					{
						std::cout << "texture index " << texIndex << " not found" << std::endl;
					}
				}
			}

			if (materialNode.HasMember("normalTexture"))
			{
				auto& normalTexNode = materialNode["normalTexture"];
				unsigned int texIndex = normalTexNode["index"].GetInt();
				if (texIndex < textures.size())
				{
					auto tex = loadTexture(textures[texIndex], path, false);

					material->addTexture("material.normalTex", tex);
					material->addProperty("material.useNormalTex", true);
					material->addProperty("material.hasNormalUVTransform", false);

					int texCoordIdx = 0;
					if (normalTexNode.HasMember("texCoord"))
						texCoordIdx = normalTexNode["texCoord"].GetInt();
					material->addProperty("material.normalUVIndex", texCoordIdx);

					float normalScale = 1.0f;
					if (normalTexNode.HasMember("scale"))
						normalScale = normalTexNode["scale"].GetFloat();
					material->addProperty("material.normalScale", normalScale);

					if (normalTexNode.HasMember("extensions"))
					{
						auto& extNode = normalTexNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
						{
							glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

							material->addProperty("material.normalUVTransform", texTransform);
							material->addProperty("material.hasNormalUVTransform", true);
						}
					}
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
					material->addProperty("material.hasOcclusionUVTransform", false);

					int texCoordIdx = 0;
					if (occlTexNode.HasMember("texCoord"))
						texCoordIdx = occlTexNode["texCoord"].GetInt();
					material->addProperty("material.occlusionUVIndex", texCoordIdx);

					if (occlTexNode.HasMember("extensions"))
					{
						auto& extNode = occlTexNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
						{
							glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

							material->addProperty("material.occlusionUVTransform", texTransform);
							material->addProperty("material.hasOcclusionUVTransform", true);
						}
					}
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

			if (materialNode.HasMember("emissiveFactor"))
			{
				const auto& baseColorNode = materialNode["emissiveFactor"];
				auto array = baseColorNode.GetArray();
				glm::vec3 color;
				color.r = array[0].GetFloat();
				color.g = array[1].GetFloat();
				color.b = array[2].GetFloat();
				material->addProperty("material.emissiveFactor", color);
			}
			else
			{
				material->addProperty("material.emissiveFactor", glm::vec3(0.0f));
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
					material->addProperty("material.hasEmissiveUVTransform", false);

					int texCoordIdx = 0;
					if (emissiveTexNode.HasMember("texCoord"))
						texCoordIdx = emissiveTexNode["texCoord"].GetInt();
					material->addProperty("material.emissiveUVIndex", texCoordIdx);

					if (emissiveTexNode.HasMember("extensions"))
					{
						auto& extNode = emissiveTexNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
						{
							glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

							material->addProperty("material.emissiveUVTransform", texTransform);
							material->addProperty("material.hasEmissiveUVTransform", true);
						}
					}
				}
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
			else
			{
				material->addProperty("material.useEmissiveTex", false);
			}

			material->addProperty("useSpecGlossMat", false);
			material->addProperty("material.sheenColorFactor", glm::vec3(0));
			material->addProperty("material.sheenRoughnessFactor", 0.0f);
			material->addProperty("material.useSheenColorTex", false);
			material->addProperty("material.useSheenRoughTex", false);
			material->addProperty("material.clearcoatFactor", 0.0f);
			material->addProperty("material.useClearCoatTex", false);
			material->addProperty("material.clearcoatRoughnessFactor", 0.0f);
			material->addProperty("material.useClearCoatRoughTex", false);
			material->addProperty("material.useClearCoatNormalTex", false);
			material->addProperty("material.transmissionFactor", 0.0f);
			material->addProperty("material.useTransmissionTex", false);
			material->addProperty("material.thicknessFactor", 0.0f);
			material->addProperty("material.useThicknessTex", false);
			material->addProperty("material.attenuationDistance", 0.0f); // TODO: default should be infinity?
			material->addProperty("material.attenuationColor", glm::vec3(1.0f));
			material->addProperty("material.ior", 1.5f);
			material->addProperty("material.specularFactor", 1.0f);
			material->addProperty("material.useSpecularTex", false);
			material->addProperty("material.specularColorFactor", glm::vec3(1.0f));
			material->addProperty("material.useSpecularColorTex", false);
			material->addProperty("material.iridescenceFactor", 0.0f);
			material->addProperty("material.useIridescenceTex", false);
			material->addProperty("material.iridescenceIOR", 1.8f);
			material->addProperty("material.iridescenceThicknessMin", 400.0f);
			material->addProperty("material.iridescenceThicknessMax", 1200.0f);
			material->addProperty("material.useIridescenceThicknessTex", false);
			material->addProperty("material.anisotropyFactor", 0.0f);
			material->addProperty("material.useAnisotropyTexture", false);
			material->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
			material->addProperty("material.useAnisotropyDirectionTexture", false);
			material->addProperty("material.unlit", false);
				
			if (materialNode.HasMember("extensions"))
			{
				const auto& extensionNode = materialNode["extensions"];
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

							material->addTexture("material2.specGlossTex", tex);
							material->addProperty("material2.useSpecularTex", true);
						}
						else
							std::cout << "texture index " << texIndex << " not found" << std::endl;
					}
				}

				if (extensionNode.HasMember("KHR_materials_sheen"))
				{
					const auto& sheenNode = extensionNode["KHR_materials_sheen"];

					material->addProperty("material.sheenColorFactor", glm::vec3(0.0f));
					if (sheenNode.HasMember("sheenColorFactor"))
					{
						const auto& sheenColorNode = sheenNode["sheenColorFactor"];
						auto array = sheenColorNode.GetArray();
						glm::vec3 color;
						color.r = array[0].GetFloat();
						color.g = array[1].GetFloat();
						color.b = array[2].GetFloat();
						material->addProperty("material.sheenColorFactor", color);
						material->addProperty("material.useSheenColorTex", false);
					}

					if (sheenNode.HasMember("sheenColorTexture"))
					{
						auto& sheenColorTexNode = sheenNode["sheenColorTexture"];
						unsigned int texIndex = sheenColorTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, true);

							material->addTexture("material.sheenColortex", tex);
							material->addProperty("material.useSheenColorTex", true);
							material->addProperty("material.hasSheenColorUVTransform", false);

							int texCoordIdx = 0;
							if (sheenColorTexNode.HasMember("texCoord"))
								texCoordIdx = sheenColorTexNode["texCoord"].GetInt();
							material->addProperty("material.sheenColorUVIndex", texCoordIdx);

							if (sheenColorTexNode.HasMember("extensions"))
							{
								auto& extNode = sheenColorTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.sheenColorUVTransform", texTransform);
									material->addProperty("material.hasSheenColorUVTransform", true);
								}
							}
						}
						else
							std::cout << "texture index " << texIndex << " not found" << std::endl;
					}

					if (sheenNode.HasMember("sheenRoughnessFactor"))
					{
						float sheenRoughness = sheenNode["sheenRoughnessFactor"].GetFloat();
						material->addProperty("material.sheenRoughnessFactor", sheenRoughness);
						material->addProperty("material.useSheenRoughTex", false);
					}
					else
					{
						material->addProperty("material.sheenRoughnessFactor", 0.0f);
					}

					if (sheenNode.HasMember("sheenRoughnessTexture"))
					{
						auto& sheenRoughTexNode = sheenNode["sheenRoughnessTexture"];
						unsigned int texIndex = sheenRoughTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.sheenRoughtex", tex);
							material->addProperty("material.useSheenRoughTex", true);
							material->addProperty("material.hasSheenRoughUVTransform", false);

							int texCoordIdx = 0;
							if (sheenRoughTexNode.HasMember("texCoord"))
								texCoordIdx = sheenRoughTexNode["texCoord"].GetInt();
							material->addProperty("material.sheenRoughUVIndex", texCoordIdx);

							if (sheenRoughTexNode.HasMember("extensions"))
							{
								auto& extNode = sheenRoughTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.sheenRoughUVTransform", texTransform);
									material->addProperty("material.hasSheenRoughUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
				}		

				if (extensionNode.HasMember("KHR_materials_clearcoat"))
				{
					const auto& clearCoatNode = extensionNode["KHR_materials_clearcoat"];
					if (clearCoatNode.HasMember("clearcoatFactor"))
					{
						float clearCoat = clearCoatNode["clearcoatFactor"].GetFloat();
						material->addProperty("material.clearcoatFactor", clearCoat);
						material->addProperty("material.useClearCoatTex", false);
					}
					else
					{
						material->addProperty("material.clearcoatFactor", 0.0f);
					}

					if (clearCoatNode.HasMember("clearcoatTexture"))
					{
						auto& clearCoatTexNode = clearCoatNode["clearcoatTexture"];
						unsigned int texIndex = clearCoatTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.clearCoatTex", tex);
							material->addProperty("material.useClearCoatTex", true);
							material->addProperty("material.hasClearCoatUVTransform", false);

							int texCoordIdx = 0;
							if (clearCoatTexNode.HasMember("texCoord"))
								texCoordIdx = clearCoatTexNode["texCoord"].GetInt();
							material->addProperty("material.clearCoatUVIndex", texCoordIdx);

							if (clearCoatTexNode.HasMember("extensions"))
							{
								auto& extNode = clearCoatTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.clearCoatUVTransform", texTransform);
									material->addProperty("material.hasClearCoatUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}

					if (clearCoatNode.HasMember("clearcoatRoughnessFactor"))
					{
						float clearCoatRoughness = clearCoatNode["clearcoatRoughnessFactor"].GetFloat();
						material->addProperty("material.clearcoatRoughnessFactor", clearCoatRoughness);
						material->addProperty("material.useClearCoatRoughTex", false);
					}
					else
					{
						material->addProperty("material.clearcoatRoughnessFactor", 0.0f);
					}

					if (clearCoatNode.HasMember("clearcoatRoughnessTexture"))
					{
						auto& clearCoatRoughTexNode = clearCoatNode["clearcoatRoughnessTexture"];
						unsigned int texIndex = clearCoatRoughTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.clearCoatRoughTex", tex);
							material->addProperty("material.useClearCoatRoughTex", true);
							material->addProperty("material.hasClearCoatRoughUVTransform", false);

							int texCoordIdx = 0;
							if (clearCoatRoughTexNode.HasMember("texCoord"))
								texCoordIdx = clearCoatRoughTexNode["texCoord"].GetInt();
							material->addProperty("material.clearCoatRoughUVIndex", texCoordIdx);

							if (clearCoatRoughTexNode.HasMember("extensions"))
							{
								auto& extNode = clearCoatRoughTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.clearCoatRoughUVTransform", texTransform);
									material->addProperty("material.hasClearCoatRoughUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}

					if (clearCoatNode.HasMember("clearcoatNormalTexture"))
					{
						auto& normalTexNode = clearCoatNode["clearcoatNormalTexture"];
						unsigned int texIndex = normalTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.clearCoatNormalTex", tex);
							material->addProperty("material.useClearCoatNormalTex", true);
							material->addProperty("material.hasClearCoatNormalUVTransform", false);

							int texCoordIdx = 0;
							if (normalTexNode.HasMember("texCoord"))
								texCoordIdx = normalTexNode["texCoord"].GetInt();
							material->addProperty("material.clearCoatNormalUVIndex", texCoordIdx);

							if (normalTexNode.HasMember("extensions"))
							{
								auto& extNode = normalTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.clearCoatNormalUVTransform", texTransform);
									material->addProperty("material.hasClearCoatNormalUVTransform", true);
								}
							}
						}
						else
							std::cout << "texture index " << texIndex << " not found" << std::endl;
					}
					else
					{
						material->addProperty("material.useClearCoatNormalTex", false);
					}
				}

				if (extensionNode.HasMember("KHR_materials_transmission"))
				{
					material->setTransmissive(true);

					const auto& transmissionNode = extensionNode["KHR_materials_transmission"];
					if (transmissionNode.HasMember("transmissionFactor"))
					{
						float transmissionFactor = transmissionNode["transmissionFactor"].GetFloat();
						material->addProperty("material.transmissionFactor", transmissionFactor);
						material->addProperty("material.useTransmissionTex", false);
					}
					else
					{
						material->addProperty("material.transmissionFactor", 0.0f);
					}

					if (transmissionNode.HasMember("transmissionTexture"))
					{
						auto& transmissionTexNode = transmissionNode["transmissionTexture"];
						unsigned int texIndex = transmissionTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.transmissionTex", tex);
							material->addProperty("material.useTransmissionTex", true);
							material->addProperty("material.hasTransmissionUVTransform", false);

							int texCoordIdx = 0;
							if (transmissionTexNode.HasMember("texCoord"))
								texCoordIdx = transmissionTexNode["texCoord"].GetInt();
							material->addProperty("material.transmissionUVIndex", texCoordIdx);

							if (transmissionTexNode.HasMember("extensions"))
							{
								auto& extNode = transmissionTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.transmissionUVTransform", texTransform);
									material->addProperty("material.hasTransmissionUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
				}

				if (extensionNode.HasMember("KHR_materials_volume"))
				{
					material->setTransmissive(true);

					const auto& volumeNode = extensionNode["KHR_materials_volume"];
					if (volumeNode.HasMember("thicknessFactor"))
					{
						float thicknessFactor = volumeNode["thicknessFactor"].GetFloat();
						material->addProperty("material.thicknessFactor", thicknessFactor);
						material->addProperty("material.useThicknessTex", false);
					}
					else
					{
						material->addProperty("material.thicknessFactor", 0.0f);
					}

					if (volumeNode.HasMember("thicknessTexture"))
					{
						auto& thicknessTexNode = volumeNode["thicknessTexture"];
						unsigned int texIndex = thicknessTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.thicknessTex", tex);
							material->addProperty("material.useThicknessTex", true);
							material->addProperty("material.hasThicknessUVTransform", false);

							int texCoordIdx = 0;
							if (thicknessTexNode.HasMember("texCoord"))
								texCoordIdx = thicknessTexNode["texCoord"].GetInt();
							material->addProperty("material.thicknessUVIndex", texCoordIdx);

							if (thicknessTexNode.HasMember("extensions"))
							{
								auto& extNode = thicknessTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.thicknessUVTransform", texTransform);
									material->addProperty("material.hasThicknessUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}

					if (volumeNode.HasMember("attenuationDistance"))
					{
						float attenuationDistance = volumeNode["attenuationDistance"].GetFloat();
						material->addProperty("material.attenuationDistance", attenuationDistance);
					}
					else
					{
						material->addProperty("material.attenuationDistance", 0.0f);
					}
										
					if (volumeNode.HasMember("attenuationColor"))
					{
						const auto& attColorNode = volumeNode["attenuationColor"];
						auto array = attColorNode.GetArray();
						glm::vec3 color;
						color.r = array[0].GetFloat();
						color.g = array[1].GetFloat();
						color.b = array[2].GetFloat();
						material->addProperty("material.attenuationColor", color);
					}
					else
					{
						material->addProperty("material.attenuationColor", glm::vec3(1.0f));
					}
				}

				if (extensionNode.HasMember("KHR_materials_ior"))
				{
					const auto& iorNode = extensionNode["KHR_materials_ior"];
					if (iorNode.HasMember("ior"))
					{
						float ior = iorNode["ior"].GetFloat();
						material->addProperty("material.ior", ior);
					}
					else
					{
						material->addProperty("material.ior", 1.5f);
					}
				}

				if (extensionNode.HasMember("KHR_materials_specular"))
				{
					const auto& specularNode = extensionNode["KHR_materials_specular"];
					if (specularNode.HasMember("specularFactor"))
					{
						float specularFactor = specularNode["specularFactor"].GetFloat();
						material->addProperty("material.specularFactor", specularFactor);
						material->addProperty("material.useSpecularTex", false);
					}
					else
					{
						material->addProperty("material.specularFactor", 1.0f);
					}

					if (specularNode.HasMember("specularTexture"))
					{
						auto& specularTexNode = specularNode["specularTexture"];
						unsigned int texIndex = specularTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.specularTex", tex);
							material->addProperty("material.useSpecularTex", true);
							material->addProperty("material.hasSpecularUVTransform", false);

							int texCoordIdx = 0;
							if (specularTexNode.HasMember("texCoord"))
								texCoordIdx = specularTexNode["texCoord"].GetInt();
							material->addProperty("material.specularUVIndex", texCoordIdx);

							if (specularTexNode.HasMember("extensions"))
							{
								auto& extNode = specularTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.specularUVTransform", texTransform);
									material->addProperty("material.hasSpecularUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}

					if (specularNode.HasMember("specularColorFactor"))
					{
						const auto& specColorNode = specularNode["specularColorFactor"];
						auto array = specColorNode.GetArray();
						glm::vec3 color;
						color.r = array[0].GetFloat();
						color.g = array[1].GetFloat();
						color.b = array[2].GetFloat();
						material->addProperty("material.specularColorFactor", color);
						material->addProperty("material.useSpecularColorTex", false);
					}
					else
					{
						material->addProperty("material.specularColorFactor", glm::vec3(1.0f));
					}

					if (specularNode.HasMember("specularColorTexture"))
					{
						auto& specularColorTexNode = specularNode["specularColorTexture"];
						unsigned int texIndex = specularColorTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.specularColorTex", tex);
							material->addProperty("material.useSpecularColorTex", true);
							material->addProperty("material.hasSpecularColorUVTransform", false);

							int texCoordIdx = 0;
							if (specularColorTexNode.HasMember("texCoord"))
								texCoordIdx = specularColorTexNode["texCoord"].GetInt();
							material->addProperty("material.specularColorUVIndex", texCoordIdx);

							if (specularColorTexNode.HasMember("extensions"))
							{
								auto& extNode = specularColorTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.specularColorUVTransform", texTransform);
									material->addProperty("material.hasSpecularColorUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
				}

				if (extensionNode.HasMember("KHR_materials_iridescence"))
				{
					const auto& iridescenceNode = extensionNode["KHR_materials_iridescence"];
					if (iridescenceNode.HasMember("iridescenceFactor"))
					{
						float iridescenceFactor = iridescenceNode["iridescenceFactor"].GetFloat();
						material->addProperty("material.iridescenceFactor", iridescenceFactor);
					}
					else
					{
						material->addProperty("material.iridescenceFactor", 0.0f);
					}

					if (iridescenceNode.HasMember("iridescenceTexture"))
					{
						auto& iridescenceTexNode = iridescenceNode["specularTexture"];
						unsigned int texIndex = iridescenceTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.iridescenceTex", tex);
							material->addProperty("material.useIridescenceTex", true);
							material->addProperty("material.hasIridescenceUVTransform", false);

							int texCoordIdx = 0;
							if (iridescenceTexNode.HasMember("texCoord"))
								texCoordIdx = iridescenceTexNode["texCoord"].GetInt();
							material->addProperty("material.specularUVIndex", texCoordIdx);

							if (iridescenceTexNode.HasMember("extensions"))
							{
								auto& extNode = iridescenceTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.iridescenceUVTransform", texTransform);
									material->addProperty("material.hasIridescenceUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
					else
					{
						material->addProperty("material.useIridescenceTex", false);
					}

					if (iridescenceNode.HasMember("iridescenceIOR"))
					{
						float iridescenceIOR = iridescenceNode["iridescenceIOR"].GetFloat();
						material->addProperty("material.iridescenceIOR", iridescenceIOR);
					}
					else
					{
						material->addProperty("material.iridescenceIOR", 1.8f);
					}

					if (iridescenceNode.HasMember("iridescenceThicknessMaximum"))
					{
						float iridescenceThicknessMax = iridescenceNode["iridescenceThicknessMaximum"].GetFloat();
						material->addProperty("material.iridescenceThicknessMax", iridescenceThicknessMax);
					}
					else
					{
						material->addProperty("material.iridescenceThicknessMax", 1200.0f);
					}

					if (iridescenceNode.HasMember("iridescenceThicknessMinimum"))
					{
						float iridescenceThicknessMin = iridescenceNode["iridescenceThicknessMinimum"].GetFloat();
						material->addProperty("material.iridescenceThicknessMin", iridescenceThicknessMin);
					}
					else
					{
						material->addProperty("material.iridescenceThicknessMin", 400.0f);
					}

					if (iridescenceNode.HasMember("iridescenceThicknessTexture"))
					{
						auto& iridescenceThicknessTexNode = iridescenceNode["iridescenceThicknessTexture"];
						unsigned int texIndex = iridescenceThicknessTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.iridescenceThicknessTex", tex);
							material->addProperty("material.useIridescenceThicknessTex", true);
							material->addProperty("material.hasIridescenceThicknessUVTransform", false);

							int texCoordIdx = 0;
							if (iridescenceThicknessTexNode.HasMember("texCoord"))
								texCoordIdx = iridescenceThicknessTexNode["texCoord"].GetInt();
							material->addProperty("material.iridescenceThicknessUVIndex", texCoordIdx);

							if (iridescenceThicknessTexNode.HasMember("extensions"))
							{
								auto& extNode = iridescenceThicknessTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.iridescenceThicknessUVTransform", texTransform);
									material->addProperty("material.hasIridescenceThicknessUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
					else
					{
						material->addProperty("material.useIridescenceThicknessTex", false);
					}
				}

				if (extensionNode.HasMember("KHR_materials_anisotropy"))
				{
					const auto& anisotropyNode = extensionNode["KHR_materials_anisotropy"];
					if (anisotropyNode.HasMember("anisotropy"))
					{
						float anisotropy = anisotropyNode["anisotropy"].GetFloat();
						material->addProperty("material.anisotropyFactor", anisotropy);
					}
					else
					{
						material->addProperty("material.anisotropyFactor", 0.0f);
					}

					if (anisotropyNode.HasMember("anisotropyTexture"))
					{
						auto& anisotropyTexNode = anisotropyNode["anisotropyTexture"];
						unsigned int texIndex = anisotropyTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.anisotropyTexture", tex);
							material->addProperty("material.useAnisotropyTexture", true);
							material->addProperty("material.hasAnisotropyUVTransform", false);

							int texCoordIdx = 0;
							if (anisotropyTexNode.HasMember("texCoord"))
								texCoordIdx = anisotropyTexNode["texCoord"].GetInt();
							material->addProperty("material.anisotropyUVIndex", texCoordIdx);

							if (anisotropyTexNode.HasMember("extensions"))
							{
								auto& extNode = anisotropyTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.anisotropyUVTransform", texTransform);
									material->addProperty("material.hasAnisotropyUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
					else
					{
						material->addProperty("material.useAnisotropyTexture", false);
					}

					if (anisotropyNode.HasMember("anisotropyDirection"))
					{
						const auto& anisotropyDirNode = anisotropyNode["anisotropyDirection"];
						auto array = anisotropyDirNode.GetArray();
						glm::vec3 dir;
						dir.r = array[0].GetFloat();
						dir.g = array[1].GetFloat();
						dir.b = array[2].GetFloat();
						material->addProperty("material.anisotropyDirection", dir);
					}
					else
					{
						material->addProperty("material.anisotropyDirection", glm::vec3(1.0f, 0.0f, 0.0f));
					}

					if (anisotropyNode.HasMember("anisotropyDirectionTexture"))
					{
						auto& anisotropyDirTexNode = anisotropyNode["anisotropyDirectionTexture"];
						unsigned int texIndex = anisotropyDirTexNode["index"].GetInt();
						if (texIndex < textures.size())
						{
							auto tex = loadTexture(textures[texIndex], path, false);

							material->addTexture("material.anisotropyDirectionTexture", tex);
							material->addProperty("material.useAnisotropyDirectionTexture", true);
							material->addProperty("material.hasAnisotropyDirectionUVTransform", false);

							int texCoordIdx = 0;
							if (anisotropyDirTexNode.HasMember("texCoord"))
								texCoordIdx = anisotropyDirTexNode["texCoord"].GetInt();
							material->addProperty("material.anisotropyDirectionUVIndex", texCoordIdx);

							if (anisotropyDirTexNode.HasMember("extensions"))
							{
								auto& extNode = anisotropyDirTexNode["extensions"];
								if (extNode.HasMember("KHR_texture_transform"))
								{
									glm::mat3 texTransform = getTexTransform(extNode["KHR_texture_transform"]);

									material->addProperty("material.anisotropyDirectionUVTransform", texTransform);
									material->addProperty("material.hasAnisotropyDirectionUVTransform", true);
								}
							}
						}
						else
						{
							std::cout << "texture index " << texIndex << " not found" << std::endl;
						}
					}
					else
					{
						material->addProperty("material.useAnisotropyDirectionTexture", false);
					}
				}

				if (extensionNode.HasMember("KHR_materials_unlit"))
				{
					material->addProperty("material.unlit", true);
				}
				else
				{
					material->addProperty("material.unlit", false);
				}
			}				

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

		std::vector<TextureSampler> samplers;
		if (doc.HasMember("samplers")) 
		{
			for (auto& samplerNode : doc["samplers"].GetArray())
			{
				TextureSampler s;
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

	Entity::Ptr GLTFImporter::traverse(int nodeIndex, glm::mat4 parentTransform)
	{
		auto node = nodes[nodeIndex];
		if (node.name.empty())
			node.name = "node_" + std::to_string(nodeIndex);

		auto entity = Entity::create(node.name);
		auto t = entity->getComponent<Transform>();
		t->setPosition(node.translation);
		t->setRotation(node.rotation);
		t->setScale(node.scale);

		glm::mat4 T = glm::translate(glm::mat4(1.0f), node.translation);
		glm::mat4 R = glm::mat4_cast(node.rotation);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), node.scale);
		glm::mat4 transform = parentTransform * (T * R * S);

		if (node.camIndex >= 0)
		{
			cameras[node.camIndex].V = glm::inverse(transform);
			cameras[node.camIndex].pos = node.translation; // TODO: this is not correct, use hierarchy to figure out pos
		}
		
		if (node.meshIndex >= 0)
		{
			auto renderable = Renderable::create();

			auto gltfMesh = meshes[node.meshIndex];
			for (auto p : gltfMesh.primitives)
				renderable->addPrimitive(p);
				//renderable->addMesh("", p.mesh, p.material);
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

				// TODO: check if there is a skeleton node defined
				skin.setSkeleton(nodeIndex);

				auto r = entity->getComponent<Renderable>();
				if (r)
					r->setSkin(skin);
			}
		}

		if (node.lightIndex >= 0)
		{
			if (node.lightIndex < lights.size())
			{
				glm::vec3 skew;
				glm::vec4 persp;
				glm::vec3 pos;
				glm::vec3 scale;
				glm::quat rot;
				glm::decompose(transform, scale, rot, pos, skew, persp);
				rot = glm::normalize(rot);
				glm::vec3 dir = glm::mat3_cast(rot) * glm::vec3(0, 0, -1);
				lights[node.lightIndex]->setPostion(pos);
				lights[node.lightIndex]->setDirection(dir);
			}
		}

		for (auto& index : node.children)
		{
			auto childEntity = traverse(index, transform);
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
				if (node.HasMember("camera"))
					gltfNode.camIndex = node["camera"].GetInt();
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

				if (node.HasMember("extensions"))
				{
					auto& extensionNode = node["extensions"];
					if (extensionNode.HasMember("KHR_lights_punctual"))
						gltfNode.lightIndex = extensionNode["KHR_lights_punctual"]["light"].GetInt();
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

		if (doc.HasMember("scenes") && doc["scenes"][0].HasMember("nodes"))
		{
			for (auto& nodeIndex : doc["scenes"][0]["nodes"].GetArray())
			{
				auto childEntity = traverse(nodeIndex.GetInt(), glm::mat4(1.0f));
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

	std::vector<Light::Ptr> GLTFImporter::getLights()
	{
		return lights;
	}

	std::vector<GLTFCamera> GLTFImporter::getCameras()
	{
		return cameras;
	}

	std::vector<std::string> GLTFImporter::getVariants()
	{
		return varaints;
	}

	void GLTFImporter::clear()
	{
		buffers.clear();
		bufferViews.clear();
		accessors.clear();
		nodes.clear();
		meshes.clear();
		materials.clear();
		lights.clear();
		animators.clear();
		animations.clear();
		textures.clear();
		entities.clear();
		skins.clear();
		cameras.clear(); 
	}
}