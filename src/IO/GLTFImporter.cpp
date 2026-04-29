#include "GLTFImporter.h"

#include "ImageLoader.h"
#include <base64/base64.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef LIBS_DRACO
#include <draco/mesh/mesh.h>
#include <draco/compression/decode.h>
#endif

#include "TangentSpace.h"

namespace fs = std::filesystem;

namespace IO
{
	namespace glTF
	{
		glm::vec2 toVec2(const json::Value& value)
		{
			glm::vec2 v(0.0f);
			if (value.IsArray() && value.Size() == 2)
			{
				auto a = value.GetArray();
				for (uint32 i = 0; i < 2; i++)
					v[i] = a[i].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to vec2" << std::endl;
			}
			return v;
		}

		glm::vec3 toVec3(const json::Value& value)
		{
			glm::vec3 v(0.0f);
			if (value.IsArray() && value.Size() == 3)
			{
				auto a = value.GetArray();
				for (uint32 i = 0; i < 3; i++)
					v[i] = a[i].GetFloat();
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
				auto a = value.GetArray();
				for (uint32 i = 0; i < 4; i++)
					v[i] = a[i].GetFloat();
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
				auto a = value.GetArray();
				for (int i = 0; i < 4; i++)
					q[i] = a[i].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to quaternion" << std::endl;
			}
			return q;
		}

		glm::mat4 toMat4(const json::Value& value)
		{
			glm::mat4 M(1.0f);
			if (value.IsArray() && value.Size() == 16)
			{
				auto a = value.GetArray();
				float* values = glm::value_ptr<float>(M);
				for (uint32 i = 0; i < a.Size(); i++)
					values[i] = a[i].GetFloat();
			}
			else
			{
				std::cout << "error parsing json array to mat4" << std::endl;
			}
			return M;
		}

