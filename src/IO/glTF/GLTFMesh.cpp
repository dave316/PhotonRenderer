#include "GLTFMesh.h"

#ifdef WITH_DRACO
#include <draco/mesh/mesh.h>
#include <draco/compression/decode.h>
#endif

#include <glm/gtc/type_ptr.hpp>

namespace IO
{
	namespace glTF
	{
		Texture2DArray::Ptr createMorphTexture(std::vector<MorphTarget>& morphTargets)
		{
			// this supports only 3 attributes now (position, normal, tangent)

			int numTargets = morphTargets.size();
			int numVertices = morphTargets[0].positions.size();
			int numAttributes = 3; // TODO: add as uniform so the texture size can be adapted
			//for (auto& t : morphTargets)
			//{
			//	int numTargetAttr = !t.positions.empty() + !t.normals.empty() + !t.tangents.empty();
			//	numAttributes = glm::max(numAttributes, numTargetAttr);
			//}
			//std::cout << "num morph targets: " << numTargets << std::endl;
			//std::cout << "positions: " << morphTargets[0].positions.size() << std::endl;
			//std::cout << "normals: " << morphTargets[0].normals.size() << std::endl;
			//std::cout << "tangents: " << morphTargets[0].tangents.size() << std::endl;
			//std::cout << "num max. attributes: " << numAttributes << std::endl;

			int width = glm::ceil(glm::sqrt(numVertices));
			//std::cout << "tex size: " << width << std::endl;

			int numLayers = numTargets * 3;
			int texSize = width * width * 3; // RGB texture for each attribute
			int totalSize = numLayers * texSize;
			float* buffer = new float[totalSize];

			int layerIndex = 0;
			for (auto& target : morphTargets)
			{
				for (int i = 0; i < target.positions.size(); i++)
				{
					glm::vec3 pos = target.positions[i];
					int idx = layerIndex * texSize + i * 3;
					buffer[idx] = pos.x;
					buffer[idx + 1] = pos.y;
					buffer[idx + 2] = pos.z;				
				}

				for (int i = 0; i < target.normals.size(); i++)
				{
					glm::vec3 normal = target.normals[i];
					int idx = (layerIndex + 1) * texSize + i * 3;
					buffer[idx] = normal.x;
					buffer[idx + 1] = normal.y;
					buffer[idx + 2] = normal.z;
				}

				for (int i = 0; i < target.tangents.size(); i++)
				{
					glm::vec3 tangent = target.tangents[i];
					int idx = (layerIndex + 2) * texSize + i * 3;
					buffer[idx] = tangent.x;
					buffer[idx + 1] = tangent.y;
					buffer[idx + 2] = tangent.z;
				}

				layerIndex += numAttributes;
			}

			auto morphTex = Texture2DArray::create(width, width, numLayers, GL::RGB32F);
			morphTex->upload(buffer);
			morphTex->setFilter(GL::NEAREST, GL::NEAREST);
			morphTex->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);

			delete[] buffer;

			return morphTex;
		}

		void loadSurface(TriangleSurface& surface, BinaryData& data, const json::Value& primitiveNode)
		{
			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texCoords0;
			std::vector<glm::vec2> texCoords1;
			std::vector<glm::vec4> tangents;
			std::vector<glm::vec4> boneIndices;
			std::vector<glm::vec4> boneWeights;			
			bool calcTangentSpace = false;
			const auto& attributesNode = primitiveNode["attributes"];

			if (attributesNode.HasMember("POSITION"))
			{
				int accIndex = attributesNode["POSITION"].GetInt();
				Accessor accessor = data.getAccessor(accIndex);
				for (int i = 0; i < 3; i++)
				{
					surface.minPoint[i] = accessor.minValues[i];
					surface.maxPoint[i] = accessor.maxValues[i];
				}
				data.loadAttribute(accIndex, positions);

				if (accessor.sparseCount > 0)
				{
					std::vector<GLuint> sparseIndices;
					std::vector<glm::vec3> sparsePositions;
					data.loadSparseData(accIndex, sparseIndices, sparsePositions);
					for (int i = 0; i < accessor.sparseCount; i++)
						positions[sparseIndices[i]] = sparsePositions[i];
				}
			}

			// TODO: check for rgb or rgba color and check accessor types
			if (attributesNode.HasMember("COLOR_0"))
				data.loadAttribute(attributesNode["COLOR_0"].GetInt(), colors);

			if (attributesNode.HasMember("NORMAL"))
				data.loadAttribute(attributesNode["NORMAL"].GetInt(), normals);
			else
				surface.calcNormals = true;

			if (attributesNode.HasMember("TEXCOORD_0"))
				data.loadAttribute(attributesNode["TEXCOORD_0"].GetInt(), texCoords0);

			if (attributesNode.HasMember("TEXCOORD_1"))
				data.loadAttribute(attributesNode["TEXCOORD_1"].GetInt(), texCoords1);

			if (attributesNode.HasMember("TANGENT"))
				data.loadAttribute(attributesNode["TANGENT"].GetInt(), tangents);
			else
				calcTangentSpace = true;

			if (attributesNode.HasMember("JOINTS_0"))
				data.loadAttribute(attributesNode["JOINTS_0"].GetInt(), boneIndices);

			if (attributesNode.HasMember("WEIGHTS_0"))
				data.loadAttribute(attributesNode["WEIGHTS_0"].GetInt(), boneWeights);

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
				if (i < tangents.size())
					v.tangent = tangents[i];
				if (i < boneIndices.size())
					v.boneIDs = boneIndices[i];
				if (i < boneWeights.size())
				{
					float weightSum = boneWeights[i].x + boneWeights[i].y + boneWeights[i].z + boneWeights[i].w;
					v.boneWeights = boneWeights[i] / weightSum;
				}
				surface.addVertex(v);
			}

