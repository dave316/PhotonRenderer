#ifndef INCLUDED_OBJECTTYPES
#define INCLUDED_OBJECTTYPES

#pragma once

#include "SimpleTypes.h"

namespace Unity
{
	namespace YAML
	{
		class Object
		{
		public:
			Object(int64 id, uint32 type);
			virtual ~Object() = 0 {};
			virtual void fromYAML(ryml::ConstNodeRef n) = 0;
			void fromBaseYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			int64 getID();
			uint32 getType();
		//protected:
			YamlField<uint32> m_ObjectHideFlags = { "m_ObjectHideFlags", 0 };
			YamlField<ObjectRef> m_CorrespondingSourceObject = { "m_CorrespondingSourceObject", { 0, 0, "" } };
			YamlField<FileID> m_PrefabInstance = { "m_PrefabInstance", 0 };
			YamlField<FileID> m_PrefabAsset = { "m_PrefabAsset", 0 };
			YamlField<FileID> m_PrefabParentObject = { "m_PrefabParentObject", 0 };
			YamlField<FileID> m_PrefabInternal = { "m_PrefabInternal", 0 };
			YamlField<FileID> m_GameObject = { "m_GameObject", 0 };
			YamlField<bool> m_Enabled = { "m_Enabled", true };
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };

			int64 id = 0;
			uint32 type = 0;
		};

		class GameObject : public Object
		{
		public:
			GameObject(int64 id, uint32 type, bool stripped);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//private:
			YamlField<std::vector<Component>> m_Component = { "m_Component", std::vector<Component>() };
			YamlField<uint32> m_Layer = { "m_Layer", 0 };
			YamlField<std::string> m_Name = { "m_Name", "" };
			YamlField<std::string> m_TagString = { "m_TagString", "" };
			YamlField<ObjectRef> m_Icon = { "m_Icon", { 0, 0, "" } };
			YamlField<uint32> m_NavMeshLayer = { "m_NavMeshLayer", 0 };
			YamlField<uint32> m_StaticEditorFlags = { "m_StaticEditorFlags", 0 };
			YamlField<bool> m_IsActive = { "m_IsActive", false };

			bool stripped = false;
		};

		class Transform : public Object
		{
		public:
			Transform(int64 id, uint32 type, bool stripped);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		/*private:*/
			YamlField<glm::quat> m_LocalRotation = { "m_LocalRotation", glm::quat(1, 0, 0, 0) };
			YamlField<glm::vec3> m_LocalPosition = { "m_LocalPosition", glm::vec3(0) };
			YamlField<glm::vec3> m_LocalScale = { "m_LocalScale", glm::vec3(1) };
			YamlField<std::vector<FileID>> m_Children = { "m_Children", std::vector<FileID>() };
			YamlField<FileID> m_Father = { "m_Father", 0 };
			YamlField<uint32> m_RootOrder = { "m_RootOrder", 0 };
			YamlField<glm::vec3> m_LocalEulerAnglesHint = { "m_LocalEulerAnglesHint", glm::vec3(0) };

			bool stripped = false;
		};

		class MeshRenderer : public Object
		{
		public:
			MeshRenderer(int64 id, uint32 type, bool stripped);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//private:
			YamlField<uint32> m_CastShadows = { "m_CastShadows", 0 }; // 0 - ON, 1 - OFF, 2 - Two Sided, 3 - Only Shadows
			YamlField<bool> m_ReceiveShadows = { "m_ReceiveShadows", false };
			YamlField<bool> m_DynamicOccludee = { "m_DynamicOccludee", false };
			YamlField<uint32> m_MotionVectors = { "m_MotionVectors", 0 }; // 0 - Camera, 1 - Object, 2 - No Motion
			YamlField<uint32> m_LightProbeUsage = { "m_LightProbeUsage", 0 }; // 0 - OFF, 1 - BlendProbes, 2 - Proxy Volume, 3 - Custom
			YamlField<uint32> m_ReflectionProbeUsage = { "m_ReflectionProbeUsage", 0 }; // 0 - OFF, 1 - BlendProbes, 2 - BlendProbes and sykybox, 3 - Simple
			YamlField<uint32> m_RayTracingMode = { "m_RayTracingMode", 0 }; // ???
			YamlField<bool> m_RayTraceProcedural = { "m_RayTraceProcedural", false }; // ???
			YamlField<uint32> m_RenderingLayerMask = { "m_RenderingLayerMask", 0 }; // What layers this renders into???
			YamlField<uint32> m_RendererPriority = { "m_RendererPriority", 0 }; // This actually comes from the shader/material
			YamlField<std::vector<ObjectRef>> m_Materials = { "m_Materials", std::vector<ObjectRef>() }; // used materials need to match the amount of submeshes
			YamlField<FileID> m_ProbeAnchor = { "m_ProbeAnchor", 0 }; // Overrides the position of the ReflectionProbe
			YamlField<FileID> m_LightProbeVolumeOverride = { "m_LightProbeVolumeOverride", 0 }; // ???
			YamlField<float> m_ScaleInLightmap = { "m_ScaleInLightmap", 0 }; // Scale of the LightUVs for this Object
			YamlField<uint32> m_ReceiveGI = { "m_ReceiveGI", 0 }; // 1 - Light probes, 2 - Light maps
			YamlField<bool> m_PreserveUVs = { "m_PreserveUVs", false }; // Preserve UVs from mesh
			YamlField<bool> m_IgnoreNormalsForChartDetection = { "m_IgnoreNormalsForChartDetection", false }; // Light map UVs are not split when normal direction changes
			YamlField<bool> m_ImportantGI = { "m_ImportantGI", false }; // ???
			YamlField<bool> m_StitchLightmapSeams = { "m_StitchLightmapSeams", false }; // Stitch seams at UV discontinuities
			YamlField<uint32> m_SelectedEditorRenderState = { "m_SelectedEditorRenderState", 0 }; // ???
			YamlField<uint32> m_MinimumChartSize = { "m_MinimumChartSize", 0 }; // Minimum texel size of a UV chart (needs atleast 4 to be stitchable)
			YamlField<float> m_AutoUVMaxDistance = { "m_AutoUVMaxDistance", 0.0f }; // max distance for simplification
			YamlField<float> m_AutoUVMaxAngle = { "m_AutoUVMaxAngle", 0.0f }; // max angle for simplification
			YamlField<ObjectRef> m_LightmapParameters = { "m_LightmapParameters", { 0, 0, "" } }; // reference to the light map asset
			YamlField<uint32> m_SortingLayerID = { "m_SortingLayerID", 0 }; // Propably for the render queue
			YamlField<uint32> m_SortingLayer = { "m_SortingLayer", 0 }; // Propably for the render queue
			YamlField<uint32> m_SortingOrder = { "m_SortingOrder", 0 }; // Propably for the render queue
			YamlField<FileID> m_AdditionalVertexStreams = { "m_AdditionalVertexStreams", 0 }; // ???

