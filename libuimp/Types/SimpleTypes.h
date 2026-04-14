#ifndef INCLUDED_SIMPLETYPES
#define INCLUDED_SIMPLETYPES

#pragma once

#include "DataTypes.h"

#include <ryml/ryml.hpp>
#include <ryml/ryml_std.hpp>

#include <iostream>

namespace Unity
{
	namespace YAML
	{
		bool read(ryml::ConstNodeRef n, glm::vec2& value);
		bool read(ryml::ConstNodeRef n, glm::vec3& value);
		bool read(ryml::ConstNodeRef n, glm::vec4& value);
		bool read(ryml::ConstNodeRef n, glm::quat& value);
		bool read(ryml::ConstNodeRef n, glm::mat4& value);

		template<class Type>
		struct SequenceType
		{
			std::vector<Type> container;
		};

		template<class Type>
		bool read(ryml::ConstNodeRef n, SequenceType<Type>* seq)
		{
			seq->container.resize(n.num_children());
			size_t index = 0;
			for (auto const ch : n.children())
			{
				std::cout << "parsing vector element " << index << std::endl;
				ch >> seq->container[index++];
			}

			return true;
		}

		template<class Key, class Value>
		struct MapType
		{
			std::map<Key, Value> container;
		};

		template<class Key, class Value>
		bool read(ryml::ConstNodeRef n, MapType<Key, Value>* map)
		{
			Key k;
			Value v;
			for (auto const ch : n)
			{
				ch >> c4::yml::key(k) >> v;
				map->container.emplace(std::make_pair(std::move(k), std::move(v)));
			}

			return true;
		}

		struct FileID
		{
			int64 fileID = 0;

			void fromYAML(ryml::ConstNodeRef n)
			{
				n["fileID"] >> fileID;
			}

			friend std::ostream& operator<< (std::ostream& stream, const FileID& value)
			{
				stream << "{fileID: " << value.fileID << "}";
				return stream;
			}
		};

		struct InstanceID
		{
			int64 instanceID = 0;

			void fromYAML(ryml::ConstNodeRef n)
			{
				n["instanceID"] >> instanceID;
			}

			friend std::ostream& operator<< (std::ostream& stream, const InstanceID& value)
			{
				stream << "{instanceID: " << value.instanceID << "}";
				return stream;
			}
		};

		struct ObjectRef
		{
			int64 fileID = 0;
			uint32 type = 0;
			std::string guid = "";

			ObjectRef() {}
			ObjectRef(int64 fileID, uint32 type, std::string guid) :
				fileID(fileID),
				type(type),
				guid(guid)
			{

			}

			void fromYAML(ryml::ConstNodeRef n)
			{
				n["fileID"] >> fileID;
				if (n.has_child("type"))
					n["type"] >> type;
				if (n.has_child("guid"))
					n["guid"] >> guid;
			}

			friend std::ostream& operator<< (std::ostream& stream, const ObjectRef& value)
			{
				if (value.guid.empty())
					stream << "{fileID: " << value.fileID << "}";
				else
					stream << "{fileID: " << value.fileID << ", guid: " << value.guid << ", type: " << value.type << "}";
				return stream;
			}
		};

		struct Component
		{
			int64 fileID;

			void fromYAML(ryml::ConstNodeRef n)
			{
				if (n.has_child("component"))
					n["component"]["fileID"] >> fileID;
				else
					std::cout << "error: cannot parse component!" << std::endl;
			}

			void print()
			{
				std::cout << "component: {fileID: " << fileID << "}";
			}
		};

		struct TextureProperty
		{
			std::string name;
			YamlField<ObjectRef> m_Texture = { "m_Texture",{} };
			YamlField<glm::vec2> m_Scale = { "m_Scale", glm::vec2(0) };
			YamlField<glm::vec2> m_Offset = { "m_Offset", glm::vec2(0) };

			void fromYAML(ryml::ConstNodeRef n)
			{
				const auto& m = n.child(0);
				c4::from_chars(m.key(), &name);
				m[m_Texture.key] >> m_Texture.value;
				read(m[m_Scale.key], m_Scale.value);
				read(m[m_Offset.key], m_Offset.value);
			}

			friend std::ostream& operator<< (std::ostream& stream, TextureProperty& value)
			{
				std::string prefix = "      ";
				stream << value.name << ":" << std::endl;
				value.m_Texture.print(prefix);
				value.m_Scale.print(prefix);
				value.m_Offset.print(prefix);
				return stream;
			}
		};

		struct FloatProperty
		{
			std::string name;
			float value;

			void fromYAML(ryml::ConstNodeRef n)
			{
				const auto& m = n.child(0);
				c4::from_chars(m.key(), &name);
				m >> value;
			}

			friend std::ostream& operator<< (std::ostream& stream, FloatProperty& prop)
			{
				stream << prop.name << ": " << prop.value << std::endl;
				return stream;
			}
		};

		struct ColorProperty
		{
			std::string name;
			glm::vec4 value;

			void fromYAML(ryml::ConstNodeRef n)
			{
				const auto& m = n.child(0);
				c4::from_chars(m.key(), &name);
				read(m, value);
			}

			friend std::ostream& operator<< (std::ostream& stream, ColorProperty& prop)
			{
				stream << prop.name << ": {";
				stream << "r: " << prop.value.x << ", ";
				stream << "g: " << prop.value.y << ", ";
				stream << "b: " << prop.value.z << ", ";
				stream << "a: " << prop.value.w << "}";
				stream << std::endl;
				return stream;
			}
		};

		struct OcclusionBakeSettings
		{
			float smallestOccluder = 0.0f;
			float smallestHole = 0.0f;
			float backfaceThreshold = 100.0f;