			std::vector<GLuint> indices;
			if (primitiveNode.HasMember("indices"))
				data.loadIndices(primitiveNode["indices"].GetInt(), indices);

			for (int i = 0; i < indices.size(); i += 3)
			{
				TriangleIndices t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
				surface.addTriangle(t);
			}

			int mode = 4;
			if (primitiveNode.HasMember("mode"))
				mode = primitiveNode["mode"].GetInt();

			if (surface.calcNormals)
			{
				if (mode == 4)
				{
					if (surface.triangles.empty())
						surface.calcFlatNormals();
					else
						surface.computeFlatNormals = true;
				}
			}
			if (calcTangentSpace)
				surface.calcTangentSpace();
		}

#ifdef WITH_DRACO
		void loadCompressedSurface(TriangleSurface& surface, BinaryData& data, const json::Value& primitiveNode)
		{
			const auto& attributesNode = primitiveNode["attributes"];
			const auto& dracoNode = primitiveNode["extensions"]["KHR_draco_mesh_compression"];

			int bufferViewIndex = 0;
			if (dracoNode.HasMember("bufferView"))
				bufferViewIndex = dracoNode["bufferView"].GetInt();
			const auto& compressedAttrNode = dracoNode["attributes"];

			std::vector<unsigned char> compressedBuffer;
			BufferView& bv = data.bufferViews[bufferViewIndex]; // TODO: get bufferview from extension
			Buffer& buffer = data.buffers[bv.buffer];
			int offset = bv.byteOffset;
			compressedBuffer.resize(bv.byteLength);
			memcpy(compressedBuffer.data(), &buffer.data[offset], bv.byteLength);

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
			std::vector<glm::vec4> boneIndices;
			std::vector<glm::vec4> boneWeights;
			bool calcTangentSpace = false;			

			if (attributesNode.HasMember("POSITION"))
			{
				int accIndex = attributesNode["POSITION"].GetInt();
				for (int i = 0; i < 3; i++)
				{
					// TODO: handle quantized values
					surface.minPoint[i] = data.accessors[accIndex].minValues[i];
					surface.maxPoint[i] = data.accessors[accIndex].maxValues[i];
				}

				if(compressedAttrNode.HasMember("POSITION"))
				{
					// TODO: actually use accessor to get data from uncompressed buffer
					int posIndex = compressedAttrNode["POSITION"].GetInt();
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
					data.loadAttribute(accIndex, positions);
				}
			}

			if (attributesNode.HasMember("COLOR_0"))
			{
				int accIndex = attributesNode["COLOR_0"].GetInt();
				if (compressedAttrNode.HasMember("COLOR_0"))
				{
					int colIndex = compressedAttrNode["COLOR_0"].GetInt();
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
					data.loadAttribute(accIndex, colors);
				}
			}

			if (attributesNode.HasMember("NORMAL"))
			{
				int accIndex = attributesNode["NORMAL"].GetInt();
				if (compressedAttrNode.HasMember("NORMAL"))
				{
					int normalIndex = compressedAttrNode["NORMAL"].GetInt();
					auto normalAttr = dracoMesh->GetAttributeByUniqueId(normalIndex);
					for (draco::PointIndex i(0); i < numPoints; i++)
					{
						glm::vec3 normal;
						auto attrIndex = normalAttr->mapped_index(i);
						normalAttr->ConvertValue<float, 3>(attrIndex, glm::value_ptr(normal));
						//normalAttr->ConvertValue(attrIndex, 3, glm::value_ptr(normal));
						normals.push_back(normal);
					}
				}
				else
				{
					data.loadAttribute(accIndex, normals);
				}
			}
			else
			{
				surface.calcNormals = true;
			}

			if (attributesNode.HasMember("TEXCOORD_0"))
			{
				int accIndex = attributesNode["TEXCOORD_0"].GetInt();
				if (compressedAttrNode.HasMember("TEXCOORD_0"))
				{
					int texCoordIndex = compressedAttrNode["TEXCOORD_0"].GetInt();
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
					data.loadAttribute(accIndex, texCoords0);
				}
			}

			if (attributesNode.HasMember("TEXCOORD_1"))
			{
				int accIndex = attributesNode["TEXCOORD_1"].GetInt();
				if (compressedAttrNode.HasMember("TEXCOORD_1"))
				{
					int texCoordIndex = compressedAttrNode["TEXCOORD_1"].GetInt();
					auto texAttr = dracoMesh->GetAttributeByUniqueId(texCoordIndex);
					for (draco::PointIndex i(0); i < numPoints; i++)
					{
						glm::vec2 texCoord;
						auto attrIndex = texAttr->mapped_index(i);
						texAttr->ConvertValue<float, 2>(attrIndex, glm::value_ptr(texCoord));
						texCoords1.push_back(texCoord);
					}
				}
				else
				{
					data.loadAttribute(accIndex, texCoords1);
				}
			}

			if (attributesNode.HasMember("TANGENT"))
			{
				calcTangentSpace = false;
				int accIndex = attributesNode["TANGENT"].GetInt();
				if (compressedAttrNode.HasMember("TANGENT"))
				{
					int tangentIndex = compressedAttrNode["TANGENT"].GetInt();
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
					data.loadAttribute(accIndex, tangents);
				}
			}

			if (attributesNode.HasMember("JOINTS_0"))
			{
				int accIndex = attributesNode["JOINTS_0"].GetInt();
				if (compressedAttrNode.HasMember("JOINTS_0"))
				{
					int jointsIndex = compressedAttrNode["JOINTS_0"].GetInt();
					auto jointAttr = dracoMesh->GetAttributeByUniqueId(jointsIndex);
					for (draco::PointIndex i(0); i < numPoints; i++)
					{
						glm::u16vec4 joints;
						auto attrIndex = jointAttr->mapped_index(i);
						jointAttr->ConvertValue<unsigned short, 4>(attrIndex, glm::value_ptr(joints));
						boneIndices.push_back(joints);
					}
				}
				else
				{
					data.loadAttribute(accIndex, boneIndices);
				}
			}

			if (attributesNode.HasMember("WEIGHTS_0"))
			{
				int accIndex = attributesNode["WEIGHTS_0"].GetInt();
				if (compressedAttrNode.HasMember("WEIGHTS_0"))
				{
					int weightsIndex = compressedAttrNode["WEIGHTS_0"].GetInt();
					auto weightsAttr = dracoMesh->GetAttributeByUniqueId(weightsIndex);
					for (draco::PointIndex i(0); i < numPoints; i++)
					{
						glm::vec4 weights;
						auto attrIndex = weightsAttr->mapped_index(i);
						weightsAttr->ConvertValue<float, 4>(attrIndex, glm::value_ptr(weights));
						boneWeights.push_back(weights);
					}

				}
				else
				{
					data.loadAttribute(accIndex, boneWeights);
				}
			}

			std::vector<GLuint> indices;
			if (primitiveNode.HasMember("indices"))
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
				Vertex v(positions[i]);
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
				if (i < boneIndices.size())
					v.boneIDs = boneIndices[i];
				if (i < boneWeights.size())
				{
					float weightSum = boneWeights[i].x + boneWeights[i].y + boneWeights[i].z + boneWeights[i].w;
					v.boneWeights = boneWeights[i] / weightSum;
				}
				surface.addVertex(v);
			}

			for (int i = 0; i < indices.size(); i += 3)
			{
				TriangleIndices t(indices[i], indices[(int64_t)i + 1], indices[(int64_t)i + 2]);
				surface.addTriangle(t);
			}

			int mode = 4;
			if (primitiveNode.HasMember("mode"))
				mode = primitiveNode["mode"].GetInt();

			if (surface.calcNormals)
			{
				if (mode == 4)
				{
					if (surface.triangles.empty())
						surface.calcFlatNormals();
					else
						surface.computeFlatNormals = true;
				}
			}
			if (calcTangentSpace)
				surface.calcTangentSpace();
		}