			bool stripped = false;
		};

		class MeshFilter : public Object
		{
		public:
			MeshFilter(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			YamlField<ObjectRef> m_Mesh = { "m_Mesh", {} };
		};

		class OcclusionCullingSettings : public Object
		{
		public:
			OcclusionCullingSettings(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<OcclusionBakeSettings> m_OcclusionBakeSettings = { "m_OcclusionBakeSettings", {} };
			YamlField<std::string> m_SceneGUID = { "m_SceneGUID", "" };
			YamlField<ObjectRef> m_OcclusionCullingData = { "m_OcclusionCullingData", {} };
		};

		class RenderSettings : public Object
		{
		public:
			RenderSettings(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			YamlField<bool> m_Fog = { "m_Fog", false };
			YamlField<glm::vec4> m_FogColor = { "m_FogColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
			YamlField<uint32> m_FogMode = { "m_FogMode", 0 };
			YamlField<float> m_FogDensity = { "m_FogDensity", 0.01f };
			YamlField<float> m_LinearFogStart = { "m_LinearFogStart", 0.0f };
			YamlField<float> m_LinearFogEnd = { "m_LinearFogEnd", 300.0f };
			YamlField<glm::vec4> m_AmbientSkyColor = { "m_AmbientSkyColor", glm::vec4(0.212f, 0.227f, 0.259f, 1.0f) };
			YamlField<glm::vec4> m_AmbientEquatorColor = { "m_AmbientEquatorColor", glm::vec4(0.114f, 0.125f, 0.133f, 1.0f) };
			YamlField<glm::vec4> m_AmbientGroundColor = { "m_AmbientGroundColor", glm::vec4(0.047f, 0.043f, 0.035f, 1.0f) };
			YamlField<float> m_AmbientIntensity = { "m_AmbientIntensity", 1.0f };
			YamlField<uint32> m_AmbientMode = { "m_AmbientMode", 0 };
			YamlField<glm::vec4> m_SubtractiveShadowColor = { "m_SubtractiveShadowColor", glm::vec4(0.42f, 0.478, 0.627f, 1.0f) };
			YamlField<ObjectRef> m_SkyboxMaterial = { "m_SkyboxMaterial", {} };
			YamlField<float> m_HaloStrength = { "m_HaloStrength", 0.5f };
			YamlField<float> m_FlareStrength = { "m_FlareStrength", 1.0f };
			YamlField<float> m_FlareFadeSpeed = { "m_FlareFadeSpeed", 3.0f };
			YamlField<ObjectRef> m_HaloTexture = { "m_HaloTexture", {} };
			YamlField<ObjectRef> m_SpotCookie = { "m_SpotCookie", {} };
			YamlField<uint32> m_DefaultReflectionMode = { "m_DefaultReflectionMode", 0 };
			YamlField<uint32> m_DefaultReflectionResolution = { "m_DefaultReflectionResolution", 128 };
			YamlField<uint32> m_ReflectionBounces = { "m_ReflectionBounces", 1 };
			YamlField<float> m_ReflectionIntensity = { "m_ReflectionIntensity", 1.0f };
			YamlField<ObjectRef> m_CustomReflection = { "m_CustomReflection", {} };
			YamlField<FileID> m_Sun = { "m_Sun", 0 };
			YamlField<glm::vec4> m_IndirectSpecularColor = { "m_IndirectSpecularColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };
			YamlField<bool> m_UseRadianceAmbientProbe = { "m_UseRadianceAmbientProbe", false };
		};

		class LightingSettings : public Object
		{
		public:
			LightingSettings(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<uint32> m_GIWorkflowMode = { "m_GIWorkflowMode", 0 };
			YamlField<bool> m_EnableBakedLightmaps = { "m_EnableBakedLightmaps", false };
			YamlField<bool> m_EnableRealtimeLightmaps = { "m_EnableRealtimeLightmaps", false };
			YamlField<bool> m_RealtimeEnvironmentLighting = { "m_RealtimeEnvironmentLighting", false };
			YamlField<float> m_BounceScale = { "m_BounceScale", 1.0f };
			YamlField<float> m_AlbedoBoost = { "m_AlbedoBoost", 1.0f };
			YamlField<float> m_IndirectOutputScale = { "m_IndirectOutputScale", 1.0f };
			YamlField<bool> m_UsingShadowmask = { "m_UsingShadowmask", false };
			YamlField<uint32> m_BakeBackend = { "m_BakeBackend", 0 };
			YamlField<uint32> m_LightmapMaxSize = { "m_LightmapMaxSize", 0 };
			YamlField<uint32> m_BakeResolution = { "m_BakeResolution", 0 };
			YamlField<uint32> m_Padding = { "m_Padding", 2 };
			YamlField<bool> m_TextureCompression = { "m_TextureCompression", false };
			YamlField<bool> m_AO = { "m_AO", false };
			YamlField<float> m_AOMaxDistance = { "m_AOMaxDistance", 0.0f };
			YamlField<float> m_CompAOExponent = { "m_CompAOExponent", 0.0f };
			YamlField<float> m_CompAOExponentDirect = { "m_CompAOExponentDirect", 0.0f };
			YamlField<bool> m_ExtractAO = { "m_ExtractAO", false };
			YamlField<uint32> m_MixedBakeMode = { "m_MixedBakeMode", 0 };
			YamlField<uint32> m_LightmapsBakeMode = { "m_LightmapsBakeMode", 0 };
			YamlField<uint32> m_FilterMode = { "m_FilterMode", 0 };
			YamlField<ObjectRef> m_LightmapParameters = { "m_LightmapParameters", {} };
			YamlField<bool> m_ExportTrainingData = { "m_ExportTrainingData", {} };
			YamlField<std::string> m_TrainingDataDestination = { "m_TrainingDataDestination", "" };
			YamlField<uint32> m_RealtimeResolution = { "m_RealtimeResolution", 0 };
			YamlField<bool> m_ForceWhiteAlbedo = { "m_ForceWhiteAlbedo", false };
			YamlField<bool> m_ForceUpdates = { "m_ForceUpdates", false };
			YamlField<bool> m_FinalGather = { "m_FinalGather", false };
			YamlField<uint32> m_FinalGatherRayCount = { "m_FinalGatherRayCount", 0 };
			YamlField<bool> m_FinalGatherFiltering = { "m_FinalGatherFiltering", false };
			YamlField<bool> m_PVRCulling = { "m_PVRCulling", false };
			YamlField<bool> m_PVRSampling = { "m_PVRSampling", false };
			YamlField<uint32> m_PVRDirectSampleCount = { "m_PVRDirectSampleCount", 0 };
			YamlField<uint32> m_PVRSampleCount = { "m_PVRSampleCount", 0 };
			YamlField<uint32> m_PVREnvironmentSampleCount = { "m_PVREnvironmentSampleCount", 0 };
			YamlField<uint32> m_PVREnvironmentReferencePointCount = { "m_PVREnvironmentReferencePointCount", 0 };
			YamlField<uint32> m_LightProbeSampleCountMultiplier = { "m_LightProbeSampleCountMultiplier", 0 };
			YamlField<uint32> m_PVRBounces = { "m_PVRBounces", 0 };
			YamlField<uint32> m_PVRMinBounces = { "m_PVRMinBounces", 0 };
			YamlField<bool> m_PVREnvironmentMIS = { "m_PVREnvironmentMIS", false };
			YamlField<uint32> m_PVRFilteringMode = { "m_PVRFilteringMode", 0 };
			YamlField<uint32> m_PVRDenoiserTypeDirect = { "m_PVRDenoiserTypeDirect", 0 };
			YamlField<uint32> m_PVRDenoiserTypeIndirect = { "m_PVRDenoiserTypeIndirect", 0 };
			YamlField<uint32> m_PVRDenoiserTypeAO = { "m_PVRDenoiserTypeAO", 0 };
			YamlField<uint32> m_PVRFilterTypeDirect = { "m_PVRFilterTypeDirect", 0 };
			YamlField<uint32> m_PVRFilterTypeIndirect = { "m_PVRFilterTypeIndirect", 0 };
			YamlField<uint32> m_PVRFilterTypeAO = { "m_PVRFilterTypeAO", 0 };
			YamlField<float> m_PVRFilteringGaussRadiusDirect = { "m_PVRFilteringGaussRadiusDirect", 0.0f };
			YamlField<float> m_PVRFilteringGaussRadiusIndirect = { "m_PVRFilteringGaussRadiusIndirect", 0.0f };
			YamlField<float> m_PVRFilteringGaussRadiusAO = { "m_PVRFilteringGaussRadiusAO", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaDirect = { "m_PVRFilteringAtrousPositionSigmaDirect", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaIndirect = { "m_PVRFilteringAtrousPositionSigmaIndirect", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaAO = { "m_PVRFilteringAtrousPositionSigmaAO", 0.0f };
		};

		class GISettings : public Object
		{
		public:
			GISettings();
			void fromYAML(ryml::ConstNodeRef n);

			void print(const std::string& prefix);
			friend std::ostream& operator<< (std::ostream& stream, GISettings& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<float> m_BounceScale = { "m_BounceScale", 1.0f };
			YamlField<float> m_IndirectOutputScale = { "m_IndirectOutputScale", 1.0f };
			YamlField<float> m_AlbedoBoost = { "m_AlbedoBoost", 1.0f };
			YamlField<uint32> m_EnvironmentLightingMode = { "m_EnvironmentLightingMode", 0 };
			YamlField<bool> m_EnableBakedLightmaps = { "m_EnableBakedLightmaps", false };
			YamlField<bool> m_EnableRealtimeLightmaps = { "m_EnableRealtimeLightmaps", false };
		};

		class LightmapEditorSettings : public Object
		{
		public:
			LightmapEditorSettings();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, LightmapEditorSettings& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<uint32> m_Resolution = { "m_Resolution", 0 };
			YamlField<uint32> m_BakeResolution = { "m_BakeResolution", 0 };
			YamlField<uint32> m_AtlasSize = { "m_AtlasSize", 0 };
			YamlField<bool> m_AO = { "m_AO", false };
			YamlField<float> m_AOMaxDistance = { "m_AOMaxDistance", 0.0f };
			YamlField<float> m_CompAOExponent = { "m_CompAOExponent", 0.0f };
			YamlField<float> m_CompAOExponentDirect = { "m_CompAOExponentDirect", 0.0f };
			YamlField<bool> m_ExtractAmbientOcclusion = { "m_ExtractAmbientOcclusion", false };
			YamlField<uint32> m_Padding = { "m_Padding", 0 };
			YamlField<ObjectRef> m_LightmapParameters = { "m_LightmapParameters", {} };
			YamlField<uint32> m_LightmapsBakeMode = { "m_LightmapsBakeMode", 0 };
			YamlField<bool> m_TextureCompression = { "m_TextureCompression", false };
			YamlField<bool> m_FinalGather = { "m_FinalGather", false };
			YamlField<bool> m_FinalGatherFiltering = { "m_FinalGatherFiltering", false };
			YamlField<uint32> m_FinalGatherRayCount = { "m_FinalGatherRayCount", 0 };
			YamlField<uint32> m_ReflectionCompression = { "m_ReflectionCompression", 0 };
			YamlField<uint32> m_MixedBakeMode = { "m_MixedBakeMode", 0 };
			YamlField<uint32> m_BakeBackend = { "m_BakeBackend", 0 };
			YamlField<bool> m_PVRSampling = { "m_PVRSampling", false };
			YamlField<uint32> m_PVRDirectSampleCount = { "m_PVRDirectSampleCount", 0 };
			YamlField<uint32> m_PVRSampleCount = { "m_PVRSampleCount", 0 };
			YamlField<uint32> m_PVRBounces = { "m_PVRBounces", 0 };
			YamlField<uint32> m_PVREnvironmentSampleCount = { "m_PVREnvironmentSampleCount", 0 };
			YamlField<uint32> m_PVREnvironmentReferencePointCount = { "m_PVREnvironmentReferencePointCount", 0 };
			YamlField<uint32> m_PVRFilteringMode = { "m_PVRFilteringMode", 0 };
			YamlField<uint32> m_PVRDenoiserTypeDirect = { "m_PVRDenoiserTypeDirect", 0 };
			YamlField<uint32> m_PVRDenoiserTypeIndirect = { "m_PVRDenoiserTypeIndirect", 0 };
			YamlField<uint32> m_PVRDenoiserTypeAO = { "m_PVRDenoiserTypeAO", 0 };
			YamlField<uint32> m_PVRFilterTypeDirect = { "m_PVRFilterTypeDirect", 0 };
			YamlField<uint32> m_PVRFilterTypeIndirect = { "m_PVRFilterTypeIndirect", 0 };
			YamlField<uint32> m_PVRFilterTypeAO = { "m_PVRFilterTypeAO", 0 };
			YamlField<bool> m_PVREnvironmentMIS = { "m_PVREnvironmentMIS", false };
			YamlField<bool> m_PVRCulling = { "m_PVRCulling", false };
			YamlField<float> m_PVRFilteringGaussRadiusDirect = { "m_PVRFilteringGaussRadiusDirect", 0.0f };
			YamlField<float> m_PVRFilteringGaussRadiusIndirect = { "m_PVRFilteringGaussRadiusIndirect", 0.0f };
			YamlField<float> m_PVRFilteringGaussRadiusAO = { "m_PVRFilteringGaussRadiusAO", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaDirect = { "m_PVRFilteringAtrousPositionSigmaDirect", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaIndirect = { "m_PVRFilteringAtrousPositionSigmaIndirect", 0.0f };
			YamlField<float> m_PVRFilteringAtrousPositionSigmaAO = { "m_PVRFilteringAtrousPositionSigmaAO", 0.0f };
			YamlField<bool> m_ExportTrainingData = { "m_ExportTrainingData", {} };
			YamlField<std::string> m_TrainingDataDestination = { "m_TrainingDataDestination", "" };
			YamlField<uint32> m_LightProbeSampleCountMultiplier = { "m_LightProbeSampleCountMultiplier", 0 };
		};

		class LightmapSettings : public Object
		{
		public:
			LightmapSettings(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<uint32> m_GIWorkflowMode = { "m_GIWorkflowMode", 0 };
			YamlField<GISettings> m_GISettings = { "m_GISettings", {} };
			YamlField<LightmapEditorSettings> m_LightmapEditorSettings = { "m_LightmapEditorSettings", {} };
			YamlField<ObjectRef> m_LightingDataAsset = { "m_LightingDataAsset", {} };
			YamlField<ObjectRef> m_LightingSettings = { "m_LightingSettings", {} };
		};

		class BuildSettings : public Object
		{
		public:
			BuildSettings();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
			friend std::ostream& operator<< (std::ostream& stream, BuildSettings& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<uint32> agentTypeID = { "agentTypeID", 0 };
			YamlField<float> agentRadius = { "agentRadius", 0.0f };
			YamlField<float> agentHeight = { "agentHeight", 0.0f };
			YamlField<float> agentSlope = { "agentSlope", 0.0f };
			YamlField<float> agentClimb = { "agentClimb", 0.0f };
			YamlField<float> ledgeDropHeight = { "ledgeDropHeight", 0.0f };
			YamlField<float> maxJumpAcrossDistance = { "maxJumpAcrossDistance", 0.0f };
			YamlField<float> minRegionArea = { "minRegionArea", 0.0f };
			YamlField<bool> manualCellSize = { "manualCellSize", false };
			YamlField<float> cellSize = { "cellSize", 0.0f };
			YamlField<bool> manualTileSize = { "manualTileSize", false };
			YamlField<uint32> tileSize = { "tileSize", 0 };
			YamlField<bool> accuratePlacement = { "accuratePlacement", false };
			YamlField<uint32> maxJobWorkers = { "maxJobWorkers", 0 };
			YamlField<bool> preserveTilesOutsideBounds = { "preserveTilesOutsideBounds", false };
		};

		class NavMeshSettings : public Object
		{
		public:
			NavMeshSettings(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<BuildSettings> m_BuildSettings = { "m_BuildSettings", {} };
			YamlField<FileID> m_NavMeshData = { "m_NavMeshData", 0 };
		};

		class PrefabInstance : public Object
		{
		public:
			PrefabInstance(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//private:
			YamlField<Modification> m_Modification = { "m_Modification", {} };
			YamlField<ObjectRef> m_SourcePrefab = { "m_SourcePrefab", {} };
		};

		class Prefab : public Object
		{
		public:
			Prefab(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//private:
			YamlField<Modification> m_Modification = { "m_Modification", {} };
			YamlField<FileID> m_ParentPrefab = { "m_ParentPrefab", {} };
			YamlField<FileID> m_RootGameObject = { "m_RootGameObject", {} };
			YamlField<bool> m_IsPrefabParent = { "m_IsPrefabParent", {} };
		};

		class MeshCollider : public Object
		{
		public:
			MeshCollider(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<ObjectRef> m_Material = { "m_Material", {} };
			YamlField<bool> m_IsTrigger = { "m_IsTrigger", false };
			YamlField<bool> m_Convex = { "m_Convex", false };
			YamlField<uint32> m_CookingOptions = { "m_CookingOptions", 0 };
			YamlField<ObjectRef> m_Mesh = { "m_Mesh", {} };
		};

		class MonoBehaviour : public Object
		{
		public:
			MonoBehaviour(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<uint32> m_EditorHideFlags = { "m_EditorHideFlags", 0 };
			YamlField<ObjectRef> m_Script = { "m_Script", {} };
			YamlField<std::string> m_Name = { "m_Name", "" };
			YamlField<std::string> m_EditorClassIdentifier = { "m_EditorClassIdentifier", "" };

			std::map<std::string, std::string> scriptValues;
		};

		class BoxCollider : public Object
		{
		public:
			BoxCollider(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<ObjectRef> m_Material = { "m_Material", {} };
			YamlField<bool> m_IsTrigger = { "m_IsTrigger", false };
			YamlField<glm::vec3> m_Size = { "m_Size", glm::vec3(0) };
			YamlField<glm::vec3> m_Center = { "m_Center", glm::vec3(0) };
		};

		class Light : public Object
		{
		public:
			Light(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			YamlField<uint32> m_Type = { "m_Type", 0 };
			YamlField<uint32> m_Shape = { "m_Shape", 0 };
			YamlField<glm::vec4> m_Color = { "m_Color", glm::vec4(0) };
			YamlField<float> m_Intensity = { "m_Intensity", 0.0f };
			YamlField<float> m_Range = { "m_Range", 0.0f };
			YamlField<float> m_SpotAngle = { "m_SpotAngle", 0.0f };
			YamlField<float> m_InnerSpotAngle = { "m_InnerSpotAngle", 0.0f };
			YamlField<float> m_CookieSize = { "m_CookieSize", 0.0f };
			YamlField<Shadows> m_Shadows = { "m_Shadows", {} };
			YamlField<ObjectRef> m_Cookie = { "m_Cookie", {} };
			YamlField<bool> m_DrawHalo = { "m_DrawHalo", false };
			YamlField<ObjectRef> m_Flare = { "m_Flare", {} };
			YamlField<uint32> m_RenderMode = { "m_RenderMode", 0 };
			YamlField<uint32> m_CullingMask = { "m_CullingMask", 0 };
			YamlField<uint32> m_RenderingLayerMask = { "m_RenderingLayerMask", 0 };
			YamlField<bool> m_Lightmapping = { "m_Lightmapping", false };
			YamlField<uint32> m_LightShadowCasterMode = { "m_LightShadowCasterMode", 0 };
			YamlField<glm::vec2> m_AreaSize = { "m_AreaSize", glm::vec2(0) };
			YamlField<float> m_BounceIntensity = { "m_BounceIntensity", 0.0f };
			YamlField<uint32> m_ColorTemperature = { "m_ColorTemperature", 0 };
			YamlField<bool> m_UseColorTemperature = { "m_UseColorTemperature", false };
			YamlField<glm::quat> m_BoundingSphereOverride = { "m_BoundingSphereOverride", glm::quat(1, 0, 0, 0) };
			YamlField<bool> m_UseBoundingSphereOverride = { "m_UseBoundingSphereOverride", false };
			YamlField<bool> m_UseViewFrustumForShadowCasterCull = { "m_UseViewFrustumForShadowCasterCull", false };
			YamlField<float> m_ShadowRadius = { "m_ShadowRadius", 0.0f };
			YamlField<float> m_ShadowAngle = { "m_ShadowAngle", 0.0f };
		};

		class LightProbeGroup : public Object
		{
		public:
			LightProbeGroup(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<std::vector<glm::vec3>> m_SourcePositions = { "m_SourcePositions", std::vector<glm::vec3>() };
		};

		class ReflectionProbe : public Object
		{
		public:
			ReflectionProbe(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//private:
			YamlField<uint32> m_Type = { "m_Type", 0 };
			YamlField<uint32> m_Mode = { "m_Mode", 0 };
			YamlField<uint32> m_RefreshMode = { "m_RefreshMode", 0 };
			YamlField<uint32> m_TimeSlicingMode = { "m_TimeSlicingMode", 0 };
			YamlField<uint32> m_Resolution = { "m_Resolution", 0 };
			YamlField<uint32> m_UpdateFrequency = { "m_UpdateFrequency", 0 };
			YamlField<glm::vec3> m_BoxSize = { "m_BoxSize", glm::vec3(0) };
			YamlField<glm::vec3> m_BoxOffset = { "m_BoxOffset", glm::vec3(0) };
			YamlField<float> m_NearClip = { "m_NearClip", 0.0f };
			YamlField<float> m_FarClip = { "m_FarClip", 0.0f };
			YamlField<float> m_ShadowDistance = { "m_ShadowDistance", 0.0f };
			YamlField<uint32> m_ClearFlags = { "m_ClearFlags", 0 };
			YamlField<glm::vec4> m_BackGroundColor = { "m_BackGroundColor", glm::vec4(0) };
			YamlField<uint32> m_CullingMask = { "m_CullingMask", 0 };
			YamlField<float> m_IntensityMultiplier = { "m_IntensityMultiplier", 0.0f };
			YamlField<float> m_BlendDistance = { "m_BlendDistance", 0.0f };
			YamlField<bool> m_HDR = { "m_HDR", false };
			YamlField<bool> m_BoxProjection = { "m_BoxProjection", false };
			YamlField<bool> m_RenderDynamicObjects = { "m_RenderDynamicObjects", false };
			YamlField<bool> m_UseOcclusionCulling = { "m_UseOcclusionCulling", false };
			YamlField<uint32> m_Importance = { "m_Importance", 0 };
			YamlField<ObjectRef> m_CustomBakedTexture = { "m_CustomBakedTexture", {} };
		};

		class CanvasRenderer : public Object
		{
		public:
			CanvasRenderer(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<bool> m_CullTransparentMesh = { "m_CullTransparentMesh", false };
		};

		class RectTransform : public Object
		{
		public:
			RectTransform(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<glm::quat> m_LocalRotation = { "m_LocalRotation", glm::quat(1, 0, 0, 0) };
			YamlField<glm::vec3> m_LocalPosition = { "m_LocalPosition", glm::vec3(0) };
			YamlField<glm::vec3> m_LocalScale = { "m_LocalScale", glm::vec3(1) };
			YamlField<std::vector<FileID>> m_Children = { "m_Children", std::vector<FileID>() };
			YamlField<FileID> m_Father = { "m_Father", 0 };
			YamlField<uint32> m_RootOrder = { "m_RootOrder", 0 };
			YamlField<glm::vec3> m_LocalEulerAnglesHint = { "m_LocalEulerAnglesHint", glm::vec3(0) };
			YamlField<glm::vec2> m_AnchorMin = { "m_AnchorMin", glm::vec2(0) };
			YamlField<glm::vec2> m_AnchorMax = { "m_AnchorMax", glm::vec2(0) };
			YamlField<glm::vec2> m_AnchoredPosition = { "m_AnchoredPosition", glm::vec2(0) };
			YamlField<glm::vec2> m_SizeDelta = { "m_SizeDelta", glm::vec2(0) };
			YamlField<glm::vec2> m_Pivot = { "m_Pivot", glm::vec2(0) };
		};

		class AudioListener : public Object
		{
		public:
			AudioListener(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
		};

		class Animator : public Object
		{
		public:
			Animator(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<ObjectRef> m_Avatar = { "m_Avatar", {} };
			YamlField<ObjectRef> m_Controller = { "m_Controller", {} };
			YamlField<uint32> m_CullingMode = { "m_CullingMode", 0 };
			YamlField<uint32> m_UpdateMode = { "m_UpdateMode", 0 };
			YamlField<bool> m_ApplyRootMotion = { "m_ApplyRootMotion", false };
			YamlField<bool> m_LinearVelocityBlending = { "m_LinearVelocityBlending", false };
			YamlField<std::string> m_WarningMessage = { "m_WarningMessage", "" };
			YamlField<bool> m_HasTransformHierarchy = { "m_HasTransformHierarchy", false };
			YamlField<bool> m_AllowConstantClipSamplingOptimization = { "m_AllowConstantClipSamplingOptimization", false };
			YamlField<bool> m_KeepAnimatorControllerStateOnDisable = { "m_KeepAnimatorControllerStateOnDisable", false };
		};

		class Point : public Object
		{
		public:
			Point();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Point& value)
			{
				stream << std::endl;
				value.print("    ");
				return stream;
			}

		private:
			YamlField<float> time = { "time", 0.0f };
			YamlField<float> value = { "value", 0.0f };
			YamlField<float> inSlope = { "inSlope", 0.0f };
			YamlField<float> outSlope = { "outSlope", 0.0f };
			YamlField<uint32> tangentMode = { "tangentMode", 0 };
			YamlField<uint32> weightedMode = { "weightedMode", 0 };
			YamlField<float> inWeight = { "inWeight", 0.0f };
			YamlField<float> outWeight = { "outWeight", 0.0f };
		};

		class Curve : public Object
		{
		public:
			Curve();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Curve& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<std::vector<Point>> m_Curve = { "m_Curve", std::vector<Point>() };
			YamlField<uint32> m_PreInfinity = { "m_PreInfinity", 0 };
			YamlField<uint32> m_PostInfinity = { "m_PostInfinity", 0 };
			YamlField<uint32> m_RotationOrder = { "m_RotationOrder", 0 };
		};

		class AudioSource : public Object
		{
		public:
			AudioSource(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<FileID> OutputAudioMixerGroup = { "OutputAudioMixerGroup", 0 };
			YamlField<ObjectRef> m_audioClip = { "m_audioClip", {} };
			YamlField<bool> m_PlayOnAwake = { "m_PlayOnAwake", false };
			YamlField<float> m_Volume = { "m_Volume", 0.0f };
			YamlField<float> m_Pitch = { "m_Pitch", 0.0f };
			YamlField<bool> Loop = { "Loop", false };
			YamlField<bool> Mute = { "Mute", false };
			YamlField<bool> Spatialize = { "Spatialize", false };
			YamlField<bool> SpatializePostEffects = { "SpatializePostEffects", false };
			YamlField<uint32> Priority = { "Priority", false };
			YamlField<float> DopplerLevel = { "DopplerLevel", 0.0f };
			YamlField<float> MinDistance = { "MinDistance", 0.0f };
			YamlField<float> MaxDistance = { "MaxDistance", 0.0f };
			YamlField<float> Pan2D = { "Pan2D", 0.0f };
			YamlField<uint32> rolloffMode = { "rolloffMode", 0 };
			YamlField<bool> BypassEffects = { "BypassEffects", false };
			YamlField<bool> BypassListenerEffects = { "BypassListenerEffects", false };
			YamlField<bool> BypassReverbZones = { "BypassReverbZones", false };
			YamlField<Curve> rolloffCustomCurve = { "rolloffCustomCurve", {} };
			YamlField<Curve> panLevelCustomCurve = { "panLevelCustomCurve", {} };
			YamlField<Curve> spreadCustomCurve = { "spreadCustomCurve", {} };
			YamlField<Curve> reverbZoneMixCustomCurve = { "reverbZoneMixCustomCurve", {} };
		};

		class NormalizedViewPortRect : public Object
		{
		public:
			NormalizedViewPortRect();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, NormalizedViewPortRect& value)
			{
				stream << std::endl;
				value.print("  ");
				return stream;
			}

		private:
			YamlField<float> x = { "x", 0.0f };
			YamlField<float> y = { "y", 0.0f };
			YamlField<float> width = { "width", 0.0f };
			YamlField<float> height = { "height", 0.0f };
		};

		class Camera : public Object
		{
		public:
			Camera(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<uint32> m_ClearFlags = { "m_ClearFlags", 0 };
			YamlField<glm::vec4> m_BackGroundColor = { "m_BackGroundColor", glm::vec4(0) };
			YamlField<uint32> m_projectionMatrixMode = { "m_projectionMatrixMode", 0 };
			YamlField<uint32> m_GateFitMode = { "m_GateFitMode", 0 };
			YamlField<uint32> m_FOVAxisMode = { "m_FOVAxisMode", 0 };
			YamlField<glm::vec2> m_SensorSize = { "m_SensorSize", glm::vec2(0) };
			YamlField<glm::vec2> m_LensShift = { "m_LensShift", glm::vec2(0) };
			YamlField<float> m_FocalLength = { "m_FocalLength", 0.0f };
			YamlField<NormalizedViewPortRect> m_NormalizedViewPortRect = { "m_NormalizedViewPortRect", {} };
			YamlField<float> near_clip_plane = { "near clip plane", 0.0f };
			YamlField<float> far_clip_plane = { "far clip plane", 0.0f };
			YamlField<float> field_of_view = { "field of view", 0.0f };
			YamlField<bool> orthographic = { "orthographic", false };
			YamlField<float> orthographic_size = { "orthographic size", 0.0f };
			YamlField<float> m_Depth = { "m_Depth", 0.0f };
			YamlField<uint32> m_CullingMask = { "m_CullingMask", 0 };
			YamlField<int32> m_RenderingPath = { "m_RenderingPath", 0 };
			YamlField<ObjectRef> m_TargetTexture = { "m_TargetTexture", {} };
			YamlField<uint32> m_TargetDisplay = { "m_TargetDisplay", 0 };
			YamlField<uint32> m_TargetEye = { "m_TargetEye", 0 };
			YamlField<uint32> m_HDR = { "m_HDR", 0 };
			YamlField<uint32> m_AllowMSAA = { "m_AllowMSAA", 0 };
			YamlField<bool> m_AllowDynamicResolution = { "m_AllowDynamicResolution", false };
			YamlField<bool> m_ForceIntoRT = { "m_ForceIntoRT", false };
			YamlField<bool> m_OcclusionCulling = { "m_OcclusionCulling", false };
			YamlField<float> m_StereoConvergence = { "m_StereoConvergence", 0.0f };
			YamlField<float> m_StereoSeparation = { "m_StereoSeparation", 0.0f };
		};

		class Canvas : public Object
		{
		public:
			Canvas(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<uint32> m_RenderMode = { "m_RenderMode", 0 };
			YamlField<FileID> m_Camera = { "m_Camera", 0 };
			YamlField<float> m_PlaneDistance = { "m_PlaneDistance", 0.0f };
			YamlField<bool> m_PixelPerfect = { "m_PixelPerfect", false };
			YamlField<bool> m_ReceivesEvents = { "m_ReceivesEvents", false };
			YamlField<bool> m_OverrideSorting = { "m_OverrideSorting", false };
			YamlField<bool> m_OverridePixelPerfect = { "m_OverridePixelPerfect", false };
			YamlField<float> m_SortingBucketNormalizedSize = { "m_SortingBucketNormalizedSize", 0.0f };
			YamlField<uint32> m_AdditionalShaderChannelsFlag = { "m_AdditionalShaderChannelsFlag", 0 };
			YamlField<uint32> m_SortingLayerID = { "m_SortingLayerID", 0 };
			YamlField<uint32> m_SortingOrder = { "m_SortingOrder", 0 };
			YamlField<uint32> m_TargetDisplay = { "m_TargetDisplay", 0 };
		};

		class CapsuleCollider : public Object
		{
		public:
			CapsuleCollider(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<ObjectRef> m_Material = { "m_Material", {} };
			YamlField<bool> m_IsTrigger = { "m_IsTrigger", false };
			YamlField<float> m_Radius = { "m_Radius", 0.0f };
			YamlField<float> m_Height = { "m_Height", 0.0f };
			YamlField<uint32> m_Direction = { "m_Direction", 0 };
			YamlField<glm::vec3> m_Center = { "m_Center", glm::vec3(0) };
		};

		class CharacterController : public Object
		{
		public:
			CharacterController(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<ObjectRef> m_Material = { "m_Material", {} };
			YamlField<bool> m_IsTrigger = { "m_IsTrigger", false };
			YamlField<float> m_Height = { "m_Height", 0.0f };
			YamlField<float> m_Radius = { "m_Radius", 0.0f };
			YamlField<float> m_SlopeLimit = { "m_SlopeLimit", 0.0f };
			YamlField<float> m_StepOffset = { "m_StepOffset", 0.0f };
			YamlField<float> m_SkinWidth = { "m_SkinWidth", 0.0f };
			YamlField<float> m_MinMoveDistance = { "m_MinMoveDistance", 0.0f };
			YamlField<glm::vec3> m_Center = { "m_Center", glm::vec3(0) };
		};

		class ParticleSystemRenderer : public Object
		{
		public:
			ParticleSystemRenderer(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		private:
			YamlField<uint32> m_CastShadows = { "m_CastShadows", 0 }; // 0 - ON, 1 - OFF, 2 - Two Sided, 3 - Only Shadows
			YamlField<bool> m_ReceiveShadows = { "m_ReceiveShadows", false };
			YamlField<bool> m_DynamicOccludee = { "m_DynamicOccludee", false };
			YamlField<uint32> m_MotionVectors = { "m_MotionVectors", 0 }; // 0 - Camera, 1 - Object, 2 - No Motion
			YamlField<uint32> m_LightProbeUsage = { "m_LightProbeUsage", 0 }; // 0 - OFF, 1 - BlendProbes, 2 - Proxy Volume, 3 - Custom
			YamlField<uint32> m_ReflectionProbeUsage = { "m_ReflectionProbeUsage", 0 }; // 0 - OFF, 1 - BlendProbes, 2 - BlendProbes and sykybox, 3 - Simple
			YamlField<uint32> m_RayTracingMode = { "m_RayTracingMode", 0 }; // ???
			YamlField<bool> m_RayTraceProcedural = { "m_RayTraceProcedural", false }; // ???
			YamlField<uint32> m_RenderingLayerMask = { "m_RenderingLayerMask", 0 }; // What layers this renders into???
			YamlField<uint32> m_RendererPriority = { "m_RendererPriority", 0 }; // This actually comes from the shader/material
			YamlField<std::vector<ObjectRef>> m_Materials = { "m_Materials", std::vector<ObjectRef>() }; // used materials need to match the amount of submeshes
			YamlField<FileID> m_ProbeAnchor = { "m_ProbeAnchor", 0 }; // Overrides the position of the ReflectionProbe
			YamlField<FileID> m_LightProbeVolumeOverride = { "m_LightProbeVolumeOverride", 0 }; // ???
			YamlField<float> m_ScaleInLightmap = { "m_ScaleInLightmap", 0 }; // Scale of the LightUVs for this Object
			YamlField<uint32> m_ReceiveGI = { "m_ReceiveGI", 0 }; // 1 - Light probes, 2 - Light maps
			YamlField<bool> m_PreserveUVs = { "m_PreserveUVs", false }; // Preserve UVs from mesh
			YamlField<bool> m_IgnoreNormalsForChartDetection = { "m_IgnoreNormalsForChartDetection", false }; // Light map UVs are not split when normal direction changes
			YamlField<bool> m_ImportantGI = { "m_ImportantGI", false }; // ???
			YamlField<bool> m_StitchLightmapSeams = { "m_StitchLightmapSeams", false }; // Stitch seams at UV discontinuities
			YamlField<uint32> m_SelectedEditorRenderState = { "m_SelectedEditorRenderState", 0 }; // ???
			YamlField<uint32> m_MinimumChartSize = { "m_MinimumChartSize", 0 }; // Minimum texel size of a UV chart (needs atleast 4 to be stitchable)
			YamlField<float> m_AutoUVMaxDistance = { "m_AutoUVMaxDistance", 0.0f }; // max distance for simplification
			YamlField<float> m_AutoUVMaxAngle = { "m_AutoUVMaxAngle", 0.0f }; // max angle for simplification
			YamlField<ObjectRef> m_LightmapParameters = { "m_LightmapParameters", { 0, 0, "" } }; // reference to the light map asset
			YamlField<uint32> m_SortingLayerID = { "m_SortingLayerID", 0 }; // Propably for the render queue
			YamlField<uint32> m_SortingLayer = { "m_SortingLayer", 0 }; // Propably for the render queue
			YamlField<uint32> m_SortingOrder = { "m_SortingOrder", 0 }; // Propably for the render queue

			YamlField<uint32> m_RenderMode = { "m_RenderMode", 0 };
			YamlField<uint32> m_SortMode = { "m_SortMode", 0 };
			YamlField<float> m_MinParticleSize = { "m_MinParticleSize", 0.0f };
			YamlField<float> m_MaxParticleSize = { "m_MaxParticleSize", 0.0f };
			YamlField<float> m_CameraVelocityScale = { "m_CameraVelocityScale", 0.0f };
			YamlField<float> m_VelocityScale = { "m_VelocityScale", 0.0f };
			YamlField<float> m_LengthScale = { "m_LengthScale", 0.0f };
			YamlField<float> m_SortingFudge = { "m_SortingFudge", 0.0f };
			YamlField<float> m_NormalDirection = { "m_NormalDirection", 0.0f };
			YamlField<float> m_ShadowBias = { "m_ShadowBias", 0.0f };
			YamlField<float> m_RenderAlignment = { "m_RenderAlignment", 0.0f };
			YamlField<glm::vec3> m_Pivot = { "m_Pivot", glm::vec3(0) };
			YamlField<glm::vec3> m_Flip = { "m_Flip", glm::vec3(0) };
			YamlField<bool> m_UseCustomVertexStreams = { "m_UseCustomVertexStreams", false };
			YamlField<bool> m_EnableGPUInstancing = { "m_EnableGPUInstancing", false };
			YamlField<bool> m_ApplyActiveColorSpace = { "m_ApplyActiveColorSpace", false };
			YamlField<bool> m_AllowRoll = { "m_AllowRoll", false };
			YamlField<bool> m_FreeformStretching = { "m_FreeformStretching", false };
			YamlField<bool> m_RotateWithStretchDirection = { "m_RotateWithStretchDirection", false };
			YamlField<uint32> m_VertexStreams = { "m_VertexStreams", 0 };
			YamlField<ObjectRef> m_Mesh = { "m_Mesh", {} };
			YamlField<ObjectRef> m_Mesh1 = { "m_Mesh1", {} };
			YamlField<ObjectRef> m_Mesh2 = { "m_Mesh2", {} };
			YamlField<ObjectRef> m_Mesh3 = { "m_Mesh3", {} };
			YamlField<uint32> m_MaskInteraction = { "m_MaskInteraction", 0 };
		};

		class ParticleSystem : public Object
		{
		public:
			ParticleSystem(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			YamlField<float> lengthInSec = { "lengthInSec", 0.0f };
			YamlField<float> simulationSpeed = { "simulationSpeed", 0.0f };
			YamlField<uint32> stopAction = { "stopAction", 0 };
			YamlField<uint32> cullingMode = { "cullingMode", 0 };
			YamlField<uint32> ringBufferMode = { "ringBufferMode", 0 };
			YamlField<glm::vec2> ringBufferLoopRange = { "ringBufferLoopRange", glm::vec2(0) };
			YamlField<bool> looping = { "looping", false };
			YamlField<bool> prewarm = { "prewarm", false };
			YamlField<bool> playOnAwake = { "playOnAwake", false };
			YamlField<uint32> useUnscaledTime = { "useUnscaledTime", 0 };
			YamlField<bool> autoRandomSeed = { "autoRandomSeed", false };
			YamlField<uint32> useRigidbodyForVelocity = { "useRigidbodyForVelocity", 0 };
			// TODO: startDelay
			YamlField<bool> moveWithTransform = { "moveWithTransform", false };
			YamlField<FileID> moveWithCustomTransform = { "moveWithCustomTransform", false };
			YamlField<uint32> scalingMode = { "scalingMode", 0 };
			YamlField<uint32> randomSeed = { "randomSeed", 0 };
			// TODO: Modules...
		};

		class Material : public Object
		{
		public:
			Material(int64 id, uint32 type);
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			YamlField<std::string> m_Name = { "m_Name", "" };
			YamlField<ObjectRef> m_Shader = { "m_Shader", {} };
			YamlField<std::string> m_ShaderKeywords = { "m_ShaderKeywords", "" };
			YamlField<uint32> m_LightmapFlags = { "m_LightmapFlags", 0 };
			YamlField<bool> m_EnableInstancingVariants = { "m_EnableInstancingVariants", false };
			YamlField<bool> m_DoubleSidedGI = { "m_DoubleSidedGI", false };
			YamlField<int32> m_CustomRenderQueue = { "m_CustomRenderQueue", 0 };
			// TODO: find out what a stringTagMap is...
			YamlField<std::vector<std::string>> disabledShaderPasses = { "disabledShaderPasses", std::vector<std::string>() };
			YamlField<std::vector<TextureProperty>> m_TexEnvs = { "m_TexEnvs", std::vector<TextureProperty>() };
			YamlField<std::vector<FloatProperty>> m_Floats = { "m_Floats", std::vector<FloatProperty>() };
			YamlField<std::vector<ColorProperty>> m_Colors = { "m_Colors", std::vector<ColorProperty>() };
		};

		bool read(ryml::ConstNodeRef n, Object* value);
		bool read(ryml::ConstNodeRef n, GISettings* value);
		bool read(ryml::ConstNodeRef n, LightmapEditorSettings* value);
		bool read(ryml::ConstNodeRef n, BuildSettings* value);
		bool read(ryml::ConstNodeRef n, Curve* value);
		bool read(ryml::ConstNodeRef n, Point* value);
		bool read(ryml::ConstNodeRef n, NormalizedViewPortRect* value);
	}
}

#endif // INCLUDED_OBJECTTYPES