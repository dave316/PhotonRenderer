#ifndef INCLUDED_UNITYYAML
#define INCLUDED_UNITYYAML

#ifdef WITH_UNITY

#pragma once

#include "UnityTypes.h"

#include <ryml/ryml.hpp>
#include <ryml/ryml_std.hpp>

namespace IO
{
	namespace Unity
	{
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
				ch >> seq->container[index++];
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
		}

		bool read(ryml::ConstNodeRef n, glm::vec2& value);
		bool read(ryml::ConstNodeRef n, glm::vec3& value);
		bool read(ryml::ConstNodeRef n, glm::vec4& value);
		bool read(ryml::ConstNodeRef n, glm::quat& value);
		bool read(ryml::ConstNodeRef n, FileID* value);
		bool read(ryml::ConstNodeRef n, Component* value);
		bool read(ryml::ConstNodeRef n, ObjectRef* value);
		bool read(ryml::ConstNodeRef n, GameObject* value);
		bool read(ryml::ConstNodeRef n, UTransform* value);
		bool read(ryml::ConstNodeRef n, MeshRenderer* value);
		bool read(ryml::ConstNodeRef n, MeshFilter* value);
		bool read(ryml::ConstNodeRef n, ReflectionProbe* value);
		bool read(ryml::ConstNodeRef n, LightProbeGroup* value);
		bool read(ryml::ConstNodeRef n, ULight* value);
		bool read(ryml::ConstNodeRef n, TexEnv* value);
		bool read(ryml::ConstNodeRef n, UnityMaterial* value);
		bool read(ryml::ConstNodeRef n, Modification* value);
		bool read(ryml::ConstNodeRef n, PrefabInstance* value);
		bool read(ryml::ConstNodeRef n, RenderSettings* value);
		bool read(ryml::ConstNodeRef n, LightmapSettings* value);
		bool read(ryml::ConstNodeRef n, Renderer* value);
		bool read(ryml::ConstNodeRef n, LOD* value);
		bool read(ryml::ConstNodeRef n, LODGroup* value);
		bool read(ryml::ConstNodeRef n, UBoxCollider* value);
	}
}

#endif
#endif // INCLUDED_UNITYYAML