#endif

		Mesh::Ptr loadMesh(const json::Value& meshNode, BinaryData& data, std::vector<Material::Ptr> materials)
		{
			std::string name = "";
			if (meshNode.HasMember("name"))
				name = meshNode["name"].GetString();

			auto mesh = Mesh::create(name);

			if (meshNode.HasMember("weights"))
			{
				std::vector<float> weights;
				auto weightsNode = meshNode.FindMember("weights");
				for (auto& weightNode : weightsNode->value.GetArray())
				{
					weights.push_back(weightNode.GetFloat());
				}
				mesh->setMorphWeights(weights);
			}			

			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				TriangleSurface surface;

#ifdef WITH_DRACO
				bool compressedPrimitive = false;
				if (primitiveNode.HasMember("extensions"))
				{
					auto& extensionNode = primitiveNode["extensions"];
					if (extensionNode.HasMember("KHR_draco_mesh_compression"))
						compressedPrimitive = true;
				}

				
				if(compressedPrimitive)
					loadCompressedSurface(surface, data, primitiveNode);
				else
#endif
					loadSurface(surface, data, primitiveNode);

				int materialIndex = -1;
				if (primitiveNode.HasMember("material"))
					materialIndex = primitiveNode["material"].GetInt();

				int mode = 4;
				if (primitiveNode.HasMember("mode"))
					mode = primitiveNode["mode"].GetInt();

				std::map<int, int> variantMapping;
				if (primitiveNode.HasMember("extensions"))
				{
					auto& extensionNode = primitiveNode["extensions"];
					if (extensionNode.HasMember("KHR_materials_variants"))
					{
						auto& variantsNode = extensionNode["KHR_materials_variants"];
						if (variantsNode.HasMember("mappings"))
						{
							for (auto& mappintNode : variantsNode["mappings"].GetArray())
							{
								int matIndex = mappintNode["material"].GetInt();
								std::vector<int> variantIndices;
								for (auto& variantNode : mappintNode["variants"].GetArray())
									variantMapping.insert(std::make_pair(variantNode.GetInt(), matIndex));
							}
						}
					}
				}

				std::vector<MorphTarget> morphTargets;
				if (primitiveNode.HasMember("targets"))
				{
					for (auto& targetNode : primitiveNode["targets"].GetArray())
					{
						MorphTarget t;
						if (targetNode.HasMember("POSITION"))
						{
							int accIndex = targetNode["POSITION"].GetInt();
							data.loadAttribute(accIndex, t.positions);
						}
						if (targetNode.HasMember("NORMAL"))
						{
							int accIndex = targetNode["NORMAL"].GetInt();
							data.loadAttribute(accIndex, t.normals);
						}
						if (targetNode.HasMember("TANGENT"))
						{
							int accIndex = targetNode["TANGENT"].GetInt();
							data.loadAttribute<3>(accIndex, t.tangents);
						}
						morphTargets.push_back(t);
					}
				}

				Texture2DArray::Ptr morphTex = nullptr;
				if (!morphTargets.empty())
					morphTex = createMorphTexture(morphTargets);

				auto defaultMaterial = getDefaultMaterial();
				if (mode != 4 && surface.calcNormals) // TODO: check outher triangle topologies
					defaultMaterial->addProperty("material.unlit", true);

				Material::Ptr mat = defaultMaterial;
				if (materialIndex < materials.size())
				{
					mat = materials[materialIndex];
					if (mode != 4 && surface.calcNormals)
					{
						// TODO: add copying of materials
						glm::vec4 baseColor = mat->getPropertyValue<glm::vec4>("material.baseColorFactor");
						glm::vec3 emission = mat->getPropertyValue<glm::vec3>("material.emissiveFactor");
						defaultMaterial->addProperty("material.baseColorFactor", baseColor);
						defaultMaterial->addProperty("material.emissiveFactor", emission);
						mat = defaultMaterial;
					}
				}

				auto prim = Primitive::create(name, surface, mode, mat);
				for (auto [varIdx, matIdx] : variantMapping)
				{
					if (matIdx < materials.size())
						prim->addVariant(varIdx, materials[matIdx]);
				}

				if (!morphTargets.empty())
					prim->setMorphTarget(morphTex);

				prim->setBoundingBox(surface.minPoint, surface.maxPoint);
				prim->setFlatNormals(surface.computeFlatNormals);
				mesh->addPrimitive(prim);
			}
			return mesh;
		}
	}
}
