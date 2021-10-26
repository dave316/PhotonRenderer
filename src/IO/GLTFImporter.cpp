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

				auto defaultMaterial = getDefaultMaterial();
				defaultMaterial->addProperty("material.computeFlatNormals", computeFlatNormals);
				
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

	void GLTFImporter::setTextureInfo(const json::Value& node, const std::string& texNodeName, Material::Ptr material, std::string texInfoStr, std::string path, bool sRGB)
	{
		if (node.HasMember(texNodeName.c_str()))
		{
			auto& texNode = node[texNodeName.c_str()];
			unsigned int texIndex = texNode["index"].GetInt();
			if (texIndex < textures.size())
			{
				auto tex = loadTexture(textures[texIndex], path, sRGB);

				material->addTexture(texInfoStr + ".tSampler", tex);
				material->addProperty(texInfoStr + ".use", true);

				int texCoordIdx = 0;
				if (texNode.HasMember("texCoord"))
					texCoordIdx = texNode["texCoord"].GetInt();
				material->addProperty(texInfoStr + ".uvIndex", texCoordIdx);

				glm::mat3 texTransform = glm::mat3(1.0f);
				if (texNode.HasMember("extensions"))
				{
					auto& extNode = texNode["extensions"];
					if (extNode.HasMember("KHR_texture_transform"))
						texTransform = getTexTransform(extNode["KHR_texture_transform"]);
				}
				material->addProperty(texInfoStr + ".uvTransform", texTransform);
			}
			else
			{
				std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
		}			
		else
		{
			material->addProperty(texInfoStr + ".use", false);
		}
	}

	glm::vec4 getVec4FromNode(const json::Value& parentNode, const std::string& nodeName, glm::vec4 defaultValue = glm::vec4(1.0f))
	{
		glm::vec4 value = defaultValue;
		if (parentNode.HasMember(nodeName.c_str()))
		{
			const auto& node = parentNode[nodeName.c_str()];
			auto array = node.GetArray();
			value.r = array[0].GetFloat();
			value.g = array[1].GetFloat();
			value.b = array[2].GetFloat();
			value.a = array[3].GetFloat();
		}
		return value;
	}

	glm::vec3 getVec3FromNode(const json::Value& parentNode, const std::string& nodeName, glm::vec3 defaultValue = glm::vec3(1.0f))
	{
		glm::vec3 value = defaultValue;
		if (parentNode.HasMember(nodeName.c_str()))
		{
			const auto& baseColorNode = parentNode[nodeName.c_str()];
			auto array = baseColorNode.GetArray();
			value.r = array[0].GetFloat();
			value.g = array[1].GetFloat();
			value.b = array[2].GetFloat();
		}
		return value;
	}

	float getFloatFromNode(const json::Value& parentNode, const std::string& nodeName, float defaultValue = 1.0f)
	{
		float value = defaultValue;
		if (parentNode.HasMember(nodeName.c_str()))
			value = parentNode[nodeName.c_str()].GetFloat();
		return value;
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
				auto& pbrNode = materialNode["pbrMetallicRoughness"];

				glm::vec4 baseColor = getVec4FromNode(pbrNode, "baseColorFactor");
				float roughnessFactor = getFloatFromNode(pbrNode, "roughnessFactor");
				float metallicFactor = getFloatFromNode(pbrNode, "metallicFactor");
				material->addProperty("material.baseColorFactor", baseColor);
				material->addProperty("material.roughnessFactor", roughnessFactor);
				material->addProperty("material.metallicFactor", metallicFactor);
				
				setTextureInfo(pbrNode, "baseColorTexture", material, "baseColorTex", path, true);
				setTextureInfo(pbrNode, "metallicRoughnessTexture", material, "pbrTex", path, false);
			}

			if (materialNode.HasMember("normalTexture"))
			{
				auto& normalTexNode = materialNode["normalTexture"];
				unsigned int texIndex = normalTexNode["index"].GetInt();
				if (texIndex < textures.size())
				{
					auto tex = loadTexture(textures[texIndex], path, false);

					material->addTexture("normalTex.tSampler", tex);
					material->addProperty("normalTex.use", true);

					int texCoordIdx = 0;
					if (normalTexNode.HasMember("texCoord"))
						texCoordIdx = normalTexNode["texCoord"].GetInt();
					material->addProperty("normalTex.uvIndex", texCoordIdx);

					float normalScale = 1.0f;
					if (normalTexNode.HasMember("scale"))
						normalScale = normalTexNode["scale"].GetFloat();
					material->addProperty("material.normalScale", normalScale);

					glm::mat3 texTransform = glm::mat3(1.0f);
					if (normalTexNode.HasMember("extensions"))
					{
						auto& extNode = normalTexNode["extensions"];
						if (extNode.HasMember("KHR_texture_transform"))
							texTransform = getTexTransform(extNode["KHR_texture_transform"]);
					}
					material->addProperty("normalTex.uvTransform", texTransform);
				}					
				else
					std::cout << "texture index " << texIndex << " not found" << std::endl;
			}
			else
			{
				material->addProperty("normalTex.use", false);
			}

			setTextureInfo(materialNode, "occlusionTexture", material, "occlusionTex", path, false);

			glm::vec3 emissiveFactor = getVec3FromNode(materialNode, "emissiveFactor", glm::vec3(0));
			material->addProperty("material.emissiveFactor", emissiveFactor);
			setTextureInfo(materialNode, "emissiveTexture", material, "emissiveTex", path, true);

			material->addProperty("useSpecGlossMat", false);
			material->addProperty("material.sheenColorFactor", glm::vec3(0));
			material->addProperty("material.sheenRoughnessFactor", 0.0f);
			material->addProperty("sheenColorTex.use", false);
			material->addProperty("sheenRoughTex.use", false);
			material->addProperty("material.clearcoatFactor", 0.0f);
			material->addProperty("clearCoatTex.use", false);
			material->addProperty("material.clearcoatRoughnessFactor", 0.0f);
			material->addProperty("clearCoatRoughTex.use", false);
			material->addProperty("clearCoatNormalTex.use", false);
			material->addProperty("material.transmissionFactor", 0.0f);
			material->addProperty("transmissionTex.use", false);
			material->addProperty("material.thicknessFactor", 0.0f);
			material->addProperty("thicknessTex.use", false);
			material->addProperty("material.attenuationDistance", 0.0f); // TODO: default should be infinity?
			material->addProperty("material.attenuationColor", glm::vec3(1.0f));
			material->addProperty("material.ior", 1.5f);
			material->addProperty("material.specularFactor", 1.0f);
			material->addProperty("specularTex.use", false);
			material->addProperty("material.specularColorFactor", glm::vec3(1.0f));
			material->addProperty("specularColorTex.use", false);
			material->addProperty("material.iridescenceFactor", 0.0f);
			material->addProperty("iridescenceTex.use", false);
			material->addProperty("material.iridescenceIOR", 1.8f);
			material->addProperty("material.iridescenceThicknessMin", 400.0f);
			material->addProperty("material.iridescenceThicknessMax", 1200.0f);
			material->addProperty("iridescenceThicknessTex.use", false);
			material->addProperty("material.anisotropyFactor", 0.0f);
			material->addProperty("anisotropyTex.use", false);
			material->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
			material->addProperty("anisotropyDirectionTex.use", false);
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
					glm::vec3 sheenColor = getVec3FromNode(sheenNode, "sheenColorFactor", glm::vec3(0));
					float sheenRough = getFloatFromNode(sheenNode, "sheenRoughnessFactor", 0.0);
					material->addProperty("material.sheenColorFactor", sheenColor);
					material->addProperty("material.sheenRoughnessFactor", sheenRough);
					setTextureInfo(sheenNode, "sheenColorTexture", material, "sheenColortex", path, true);
					setTextureInfo(sheenNode, "sheenRoughnessTexture", material, "sheenRoughtex", path, false);
				}		

				if (extensionNode.HasMember("KHR_materials_clearcoat"))
				{
					const auto& clearcoatNode = extensionNode["KHR_materials_clearcoat"];
					float clearcoat = getFloatFromNode(clearcoatNode, "clearcoatFactor", 0.0f);
					float clearcoatRough = getFloatFromNode(clearcoatNode, "clearcoatRoughnessFactor", 0.0f);
					material->addProperty("material.clearcoatFactor", clearcoat);
					material->addProperty("material.clearcoatRoughnessFactor", clearcoatRough);
					setTextureInfo(clearcoatNode, "clearcoatTexture", material, "clearCoatTex", path, false);
					setTextureInfo(clearcoatNode, "clearcoatRoughnessTexture", material, "clearCoatRoughTex", path, false);
					setTextureInfo(clearcoatNode, "clearcoatNormalTexture", material, "clearCoatNormalTex", path, false);
				}

				if (extensionNode.HasMember("KHR_materials_transmission"))
				{
					const auto& transmissionNode = extensionNode["KHR_materials_transmission"];
					float transmissionFactor = getFloatFromNode(transmissionNode, "transmissionFactor", 0.0f);
					material->addProperty("material.transmissionFactor", transmissionFactor);
					material->setTransmissive(true);
					setTextureInfo(transmissionNode, "transmissionTexture", material, "transmissionTex", path, false);
				}

				if (extensionNode.HasMember("KHR_materials_volume"))
				{
					const auto& volumeNode = extensionNode["KHR_materials_volume"];
					float thicknessFactor = getFloatFromNode(volumeNode, "thicknessFactor", 0.0f);
					float attenuationDistance = getFloatFromNode(volumeNode, "attenuationDistance", 0.0f);
					glm::vec3 attenuationColor = getVec3FromNode(volumeNode, "attenuationColor");
					material->addProperty("material.thicknessFactor", thicknessFactor);
					material->addProperty("material.attenuationDistance", attenuationDistance);
					material->addProperty("material.attenuationColor", attenuationColor);
					material->setTransmissive(true);
					setTextureInfo(volumeNode, "thicknessTexture", material, "thicknessTex", path, false);
				}

				if (extensionNode.HasMember("KHR_materials_ior"))
				{
					const auto& iorNode = extensionNode["KHR_materials_ior"];
					material->addProperty("material.ior", getFloatFromNode(iorNode, "ior", 1.5f));
				}

				if (extensionNode.HasMember("KHR_materials_specular"))
				{
					const auto& specularNode = extensionNode["KHR_materials_specular"];
					float specularFactor = getFloatFromNode(specularNode, "specularFactor");
					glm::vec3 specularColor = getVec3FromNode(specularNode, "specularColorFactor");
					material->addProperty("material.specularFactor", specularFactor);
					material->addProperty("material.specularColorFactor", specularColor);
					setTextureInfo(specularNode, "specularTexture", material, "specularTex", path, false);
					setTextureInfo(specularNode, "specularColorTexture", material, "specularColorTex", path, true);
				}

				if (extensionNode.HasMember("KHR_materials_iridescence"))
				{
					const auto& iridescenceNode = extensionNode["KHR_materials_iridescence"];
					float iridescenceFactor = getFloatFromNode(iridescenceNode, "iridescenceFactor", 0.0f);
					float iridescenceIOR = getFloatFromNode(iridescenceNode, "iridescenceIOR", 1.8f);
					float iridescenceThicknessMin = getFloatFromNode(iridescenceNode, "iridescenceThicknessMinimum", 400.0f);
					float iridescenceThicknessMax = getFloatFromNode(iridescenceNode, "iridescenceThicknessMaximum", 1200.0f);
					material->addProperty("material.iridescenceFactor", iridescenceFactor);
					material->addProperty("material.iridescenceIOR", iridescenceIOR);
					material->addProperty("material.iridescenceThicknessMin", iridescenceThicknessMin);
					material->addProperty("material.iridescenceThicknessMax", iridescenceThicknessMax);
					setTextureInfo(iridescenceNode, "iridescenceThicknessTexture", material, "iridescenceThicknessTex", path, false);
				}

				if (extensionNode.HasMember("KHR_materials_anisotropy"))
				{
					const auto& anisotropyNode = extensionNode["KHR_materials_anisotropy"];
					float anisotropy = getFloatFromNode(anisotropyNode, "anisotropy", 0.0f);
					glm::vec3 anisotropyDirection = getVec3FromNode(anisotropyNode, "anisotropyDirection", glm::vec3(1.0f, 0.0f, 0.0f));
					material->addProperty("material.anisotropyFactor", anisotropy);
					material->addProperty("material.anisotropyDirection", anisotropyDirection);
					setTextureInfo(anisotropyNode, "anisotropyTexture", material, "anisotropyTex", path, false);
					setTextureInfo(anisotropyNode, "anisotropyDirectionTexture", material, "anisotropyDirectionTex", path, false);
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