		template<typename T>
		void loadElements(json::Document& doc, std::string nodeName, std::vector<T>& elements)
		{
			if (doc.HasMember(nodeName.c_str()))
			{
				//std::cout << "parsing buffers" << std::endl;
				auto arrayNode = doc.FindMember(nodeName.c_str());
				for (auto& node : arrayNode->value.GetArray())
				{
					T element;
					element.parse(node);
					elements.push_back(element);
				}
			}
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

		Importer::Importer()
		{
			supportedExtensions.insert("KHR_animation_pointer");
#ifdef LIBS_DRACO
			supportedExtensions.insert("KHR_draco_mesh_compression");
#endif
			supportedExtensions.insert("KHR_lights_punctual");
			supportedExtensions.insert("KHR_materials_anisotropy");
			supportedExtensions.insert("KHR_materials_clearcoat");
			supportedExtensions.insert("KHR_materials_emissive_strength");
			supportedExtensions.insert("KHR_materials_ior");
			supportedExtensions.insert("KHR_materials_iridescence");
			//supportedExtensions.insert("KHR_materials_pbrSpecularGlossiness");
			supportedExtensions.insert("KHR_materials_sheen");
			supportedExtensions.insert("KHR_materials_specular");
			supportedExtensions.insert("KHR_materials_transmission");
			supportedExtensions.insert("KHR_materials_diffuse_transmission");
			supportedExtensions.insert("KHR_materials_unlit");
			supportedExtensions.insert("KHR_materials_variants");
			supportedExtensions.insert("KHR_materials_volume");
			supportedExtensions.insert("KHR_mesh_quantization");
#ifdef IMAGE_KTX
			supportedExtensions.insert("KHR_texture_basisu");
#endif
			supportedExtensions.insert("KHR_texture_transform");
#ifdef IMAGE_WEBP			
			supportedExtensions.insert("EXT_texture_webp");
#endif
		}

		bool Importer::checkExtensions(const json::Document& doc)
		{
			// TODO: check required extensions
			//if (doc.HasMember("extensionsUsed"))
			//{
			//	auto extensionsNode = doc.FindMember("extensionsUsed");
			//	for (auto& extNode : extensionsNode->value.GetArray())
			//	{
			//		std::string extension(extNode.GetString());
			//		if (supportedExtensions.find(extension) == supportedExtensions.end())
			//			std::cout << "extension " << extension << " not supported" << std::endl;
			//	}
			//}

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

			// TODO: add multiple chunks if present
			std::vector<uint8> binChunk;
			binChunk.resize(chunkLen);
			std::copy(buffer.begin() + baseIndex, buffer.begin() + baseIndex + chunkLen, binChunk.begin());
			buffers.push_back(binChunk);

			return content;
		}

		bool Importer::loadJSON(const std::string& filename)
		{	
			auto p = fs::path(filename);
			auto path = p.parent_path().string();
			auto f = p.filename().string();
			auto extension = p.extension().string();

			json::Document document;
			if (extension.compare(".gltf") == 0)
			{
				std::ifstream file(filename);
				if (file.is_open())
				{
					std::stringstream ss;
					ss << file.rdbuf();
					std::string content = ss.str();
					document.Parse(content.c_str());
				}
				else
				{
					std::cout << "error opening file " << filename << std::endl;
					return false;
				}
			}
			else if (extension.compare(".glb") == 0)
			{
				std::string content = loadGLB(filename);
				document.Parse(content.c_str());
			}

			if (!checkExtensions(document)) {
				std::cout << "required extension not supported, model cannot be loaded!" << std::endl;
				return false;
			}

			if (document.HasMember("scene"))
				gltf.defaultScene = document["scene"].GetUint();

			loadElements(document, "buffers", gltf.buffers);
			loadElements(document, "bufferViews", gltf.bufferViews);
			loadElements(document, "accessors", gltf.accessors);
			loadElements(document, "scenes", gltf.scenes);
			loadElements(document, "nodes", gltf.nodes);
			loadElements(document, "meshes", gltf.meshes);
			loadElements(document, "materials", gltf.materials);
			loadElements(document, "images", gltf.images);
			loadElements(document, "samplers", gltf.samplers);
			loadElements(document, "textures", gltf.textures);
			loadElements(document, "animations", gltf.animations);
			loadElements(document, "skins", gltf.skins);
			loadElements(document, "cameras", gltf.cameras);

			if (document.HasMember("asset"))
				gltf.asset.parse(document["asset"]);

			if (document.HasMember("extensions"))
			{
				auto& extNode = document["extensions"];
				if (extNode.HasMember("KHR_lights_punctual"))
				{
					auto& extLight = extNode["KHR_lights_punctual"];
					if (extLight.HasMember("lights"))
					{
						auto arrayNode = extLight.FindMember("lights");
						for (auto& node : arrayNode->value.GetArray())
						{
							Light light;
							light.parse(node);
							gltf.lights.push_back(light);
						}
					}
				}
				if (extNode.HasMember("KHR_materials_variants"))
				{
					int index = 0;
					auto& variantsNode = extNode["KHR_materials_variants"];
					for (auto& variantNode : variantsNode["variants"].GetArray())
					{
						if (variantNode.HasMember("name"))
							gltf.variants.push_back(variantNode["name"].GetString());
						else
							gltf.variants.push_back("variant_" + std::to_string(index));
						index++;
					}
				}
			}

			return true;
		}

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

		pr::Texture2DArray::Ptr Importer::createMorphTexture(std::vector<MorphTarget> morphTargets)
		{
			int numTargets = morphTargets.size();
			int numVertices = morphTargets[0].positions.size();
			int numAttributes = 3;
			int width = glm::ceil(glm::sqrt(numVertices));
			int numLayers = numTargets * numAttributes;
			int texSize = width * width * 4;
			int totalSize = numLayers * texSize;
			float* buffer = new float[totalSize];
			for (int i = 0; i < totalSize; i++)
				buffer[i] = 0.0f;

			int layerIndex = 0;
			for (auto& target : morphTargets)
			{
				for (int i = 0; i < target.positions.size(); i++)
				{
					glm::vec3 pos = target.positions[i];
					int idx = layerIndex * texSize + i * 4;
					buffer[idx] = pos.x;
					buffer[idx + 1] = pos.y;
					buffer[idx + 2] = pos.z;
				}

				for (int i = 0; i < target.normals.size(); i++)
				{
					glm::vec3 normal = target.normals[i];
					int idx = (layerIndex + 1) * texSize + i * 4;
					buffer[idx] = normal.x;
					buffer[idx + 1] = normal.y;
					buffer[idx + 2] = normal.z;
				}

				for (int i = 0; i < target.tangents.size(); i++)
				{
					glm::vec3 tangent = target.tangents[i];
					int idx = (layerIndex + 2) * texSize + i * 4;
					buffer[idx] = tangent.x;
					buffer[idx + 1] = tangent.y;
					buffer[idx + 2] = tangent.z;
				}

				layerIndex += numAttributes;
			}

			auto tex = pr::Texture2DArray::create(width, width, numLayers, GPU::Format::RGBA32F, 1, GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled);
			tex->upload((uint8*)buffer, totalSize * 4);
			tex->setFilter(GPU::Filter::Nearest, GPU::Filter::Nearest);
			tex->setAddressMode(GPU::AddressMode::ClampToEdge);
			delete[] buffer;

			return tex;
		}

		GPU::AddressMode getAddressMode(uint32 wrap)
		{
			GPU::AddressMode mode = GPU::AddressMode::Repeat;
			switch (wrap)
			{
				case 10497: mode = GPU::AddressMode::Repeat; break;
				case 33648: mode = GPU::AddressMode::MirroredRepeat; break;
				case 33071: mode = GPU::AddressMode::ClampToEdge; break;
			}
			return mode;
		}

		GPU::Filter getFilter(uint32 glFilter)
		{
			GPU::Filter filter = GPU::Filter::Linear;
			switch (glFilter)
			{
				case 9728: filter = GPU::Filter::Nearest; break;
				case 9729: filter = GPU::Filter::Linear; break;
				case 9984: filter = GPU::Filter::NearestMipmapNearest; break;
				case 9985: filter = GPU::Filter::NearestMipmapLinear; break;
				case 9986: filter = GPU::Filter::LinearMipmapNearest; break;
				case 9987: filter = GPU::Filter::LinearMipmapLinear; break;
			}
			return filter;
		}

		void Importer::loadSurface(TriangleSurface& surface, Primitive& primitive)
		{
			glm::vec3 minPoint;
			glm::vec3 maxPoint;
			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec2> texCoords0;
			std::vector<glm::vec2> texCoords1;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec4> tangents;
			std::vector<glm::vec4> joints;
			std::vector<glm::vec4> weights;
			for (auto&& [name, accIndex] : primitive.attributes)
			{
				// TODO: check component type
				if (name.compare("POSITION") == 0)
				{
					for (int i = 0; i < 3; i++)
					{
						minPoint[i] = gltf.accessors[accIndex].min[i];
						maxPoint[i] = gltf.accessors[accIndex].max[i];
					}

					loadAttribute(accIndex, positions);
					Accessor& acc = gltf.accessors[accIndex];
					if (acc.sparse.has_value())
					{
						std::vector<GLuint> sparseIndices;
						std::vector<glm::vec3> sparsePositions;
						loadSparseData(accIndex, sparseIndices, sparsePositions);
						for (int i = 0; i < acc.sparse.value().count; i++)
							positions[sparseIndices[i]] = sparsePositions[i];
					}
				}
				if (name.compare("COLOR_0") == 0)
				{
					if (gltf.accessors[accIndex].type.compare("VEC3") == 0)
					{
						std::vector<glm::vec3> colorsRGB;
						loadAttribute(accIndex, colorsRGB);
						for (auto c : colorsRGB)
							colors.push_back(glm::vec4(c, 1.0f));
					}
					else
					{
						loadAttribute(accIndex, colors);
					}					
				}					
				if (name.compare("NORMAL") == 0)
					loadAttribute(accIndex, normals);
				if (name.compare("TEXCOORD_0") == 0)
					loadAttribute(accIndex, texCoords0);
				if (name.compare("TEXCOORD_1") == 0)
					loadAttribute(accIndex, texCoords1);
				if (name.compare("TANGENT") == 0)
					loadAttribute(accIndex, tangents);
				if (name.compare("JOINTS_0") == 0)
					loadAttribute(accIndex, joints);
				//{
				//	std::vector<glm::u16vec4> jointsUInt16;
				//	loadData(accIndex, jointsUInt16);
				//	for (auto i : jointsUInt16)
				//		joints.push_back(i);
				//}
				if (name.compare("WEIGHTS_0") == 0)
					loadAttribute(accIndex, weights);
			}

			std::vector<GLuint> indices;
			if (primitive.indices.has_value())
			{
				Accessor& acc = gltf.accessors[primitive.indices.value()];
				BufferView& bv = gltf.bufferViews[acc.bufferView.value()];
				Buffer& buffer = gltf.buffers[bv.buffer];
				uint32 offset = bv.byteOffset + acc.byteOffset;

				switch (acc.componentType)
				{
					case GL_UNSIGNED_BYTE:
					{
						std::vector<GLubyte> byteIndices(acc.count);
						std::memcpy(byteIndices.data(), &buffers[bv.buffer][offset], acc.count * sizeof(GLubyte));
						for (GLubyte i : byteIndices)
							indices.push_back(i);
						break;
					}
					case GL_UNSIGNED_SHORT:
					{
						std::vector<GLushort> shortIndices(acc.count);
						std::memcpy(shortIndices.data(), &buffers[bv.buffer][offset], acc.count * sizeof(GLushort));
						for (GLushort i : shortIndices)
							indices.push_back(i);
						break;
					}
					case GL_UNSIGNED_INT:
					{
						indices.resize(acc.count);
						std::memcpy(indices.data(), &buffers[bv.buffer][offset], acc.count * sizeof(GLuint));
						break;
					}
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
				if (i < texCoords0.size())
					v.texCoord0 = texCoords0[i];
				if (i < texCoords1.size())
					v.texCoord1 = texCoords1[i];
				if (i < tangents.size())
					v.tangent = tangents[i];
				if (i < joints.size())
					v.joints = joints[i];
				if (i < weights.size())
				{
					float sum = 0.0f;
					for (int j = 0; j < 4; j++)
						sum += weights[i][j];
					v.weights = weights[i] / sum;
				}

				surface.vertices.push_back(v);
			}

			surface.minPoint = minPoint;
			surface.maxPoint = maxPoint;

			for (int i = 0; i < indices.size(); i++)
				surface.indices.push_back(indices[i]);

			uint32 mode = primitive.mode;
			if (mode >= 4)
			{
				if (normals.empty())
				{
					if (indices.empty())
						surface.calcFlatNormals();
					else
						surface.computeFlatNormals = true;
				}
				if (tangents.empty())
				{
					TangentSpace ts;
					ts.generateTangents(surface, true);
				}					
			}

			if (indices.empty())
				for (int i = 0; i < surface.vertices.size(); i++)
					surface.indices.push_back(i);
		}

#ifdef LIBS_DRACO

		void Importer::loadCompressedSurface(TriangleSurface& surface, Primitive& primitive)
		{
			auto draco = primitive.draco.value();
			uint32 bufferViewIndex = draco.bufferView;

			std::vector<unsigned char> compressedBuffer;
			BufferView& bv = gltf.bufferViews[bufferViewIndex]; // TODO: get bufferview from $

			int offset = bv.byteOffset;
			compressedBuffer.resize(bv.byteLength);
			memcpy(compressedBuffer.data(), &buffers[bv.buffer][offset], bv.byteLength);

			draco::DecoderBuffer dracoBuffer;
			draco::Decoder decoder;
			dracoBuffer.Init((char*)compressedBuffer.data(), compressedBuffer.size());
			auto status = decoder.DecodeMeshFromBuffer(&dracoBuffer);
			auto dracoMesh = std::move(status).value();
			int numPoints = dracoMesh->num_points();

			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texCoords0;
			std::vector<glm::vec2> texCoords1;
			std::vector<glm::vec4> tangents;
			std::vector<glm::vec4> joints;
			std::vector<glm::vec4> weights;

			for (auto&& [name, accIndex] : primitive.attributes)
			{
				if (name.compare("POSITION") == 0)
				{
					for (int i = 0; i < 3; i++)
					{
						// TODO: handle quantized values
						surface.minPoint[i] = gltf.accessors[accIndex].min[i];
						surface.maxPoint[i] = gltf.accessors[accIndex].max[i];
					}

					if (draco.attributes.find("POSITION") != draco.attributes.end())
					{
						// TODO: actually use accessor to get data from uncompressed buffer
						int posIndex = draco.attributes["POSITION"];
						auto positionAttr = dracoMesh->GetAttributeByUniqueId(posIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec3 pos;
							auto attrIndex = positionAttr->mapped_index(i);
							positionAttr->ConvertValue<float, 3>(attrIndex, glm::value_ptr(pos));
							positions.push_back(pos);
						}
					}
					else
					{
						loadAttribute(accIndex, positions);
					}
				}

				if (name.compare("COLOR_0") == 0)
				{
					if (draco.attributes.find("COLOR_0") != draco.attributes.end())
					{
						int colIndex = draco.attributes["COLOR_0"];
						auto colorAttr = dracoMesh->GetAttributeByUniqueId(colIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec4 color;
							auto attrIndex = colorAttr->mapped_index(i);
							colorAttr->ConvertValue<float, 4>(attrIndex, glm::value_ptr(color));
							colors.push_back(color);
						}
					}
					else
					{
						loadAttribute(accIndex, colors);
					}
				}

				if (name.compare("NORMAL") == 0)
				{
					if (draco.attributes.find("NORMAL") != draco.attributes.end())
					{
						int normalIndex = draco.attributes["NORMAL"];
						auto normalAttr = dracoMesh->GetAttributeByUniqueId(normalIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec3 normal;
							auto attrIndex = normalAttr->mapped_index(i);
							normalAttr->ConvertValue<float, 3>(attrIndex, glm::value_ptr(normal));
							normals.push_back(normal);
						}
					}
					else
					{
						loadAttribute(accIndex, normals);
					}
				}

				if (name.compare("TEXCOORD_0") == 0)
				{
					if (draco.attributes.find("TEXCOORD_0") != draco.attributes.end())
					{
						int texCoordIndex = draco.attributes["TEXCOORD_0"];
						auto texAttr = dracoMesh->GetAttributeByUniqueId(texCoordIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec2 texCoord;
							auto attrIndex = texAttr->mapped_index(i);
							texAttr->ConvertValue<float, 2>(attrIndex, glm::value_ptr(texCoord));
							texCoords0.push_back(texCoord);
						}
					}
					else
					{
						loadAttribute(accIndex, texCoords0);
					}
				}

				if (name.compare("TEXCOORD_1") == 0)
				{
					if (draco.attributes.find("TEXCOORD_1") != draco.attributes.end())
					{
						int texCoordIndex = draco.attributes["TEXCOORD_1"];
						auto texAttr = dracoMesh->GetAttributeByUniqueId(texCoordIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec2 texCoord;
							auto attrIndex = texAttr->mapped_index(i);
							texAttr->ConvertValue<float, 2>(attrIndex, glm::value_ptr(texCoord));
							texCoords0.push_back(texCoord);
						}
					}
					else
					{
						loadAttribute(accIndex, texCoords0);
					}
				}

				if (name.compare("TANGENT") == 0)
				{
					if (draco.attributes.find("TANGENT") != draco.attributes.end())
					{
						int tangentIndex = draco.attributes["TANGENT"];
						auto tangetAttr = dracoMesh->GetAttributeByUniqueId(tangentIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec4 tangent;
							auto attrIndex = tangetAttr->mapped_index(i);
							tangetAttr->ConvertValue<float, 4>(attrIndex, glm::value_ptr(tangent));
							tangents.push_back(tangent);
						}
					}
					else
					{
						loadAttribute(accIndex, tangents);
					}
				}

				if (name.compare("JOINTS_0") == 0)
				{
					if (draco.attributes.find("JOINTS_0") != draco.attributes.end())
					{
						int jointsIndex = draco.attributes["JOINTS_0"];
						auto jointAttr = dracoMesh->GetAttributeByUniqueId(jointsIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::u16vec4 jointsUInt16;
							auto attrIndex = jointAttr->mapped_index(i);
							jointAttr->ConvertValue<unsigned short, 4>(attrIndex, glm::value_ptr(jointsUInt16));
							joints.push_back(jointsUInt16);
						}
					}
					else
					{
						loadAttribute(accIndex, joints);
					}
				}

				if (name.compare("WEIGHTS_0") == 0)
				{
					if (draco.attributes.find("WEIGHTS_0") != draco.attributes.end())
					{
						int weightsIndex = draco.attributes["WEIGHTS_0"];
						auto weightsAttr = dracoMesh->GetAttributeByUniqueId(weightsIndex);
						for (draco::PointIndex i(0); i < numPoints; i++)
						{
							glm::vec4 weight;
							auto attrIndex = weightsAttr->mapped_index(i);
							weightsAttr->ConvertValue<float, 4>(attrIndex, glm::value_ptr(weight));
							weights.push_back(weight);
						}

					}
					else
					{
						loadAttribute(accIndex, weights);
					}
				}
			}

			std::vector<GLuint> indices;
			if (primitive.indices.has_value())
			{
				{
					// TODO: check accessor index type.
					for (draco::FaceIndex f(0); f < dracoMesh->num_faces(); f++)
					{
						auto face = dracoMesh->face(f);
						for (int i = 0; i < face.size(); i++)
							indices.push_back(face[i].value());
					}
				}
				//else
				//{
				//	auto indicesNode = primitiveNode.FindMember("indices");
				//	int accIndex = indicesNode->value.GetInt();
				//	int type = data.accessors[accIndex].componentType;
				//	switch (type)
				//	{
				//	case GL_UNSIGNED_BYTE:
				//	{
				//		std::vector<GLubyte> byteIndices;
				//		data.loadData(accIndex, byteIndices);
				//		for (auto i : byteIndices)
				//			indices.push_back(i);
				//		break;
				//	}
				//	case GL_UNSIGNED_SHORT:
				//	{
				//		std::vector<GLushort> shortIndices;
				//		data.loadData(accIndex, shortIndices);
				//		for (auto i : shortIndices)
				//			indices.push_back(i);
				//		break;
				//	}
				//	case GL_UNSIGNED_INT:
				//		data.loadData(accIndex, indices);
				//		break;
				//	default:
				//		std::cout << "index type not supported!!!" << std::endl;
				//		break;
				//	}
				//}
			}

			for (int i = 0; i < positions.size(); i++)
			{
				Vertex v;
				v.position = positions[i];
				if (i < colors.size())
					v.color = colors[i];
				if (i < normals.size())
					v.normal = normals[i];
				if (i < texCoords0.size())
					v.texCoord0 = texCoords0[i];
				if (i < texCoords1.size())
					v.texCoord1 = texCoords1[i];
				if (i < tangents.size())
					v.tangent = tangents[i];
				if (i < joints.size())
					v.joints = joints[i];
				if (i < weights.size())
				{
					float sum = 0.0f;
					for (int j = 0; j < 4; j++)
						sum += weights[i][j];
					v.weights = weights[i] / sum;
				}
				surface.vertices.push_back(v);
			}

			for (int i = 0; i < indices.size(); i++)
				surface.indices.push_back(indices[i]);

			bool computeFlatNormals = false;
			uint32 mode = primitive.mode;
			if (mode == 4)
			{
				if (normals.empty())
				{
					if (indices.empty())
						surface.calcFlatNormals();
					else
						computeFlatNormals = true;
				}
				if (tangents.empty())
				{
					TangentSpace ts;
					ts.generateTangents(surface, true);
					//surface.calcTangentSpace();
				}					
			}
		}
#endif

		void Importer::loadMeshes(std::string& path)
		{
			for (auto& buf : gltf.buffers)
			{
				if (buf.uri.has_value())
				{
					std::string uri = buf.uri.value();
					std::vector<uint8> binaryData;
					if (uri.find(':') != std::string::npos)
					{
						int sepIndex = uri.find_last_of(',');
						int dataStart = sepIndex + 1;
						int dataLen = uri.length() - dataStart;
						std::string dataURI = uri.substr(0, sepIndex); // TODO: check if media type is correct etc...
						std::string dataBase64 = uri.substr(dataStart, dataLen);
						std::string data = base64_decode(dataBase64);
						binaryData.insert(binaryData.end(), data.begin(), data.end());
					}
					else
					{
						binaryData = readBinaryFile(path + "/" + uri, buf.byteLength);
					}
					
					buffers.push_back(binaryData);
				}
				else
				{
					// nothing todo here since the binary buffer has been loaded from the glb file
				}
			}

			for (int meshIdx = 0; meshIdx < gltf.meshes.size(); meshIdx++)
			{
				auto& gltfMesh = gltf.meshes[meshIdx];
				std::string name = gltfMesh.name;
				if (name.empty())
				{
					std::stringstream ss;
					ss << "Mesh_" << std::setfill('0') << std::setw(3) << meshIdx;
					name = ss.str();
				}
				auto mesh = pr::Mesh::create(name);
				for (auto v : gltf.variants)
					mesh->addVariant(v);
				
				for (int primIdx = 0; primIdx < gltfMesh.primitives.size(); primIdx++)
				{
					auto& gltfPrimitve = gltfMesh.primitives[primIdx];

					TriangleSurface surface;
#ifdef LIBS_DRACO
					if (gltfPrimitve.draco.has_value())
						loadCompressedSurface(surface, gltfPrimitve);
					else
#endif
						loadSurface(surface, gltfPrimitve);

					std::vector<MorphTarget> morphTargets;
					for (auto& target : gltfPrimitve.targets)
					{
						MorphTarget morphTarget;
						for (auto& [name, accIndex] : target)
						{
							if (name.compare("POSITION") == 0)
								loadAttribute(accIndex, morphTarget.positions);
							if (name.compare("NORMAL") == 0)
								loadAttribute(accIndex, morphTarget.normals);
							if (name.compare("TANGENT") == 0)
								loadAttribute(accIndex, morphTarget.tangents);
						}
						morphTargets.push_back(morphTarget);
					}

					auto mat = pr::Material::create("Default", "Default");
					mat->addProperty("baseColor", glm::vec4(1));
					mat->addProperty("emissive", glm::vec4(0));
					mat->addProperty("roughness", 1.0f);
					mat->addProperty("metallic", 1.0f);
					mat->addProperty("occlusion", 1.0f);
					mat->addProperty("normalScale", 1.0f);
					mat->addProperty("alphaMode", 0);
					mat->addProperty("alphaCutOff", 0.5f);
					mat->addProperty("computeFlatNormals", (int)surface.computeFlatNormals);
					mat->addProperty("ior", 1.5f);
					for (int i = 0; i < 5; i++)
						mat->addTexture("", nullptr);

					if (gltfPrimitve.material.has_value())
						mat = materials[gltfPrimitve.material.value()];

					std::stringstream ss;
					ss << "Primitive_" << std::setfill('0') << std::setw(3) << primIdx;
					auto primitive = pr::Primitive::create(ss.str(), surface, GPU::Topology(gltfPrimitve.mode));

					if (!morphTargets.empty())
						primitive->setMorphTarget(createMorphTexture(morphTargets));

					pr::SubMesh m;
					m.primitive = primitive;
					if (gltfPrimitve.mode >= 4 && surface.computeFlatNormals)
					{
						auto mat2 = pr::Material::create("Default", "Default");
						mat2->addProperty("baseColor", mat->getProperty("baseColor"));
						mat2->addProperty("emissive", mat->getProperty("emissive"));
						mat2->addProperty("roughness", mat->getProperty("roughness"));
						mat2->addProperty("metallic", mat->getProperty("metallic"));
						mat2->addProperty("occlusion", mat->getProperty("occlusion"));
						mat2->addProperty("normalScale", mat->getProperty("normalScale"));
						mat2->addProperty("alphaMode", mat->getProperty("alphaMode"));
						mat2->addProperty("alphaCutOff", mat->getProperty("alphaCutOff"));
						mat2->addProperty("computeFlatNormals", (int)surface.computeFlatNormals);
						mat2->addProperty("ior", mat->getProperty("ior"));
						mat2->setDoubleSided(mat->isDoubleSided());
						std::vector<std::string> texNames = {
							"baseColorTex", "normalTex", "metalRoughTex", "emissiveTex", "occlusionTex"
						};
						for (int i = 0; i < texNames.size(); i++)
						{
							auto texInfo = mat->getTexInfo(texNames[i]);
							auto texture = mat->getTexture(texNames[i]);
							if (texture)
								mat2->addTexture(texNames[i], texture, texInfo);
							else
								mat2->addTexture("", nullptr);
						}
							
						m.material = mat2;
					}						
					else
						m.material = mat;
					for (auto [varIdx, matIdx] : gltfPrimitve.variants)
					{
						if (matIdx < materials.size())
							m.variants.push_back(materials[matIdx]);
					}

					mesh->addSubMesh(m);
				}

				mesh->setMorphWeights(gltfMesh.weights);

				meshes.push_back(mesh);
			}
		}

		void Importer::loadTexture(std::string path, int index, bool useSRGB)
		{
			//std::string fn = path + "/" + gltfImage.uri;
			//std::cout << fn << std::endl;
			if (textures[index])
				return;

			auto gltfTexture = gltf.textures[index];
			auto gltfImage = gltf.images[gltfTexture.source.value()];

			::Image::Ptr img; // TODO: better namespace handling...
			if (gltfImage.bufferView.has_value())
			{
				// TODO: load image from embedded binary
				uint32 bufferViewIdx = gltfImage.bufferView.value();
				BufferView& bv = gltf.bufferViews[bufferViewIdx];
				Buffer& buffer = gltf.buffers[bv.buffer];

				//std::cout << gltfImage.mimeType << std::endl;

				//std::cout << "loading embedded texture " << gltfImage.name << std::endl;
				if (gltfImage.mimeType.compare("image/jpeg") == 0)
					img = IO::ImageLoader::decodeJPGFromMemory(&buffers[bv.buffer][bv.byteOffset], bv.byteLength);
				else if (gltfImage.mimeType.compare("image/png") == 0)
					img = IO::ImageLoader::decodePNGFromMemory(&buffers[bv.buffer][bv.byteOffset], bv.byteLength);
#ifdef IMAGE_WEBP
				else if (gltfImage.mimeType.compare("image/webp") == 0)
					img = IO::ImageLoader::decodeWebPFromMemory(&buffers[bv.buffer][bv.byteOffset], bv.byteLength);
#endif
#ifdef IMAGE_KTX
				else if (gltfImage.mimeType.compare("image/ktx2") == 0)
				{
					textures[index] = IO::ImageLoader::decodeKTXFromMemory(&buffers[bv.buffer][bv.byteOffset], bv.byteLength);
					textures[index]->setAddressMode(GPU::AddressMode::Repeat);
					textures[index]->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
					return;
				}
#endif
				else
					std::cout << "error: not supported mime type: " << gltfImage.mimeType << std::endl;

			}
			else
			{
				std::string uri = gltfImage.uri;
				if (uri.find(':') != std::string::npos)
				{
					int mimeStart = gltfImage.uri.find_last_of(':') + 1;
					int mimeEnd = gltfImage.uri.find_last_of(';');
					int mimeLen = mimeEnd - mimeStart;
					std::string mimeType = gltfImage.uri.substr(mimeStart, mimeLen);
					//std::cout << "mimeType: " << mimeType << std::endl;
					int sepIndex = gltfImage.uri.find_last_of(',');
					int dataStart = sepIndex + 1;
					int dataLen = gltfImage.uri.length() - dataStart;
					std::string dataURI = uri.substr(0, sepIndex); // TODO: check if media type is correct etc...
					std::string dataBase64 = uri.substr(dataStart, dataLen);
					std::string dataBinary = base64_decode(dataBase64);
					//std::vector<unsigned char> data;
					//data.insert(data.end(), dataBinary.begin(), dataBinary.end());
					//std::cout << "loading embedded texture " << gltfImage.name << std::endl;

					// TODO: other mime types
					if (mimeType.compare("image/jpeg") == 0)
						img = IO::ImageLoader::decodeJPGFromMemory((uint8*)dataBinary.data(), dataBinary.size());
					else if (mimeType.compare("image/png") == 0)
						img = IO::ImageLoader::decodePNGFromMemory((uint8*)dataBinary.data(), dataBinary.size());
#ifdef IMAGE_WEBP
					else if (mimeType.compare("image/webp") == 0)
						img = IO::ImageLoader::decodeWebPFromMemory((uint8*)dataBinary.data(), dataBinary.size());
#endif
#ifdef IMAGE_KTX
					else if (mimeType.compare("image/ktx2") == 0)
					{
						textures[index] = IO::ImageLoader::decodeKTXFromMemory((uint8*)dataBinary.data(), dataBinary.size());
						textures[index]->setAddressMode(GPU::AddressMode::Repeat);
						textures[index]->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
						return;
					}
#endif
					else
						std::cout << "error: not supported mime type: " << gltfImage.mimeType << std::endl;

				}
				else
				{
					std::string fn = path + "/" + gltfImage.uri;

					auto p = fs::path(fn);
					auto extension = p.extension().string();

					//std::cout << "loading texture " << fn << std::endl;

#ifdef IMAGE_KTX
					if (extension.compare(".ktx2") == 0)
					{
						textures[index] = IO::ImageLoader::loadKTXFromFile(fn);
						textures[index]->setAddressMode(GPU::AddressMode::Repeat);
						textures[index]->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
						return;
					}
					else
#endif
					{
						img = IO::ImageLoader::loadFromFile(fn);
					}				
				}
			}

			uint32 width = img->getWidth();
			uint32 height = img->getHeight();
			uint8* data = img->getRawPtr();
			uint32 dataSize = width * height * 4;

			GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
			GPU::Format format = useSRGB ? GPU::Format::SRGBA8 : GPU::Format::RGBA8;
			uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1;

			if (gltfTexture.sampler.has_value())
			{
				auto& sampler = gltf.samplers[gltfTexture.sampler.value()];
				if (sampler.minFilter >= GL_NEAREST_MIPMAP_NEAREST &&
					sampler.minFilter <= GL_LINEAR_MIPMAP_LINEAR)
				{
					// generate mipmaps
					uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1;
					auto texture = pr::Texture2D::create(width, height, format, levels);
					texture->upload(data, dataSize);
					texture->generateMipmaps();
					textures[index] = texture;
				}
				else
				{
					auto texture = pr::Texture2D::create(width, height, format);
					texture->upload(data, dataSize);
					textures[index] = texture;
				}
											
				textures[index]->setFilter(getFilter(sampler.minFilter), getFilter(sampler.magFilter));
				textures[index]->setAddressMode(getAddressMode(sampler.wrapS), getAddressMode(sampler.wrapT), getAddressMode(sampler.wrapT));
			}
			else
			{
				auto texture = pr::Texture2D::create(width, height, format);
				texture->upload(data, dataSize);
				textures[index] = texture;

				textures[index]->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
				textures[index]->setAddressMode(GPU::AddressMode::Repeat);
			}
		}

		glm::mat4 getTransform(TextureTransform texTransform)
		{
			glm::vec2 offset = texTransform.offset;
			float rotation = texTransform.rotation;
			glm::vec2 scale = texTransform.scale;

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


		void Importer::addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<TextureInfo> texInfo, bool useSRGB, bool isMainTex)
		{
			if (texInfo.has_value())
			{
				TextureInfo& gltfTexInfo = texInfo.value();
				loadTexture(path, gltfTexInfo.index, useSRGB);

				pr::TextureInfo texInfo;
				texInfo.uvIndex = gltfTexInfo.texCoord;
				texInfo.isMainTexture = isMainTex;

				glm::mat3 transform = glm::mat3(1.0f);
				if (gltfTexInfo.textureTransform.has_value())
				{
					auto texTransform = gltfTexInfo.textureTransform.value();
					texInfo.offset = texTransform.offset;
					texInfo.scale = texTransform.scale;
					texInfo.rotation = texTransform.rotation;
				}

				material->addTexture(name, textures[gltfTexInfo.index], texInfo);
			}
			else
			{
				material->addTexture(name, nullptr);
			}
		}

		void Importer::addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<NormalTextureInfo> texInfo)
		{
			if (texInfo.has_value())
			{
				NormalTextureInfo& gltfTexInfo = texInfo.value();
				loadTexture(path, gltfTexInfo.index, false);

				pr::TextureInfo texInfo;
				texInfo.uvIndex = gltfTexInfo.texCoord;

				glm::mat3 transform = glm::mat3(1.0f);
				if (gltfTexInfo.textureTransform.has_value())
				{
					auto texTransform = gltfTexInfo.textureTransform.value();
					texInfo.offset = texTransform.offset;
					texInfo.scale = texTransform.scale;
					texInfo.rotation = texTransform.rotation;
				}

				material->addTexture(name, textures[gltfTexInfo.index], texInfo);
			}
			else
			{
				material->addTexture(name, nullptr);
			}
		}

		void Importer::addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<OcclusionTextureInfo> texInfo)
		{
			if (texInfo.has_value())
			{
				OcclusionTextureInfo& gltfTexInfo = texInfo.value();
				loadTexture(path, gltfTexInfo.index, false);

				pr::TextureInfo texInfo;
				texInfo.uvIndex = gltfTexInfo.texCoord;

				glm::mat3 transform = glm::mat3(1.0f);
				if (gltfTexInfo.textureTransform.has_value())
				{
					auto texTransform = gltfTexInfo.textureTransform.value();
					texInfo.offset = texTransform.offset;
					texInfo.scale = texTransform.scale;
					texInfo.rotation = texTransform.rotation;
				}

				material->addTexture(name, textures[gltfTexInfo.index], texInfo);
			}
			else
			{
				material->addTexture(name, nullptr);
			}
		}

