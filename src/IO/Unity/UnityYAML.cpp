#ifdef WITH_UNITY

#include "UnityYAML.h"

namespace IO
{
	namespace Unity
	{
		bool read(ryml::ConstNodeRef n, glm::vec2& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::vec3& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			n["z"] >> value.z;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::quat& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			n["z"] >> value.z;
			n["w"] >> value.w;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::vec4& value)
		{
			n["r"] >> value.r;
			n["g"] >> value.g;
			n["b"] >> value.b;
			n["a"] >> value.a;
			return true;
		}

		bool read(ryml::ConstNodeRef n, FileID* value)
		{
			n["fileID"] >> value->id;
			return true;
		}

		bool read(ryml::ConstNodeRef n, Component* value)
		{
			if (n.has_child("component"))
				n["component"] >> value->fileID;
			else
				n.child(0) >> value->fileID;
			return true;
		}

		bool read(ryml::ConstNodeRef n, ObjectRef* value)
		{
			n["fileID"] >> value->fileID;
			if(n.has_child("type"))
				n["type"] >> value->type;
			if(n.has_child("guid"))
				n["guid"] >> value->guid;
			return true;
		}

		bool read(ryml::ConstNodeRef n, GameObject* value)
		{
			// TODO: check if stripped version ob gameobject
			if (n.has_child("m_PrefabParentObject"))
			{
				n["m_Component"] >> value->components;
				n["m_Layer"] >> value->layer;
				n["m_Name"] >> value->name;
				n["m_TagString"] >> value->tag;
				n["m_IsActive"] >> value->isActive;
			}
			else if (n.has_child("m_ObjectHideFlags"))
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
				n["m_Component"] >> value->components;
				n["m_Layer"] >> value->layer;
				n["m_Name"] >> value->name;
				n["m_TagString"] >> value->tag;
				n["m_IsActive"] >> value->isActive;
			}
			else
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
			}
			return true;
		}

		bool read(ryml::ConstNodeRef n, UTransform* value)
		{
			if (n.has_child("m_PrefabParentObject"))
			{
				n["m_GameObject"] >> value->gameObject;
				n["m_Children"] >> value->children;
				n["m_Father"] >> value->father;
				n["m_RootOrder"] >> value->rootOrder;

				read(n["m_LocalRotation"], value->localRotation);
				read(n["m_LocalPosition"], value->localPosition);
				read(n["m_LocalScale"], value->localScale);
			}
			else if (n.has_child("m_ObjectHideFlags"))
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
				n["m_GameObject"] >> value->gameObject;
				n["m_Children"] >> value->children;
				n["m_Father"] >> value->father;
				n["m_RootOrder"] >> value->rootOrder;

				auto x = n["m_LocalRotation"];
				read(n["m_LocalRotation"], value->localRotation);
				read(n["m_LocalPosition"], value->localPosition);
				read(n["m_LocalScale"], value->localScale);
			}
			else
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
			}
			return true;
		}

		bool read(ryml::ConstNodeRef n, MeshRenderer* value)
		{
			if (n.has_child("m_PrefabParentObject"))
			{
				n["m_GameObject"] >> value->gameObject;
				n["m_Enabled"] >> value->enabled;
				n["m_CastShadows"] >> value->castShadow;
				n["m_ReceiveShadows"] >> value->receiveShadows;
				//n["m_LightProbeUsage"] >> value->lightProbeUsage;
				n["m_ReflectionProbeUsage"] >> value->reflectionProbeUsage;
				n["m_Materials"] >> value->materials;
				n["m_ProbeAnchor"] >> value->probeAnchor;
				//n["m_LightProbeVolumeOverride"] >> value->lightProbeVolumeOverride;
				n["m_ScaleInLightmap"] >> value->scaleInLightmap;
				n["m_LightmapParameters"] >> value->lightmapParameters;
			}
			else if (n.has_child("m_ObjectHideFlags"))
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
				n["m_GameObject"] >> value->gameObject;
				n["m_Enabled"] >> value->enabled;
				n["m_CastShadows"] >> value->castShadow;
				n["m_ReceiveShadows"] >> value->receiveShadows;
				n["m_LightProbeUsage"] >> value->lightProbeUsage;
				n["m_ReflectionProbeUsage"] >> value->reflectionProbeUsage;
				n["m_RenderingLayerMask"] >> value->renderingLayerMask;
				n["m_RendererPriority"] >> value->rendererPriority;
				n["m_Materials"] >> value->materials;
				n["m_ProbeAnchor"] >> value->probeAnchor;
				n["m_LightProbeVolumeOverride"] >> value->lightProbeVolumeOverride;
				n["m_ScaleInLightmap"] >> value->scaleInLightmap;
				n["m_ReceiveGI"] >> value->receiveGI;
				n["m_LightmapParameters"] >> value->lightmapParameters;
			}
			else
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
			}

			return true;
		}

		bool read(ryml::ConstNodeRef n, MeshFilter* value)
		{
			if (n.has_child("m_PrefabParentObject"))
			{
				n["m_GameObject"] >> value->gameObject;
				n["m_Mesh"] >> value->mesh;
			}
			else
			{
				n["m_CorrespondingSourceObject"] >> value->correspondingSourceObject;
				n["m_PrefabInstance"] >> value->prefabInstance;
				n["m_PrefabAsset"] >> value->prefabAsset;
				n["m_GameObject"] >> value->gameObject;
				n["m_Mesh"] >> value->mesh;
			}

			return true;
		}

		bool read(ryml::ConstNodeRef n, ReflectionProbe* value)
		{
			n["m_GameObject"] >> value->gameObject;
			n["m_Resolution"] >> value->resolution;
			n["m_NearClip"] >> value->nearClip;
			n["m_FarClip"] >> value->farClip;
			read(n["m_BoxSize"], value->boxSize);
			read(n["m_BoxOffset"], value->boxOffset);
			read(n["m_BackGroundColor"], value->backGroundColor);
			return true;
		}

		bool read(ryml::ConstNodeRef n, LightProbeGroup* value)
		{
			n["m_GameObject"] >> value->gameObject;
			auto& posNode = n["m_SourcePositions"];
			for (auto& posEntry : posNode)
			{
				glm::vec3 pos;
				read(posEntry, pos);
				value->sourcePositions.push_back(pos);
			}
			return true;
		}

		bool read(ryml::ConstNodeRef n, ULight* value)
		{
			n["m_Type"] >> value->type;
			n["m_Shape"] >> value->shape;
			read(n["m_Color"], value->color);
			n["m_Intensity"] >> value->intensity;
			n["m_Range"] >> value->range;
			return true;
		}

		bool read(ryml::ConstNodeRef n, TexEnv* value)
		{
			n["m_Texture"] >> value->texture;
			read(n["m_Offset"], value->offset);
			read(n["m_Scale"], value->scale);
			return true;
		}

		bool read(ryml::ConstNodeRef n, UnityMaterial* value)
		{
			n["m_Name"] >> value->name;
			n["m_Shader"] >> value->shader;
			auto& propsNode = n["m_SavedProperties"];
			//propsNode["m_TexEnvs"] >> value->texEnvs;
			
			auto& texNode = propsNode["m_TexEnvs"];
			for (auto& texEntry : texNode)
			{
				auto& t = texEntry.child(0);
				std::string texName;
				c4::from_chars(t.key(), &texName);

				TexEnv texEnv;
				t >> texEnv;
				if(texEnv.texture.fileID > 0)
					value->texEnvs.insert(std::make_pair(texName, texEnv));
			}

			auto& floatNode = propsNode["m_Floats"];
			for (auto& floadEntry : floatNode)
			{
				auto& f = floadEntry.child(0);
				std::string floatName;
				c4::from_chars(f.key(), &floatName);

				float v;
				f >> v;
				value->floats.insert(std::make_pair(floatName, v));
			}

			auto& colorNode = propsNode["m_Colors"];
			for (auto& colorEntry : colorNode)
			{
				auto& c = colorEntry.child(0);
				std::string colorName;
				c4::from_chars(c.key(), &colorName);

				glm::vec4 v;
				read(c, v);
				value->colors.insert(std::make_pair(colorName, v));
			}

			return true;
		}

		bool read(ryml::ConstNodeRef n, Modification* value)
		{
			n["target"] >> value->target;
			n["propertyPath"] >> value->propertyPath;
			if(n["value"].has_val())
				n["value"] >> value->value;
			n["objectReference"] >> value->objectReference;
			return true;
		}

		bool read(ryml::ConstNodeRef n, PrefabInstance* value)
		{
			n["m_Modification"]["m_Modifications"] >> value->modifications;
			return true;
		}

		bool read(ryml::ConstNodeRef n, RenderSettings* value)
		{
			n["m_Fog"] >> value->fog;
			read(n["m_FogColor"], value->fogColor);
			n["m_FogMode"] >> value->fogMode;
			n["m_FogDensity"] >> value->fogDensity;
			n["m_LinearFogStart"] >> value->linearFogStart;
			n["m_LinearFogEnd"] >> value->linearFogEnd;
			read(n["m_AmbientSkyColor"], value->ambientSkyColor);
			read(n["m_AmbientEquatorColor"], value->ambientEquatorColor);
			read(n["m_AmbientGroundColor"], value->ambientGroundColor);
			n["m_AmbientIntensity"] >> value->ambientIntensity;
			n["m_SkyboxMaterial"] >> value->skyboxMaterial;
			return true;
		}

		bool read(ryml::ConstNodeRef n, LightmapSettings* value)
		{
			n["m_LightingDataAsset"] >> value->lightingDataAsset;
			return true;
		}

		bool read(ryml::ConstNodeRef n, Renderer* value)
		{
			n["renderer"] >> value->fileID;
			return true;
		}

		bool read(ryml::ConstNodeRef n, LOD* value)
		{
			n["screenRelativeHeight"] >> value->screenRelativeHeight;
			n["fadeTransitionWidth"] >> value->fadeTransitionWidth;
			n["renderers"] >> value->renderers;
			//auto& rendsNode = n["renderers"];
			//auto& rendNode = rendsNode.child(0);
			//rendNode["renderer"]["fileID"] >> value->renderer;
			return true;
		}

		bool read(ryml::ConstNodeRef n, LODGroup* value)
		{
			n["m_GameObject"] >> value->gameObject;
			read(n["m_LocalReferencePoint"], value->localReferencePoint);
			n["m_Size"] >> value->size;
			n["m_FadeMode"] >> value->fadeMode;
			n["m_AnimateCrossFading"] >> value->animateCrossFading;
			n["m_LODs"] >> value->lods;
			n["m_Enabled"] >> value->enabled;
			return true;
		}

		bool read(ryml::ConstNodeRef n, UBoxCollider* value)
		{
			read(n["m_Size"], value->size);
			read(n["m_Center"], value->center);
			return true;
		}
	}
}

#endif