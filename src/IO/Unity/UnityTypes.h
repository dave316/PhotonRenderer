#ifndef INCLUDED_UNITYTYPES
#define INCLUDED_UNITYTYPES

#pragma once

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace IO
{
	namespace Unity
	{
		struct ModelImporter
		{
			std::map<uint32_t, std::string> fileIDToName;
		};

		struct Object
		{
			uint64_t id;
			uint32_t type;
		};

		struct FileID
		{
			int64_t id = -1;
		};

		struct Component
		{
			FileID fileID;
		};

		struct ObjectRef
		{
			int64_t fileID = 0;
			uint32_t type = 0;
			std::string guid = "";
		};

		struct GameObject : public Object
		{
			ObjectRef correspondingSourceObject;
			FileID prefabInstance;
			FileID prefabAsset;
			std::vector<Component> components;
			uint32_t layer;
			std::string name;
			std::string tag;
			bool isActive;
		};

		struct UTransform : public Object
		{
			ObjectRef correspondingSourceObject;
			FileID prefabInstance;
			FileID prefabAsset;
			FileID gameObject;
			glm::quat localRotation;
			glm::vec3 localPosition;
			glm::vec3 localScale;
			std::vector<FileID> children;
			FileID father;
			uint32_t rootOrder;
		};

		struct MeshRenderer : public Object
		{
			ObjectRef correspondingSourceObject;
			FileID prefabInstance;
			FileID prefabAsset;
			FileID gameObject;
			bool enabled;
			bool castShadow;
			bool receiveShadows;
			uint32_t lightProbeUsage;
			uint32_t reflectionProbeUsage;
			uint32_t renderingLayerMask;
			int32_t rendererPriority;
			std::vector<ObjectRef> materials;
			FileID probeAnchor;
			FileID lightProbeVolumeOverride;
			float scaleInLightmap;
			uint32_t receiveGI;
			FileID lightmapParameters;
		};

		struct MeshFilter : public Object
		{
			ObjectRef correspondingSourceObject;
			FileID prefabInstance;
			FileID prefabAsset;
			FileID gameObject;
			ObjectRef mesh;
		};

		struct ReflectionProbe
		{
			FileID gameObject;
			uint32_t resolution;
			float nearClip;
			float farClip;
			glm::vec3 boxSize;
			glm::vec3 boxOffset;
			glm::vec4 backGroundColor;
			bool boxProjection;
			int importance;
		};

		struct LightProbeGroup
		{
			FileID gameObject;
			std::vector<glm::vec3> sourcePositions;
		};

		struct ULight
		{
			uint32_t type;
			uint32_t shape;
			glm::vec4 color;
			float intensity;
			float range;
		};

		struct TexEnv
		{
			ObjectRef texture;
			glm::vec2 offset;
			glm::vec2 scale;
		};

		struct UnityMaterial : public Object
		{
			std::string name;
			ObjectRef shader;
			std::map<std::string, TexEnv> texEnvs;
			std::map<std::string, float> floats;
			std::map<std::string, glm::vec4> colors;
		};

		struct Modification
		{
			ObjectRef target;
			ObjectRef objectReference;
			std::string propertyPath;
			std::string value;
		};

		struct PrefabInstance : public Object
		{
			std::vector<Modification> modifications;
		};

		struct RenderSettings
		{
			bool fog;
			glm::vec4 fogColor;
			int fogMode;
			float fogDensity;
			float linearFogStart;
			float linearFogEnd;
			glm::vec4 ambientSkyColor;
			glm::vec4 ambientEquatorColor;
			glm::vec4 ambientGroundColor;
			float ambientIntensity;
			ObjectRef skyboxMaterial;
		};

		struct LightmapSettings
		{
			ObjectRef lightingDataAsset;
		};

		struct Renderer
		{
			FileID fileID;
		};

		struct LOD
		{
			float screenRelativeHeight;
			float fadeTransitionWidth;
			std::vector<Renderer> renderers;
		};

		struct LODGroup
		{
			FileID gameObject;
			glm::vec3 localReferencePoint;
			float size;
			int fadeMode;
			int animateCrossFading;
			std::vector<LOD> lods;
			bool enabled;
		};

		struct UVolume
		{
			FileID gameObject;
			bool isGlobal;
			float blendDistance;
			ObjectRef sharedProfile;
		};

		struct UBoxCollider
		{
			glm::vec3 center;
			glm::vec3 size;
		};
	}
}

#endif // INCLUDED_UNITYTYPES