		void Importer::loadMaterials(std::string& path)
		{
			int matIdx = 0;
			for (int matIdx = 0; matIdx < gltf.materials.size(); matIdx++)
			{
				auto& gltfMaterial = gltf.materials[matIdx];
				std::string name = gltfMaterial.name;
				if (name.empty())
				{
					std::stringstream ss;
					ss << "Material_" << std::setfill('0') << std::setw(3) << matIdx;
					name = ss.str();
				}

				std::string shaderName = "Default";
				std::cout << "material " << gltfMaterial.name << std::endl;
				glm::vec4 emissive = glm::vec4(gltfMaterial.emissiveFactor, gltfMaterial.emissiveStrength);
				auto mat = pr::Material::create(name, shaderName);
				mat->setMainColor(gltfMaterial.baseColorFactor);
				mat->addProperty("baseColor", gltfMaterial.baseColorFactor);
				mat->addProperty("emissive", emissive);
				mat->addProperty("roughness", gltfMaterial.roughnessFactor);
				mat->addProperty("metallic", gltfMaterial.metallicFactor);
				if (gltfMaterial.occlusionTexture.has_value())
					mat->addProperty("occlusion", gltfMaterial.occlusionTexture.value().strength);
				else
					mat->addProperty("occlusion", 1.0f);
				if (gltfMaterial.normalTexture.has_value())
					mat->addProperty("normalScale", gltfMaterial.normalTexture.value().scale);
				else
					mat->addProperty("normalScale", 1.0f);
				int alphaMode = 0;
				if (gltfMaterial.alphaMode.compare("OPAQUE") == 0)
					alphaMode = 0;
				else if (gltfMaterial.alphaMode.compare("MASK") == 0)
					alphaMode = 1;
				else if (gltfMaterial.alphaMode.compare("BLEND") == 0)
					alphaMode = 2;
				mat->addProperty("alphaMode", alphaMode);
				mat->addProperty("alphaCutOff", gltfMaterial.alphaCutoff);
				mat->addProperty("computeFlatNormals", 0);
				if (alphaMode == 2)
					mat->setTransparent(true);
				mat->setAlphaMode(alphaMode, gltfMaterial.alphaCutoff);

				if (gltfMaterial.ior.has_value())
				{
					IOR ior = gltfMaterial.ior.value();
					mat->addProperty("ior", ior.ior);
				}
				else
				{
					mat->addProperty("ior", 1.5f);
				}

				//mat->addProperty("unlit", 0);
				mat->setDoubleSided(gltfMaterial.doubleSided);
				if (gltfMaterial.sheen.has_value())
				{
					shaderName += "Sheen";
					Sheen sheen = gltfMaterial.sheen.value();
					mat->addProperty("sheen", glm::vec4(sheen.sheenColorFactor, sheen.sheenRoughnessFactor));
				}
				if (gltfMaterial.clearcoat.has_value())
				{
					shaderName += "Clearcoat";
					Clearcoat clearcoat = gltfMaterial.clearcoat.value();
					mat->addProperty("clearcoat", clearcoat.clearcoatFactor);
					mat->addProperty("clearcoatRoughness", clearcoat.clearcoatRoughnessFactor);
					mat->addProperty("padding1", 0.0f);
					mat->addProperty("padding2", 0.0f);
				}
				if (gltfMaterial.transmission.has_value())
				{
					shaderName += "Transmission";
					mat->setTransmissive(true);
					Transmission transmission = gltfMaterial.transmission.value();
					mat->addProperty("transmission", transmission.transmissionFactor);

					if (gltfMaterial.volume.has_value())
					{
						float dispersion = 0.0f;
						if (gltfMaterial.dispersion.has_value())
							dispersion = gltfMaterial.dispersion.value().dispersion;
						Volume volume = gltfMaterial.volume.value();
						mat->addProperty("thickness", volume.thicknessFactor);
						mat->addProperty("dispersion", dispersion);
						mat->addProperty("padding2", 0.0f);
						mat->addProperty("attenuation", glm::vec4(volume.attenuationColor, volume.attenuationDistance));
					}
					else
					{
						mat->addProperty("thickness", 0.0f);
						mat->addProperty("dispersion", 0.0f);
						mat->addProperty("padding", 0.0f);
						mat->addProperty("attenuation", glm::vec4(1, 1, 1, 0));
					}
				}

				if (gltfMaterial.specular.has_value())
				{
					shaderName += "Specular";
					Specular specular = gltfMaterial.specular.value();
					mat->addProperty("specular", glm::vec4(specular.specularColorFactor, specular.specularFactor));
				}

				if (gltfMaterial.iridescence.has_value())
				{
					shaderName += "Iridescence";
					Iridescence iridescence = gltfMaterial.iridescence.value();
					mat->addProperty("iridescence", glm::vec4(
						iridescence.iridescenceFactor,
						iridescence.iridescenceIor,
						iridescence.iridescenceThicknessMinimum,
						iridescence.iridescenceThicknessMaximum
					));
				}

				if (gltfMaterial.anisotropy.has_value())
				{
					shaderName += "Anisotropy";
					Anisotropy anisotropy = gltfMaterial.anisotropy.value();
					mat->addProperty("anisotropyStrength", anisotropy.anisotropyStrength);
					mat->addProperty("anisotropyRotation", anisotropy.anisotropyRotation);
					mat->addProperty("padding1", 0.0f);
					mat->addProperty("padding2", 0.0f);
				}

				if (gltfMaterial.translucency.has_value())
				{
					shaderName += "Translucency";
					Translucency translucency = gltfMaterial.translucency.value();
					mat->addProperty("translucency", glm::vec4(translucency.diffuseTransmissionColorFactor, translucency.diffuseTransmissionFactor));

					if (gltfMaterial.volumeScatter .has_value())
					{
						float dispersion = 0.0f;
						if (gltfMaterial.dispersion.has_value())
							dispersion = gltfMaterial.dispersion.value().dispersion;
						VolumeScatter volumeScatter = gltfMaterial.volumeScatter.value();
						mat->addProperty("scattering", glm::vec4(volumeScatter.multiscatterColor, volumeScatter.scatterAnisotropy));

						if (gltfMaterial.volume.has_value())
						{
							Volume volume = gltfMaterial.volume.value();
							mat->addProperty("attenuation", glm::vec4(volume.attenuationColor, volume.attenuationDistance));
							mat->addProperty("thickness", volume.thicknessFactor);
							mat->addProperty("dispersion", dispersion);
							mat->addProperty("padding0", 0.0f);
							mat->addProperty("padding1", 0.0f);
						}
						else
						{
							mat->addProperty("attenuation", glm::vec4(0.0f));
							mat->addProperty("thickness", 0.0f);
							mat->addProperty("dispersion", 0.0f);
							mat->addProperty("padding0", 0.0f);
							mat->addProperty("padding1", 0.0f);
						}
					}
					else
					{
						mat->addProperty("scattering", glm::vec4(0));
						mat->addProperty("attenuation", glm::vec4(0.0f));
						mat->addProperty("thickness", 0.0f);
						mat->addProperty("dispersion", 0.0f);
						mat->addProperty("padding0", 0.0f);
						mat->addProperty("padding1", 0.0f);
					}
				}

				addTexture("baseColorTex", path, mat, gltfMaterial.baseColorTexture, true, true);
				addTexture("normalTex", path, mat, gltfMaterial.normalTexture);
				addTexture("metalRoughTex", path, mat, gltfMaterial.metallicRoughnessTexture, false);
				addTexture("emissiveTex", path, mat, gltfMaterial.emissiveTexture, true);
				addTexture("occlusionTex", path, mat, gltfMaterial.occlusionTexture);
				if (gltfMaterial.sheen.has_value())
				{
					Sheen sheen = gltfMaterial.sheen.value();
					addTexture("sheenColorTex", path, mat, sheen.sheenColorTexture, true);
					addTexture("sheenRoughnessTex", path, mat, sheen.sheenRoughnessTexture, false);
				}
				if (gltfMaterial.clearcoat.has_value())
				{
					Clearcoat clearcoat = gltfMaterial.clearcoat.value();
					addTexture("clearcoatTex", path, mat, clearcoat.clearcoatTexture, true);
					addTexture("clearcoatRoughnessTex", path, mat, clearcoat.clearcoatRoughnessTexture, false);
					addTexture("clearcoatNormalTex", path, mat, clearcoat.clearcoatNormalTexture);
				}
				if (gltfMaterial.transmission.has_value())
				{
					Transmission transmission = gltfMaterial.transmission.value();
					addTexture("transmissionTex", path, mat, transmission.transmissionTexture, false);

					if (gltfMaterial.volume.has_value())
					{
						Volume volume = gltfMaterial.volume.value();
						addTexture("thicknessTex", path, mat, volume.thicknessTexture, false);
					}
				}

				if (gltfMaterial.specular.has_value())
				{
					Specular specular = gltfMaterial.specular.value();
					addTexture("specularTex", path, mat, specular.specularTexture, false);
					addTexture("specularColorTex", path, mat, specular.specularColorTexture, true);
				}

				if (gltfMaterial.iridescence.has_value())
				{
					Iridescence iridescence = gltfMaterial.iridescence.value();
					addTexture("iridescenceTex", path, mat, iridescence.iridescenceTexture, false);
					addTexture("iridescenceThicknessTex", path, mat, iridescence.iridescenceThicknessTexture, false);
				}

				if (gltfMaterial.anisotropy.has_value())
				{
					Anisotropy anisotropy = gltfMaterial.anisotropy.value();
					addTexture("anisotropyTex", path, mat, anisotropy.anisotropyTexture, false);
				}

				if (gltfMaterial.translucency.has_value())
				{
					Translucency translucency = gltfMaterial.translucency.value();
					addTexture("translucencyTex", path, mat, translucency.diffuseTransmissionTexture, false);
					addTexture("translucencyColorTex", path, mat, translucency.diffuseTransmissionColorTexture, true);
					if (gltfMaterial.volume.has_value())
					{
						Volume volume = gltfMaterial.volume.value();
						addTexture("thicknessTex", path, mat, volume.thicknessTexture, false);
					}
				}

				mat->setShaderName(shaderName);
				materials.push_back(mat);
			}
		}

