#ifndef INCLUDED_GLTFIMPORTER
#define INCLUDED_GLTFIMPORTER

#pragma once

#include <optional>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <GPU/GL/GLPlatform.h>
#include <rapidjson/document.h>
#include <Core/Renderable.h>
#include <Core/Camera.h>
#include <Core/Light.h>
#include <Core/Entity.h>
#include <Core/Animator.h>
#include <Core/Scene.h>
#include <Graphics/Texture.h>
#include <Graphics/Skin.h>
#include <Platform/Types.h>
namespace json = rapidjson;
namespace IO
{
	namespace glTF
	{
		struct Asset
		{
			std::string copyright;
			std::string generator;
			std::string version;
			std::string minVersion;

			void parse(const json::Value& value)
			{
				if (value.HasMember("copyright"))
					copyright = value["copyright"].GetString();
				if (value.HasMember("generator"))
					generator = value["generator"].GetString();
				if (value.HasMember("version"))
					version = value["version"].GetString();
				else
					std::cout << "glTF json error: asset object requires version!" << std::endl;
				if (value.HasMember("minVersion"))
					minVersion = value["minVersion"].GetString();
			}
		};

		struct Scene
		{
			std::string name;
			std::vector<uint32> nodes;

			void parse(const json::Value& value)
			{
				if (value.HasMember("name"))
					name = value["name"].GetString();
				if (value.HasMember("nodes"))
					for (auto& node : value["nodes"].GetArray())
						nodes.push_back(node.GetUint());
			}
		};

		glm::vec2 toVec2(const json::Value& value);
		glm::vec3 toVec3(const json::Value& value);
		glm::vec4 toVec4(const json::Value& value);
		glm::quat toQuat(const json::Value& value);
		glm::mat4 toMat4(const json::Value& value);

		struct Node
		{
			std::optional<uint32> camera;
			std::optional<uint32> mesh;
			std::optional<uint32> skin;
			std::optional<uint32> light;
			std::string name;
			glm::vec3 translation = glm::vec3(0.0f);
			glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			std::vector<float> weights;
			std::vector<uint32> children;

			void parse(const json::Value& value)
			{
				if (value.HasMember("camera"))
					camera = value["camera"].GetUint();
				if (value.HasMember("mesh"))
					mesh = value["mesh"].GetUint();
				if (value.HasMember("skin"))
					skin = value["skin"].GetUint();
				if (value.HasMember("name"))
					name = value["name"].GetString();
				if (value.HasMember("translation"))
					translation = toVec3(value["translation"]);
				if (value.HasMember("rotation"))
					rotation = toQuat(value["rotation"]);
				if (value.HasMember("scale"))
					scale = toVec3(value["scale"]);
				if (value.HasMember("matrix"))
				{
					glm::mat4 M = toMat4(value["matrix"]);
					glm::vec3 skew;
					glm::vec4 perspective;
					glm::decompose(M, scale, rotation, translation, skew, perspective);
				}
				if (value.HasMember("weights"))
					for(auto& weightNode : value["weights"].GetArray())
						weights.push_back(weightNode.GetFloat());
				if (value.HasMember("children"))
					for (auto& childNode : value["children"].GetArray())
						children.push_back(childNode.GetUint());
				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_lights_punctual"))
					{
						auto& extLight = extNode["KHR_lights_punctual"];
						if (extLight.HasMember("light"))
							light = extLight["light"].GetUint();
					}
				}
			}
		};

		typedef std::map<std::string, uint32> Target;

		struct DracoMeshCompression
		{
			uint32 bufferView = 0;
			std::map<std::string, uint32> attributes;

			void parse(const json::Value& value)
			{
				if (value.HasMember("bufferView"))
					bufferView = value["bufferView"].GetUint();
				else
					std::cout << "gltf json error: sparse accessor requires bufferView!" << std::endl;
				if (value.HasMember("attributes"))
					for (auto& attributeNode : value["attributes"].GetObj())
						attributes.insert(std::make_pair(attributeNode.name.GetString(), attributeNode.value.GetUint()));
			}
		};

		struct Primitive
		{
			std::map<std::string, uint32> attributes;
			std::optional<uint32> indices;
			std::optional<uint32> material;
			uint32 mode = 4;
			std::vector<Target> targets;
			std::map<int, int> variants;
			std::optional<DracoMeshCompression> draco;

			void parse(const json::Value& value)
			{
				if (value.HasMember("attributes"))
					for (auto& attributeNode : value["attributes"].GetObj())
						attributes.insert(std::make_pair(attributeNode.name.GetString(), attributeNode.value.GetUint()));
				if (value.HasMember("indices"))
					indices = value["indices"].GetUint();
				if (value.HasMember("material"))
					material = value["material"].GetUint();
				if (value.HasMember("mode"))
					mode = value["mode"].GetUint();
				if (value.HasMember("targets"))
				{
					for (auto& targetNode : value["targets"].GetArray())
					{
						Target target;
						for (auto& targetAttr : targetNode.GetObj())
							target.insert(std::make_pair(targetAttr.name.GetString(), targetAttr.value.GetUint()));
						targets.push_back(target);
					}
				}
				if (value.HasMember("extensions"))
				{
					auto& extensionNode = value["extensions"];
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
									variants.insert(std::make_pair(variantNode.GetInt(), matIndex));
							}
						}
					}