			OcclusionBakeSettings() {}
			OcclusionBakeSettings(float smallestOccluder, float smallestHole, float backfaceThreshold) :
				smallestOccluder(smallestOccluder),
				smallestHole(smallestHole),
				backfaceThreshold(backfaceThreshold)
			{

			}

			void fromYAML(ryml::ConstNodeRef n)
			{
				n["smallestOccluder"] >> smallestOccluder;
				if (n.has_child("type"))
					n["smallestHole"] >> smallestHole;
				if (n.has_child("guid"))
					n["backfaceThreshold"] >> backfaceThreshold;
			}

			friend std::ostream& operator<< (std::ostream& stream, const OcclusionBakeSettings& value)
			{
				stream << std::endl;
				stream << "  smallestOccluder: " << value.smallestOccluder << std::endl;
				stream << "  smallestHole: " << value.smallestHole << std::endl;
				stream << "  backfaceThreshold: " << value.backfaceThreshold;
				return stream;
			}
		};

		struct Override
		{
		public:
			Override() {}
			void fromYAML(ryml::ConstNodeRef n)
			{
				n[target.key] >> target.value;
				n[propertyPath.key] >> propertyPath.value;
				if (!n[value.key].val_is_null())
					n[value.key] >> value.value;
				n[objectReference.key] >> objectReference.value;
			}

			void print(const std::string& prefix)
			{
				target.print("");
				propertyPath.print(prefix);
				value.print(prefix);
				objectReference.print(prefix);
			}

			friend std::ostream& operator<< (std::ostream& stream, Override& value)
			{
				value.print("  ");
				return stream;
			}

		//private:
			YamlField<ObjectRef> target = { "target", {} };
			YamlField<std::string> propertyPath = { "propertyPath", "" };
			YamlField<std::string> value = { "value", "" };
			YamlField<ObjectRef> objectReference = { "objectReference", {} };
		};

		class Modification
		{
		public:
			Modification() {}
			void fromYAML(ryml::ConstNodeRef n)
			{
				n[m_TransformParent.key] >> m_TransformParent.value;
				n[m_Modifications.key] >> m_Modifications.value;
				n[m_RemovedComponents.key] >> m_RemovedComponents.value;
			}

			void print(const std::string& prefix)
			{
				m_TransformParent.print(prefix);
				std::cout << m_Modifications.key << ":" << std::endl;
				//for (auto& or : m_Modifications.value)
				//	std::cout << "- " << or;
			}

			friend std::ostream& operator<< (std::ostream& stream, Modification& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		//private:
			YamlField<FileID> m_TransformParent = { "m_TransformParent", 0 };
			YamlField<std::vector<Override>> m_Modifications = { "m_Modifications", std::vector<Override>() };
			YamlField<std::vector<ObjectRef>> m_RemovedComponents = { "m_RemovedComponents", std::vector<ObjectRef>() };
		};

		class Shadows
		{
		public:
			Shadows() {}
			void fromYAML(ryml::ConstNodeRef n)
			{
				n[m_Type.key] >> m_Type.value;
				n[m_Resolution.key] >> m_Resolution.value;
				n[m_CustomResolution.key] >> m_CustomResolution.value;
				n[m_Strength.key] >> m_Strength.value;
				n[m_Bias.key] >> m_Bias.value;
				n[m_NormalBias.key] >> m_NormalBias.value;
				n[m_NearPlane.key] >> m_NearPlane.value;
				read(n[m_CullingMatrixOverride.key], m_CullingMatrixOverride.value);
				n[m_UseCullingMatrixOverride.key] >> m_UseCullingMatrixOverride.value;
			}

			void print(const std::string& prefix)
			{
				m_Type.print(prefix);
				m_Resolution.print(prefix);
				m_CustomResolution.print(prefix);
				m_Strength.print(prefix);
				m_Bias.print(prefix);
				m_NormalBias.print(prefix);
				m_NearPlane.print(prefix);
				m_CullingMatrixOverride.print(prefix);
				m_UseCullingMatrixOverride.print(prefix);
			}

			friend std::ostream& operator<< (std::ostream& stream, Shadows& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<uint32> m_Type = { "m_Type", 0 };
			YamlField<int32> m_Resolution = { "m_Resolution", -1 };
			YamlField<int32> m_CustomResolution = { "m_CustomResolution", -1 };
			YamlField<float> m_Strength = { "m_Strength", 0.0f };
			YamlField<float> m_Bias = { "m_Bias", 0.0f };
			YamlField<float> m_NormalBias = { "m_NormalBias", 0.0f };
			YamlField<float> m_NearPlane = { "m_NearPlane", 0.0f };
			YamlField<glm::mat4> m_CullingMatrixOverride = { "m_CullingMatrixOverride", glm::mat4(1.0f) };
			YamlField<bool> m_UseCullingMatrixOverride = { "m_UseCullingMatrixOverride", false };
		};

		bool read(ryml::ConstNodeRef n, FileID* value);
		bool read(ryml::ConstNodeRef n, InstanceID* value);
		bool read(ryml::ConstNodeRef n, Component* value);
		bool read(ryml::ConstNodeRef n, ObjectRef* value);
		bool read(ryml::ConstNodeRef n, TextureProperty* value);
		bool read(ryml::ConstNodeRef n, FloatProperty* value);
		bool read(ryml::ConstNodeRef n, ColorProperty* value);
		bool read(ryml::ConstNodeRef n, OcclusionBakeSettings* value);
		bool read(ryml::ConstNodeRef n, Override* value);
		bool read(ryml::ConstNodeRef n, Modification* value);
		bool read(ryml::ConstNodeRef n, Shadows* value);
	}
}

#endif // INCLUDED_SIMPLETYPES