		pr::IChannel::Ptr Importer::loadTexTransform(Animation::Sampler& sampler, pr::AnimAttribute texAttribute, std::string texTransform, int matIndex)
		{
			if (texTransform.compare("offset") == 0)
				return loadChannel<glm::vec2>(sampler, static_cast<pr::AnimAttribute>(texAttribute | pr::AnimAttribute::TEXTURE_OFFSET), matIndex);
			else if (texTransform.compare("rotation") == 0)
				return loadChannel<float>(sampler, static_cast<pr::AnimAttribute>(texAttribute | pr::AnimAttribute::TEXTURE_ROTATION), matIndex);
			else if (texTransform.compare("scale") == 0)
				return loadChannel<glm::vec2>(sampler, static_cast<pr::AnimAttribute>(texAttribute | pr::AnimAttribute::TEXTURE_SCALE), matIndex);
			return nullptr;
		}

		pr::IChannel::Ptr Importer::loadPointer(Animation::Sampler& sampler, Animation::Channel& channel)
		{
			std::stringstream ss(channel.target.pointer.value());
			std::string element;
			std::getline(ss, element, '/');
			std::vector<std::string> elements;
			while (std::getline(ss, element, '/'))
				elements.push_back(element);

			// TODO: better error checking/parsing of pointer

			std::string group = elements[0];
			if (group.compare("cameras") == 0)
			{
				int camIndex = std::stoi(elements[1]);
				std::string camType = elements[2];
				if (camType.compare("perspective") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("aspect") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::CAMERA_ASPECT, camIndex);
					else if (attribute.compare("yfov") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::CAMERA_YFOV, camIndex);
					else if (attribute.compare("znear") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::CAMERA_ZNEAR, camIndex);
					else if (attribute.compare("zfar") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::CAMERA_ZFAR, camIndex);
				}
				else if (camType.compare("orthographic") == 0)
				{
					// TODO: add orthographic camera animation
				}
			}
			else if (group.compare("extensions") == 0)
			{
				std::string extension = elements[1];
				if (extension.compare("KHR_lights_punctual") == 0)
				{
					int lightIndex = std::stoi(elements[3]);
					std::string attribute = elements[4];
					if (attribute.compare("color") == 0)
						return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::LIGHT_COLOR, lightIndex);
					else if (attribute.compare("intensity") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::LIGHT_INTENSITY, lightIndex);
					else if (attribute.compare("range") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::LIGHT_RANGE, lightIndex);
					else if (attribute.compare("spot") == 0)
					{
						std::string angleAttrib = elements[5];
						if (angleAttrib.compare("innerConeAngle"))
							return loadChannel<float>(sampler, pr::AnimAttribute::LIGHT_INNERANGLE, lightIndex);
						else if (angleAttrib.compare("outerConeAngle"))
							return loadChannel<float>(sampler, pr::AnimAttribute::LIGHT_OUTERANGLE, lightIndex);
					}
				}
			}
			else if (group.compare("materials") == 0)
			{
				int index = std::stoi(elements[1]);
				std::string matType = elements[2];

				auto mat = materials[index];
				auto matIndex = mat->getID();
				components.insert(std::make_pair(matIndex, mat));

				if (matType.compare("extensions") == 0)
				{
					std::string extension = elements[3];
					std::string extAttribute = elements[4];
					if (extension.compare("KHR_materials_ior") == 0)
					{
						if (extAttribute.compare("ior") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_IOR, matIndex);
					}
					else if (extension.compare("KHR_materials_emissive_strength") == 0)
					{
						if (extAttribute.compare("emissiveStrength") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_EMISSIVE_STRENGTH, matIndex);
					}
					else if (extension.compare("KHR_materials_sheen") == 0)
					{
						if (extAttribute.compare("sheenColorFactor") == 0)
							return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::MATERIAL_SHEEN_COLOR_FACTOR, matIndex);
						else if (extAttribute.compare("sheenRoughnessFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_SHEEN_ROUGHNESS_FACTOR, matIndex);
						else if (extAttribute.compare("sheenColorTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_SHEEN_COLOR_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("sheenRoughnessTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_SHEEN_ROUGHNESS_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_clearcoat") == 0)
					{
						if (extAttribute.compare("clearcoatFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_CLEARCOAT_FACTOR, matIndex);
						else if (extAttribute.compare("clearcoatRoughnessFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_CLEARCOAT_ROUGHNESS_FACTOR, matIndex);
						else if (extAttribute.compare("clearcoatTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_CLEARCOAT_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("clearcoatRoughnessTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_CLEARCOAT_ROUGHNESS_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("clearcoatNormalTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_CLEARCOAT_NORMAL_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_transmission") == 0)
					{
						if (extAttribute.compare("transmissionFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_TRANSMISSION_FACTOR, matIndex);
						else if (extAttribute.compare("transmissionTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_TRANSMISSION_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_volume") == 0)
					{
						if (extAttribute.compare("thicknessFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_THICKNESS_FACTOR, matIndex);
						else if (extAttribute.compare("attenuationDistance") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_ATTENUATION_DISTANCE, matIndex);
						else if (extAttribute.compare("attenuationColor") == 0)
							return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::MATERIAL_ATTENUATION_COLOR, matIndex);
						else if (extAttribute.compare("thicknessTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_THICKNESS_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_specular") == 0)
					{
						if (extAttribute.compare("specularFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_SPECULAR_FACTOR, matIndex);
						else if (extAttribute.compare("specularColorFactor") == 0)
							return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::MATERIAL_SPECULAR_COLOR_FACTOR, matIndex);
						else if (extAttribute.compare("specularTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_SPECULAR_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("specularColorTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_SPECULAR_COLOR_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						}
					else if (extension.compare("KHR_materials_iridescence") == 0)
					{
						if (extAttribute.compare("iridescenceFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_IRIDESCENCE_FACTOR, matIndex);
						else if (extAttribute.compare("iridescenceIor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_IRIDESCENCE_IOR, matIndex);
						else if (extAttribute.compare("iridescenceThicknessMaximum") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_MINIMUM, matIndex);
						else if (extAttribute.compare("iridescenceThicknessMaximum") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM, matIndex);
						else if (extAttribute.compare("iridescenceTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_IRIDESCENCE_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("iridescenceThicknessTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_anisotropy") == 0)
					{
						if (extAttribute.compare("anisotropyStrength") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_ANISOTROPY_STRENGTH, matIndex);
						else if (extAttribute.compare("anisotropyRotation") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_ANISOTROPY_ROTATION, matIndex);
						else if (extAttribute.compare("anisotropyTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_ANISOTROPY_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_diffuse_transmission") == 0)
					{
						if (extAttribute.compare("diffuseTransmissionFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_FACTOR, matIndex);
						else if (extAttribute.compare("diffuseTransmissionColorFactor") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_COLOR_FACTOR, matIndex);
						else if (extAttribute.compare("diffuseTransmissionTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
						else if (extAttribute.compare("diffuseTransmissionColorTexture") == 0)
						{
							std::string extension = elements[6];
							if (extension.compare("KHR_texture_transform") == 0)
							{
								pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_COLOR_TEXTURE;
								std::string texTransform = elements[7];
								return loadTexTransform(sampler, attr, texTransform, matIndex);
							}
						}
					}
					else if (extension.compare("KHR_materials_dispersion") == 0)
					{
						if (extAttribute.compare("dispersion") == 0)
							return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_DISPERSION, matIndex);
					}
				}
				else if (matType.compare("pbrMetallicRoughness") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("baseColorFactor") == 0)
						return loadChannel<glm::vec4>(sampler, pr::AnimAttribute::MATERIAL_BASECOLOR, matIndex);
					else if (attribute.compare("roughnessFactor") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_ROUGHNESS, matIndex);
					else if (attribute.compare("metallicFactor") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_METALLIC, matIndex);
					else if (attribute.compare("baseColorTexture") == 0)
					{
						std::string extension = elements[5];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_BASECOLOR_TEXTURE;
							std::string texTransform = elements[6];
							return loadTexTransform(sampler, attr, texTransform, matIndex);
						}
					}
					else if (attribute.compare("metallicRoughnessTexture") == 0)
					{
						std::string extension = elements[5];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_METALROUGH_TEXTURE;
							std::string texTransform = elements[6];
							return loadTexTransform(sampler, attr, texTransform, matIndex);
						}
					}
				}
				else if (matType.compare("alphaCutoff") == 0)
					return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_ALPHA_CUTOFF, matIndex);
				else if (matType.compare("emissiveFactor") == 0)
					return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::MATERIAL_EMISSIVE_FACTOR, matIndex);
				else if (matType.compare("emissiveTexture") == 0)
				{
					std::string extension = elements[4];
					if (extension.compare("KHR_texture_transform") == 0)
					{
						pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_EMISSIVE_TEXTURE;
						std::string texTransform = elements[5];
						return loadTexTransform(sampler, attr, texTransform, matIndex);
					}
				}
				else if (matType.compare("normalTexture") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("extensions") == 0)
					{
						std::string extension = elements[4];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_NORMAL_TEXTURE;
							std::string texTransform = elements[5];
							return loadTexTransform(sampler, attr, texTransform, matIndex);
						}
					}
					else if (attribute.compare("scale") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_NORMAL_SCALE, matIndex);
				}
				else if (matType.compare("occlusionTexture") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("extensions") == 0)
					{
						std::string extension = elements[4];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							pr::AnimAttribute attr = pr::AnimAttribute::MATERIAL_OCCLUSION_TEXTURE;
							std::string texTransform = elements[5];
							return loadTexTransform(sampler, attr, texTransform, matIndex);
						}
					}
					else if (attribute.compare("strength") == 0)
						return loadChannel<float>(sampler, pr::AnimAttribute::MATERIAL_OCCLUSION_STRENGTH, matIndex);
				}
			}
			else if (group.compare("nodes") == 0)
			{
				int nodeIndex = std::stoi(elements[1]);
				std::string attribute = elements[2];
				if (attribute.compare("translation") == 0)
					return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::TRANSFORM_POSITION, nodeIndex);
				else if (attribute.compare("rotation") == 0)
					return loadChannel<glm::quat>(sampler, pr::AnimAttribute::TRANSFORM_ROTATION, nodeIndex);
				else if (attribute.compare("scale") == 0)
					return loadChannel<glm::vec3>(sampler, pr::AnimAttribute::TRANSFORM_SCALE, nodeIndex);
				else if (attribute.compare("weights") == 0)
					return loadChannel<float>(sampler, pr::AnimAttribute::TRANSFORM_WEIGHTS, nodeIndex);
			}

			std::cout << "pointer " << channel.target.pointer.value() << " not supported!" << std::endl;

			return nullptr;
		}

		void Importer::loadAnimations()
		{
			for (auto gltfAnim : gltf.animations)
			{
				if (gltfAnim.name.empty())
					gltfAnim.name = "Animation";
				auto anim = pr::Animation::create(gltfAnim.name);
				float maxTime = 0.0f;
				for (auto gltfChannel : gltfAnim.channels)
				{
					auto sampler = gltfAnim.samplers[gltfChannel.sampler];
					auto path = gltfChannel.target.path;
					uint32 index = 0;
					if (gltfChannel.target.node.has_value())
						index = gltfChannel.target.node.value();

					pr::Interpolation interp = pr::Interpolation::LINEAR;
					if (sampler.interpolation.compare("STEP") == 0)
						interp = pr::Interpolation::STEP;
					else if (sampler.interpolation.compare("LINEAR") == 0)
						interp = pr::Interpolation::LINEAR;
					else if (sampler.interpolation.compare("CUBICSPLINE") == 0)
						interp = pr::Interpolation::CUBIC;

					auto t = entities[index]->getComponent<pr::Transform>();
					components.insert(std::make_pair(t->getID(), t));

					pr::IChannel::Ptr channel;
					if (path.compare("translation") == 0)
						channel = loadChannel<glm::vec3>(sampler, pr::AnimAttribute::TRANSFORM_POSITION, t->getID());
					else if (path.compare("rotation") == 0)
						channel = loadChannel<glm::quat>(sampler, pr::AnimAttribute::TRANSFORM_ROTATION, t->getID());
					else if (path.compare("scale") == 0)
						channel = loadChannel<glm::vec3>(sampler, pr::AnimAttribute::TRANSFORM_SCALE, t->getID());
					else if (path.compare("weights") == 0)
						channel = loadChannel<float>(sampler, pr::AnimAttribute::TRANSFORM_WEIGHTS, t->getID());
					else if (path.compare("pointer") == 0)
						channel = loadPointer(sampler, gltfChannel);

					if (channel)
					{
						anim->addChannel(channel);
						maxTime = std::max(maxTime, channel->getMaxTime());
					}
				}
				anim->setDuration(maxTime);
				animations.push_back(anim);
			}
		}

		void Importer::loadSkins()
		{
			for (auto gltfSkin : gltf.skins)
			{
				std::vector<glm::mat4> inverseBindMatrices;
				loadData(gltfSkin.inverseBindMatrices.value(), inverseBindMatrices);

				auto skin = pr::Skin::create(gltfSkin.name);
				for (int i = 0; i < gltfSkin.joints.size(); i++)
					skin->addJoint(gltfSkin.joints[i], inverseBindMatrices[i]);
				if(gltfSkin.skeleton.has_value())
					skin->setSkeleton(gltfSkin.skeleton.value());

				skins.push_back(skin);
			}
		}

		pr::Entity::Ptr Importer::traverse(uint32 nodeIndex, pr::Entity::Ptr parent)
		{
			auto node = gltf.nodes[nodeIndex];
			if (node.name.empty())
				node.name = "node_" + std::to_string(nodeIndex);
			auto entity = pr::Entity::create(node.name, parent);
			auto t = entity->getComponent<pr::Transform>();
			t->setLocalPosition(node.translation);
			t->setLocalRotation(node.rotation);
			t->setLocalScale(node.scale);

			if (node.mesh.has_value())
			{
				auto mesh = meshes[node.mesh.value()];
				entity->addComponent(pr::Renderable::create(mesh));
			}			

			if (node.skin.has_value())
			{
				auto r = entity->getComponent<pr::Renderable>(); // accourding to GLTF spec a node with a skin MUST have a mesh
				auto skin = skins[node.skin.value()];
				skin->setSkeleton(nodeIndex);
				r->setSkin(skin);
			}

			if (node.camera.has_value())
			{
				Camera gltfCamera = gltf.cameras[node.camera.value()];
				if (gltfCamera.name.empty())
					gltfCamera.name = "Camera_" + std::to_string(node.camera.value());
				if (gltfCamera.perspective.has_value())
				{
					auto perspCamera = gltfCamera.perspective.value();
					auto camera = pr::Camera::create(pr::Camera::PERSPECTIVE, gltfCamera.name);
					camera->setAspectRatio(perspCamera.aspectRatio);
					camera->setFieldOfView(perspCamera.yfov);
					camera->setNearPlane(perspCamera.znear);
					camera->setFarPlane(perspCamera.zfar);
					entity->addComponent(camera);
				}
				else if (gltfCamera.orthographic.has_value())
				{
					auto orthoCamera = gltfCamera.orthographic.value();
					auto camera = pr::Camera::create(pr::Camera::ORTHOGRAPHIC, gltfCamera.name);
					camera->setOrthoProjection(orthoCamera.xmag, orthoCamera.ymag);
					camera->setNearPlane(orthoCamera.znear);
					camera->setFarPlane(orthoCamera.zfar);
					entity->addComponent(camera);
				}
				else
				{
					std::cout << "error: unknown camera type!" << std::endl;
				}
			}

			if (node.light.has_value())
			{
				Light gltfLight = gltf.lights[node.light.value()];
				pr::LightType type = pr::LightType::POINT;
				if (gltfLight.type.compare("directional") == 0)
					type = pr::LightType::DIRECTIONAL;
				else if (gltfLight.type.compare("point") == 0)
					type = pr::LightType::POINT;
				else if (gltfLight.type.compare("spot") == 0)
					type = pr::LightType::SPOT;

				auto light = pr::Light::create(type, gltfLight.color, gltfLight.intensity, gltfLight.range);
				light->setCastShadows(false);
				if (type == pr::LightType::SPOT)
					light->setConeAngles(gltfLight.innerConeAngle, gltfLight.outerConeAngle);
				entity->addComponent(light);
			}

			for (auto childIndex : node.children)
				entity->addChild(traverse(childIndex, entity));

			entities[nodeIndex] = entity;

			return entity;
		}

		pr::Entity::Ptr Importer::importModel(const std::string& filepath, uint32 sceneIndex)
		{
			auto p = fs::path(filepath);
			auto path = p.parent_path().string();
			auto filename = p.filename().string();
			auto extension = p.extension().string();
			auto name = filename.substr(0, filename.find_last_of('.'));

			if (!loadJSON(filepath))
				return nullptr;

			if (sceneIndex >= gltf.scenes.size())
			{
				std::cout << "error scene index " << sceneIndex << " does not exist in " << filename << std::endl;
				return nullptr;
			}

			textures.resize(gltf.textures.size());
			entities.resize(gltf.nodes.size());

			loadMaterials(path);
			loadMeshes(path);
			loadSkins();

			pr::Entity::Ptr root = nullptr;
			auto gltfScene = gltf.scenes[sceneIndex];
			if (gltfScene.nodes.size() == 1) // if we have only one root node, use it as scene root
			{
				uint32 rootIndex = gltfScene.nodes[0];
				root = traverse(rootIndex, nullptr);
				root->setName(name); // overwrite root name by the filename
			}				
			else // if we have multiple root nodes add them to a new root node
			{
				root = pr::Entity::create(name, nullptr);
				for (auto nodeIndex : gltfScene.nodes)
					root->addChild(traverse(nodeIndex, root));
			}

			// this makes sure we always have only one root node where we add the animator
			loadAnimations();

			if (!animations.empty())
			{
				bool playAll = root->numChildren() > 1 && skins.empty();
				auto animator = pr::Animator::create(playAll);
				animator->setNodes(entities);
				animator->setComponents(components);
				for (auto a : animations)
					animator->addAnimation(a);
				root->addComponent<pr::Animator>(animator);
			}

			return root;
		}

		int Importer::importModel(const std::string& filepath, std::vector<pr::Scene::Ptr>& scenes)
		{
			auto p = fs::path(filepath);
			auto path = p.parent_path().string();
			auto filename = p.filename().string();
			auto extension = p.extension().string();
			auto name = filename.substr(0, filename.find_last_of('.'));

			if (!loadJSON(filepath))
				return -1;

			textures.resize(gltf.textures.size());
			entities.resize(gltf.nodes.size());

			loadMaterials(path);
			loadMeshes(path);
			loadSkins();

			for (uint32 i = 0; i < gltf.scenes.size(); i++)
			{
				auto gltfScene = gltf.scenes[i];
				auto root = pr::Entity::create(name, nullptr);
				for (auto nodeIndex : gltfScene.nodes)
					root->addChild(traverse(nodeIndex, root));

				if (gltfScene.name.empty())
					gltfScene.name = "Scene_" + std::to_string(i);
				auto scene = pr::Scene::create(gltfScene.name);
				scene->addRoot(root);
				scene->checkWindingOrder();
				scenes.push_back(scene);
			}

			loadAnimations();

			if (!animations.empty())
			{
				// TODO: is this correct? 
				// animations are not tied to a scene so it should be fine to add an animator to each root node
				for (auto scene : scenes)
				{
					auto root = scene->getRootNodes()[0];
					bool playAll = root->numChildren() > 1 && skins.empty();
					auto animator = pr::Animator::create(playAll);
					animator->setNodes(entities);
					animator->setComponents(components);
					for (auto a : animations)
						animator->addAnimation(a);

					auto roots = scene->getRootNodes();
					roots[0]->addComponent<pr::Animator>(animator);
				}
			}

			return gltf.defaultScene;
		}
	}
}