					if (extensionNode.HasMember("KHR_draco_mesh_compression"))
					{
						draco = DracoMeshCompression();
						draco.value().parse(extensionNode["KHR_draco_mesh_compression"]);
					}
				}
			}
		};

		struct Mesh
		{
			std::vector<Primitive> primitives;
			std::vector<float> weights;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("primitives"))
				{
					for (auto& primitiveNode : value["primitives"].GetArray())
					{
						Primitive primitive;
						primitive.parse(primitiveNode);
						primitives.push_back(primitive);
					}
				}
				else
				{
					std::cout << "glTF json error: mesh requires a list of primitives" << std::endl;
				}

				if (value.HasMember("weights"))
					for (auto& weightNode : value["weights"].GetArray())
						weights.push_back(weightNode.GetFloat());

				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct TextureTransform
		{
			glm::vec2 offset = glm::vec2(0.0f);
			float rotation = 0.0f;
			glm::vec2 scale = glm::vec2(1.0f);
			std::optional<uint32> texCoord;

			void parse(const json::Value& value)
			{
				if (value.HasMember("offset"))
					offset = toVec2(value["offset"]);
				if (value.HasMember("rotation"))
					rotation = value["rotation"].GetFloat();
				if (value.HasMember("scale"))
					scale = toVec2(value["scale"]);
				if (value.HasMember("texCoord"))
					texCoord = value["texCoord"].GetUint();
			}
		};

		struct TextureInfo
		{
			uint32 index;
			uint32 texCoord = 0;
			std::optional<TextureTransform> textureTransform;

			void parse(const json::Value& value)
			{
				if (value.HasMember("index"))
					index = value["index"].GetUint();
				else
					std::cout << "glTF json error: texture info requires an index" << std::endl;

				if (value.HasMember("texCoord"))
					texCoord = value["texCoord"].GetUint();

				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_texture_transform"))
					{
						TextureTransform  texTransform;
						texTransform.parse(extNode["KHR_texture_transform"]);
						textureTransform = texTransform;
					}						
				}
			}
		};

		struct NormalTextureInfo
		{
			uint32 index;
			uint32 texCoord = 0;
			float scale = 1.0f;
			std::optional<TextureTransform> textureTransform;

			void parse(const json::Value& value)
			{
				if (value.HasMember("index"))
					index = value["index"].GetUint();
				else
					std::cout << "glTF json error: texture info requires an index" << std::endl;

				if (value.HasMember("texCoord"))
					texCoord = value["texCoord"].GetUint();

				if (value.HasMember("scale"))
					scale = value["scale"].GetFloat();

				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_texture_transform"))
					{
						TextureTransform  texTransform;
						texTransform.parse(extNode["KHR_texture_transform"]);
						textureTransform = texTransform;
					}
				}
			}
		};

		struct OcclusionTextureInfo
		{
			uint32 index;
			uint32 texCoord = 0;
			float strength = 1.0f;
			std::optional<TextureTransform> textureTransform;

			void parse(const json::Value& value)
			{
				if (value.HasMember("index"))
					index = value["index"].GetUint();
				else
					std::cout << "glTF json error: texture info requires an index" << std::endl;

				if (value.HasMember("texCoord"))
					texCoord = value["texCoord"].GetUint();

				if (value.HasMember("strength"))
					strength = value["strength"].GetFloat();

				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_texture_transform"))
					{
						TextureTransform  texTransform;
						texTransform.parse(extNode["KHR_texture_transform"]);
						textureTransform = texTransform;
					}
				}
			}
		};

		struct Sheen
		{
			glm::vec3 sheenColorFactor = glm::vec3(0.0f);
			float sheenRoughnessFactor = 0.0f;
			std::optional<TextureInfo> sheenColorTexture;
			std::optional<TextureInfo> sheenRoughnessTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("sheenColorFactor"))
					sheenColorFactor = toVec3(value["sheenColorFactor"]);
				if (value.HasMember("sheenRoughnessFactor"))
					sheenRoughnessFactor = value["sheenRoughnessFactor"].GetFloat();
				if (value.HasMember("sheenColorTexture"))
				{
					sheenColorTexture = TextureInfo();
					sheenColorTexture.value().parse(value["sheenColorTexture"]);
				}
				if (value.HasMember("sheenRoughnessTexture"))
				{
					sheenRoughnessTexture = TextureInfo();
					sheenRoughnessTexture.value().parse(value["sheenRoughnessTexture"]);
				}
			}
		};

		struct Clearcoat
		{
			float clearcoatFactor = 0.0f;
			float clearcoatRoughnessFactor = 0.0f;
			std::optional<TextureInfo> clearcoatTexture;
			std::optional<TextureInfo> clearcoatRoughnessTexture;
			std::optional<NormalTextureInfo> clearcoatNormalTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("clearcoatFactor"))
					clearcoatFactor = value["clearcoatFactor"].GetFloat();
				if (value.HasMember("clearcoatRoughnessFactor"))
					clearcoatRoughnessFactor = value["clearcoatRoughnessFactor"].GetFloat();
				if (value.HasMember("clearcoatTexture"))
				{
					clearcoatTexture = TextureInfo();
					clearcoatTexture.value().parse(value["clearcoatTexture"]);
				}
				if (value.HasMember("clearcoatRoughnessTexture"))
				{
					clearcoatRoughnessTexture = TextureInfo();
					clearcoatRoughnessTexture.value().parse(value["clearcoatRoughnessTexture"]);
				}
				if (value.HasMember("clearcoatNormalTexture"))
				{
					clearcoatNormalTexture = NormalTextureInfo();
					clearcoatNormalTexture.value().parse(value["clearcoatNormalTexture"]);
				}
			}
		};

		struct Transmission
		{
			float transmissionFactor = 0.0f;
			std::optional<TextureInfo> transmissionTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("transmissionFactor"))
					transmissionFactor = value["transmissionFactor"].GetFloat();
				if (value.HasMember("transmissionTexture"))
				{
					transmissionTexture = TextureInfo();
					transmissionTexture.value().parse(value["transmissionTexture"]);
				}
			}
		};

		struct Volume
		{
			float thicknessFactor = 0.0f;
			std::optional<TextureInfo> thicknessTexture;
			float attenuationDistance = 0.0f;
			glm::vec3 attenuationColor = glm::vec3(1.0f);

			void parse(const json::Value& value)
			{
				if (value.HasMember("thicknessFactor"))
					thicknessFactor = value["thicknessFactor"].GetFloat();
				if (value.HasMember("thicknessTexture"))
				{
					thicknessTexture = TextureInfo();
					thicknessTexture.value().parse(value["thicknessTexture"]);
				}
				if (value.HasMember("attenuationDistance"))
					attenuationDistance = value["attenuationDistance"].GetFloat();
				if (value.HasMember("attenuationColor"))
					attenuationColor = toVec3(value["attenuationColor"]);
			}
		};

		struct IOR
		{
			float ior = 1.5f;
			void parse(const json::Value& value)
			{
				if (value.HasMember("ior"))
					ior = value["ior"].GetFloat();
			}
		};

		struct Specular
		{
			float specularFactor = 1.0;
			glm::vec3 specularColorFactor = glm::vec3(1.0f);
			std::optional<TextureInfo> specularTexture;
			std::optional<TextureInfo> specularColorTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("specularFactor"))
					specularFactor = value["specularFactor"].GetFloat();
				if (value.HasMember("specularColorFactor"))
					specularColorFactor = toVec3(value["specularColorFactor"]);
				if (value.HasMember("specularTexture"))
				{
					specularTexture = TextureInfo();
					specularTexture.value().parse(value["specularTexture"]);
				}
				if (value.HasMember("specularColorTexture"))
				{
					specularColorTexture = TextureInfo();
					specularColorTexture.value().parse(value["specularColorTexture"]);
				}
			}
		};

		struct Iridescence
		{
			float iridescenceFactor = 0.0f;
			float iridescenceIor = 1.3f;
			float iridescenceThicknessMinimum = 100.0f;
			float iridescenceThicknessMaximum = 400.0f;
			std::optional<TextureInfo> iridescenceTexture;
			std::optional<TextureInfo> iridescenceThicknessTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("iridescenceFactor"))
					iridescenceFactor = value["iridescenceFactor"].GetFloat();
				if (value.HasMember("iridescenceIor"))
					iridescenceIor = value["iridescenceIor"].GetFloat();
				if (value.HasMember("iridescenceThicknessMinimum"))
					iridescenceThicknessMinimum = value["iridescenceThicknessMinimum"].GetFloat();
				if (value.HasMember("iridescenceThicknessMaximum"))
					iridescenceThicknessMaximum = value["iridescenceThicknessMaximum"].GetFloat();
				if (value.HasMember("iridescenceTexture"))
				{
					iridescenceTexture = TextureInfo();
					iridescenceTexture.value().parse(value["iridescenceTexture"]);
				}
				if (value.HasMember("iridescenceThicknessTexture"))
				{
					iridescenceThicknessTexture = TextureInfo();
					iridescenceThicknessTexture.value().parse(value["iridescenceThicknessTexture"]);
				}
			}
		};

		struct Anisotropy
		{
			float anisotropyStrength = 0.0f;
			float anisotropyRotation = 0.0f;
			std::optional<TextureInfo> anisotropyTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("anisotropyStrength"))
					anisotropyStrength = value["anisotropyStrength"].GetFloat();
				if (value.HasMember("anisotropyRotation"))
					anisotropyRotation = value["anisotropyRotation"].GetFloat();
				if (value.HasMember("anisotropyTexture"))
				{
					anisotropyTexture = TextureInfo();
					anisotropyTexture.value().parse(value["anisotropyTexture"]);
				}
			}
		};

		struct Translucency
		{
			float diffuseTransmissionFactor = 0.0f;
			glm::vec3 diffuseTransmissionColorFactor = glm::vec3(1.0f);
			std::optional<TextureInfo> diffuseTransmissionTexture;
			std::optional<TextureInfo> diffuseTransmissionColorTexture;

			void parse(const json::Value& value)
			{
				if (value.HasMember("diffuseTransmissionFactor"))
					diffuseTransmissionFactor = value["diffuseTransmissionFactor"].GetFloat();
				if (value.HasMember("diffuseTransmissionColorFactor"))
					diffuseTransmissionColorFactor = toVec3(value["diffuseTransmissionColorFactor"]);
				if (value.HasMember("diffuseTransmissionTexture"))
				{
					diffuseTransmissionTexture = TextureInfo();
					diffuseTransmissionTexture.value().parse(value["diffuseTransmissionTexture"]);
				}
				if (value.HasMember("diffuseTransmissionColorTexture"))
				{
					diffuseTransmissionColorTexture = TextureInfo();
					diffuseTransmissionColorTexture.value().parse(value["diffuseTransmissionColorTexture"]);
				}
			}
		};

		struct Dispersion
		{
			float dispersion = 0.0f;
			void parse(const json::Value& value)
			{
				if (value.HasMember("dispersion"))
					dispersion = value["dispersion"].GetFloat();
			}
		};

		struct VolumeScatter
		{
			glm::vec3 multiscatterColor = glm::vec3(0);
			float scatterAnisotropy = 0.0f;

			void parse(const json::Value& value)
			{
				if (value.HasMember("multiscatterColor"))
					multiscatterColor = toVec3(value["multiscatterColor"]);
				if (value.HasMember("scatterAnisotropy"))
					scatterAnisotropy = value["scatterAnisotropy"].GetFloat();
			}
		};

		struct Material
		{
			std::string name;
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			std::optional<TextureInfo> baseColorTexture;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			float emissiveStrength = 1.0f;
			std::optional<TextureInfo> metallicRoughnessTexture;
			std::optional<NormalTextureInfo> normalTexture;
			std::optional<OcclusionTextureInfo> occlusionTexture;
			std::optional<TextureInfo> emissiveTexture;
			glm::vec3 emissiveFactor = glm::vec3(0.0f);
			std::string alphaMode = "OPAQUE";
			float alphaCutoff = 0.5f;
			bool doubleSided = false;
			bool unlit = false;

			std::optional<Sheen> sheen;
			std::optional<Clearcoat> clearcoat;
			std::optional<Transmission> transmission;
			std::optional<Volume> volume;
			std::optional<IOR> ior;
			std::optional<Specular> specular;
			std::optional<Iridescence> iridescence;
			std::optional<Anisotropy> anisotropy;
			std::optional<Translucency> translucency;
			std::optional<Dispersion> dispersion;
			std::optional<VolumeScatter> volumeScatter;

			void parse(const json::Value& value)
			{
				if (value.HasMember("name"))
					name = value["name"].GetString();

				if (value.HasMember("pbrMetallicRoughness"))
				{
					auto& pbrNode = value["pbrMetallicRoughness"];
					if (pbrNode.HasMember("baseColorFactor"))
						baseColorFactor = toVec4(pbrNode["baseColorFactor"]);
					if (pbrNode.HasMember("metallicFactor"))
						metallicFactor = pbrNode["metallicFactor"].GetFloat();
					if (pbrNode.HasMember("roughnessFactor"))
						roughnessFactor = pbrNode["roughnessFactor"].GetFloat();
					if (pbrNode.HasMember("baseColorTexture"))
					{
						TextureInfo texInfo;
						texInfo.parse(pbrNode["baseColorTexture"]);
						baseColorTexture = texInfo;
					}
					if (pbrNode.HasMember("metallicRoughnessTexture"))
					{
						TextureInfo texInfo;
						texInfo.parse(pbrNode["metallicRoughnessTexture"]);
						metallicRoughnessTexture = texInfo;
					}
				}

				if (value.HasMember("normalTexture"))
				{
					NormalTextureInfo texInfo;
					texInfo.parse(value["normalTexture"]);
					normalTexture = texInfo;
				}

				if (value.HasMember("occlusionTexture"))
				{
					OcclusionTextureInfo texInfo;
					texInfo.parse(value["occlusionTexture"]);
					occlusionTexture = texInfo;
				}

				if (value.HasMember("emissiveTexture"))
				{
					TextureInfo texInfo;
					texInfo.parse(value["emissiveTexture"]);
					emissiveTexture = texInfo;
				}

				if (value.HasMember("emissiveFactor"))
					emissiveFactor = toVec3(value["emissiveFactor"]);
				if (value.HasMember("alphaMode"))
					alphaMode = value["alphaMode"].GetString();
				if (value.HasMember("alphaCutoff"))
					alphaCutoff = value["alphaCutoff"].GetFloat();
				if (value.HasMember("doubleSided"))
					doubleSided = value["doubleSided"].GetBool();

				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_materials_sheen"))
					{
						sheen = Sheen();
						sheen.value().parse(extNode["KHR_materials_sheen"]);
					}
					if (extNode.HasMember("KHR_materials_clearcoat"))
					{
						clearcoat = Clearcoat();
						clearcoat.value().parse(extNode["KHR_materials_clearcoat"]);
					}
					if (extNode.HasMember("KHR_materials_transmission"))
					{
						transmission = Transmission();
						transmission.value().parse(extNode["KHR_materials_transmission"]);
					}
					if (extNode.HasMember("KHR_materials_volume"))
					{
						volume = Volume();
						volume.value().parse(extNode["KHR_materials_volume"]);
					}
					if (extNode.HasMember("KHR_materials_ior"))
					{
						ior = IOR();
						ior.value().parse(extNode["KHR_materials_ior"]);
					}
					if (extNode.HasMember("KHR_materials_specular"))
					{
						specular = Specular();
						specular.value().parse(extNode["KHR_materials_specular"]);
					}
					if (extNode.HasMember("KHR_materials_iridescence"))
					{
						iridescence = Iridescence();
						iridescence.value().parse(extNode["KHR_materials_iridescence"]);
					}
					if (extNode.HasMember("KHR_materials_anisotropy"))
					{
						anisotropy = Anisotropy();
						anisotropy.value().parse(extNode["KHR_materials_anisotropy"]);
					}
					if (extNode.HasMember("KHR_materials_diffuse_transmission"))
					{
						translucency = Translucency();
						translucency.value().parse(extNode["KHR_materials_diffuse_transmission"]);
					}
					if (extNode.HasMember("KHR_materials_emissive_strength"))
					{
						emissiveStrength = extNode["KHR_materials_emissive_strength"]["emissiveStrength"].GetFloat();
					}
					if (extNode.HasMember("KHR_materials_unlit"))
					{
						unlit = true;
					}
					if (extNode.HasMember("KHR_materials_dispersion"))
					{
						dispersion = Dispersion();
						dispersion.value().parse(extNode["KHR_materials_dispersion"]);
					}
					if (extNode.HasMember("KHR_materials_volume_scatter"))
					{
						volumeScatter = VolumeScatter();
						volumeScatter.value().parse(extNode["KHR_materials_volume_scatter"]);
					}
				}
			}
		};

		struct Buffer
		{
			std::string name;
			std::optional<std::string> uri;
			uint32 byteLength;

			void parse(const json::Value& value)
			{
				if (value.HasMember("name"))
					name = value["name"].GetString();
				if (value.HasMember("uri"))
					uri = value["uri"].GetString();
				if (value.HasMember("byteLength"))
					byteLength = value["byteLength"].GetUint();
				else
					std::cout << "glTF json error: buffer object requires byteLength!" << std::endl;
			}
		};

		struct BufferView
		{
			uint32 buffer;
			uint32 byteLength;
			uint32 byteOffset = 0;
			std::optional<uint32> byteStride;
			std::optional<uint32> target;
			std::optional<std::string> name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("buffer"))
					buffer = value["buffer"].GetUint();
				else
					std::cout << "glTF json error: buffer view requires index to buffer!" << std::endl;

				if (value.HasMember("byteLength"))
					byteLength = value["byteLength"].GetUint();
				else
					std::cout << "glTF json error: buffer view requires byteLength!" << std::endl;

				if (value.HasMember("byteOffset"))
					byteOffset = value["byteOffset"].GetUint();
				if (value.HasMember("byteStride"))
					byteStride = value["byteStride"].GetUint();
				if (value.HasMember("target"))
					target = value["target"].GetUint();
				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct Accessor
		{
			struct Sparse
			{
				struct Indices
				{
					uint32 bufferView;
					uint32 byteOffset = 0;
					uint32 componentType;

					void parse(const json::Value& value)
					{
						if (value.HasMember("bufferView"))
							bufferView = value["bufferView"].GetUint();
						else
							std::cout << "gltf json error: sparse accessor requires bufferView!" << std::endl;

						if (value.HasMember("byteOffset"))
							byteOffset = value["byteOffset"].GetUint();

						if (value.HasMember("componentType"))
							componentType = value["componentType"].GetUint();
						else
							std::cout << "glTF json error: accessor requires component type!" << std::endl;
					}
				};

				struct Values
				{
					uint32 bufferView;
					uint32 byteOffset = 0;

					void parse(const json::Value& value)
					{
						if (value.HasMember("bufferView"))
							bufferView = value["bufferView"].GetUint();
						else
							std::cout << "gltf json error: sparse accessor requires bufferView!" << std::endl;

						if (value.HasMember("byteOffset"))
							byteOffset = value["byteOffset"].GetUint();
					}
				};

				uint32 count;
				Indices indices;
				Values values;

				void parse(const json::Value& value)
				{
					if (value.HasMember("count"))
						count = value["count"].GetUint();
					else
						std::cout << "glTF json error: sparse accessor required count!" << std::endl;
										
					if (value.HasMember("indices"))
						indices.parse(value["indices"]);
					else
						std::cout << "glTF json error: sparse accessor requires indices!" << std::endl;

					if (value.HasMember("values"))
						values.parse(value["values"]);
					else
						std::cout << "glTF json error: sparse accessor requires values!" << std::endl;
				}
			};

			std::optional<uint32> bufferView;
			uint32 byteOffset = 0;
			uint32 componentType;
			bool normalized = false;
			uint32 count;
			std::string type;
			std::vector<float> max;
			std::vector<float> min;
			std::optional<std::string> name;
			std::optional<Sparse> sparse;

			void parse(const json::Value& value)
			{
				if (value.HasMember("bufferView"))
					bufferView = value["bufferView"].GetUint();
				if (value.HasMember("byteOffset"))
					byteOffset = value["byteOffset"].GetUint();

				if (value.HasMember("componentType"))
					componentType = value["componentType"].GetUint();
				else
					std::cout << "glTF json error: accessor requires component type!" << std::endl;

				if (value.HasMember("normalized"))
					normalized = value["normalized"].GetBool();

				if (value.HasMember("count"))
					count = value["count"].GetUint();
				else
					std::cout << "glTF json error: accessor requires count!" << std::endl;

				if (value.HasMember("type"))
					type = value["type"].GetString();
				else
					std::cout << "glTF json error: accessor requires type!" << std::endl;

				if (value.HasMember("min"))
				{
					for (auto& minNode : value["min"].GetArray())
						min.push_back(minNode.GetFloat());
				}
				if (value.HasMember("max"))
				{
					for (auto& maxNode : value["max"].GetArray())
						max.push_back(maxNode.GetFloat());
				}

				if (value.HasMember("name"))
					name = value["name"].GetString();

				if (value.HasMember("sparse"))
				{
					sparse = Sparse();
					sparse.value().parse(value["sparse"]);
				}
			}
		};

		struct Image
		{
			std::string uri;
			std::string mimeType;
			std::optional<uint32> bufferView;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("uri"))
				{
					uri = value["uri"].GetString();
					size_t pos = uri.find("%20");
					while (pos != std::string::npos)
					{
						uri.replace(pos, 3, " ");
						pos = uri.find("%20", pos + 1);
					}
				}					
				if (value.HasMember("mimeType"))
					mimeType = value["mimeType"].GetString();
				if (value.HasMember("bufferView"))
					bufferView = value["bufferView"].GetUint();
				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct Sampler
		{
			uint32 magFilter = GL_LINEAR;
			uint32 minFilter = GL_LINEAR_MIPMAP_LINEAR;
			uint32 wrapS = GL_REPEAT;
			uint32 wrapT = GL_REPEAT;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("magFilter"))
					magFilter = value["magFilter"].GetUint();
				if (value.HasMember("minFilter"))
					minFilter = value["minFilter"].GetUint();
				if (value.HasMember("wrapS"))
					wrapS = value["wrapS"].GetUint();
				if (value.HasMember("wrapT"))
					wrapT = value["wrapT"].GetUint();
				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct Texture
		{
			std::optional<uint32> sampler;
			std::optional<uint32> source;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("sampler"))
					sampler = value["sampler"].GetUint();
				if (value.HasMember("source"))
					source = value["source"].GetUint();
				if (value.HasMember("name"))
					name = value["name"].GetString();
				if (value.HasMember("extensions"))
				{
					auto& extNode = value["extensions"];
					if (extNode.HasMember("KHR_texture_basisu"))
						source = extNode["KHR_texture_basisu"]["source"].GetInt();
					if (extNode.HasMember("EXT_texture_webp"))
						source = extNode["EXT_texture_webp"]["source"].GetInt();
				}
			}
		};

		struct Animation
		{
			struct Target
			{
				std::optional<uint32> node;
				std::string path;
				std::optional<std::string> pointer;

				void parse(const json::Value& value)
				{
					if (value.HasMember("node"))
						node = value["node"].GetUint();
					if (value.HasMember("path"))
						path = value["path"].GetString();
					else
						std::cout << "error: animation channel target needs a non empty path!" << std::endl;
					if (value.HasMember("extensions"))
					{
						auto& extNode = value["extensions"];
						if (extNode.HasMember("KHR_animation_pointer"))
						{
							auto& extLight = extNode["KHR_animation_pointer"];
							if (extLight.HasMember("pointer"))
								pointer = extLight["pointer"].GetString();
						}
					}
				}
			};

			struct Channel
			{
				uint32 sampler;
				Target target;

				void parse(const json::Value& value)
				{
					if (value.HasMember("sampler"))
						sampler = value["sampler"].GetUint();
					else
						std::cout << "error: animation channel must contain a sampler!" << std::endl;
					if (value.HasMember("target"))
						target.parse(value["target"]);
					else
						std::cout << "error: animation channel has no target!" << std::endl;
				}
			};

			struct Sampler
			{
				uint32 input;
				std::string interpolation = "LINEAR";
				uint32 output;

				void parse(const json::Value& value)
				{
					if (value.HasMember("input"))
						input = value["input"].GetUint();
					else
						std::cout << "error: animation sampler must contain input index!" << std::endl;
					if (value.HasMember("interpolation"))
						interpolation = value["interpolation"].GetString();
					if (value.HasMember("output"))
						output = value["output"].GetUint();
					else
						std::cout << "error: animation sampler must contain output index!" << std::endl;
				}
			};

			std::vector<Channel> channels;
			std::vector<Sampler> samplers;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("channels"))
				{
					for (auto& channelNode : value["channels"].GetArray())
					{
						Channel channel;
						channel.parse(channelNode);
						channels.push_back(channel);
					}
				}
				else
				{
					std::cout << "error: animation must contain atleast 1 channel!" << std::endl;
				}

				if (value.HasMember("name"))
					name = value["name"].GetString();

				if (value.HasMember("samplers"))
				{
					for (auto& samplerNode : value["samplers"].GetArray())
					{
						Sampler sampler;
						sampler.parse(samplerNode);
						samplers.push_back(sampler);
					}
				}
				else
				{
					std::cout << "error: animation must contain atleast 1 sampler!" << std::endl;
				}
			}
		};

		struct Skin
		{
			std::optional<uint32> inverseBindMatrices;
			std::optional<uint32> skeleton;
			std::vector<uint32> joints;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("inverseBindMatrices"))
					inverseBindMatrices = value["inverseBindMatrices"].GetUint();
				if (value.HasMember("skeleton"))
					skeleton = value["skeleton"].GetUint();
				if (value.HasMember("joints"))
				{
					for (auto& joint : value["joints"].GetArray())
						joints.push_back(joint.GetUint());
				}
				else
				{
					std::cout << "error: skin must contain atleast 1 joint!" << std::endl;
				}

				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct CameraOrthographic
		{
			float xmag = 1.0f;
			float ymag = 1.0f;
			float znear = 0.1f;
			float zfar = 1.0f;

			void parse(const json::Value& value)
			{
				if (value.HasMember("xmag"))
				{
					xmag = value["xmag"].GetFloat();
					if(xmag <= 0)
						std::cout << "error: orthographic camera xmag must be greater than zero!" << std::endl;
				}					
				else
					std::cout << "error: orthographic camera needs to have xmag defined!" << std::endl;

				if (value.HasMember("ymag"))
				{
					ymag = value["ymag"].GetFloat();
					if (ymag <= 0)
						std::cout << "error: orthographic camera ymag must be greater than zero!" << std::endl;
				}
				else
					std::cout << "error: orthographic camera needs to have ymag defined!" << std::endl;
					
				if (value.HasMember("znear"))
				{
					znear = value["znear"].GetFloat();
					if(znear < 0)
						std::cout << "error: orthographic camera znear must be greater or equal to zero!" << std::endl;
				}					
				else
					std::cout << "error: orthographic camera needs to have znear defined!" << std::endl;

				if (value.HasMember("zfar"))
				{
					zfar = value["zfar"].GetFloat();
					if (zfar <= 0)
						std::cout << "error: orthographic camera zfar must be greater than zero!" << std::endl;
					if (zfar <= znear)
						std::cout << "error: orthographic camera znear must be smaller than zfar!";
				}
				else
					std::cout << "error: orthographic camera needs to have zfar defined!" << std::endl;
			}
		};

		struct CameraPerspective
		{
			float aspectRatio = 1.0f;
			float yfov = glm::pi<float>();
			float znear = 0.1f;
			float zfar = 1.0f;

			void parse(const json::Value& value)
			{
				if (value.HasMember("aspectRatio"))
				{
					aspectRatio = value["aspectRatio"].GetFloat();
					if (aspectRatio <= 0)
						std::cout << "error: perspective camera aspect ratio must be greater than zero!" << std::endl;
				}

				if (value.HasMember("yfov"))
				{
					yfov = value["yfov"].GetFloat();
					if (yfov <= 0)
						std::cout << "error: perspective camera yfov must be greater than zero!" << std::endl;
				}
				else
					std::cout << "error: perspective camera needs to have yfov defined!" << std::endl;

				if (value.HasMember("znear"))
				{
					znear = value["znear"].GetFloat();
					if (znear <= 0)
						std::cout << "error: orthographic camera znear must be greater than zero!" << std::endl;
				}
				else
					std::cout << "error: orthographic camera needs to have znear defined!" << std::endl;

				if (value.HasMember("zfar"))
				{
					zfar = value["zfar"].GetFloat();
					if (zfar <= 0)
						std::cout << "error: orthographic camera znear must be greater than zero!" << std::endl;
					if (zfar <= znear)
						std::cout << "error: orthographic camera znear must be smaller than zfar!";
				}
			}
		};

		struct Camera
		{
			std::optional<CameraOrthographic> orthographic;
			std::optional<CameraPerspective> perspective;
			std::string type;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("orthographic"))
				{
					orthographic = CameraOrthographic();
					orthographic.value().parse(value["orthographic"]);
				}
				if (value.HasMember("perspective"))
				{
					perspective = CameraPerspective();
					perspective.value().parse(value["perspective"]);
				}

				if (value.HasMember("type"))
					type = value["type"].GetString();
				else
					std::cout << "error: camera type must be defined!" << std::endl;

				if (value.HasMember("name"))
					name = value["name"].GetString();
			}
		};

		struct Light
		{
			glm::vec3 color = glm::vec3(1);
			float intensity = 1.0f;
			float range = -1.0f;
			float innerConeAngle = 0.0f;
			float outerConeAngle = glm::quarter_pi<float>();
			std::string type;
			std::string name;

			void parse(const json::Value& value)
			{
				if (value.HasMember("color"))
					color = toVec3(value["color"]);
				if (value.HasMember("intensity"))
					intensity = value["intensity"].GetFloat();
				if (value.HasMember("range"))
					range = value["range"].GetFloat();
				if (value.HasMember("spot"))
				{
					auto& spotNode = value["spot"];
					if (spotNode.HasMember("innerConeAngle"))
						innerConeAngle = value["innerConeAngle"].GetFloat();
					if (spotNode.HasMember("outerConeAngle"))
						outerConeAngle = value["outerConeAngle"].GetFloat();
				}
				if (value.HasMember("name"))
					name = value["name"].GetString();
				if (value.HasMember("type"))
					type = value["type"].GetString();
				else
					std::cout << "error light must have a valid light type!" << std::endl;
			}
		};

		class Importer
		{
		public:
			struct MorphTarget
			{
				std::vector<glm::vec3> positions;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec3> tangents;
			};

			bool loadJSON(const std::string& filename);
			std::string loadGLB(const std::string& filename);
			pr::Entity::Ptr Importer::importModel(const std::string& filepath, uint32 sceneIndex = 0);
			int importModel(const std::string& filepath, std::vector<pr::Scene::Ptr>& scenes);
			bool checkExtensions(const json::Document& doc);
			Importer();
			template<typename T>
			void loadData(uint32 accIndex, std::vector<T>& data)
			{
				Accessor& acc = gltf.accessors[accIndex];
				BufferView& bv = gltf.bufferViews[acc.bufferView.value()];
				Buffer& buffer = gltf.buffers[bv.buffer];
				uint32 offset = bv.byteOffset + acc.byteOffset;
				data.resize(acc.count);

				if (bv.byteStride.has_value() && bv.byteStride.value() > 0) // interleaving
				{
					for (int i = 0; i < data.size(); i++)
					{
						std::memcpy(&data[i], &buffers[bv.buffer][offset], sizeof(T));
						offset += bv.byteStride.value();
					}
				}
				else
				{
					std::memcpy(data.data(), &buffers[bv.buffer][offset], acc.count * sizeof(T));
				}				
			}

			template<typename T>
			void loadSparseData(uint32 accIndex, std::vector<GLuint>& indices, std::vector<T>& values)
			{
				Accessor& acc = gltf.accessors[accIndex];
				auto sparseIndices = acc.sparse.value().indices;
				BufferView& bvIdx = gltf.bufferViews[sparseIndices.bufferView];
				Buffer& bufferIdx = gltf.buffers[bvIdx.buffer];
				int offsetIdx = bvIdx.byteOffset + sparseIndices.byteOffset;
				int type = sparseIndices.componentType;
				int count = acc.sparse.value().count;
				switch (type)
				{
				case GL_UNSIGNED_BYTE:
				{
					std::vector<GLubyte> byteIndices(count);
					std::memcpy(byteIndices.data(), &buffers[bvIdx.buffer][offsetIdx], count * sizeof(GLubyte));
					for (auto i : byteIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_SHORT:
				{
					std::vector<GLushort> shortIndices(count);
					std::memcpy(shortIndices.data(), &buffers[bvIdx.buffer][offsetIdx], count * sizeof(GLushort));
					for (auto i : shortIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_INT:
					indices.resize(count);
					std::memcpy(indices.data(), &buffers[bvIdx.buffer][offsetIdx], count * sizeof(GLuint));
					break;
				default:
					std::cout << "index type not supported!!!" << std::endl;
					break;
				}

				auto sparseValues = acc.sparse.value().values;
				BufferView& bvValues = gltf.bufferViews[sparseValues.bufferView];
				Buffer& bufferValues = gltf.buffers[bvValues.buffer];
				int offsetValues = bvValues.byteOffset + sparseValues.byteOffset;
				values.resize(count);
				std::memcpy(values.data(), &buffers[bvValues.buffer][offsetValues], count * sizeof(T));
			}

			template<int n>
			void loadAttribute(int accIndex, std::vector<glm::vec<n, float, glm::packed_highp>>& buffer)
			{
				typedef glm::vec<n, float, glm::packed_highp> outVec;
				int type = gltf.accessors[accIndex].componentType;
				bool normalized = gltf.accessors[accIndex].normalized;
				switch (type)
				{
				case GL_BYTE:
				{
					std::vector<glm::vec<n, glm::i8, glm::packed_highp>> positionsInt8;
					loadData(accIndex, positionsInt8);
					for (auto c : positionsInt8)
					{
						if (normalized)
							buffer.push_back(glm::max(outVec(c) / 127.0f, -1.0f));
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_UNSIGNED_BYTE:
				{
					std::vector<glm::vec<n, glm::u8, glm::packed_highp>> positionsUInt8;
					loadData(accIndex, positionsUInt8);
					for (auto c : positionsUInt8)
					{
						if (normalized)
							buffer.push_back(outVec(c) / 255.0f);
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_SHORT:
				{
					std::vector<glm::vec<n, glm::i16, glm::packed_highp>> positionsInt16;
					loadData(accIndex, positionsInt16);
					for (auto c : positionsInt16)
					{
						if (normalized)
							buffer.push_back(glm::max(outVec(c) / 32767.0f, -1.0f));
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_UNSIGNED_SHORT:
				{
					std::vector<glm::vec<n, glm::u16, glm::packed_highp>> positionsUInt16;
					loadData(accIndex, positionsUInt16);
					for (auto c : positionsUInt16)
					{
						if (normalized)
							buffer.push_back(outVec(c) / 65535.0f);
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_FLOAT:
				{
					loadData(accIndex, buffer);
					break;
				}
				default:
					std::cout << "component type " << type << " not supported!" << std::endl;
					break;
				}
			}

			template<typename Type>
			pr::IChannel::Ptr loadChannel(Animation::Sampler sampler, pr::AnimAttribute attribute, unsigned int targetIndex)
			{
				std::vector<float> times;
				std::vector<Type> values;
				loadData(sampler.input, times);
				loadData(sampler.output, values);

				pr::Interpolation interp = pr::Interpolation::LINEAR;
				if (sampler.interpolation.compare("STEP") == 0)
					interp = pr::Interpolation::STEP;
				else if (sampler.interpolation.compare("LINEAR") == 0)
					interp = pr::Interpolation::LINEAR;
				else if (sampler.interpolation.compare("CUBICSPLINE") == 0)
					interp = pr::Interpolation::CUBIC;

				auto channel = pr::Channel<Type>::create(attribute, interp, targetIndex);
				int numValues = values.size() / times.size();
				for (int i = 0; i < times.size(); i++)
				{
					std::vector<Type> allValues;
					for (int j = 0; j < numValues; j++)
					{
						int index = i * numValues + j;
						allValues.push_back(values[index]);
					}
					channel->addValue(times[i], allValues);
				}
				return channel;
			}

			void loadSurface(TriangleSurface& surface, Primitive& primitive);
#ifdef LIBS_DRACO
			void loadCompressedSurface(TriangleSurface& surface, Primitive& primitive);
#endif
			void loadMeshes(std::string& path);
			void addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<TextureInfo> texInfo, bool useSRGB, bool isMainTex = false);
			void addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<NormalTextureInfo> texInfo);
			void addTexture(std::string name, std::string path, pr::Material::Ptr material, std::optional<OcclusionTextureInfo> texInfo);
			void loadTexture(std::string path, int index, bool useSRGB);
			void loadMaterials(std::string& path);
			void loadAnimations();
			void loadSkins();
			pr::IChannel::Ptr loadTexTransform(Animation::Sampler& sampler, pr::AnimAttribute texAttribute, std::string texTransform, int matIndex);
			pr::IChannel::Ptr loadPointer(Animation::Sampler& sampler, Animation::Channel& channel);
			pr::Texture2DArray::Ptr createMorphTexture(std::vector<MorphTarget> morphTargets);
			pr::Entity::Ptr traverse(uint32 nodeIndex, pr::Entity::Ptr parent);
			std::vector<pr::Material::Ptr> getMaterials()
			{
				return materials;
			}
		private:
			Importer(const Importer&) = delete;
			Importer& operator=(const Importer&) = delete;

			// GLTF raw data
			struct GLTF
			{
				Asset asset;
				uint32 defaultScene = 0;
				std::vector<std::string> variants;
				std::vector<Scene> scenes;
				std::vector<Node> nodes;
				std::vector<Buffer> buffers;
				std::vector<BufferView> bufferViews;
				std::vector<Accessor> accessors;
				std::vector<Mesh> meshes;
				std::vector<Material> materials;
				std::vector<Image> images;
				std::vector<Sampler> samplers;
				std::vector<Texture> textures;
				std::vector<Animation> animations;
				std::vector<Skin> skins;
				std::vector<Camera> cameras;
				std::vector<Light> lights;
			} gltf;

			std::vector<std::vector<unsigned char>> buffers;
			std::set<std::string> supportedExtensions;

			// photon renderer data
			std::vector<pr::Entity::Ptr> entities;
			std::vector<pr::Mesh::Ptr>  meshes;
			std::vector<pr::Material::Ptr> materials;
			std::vector<pr::Animation::Ptr> animations;
			std::vector<pr::Skin::Ptr> skins;
			std::vector<pr::Texture2D::Ptr> textures;
			std::map<uint32, pr::Component::Ptr> components;
		};
	}
}

#endif // INCLUDED_GLTFIMPORTER