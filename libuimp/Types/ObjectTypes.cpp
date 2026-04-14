#include "DataTypes.h"
#include "ObjectTypes.h"

namespace Unity
{
	namespace YAML
	{
		Object::Object(int64 id, uint32 type) :
			id(id),
			type(type)
		{
		}

		void Object::fromBaseYAML(ryml::ConstNodeRef n)
		{
			if (m_ObjectHideFlags.isPresent = n.has_child(m_ObjectHideFlags.key))
				n[m_ObjectHideFlags.key] >> m_ObjectHideFlags.value;
			if (m_CorrespondingSourceObject.isPresent = n.has_child(m_CorrespondingSourceObject.key))
				n[m_CorrespondingSourceObject.key] >> m_CorrespondingSourceObject.value;
			if (m_PrefabInstance.isPresent = n.has_child(m_PrefabInstance.key))
				n[m_PrefabInstance.key] >> m_PrefabInstance.value;
			if (m_PrefabAsset.isPresent = n.has_child(m_PrefabAsset.key))
				n[m_PrefabAsset.key] >> m_PrefabAsset.value;
			if (m_PrefabParentObject.isPresent = n.has_child(m_PrefabParentObject.key))
				n[m_PrefabParentObject.key] >> m_PrefabParentObject.value;
			if (m_PrefabInternal.isPresent = n.has_child(m_PrefabInternal.key))
				n[m_PrefabInternal.key] >> m_PrefabInternal.value;
			if (m_GameObject.isPresent = n.has_child(m_GameObject.key))
				n[m_GameObject.key] >> m_GameObject.value;
			if (m_Enabled.isPresent = n.has_child(m_Enabled.key))
				n[m_Enabled.key] >> m_Enabled.value;
			if (serializedVersion.isPresent = n.has_child(serializedVersion.key))
				n[serializedVersion.key] >> serializedVersion.value;
		}

		void Object::print(const std::string& prefix)
		{
			if (m_ObjectHideFlags.isPresent)
				m_ObjectHideFlags.print(prefix);
			if (m_CorrespondingSourceObject.isPresent)
				m_CorrespondingSourceObject.print(prefix);
			if (m_PrefabInstance.isPresent)
				m_PrefabInstance.print(prefix);
			if (m_PrefabAsset.isPresent)
				m_PrefabAsset.print(prefix);
			if (m_PrefabParentObject.isPresent)
				m_PrefabParentObject.print(prefix);
			if (m_PrefabInternal.isPresent)
				m_PrefabInternal.print(prefix);
			if (m_GameObject.isPresent)
				m_GameObject.print(prefix);
			if (m_Enabled.isPresent)
				m_Enabled.print(prefix);
			if (serializedVersion.isPresent)
				serializedVersion.print(prefix);
		}

		int64 Object::getID()
		{
			return id;
		}

		uint32 Object::getType()
		{
			return type;
		}

		GameObject::GameObject(int64 id, uint32 type, bool stripped) :
			Object(id, type),
			stripped(stripped) 
		{}

		void GameObject::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);

			if (!stripped) {
				n[m_Component.key] >> m_Component.value;
				n[m_Layer.key] >> m_Layer.value;
				n[m_Name.key] >> m_Name.value;
				n[m_TagString.key] >> m_TagString.value;
				n[m_Icon.key] >> m_Icon.value;
				n[m_NavMeshLayer.key] >> m_NavMeshLayer.value;
				n[m_StaticEditorFlags.key] >> m_StaticEditorFlags.value;
				n[m_IsActive.key] >> m_IsActive.value;
			}
		}

		void GameObject::print(const std::string& prefix)
		{
			Object::print(prefix);
			std::cout << m_Component.key << std::endl;
			for (auto& component : m_Component.value)
			{
				std::cout << prefix << "- ";
				component.print();
				std::cout << std::endl;
			}
			m_Layer.print(prefix);
			m_Name.print(prefix);
			m_TagString.print(prefix);
			m_Icon.print(prefix);
			m_NavMeshLayer.print(prefix);
			m_StaticEditorFlags.print(prefix);
			m_IsActive.print(prefix);
		}

		Transform::Transform(int64 id, uint32 type, bool stripped) :
			Object(id, type),
			stripped(stripped)
		{
		}

		void Transform::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);

			if (!stripped) {
				read(n[m_LocalRotation.key], m_LocalRotation.value);
				read(n[m_LocalPosition.key], m_LocalPosition.value);
				read(n[m_LocalScale.key], m_LocalScale.value);
				n[m_Children.key] >> m_Children.value;
				n[m_Father.key] >> m_Father.value;
				n[m_RootOrder.key] >> m_RootOrder.value;
				read(n[m_LocalEulerAnglesHint.key], m_LocalEulerAnglesHint.value);
			}
		}

		void Transform::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_LocalRotation.print(prefix);
			m_LocalPosition.print(prefix);
			m_LocalScale.print(prefix);
			std::cout << m_Children.key << std::endl;
			for (auto& child : m_Children.value)
				std::cout << prefix << "- " << child << std::endl;
			m_Father.print(prefix);
			m_RootOrder.print(prefix);
			m_LocalEulerAnglesHint.print(prefix);
		}

		MeshRenderer::MeshRenderer(int64 id, uint32 type, bool stripped) :
			Object(id, type),
			stripped(stripped)
		{
		}

		void MeshRenderer::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);

			if (!stripped) {
				n[m_CastShadows.key] >> m_CastShadows.value;
				n[m_ReceiveShadows.key] >> m_ReceiveShadows.value;
				n[m_DynamicOccludee.key] >> m_DynamicOccludee.value;
				n[m_MotionVectors.key] >> m_MotionVectors.value;
				n[m_LightProbeUsage.key] >> m_LightProbeUsage.value;
				n[m_ReflectionProbeUsage.key] >> m_ReflectionProbeUsage.value;
				if (n.has_child(m_RayTracingMode.key))
					n[m_RayTracingMode.key] >> m_RayTracingMode.value;
				if (n.has_child(m_RayTraceProcedural.key))
					n[m_RayTraceProcedural.key] >> m_RayTraceProcedural.value;
				n[m_RenderingLayerMask.key] >> m_RenderingLayerMask.value;
				if (n.has_child(m_RendererPriority.key))
					n[m_RendererPriority.key] >> m_RendererPriority.value;
				n[m_Materials.key] >> m_Materials.value;
				n[m_ProbeAnchor.key] >> m_ProbeAnchor.value;
				n[m_LightProbeVolumeOverride.key] >> m_LightProbeVolumeOverride.value;
				n[m_ScaleInLightmap.key] >> m_ScaleInLightmap.value;
				if (n.has_child(m_ReceiveGI.key))
					n[m_ReceiveGI.key] >> m_ReceiveGI.value;
				n[m_PreserveUVs.key] >> m_PreserveUVs.value;
				n[m_IgnoreNormalsForChartDetection.key] >> m_IgnoreNormalsForChartDetection.value;
				n[m_ImportantGI.key] >> m_ImportantGI.value;
				n[m_StitchLightmapSeams.key] >> m_StitchLightmapSeams.value;
				n[m_SelectedEditorRenderState.key] >> m_SelectedEditorRenderState.value;
				n[m_MinimumChartSize.key] >> m_MinimumChartSize.value;
				n[m_AutoUVMaxDistance.key] >> m_AutoUVMaxDistance.value;
				n[m_AutoUVMaxAngle.key] >> m_AutoUVMaxAngle.value;
				n[m_LightmapParameters.key] >> m_LightmapParameters.value;
				n[m_SortingLayerID.key] >> m_SortingLayerID.value;
				n[m_SortingOrder.key] >> m_SortingOrder.value;
				if (n.has_child(m_AdditionalVertexStreams.key))
					n[m_AdditionalVertexStreams.key] >> m_AdditionalVertexStreams.value;
			}
		}

		void MeshRenderer::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_CastShadows.print(prefix);
			m_ReceiveShadows.print(prefix);
			m_DynamicOccludee.print(prefix);
			m_MotionVectors.print(prefix);
			m_LightProbeUsage.print(prefix);
			m_ReflectionProbeUsage.print(prefix);
			m_RayTracingMode.print(prefix);
			m_RayTraceProcedural.print(prefix);
			m_RenderingLayerMask.print(prefix);
			m_RendererPriority.print(prefix);
			std::cout << m_Materials.key << std::endl;
			for (auto& mat : m_Materials.value)
				std::cout << "- " << mat << std::endl;
			m_ProbeAnchor.print(prefix);
			m_LightProbeVolumeOverride.print(prefix);
			m_ScaleInLightmap.print(prefix);
			m_ReceiveGI.print(prefix);
			m_PreserveUVs.print(prefix);
			m_IgnoreNormalsForChartDetection.print(prefix);
			m_ImportantGI.print(prefix);
			m_StitchLightmapSeams.print(prefix);
			m_SelectedEditorRenderState.print(prefix);
			m_MinimumChartSize.print(prefix);
			m_AutoUVMaxDistance.print(prefix);
			m_AutoUVMaxAngle.print(prefix);
			m_LightmapParameters.print(prefix);
			m_SortingLayerID.print(prefix);
			m_SortingLayer.print(prefix);
			m_SortingOrder.print(prefix);
			m_AdditionalVertexStreams.print(prefix);
		}

		MeshFilter::MeshFilter(int64 id, uint32 type) : 
			Object(id, type)
		{}

		void MeshFilter::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Mesh.key] >> m_Mesh.value;
		}

		void MeshFilter::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Mesh.print(prefix);
		}

		OcclusionCullingSettings::OcclusionCullingSettings(int64 id, uint32 type) : 
			Object(id, type) 
		{}
		
		void OcclusionCullingSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_OcclusionBakeSettings.key] >> m_OcclusionBakeSettings.value;
			n[m_SceneGUID.key] >> m_SceneGUID.value;
			n[m_OcclusionCullingData.key] >> m_OcclusionCullingData.value;
		}

		void OcclusionCullingSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_OcclusionBakeSettings.print(prefix);
			m_SceneGUID.print(prefix);
			m_OcclusionCullingData.print(prefix);
		}

		RenderSettings::RenderSettings(int64 id, uint32 type) : 
			Object(id, type) 
		{}
		
		void RenderSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Fog.key] >> m_Fog.value;
			read(n[m_FogColor.key], m_FogColor.value);
			n[m_FogMode.key] >> m_FogMode.value;
			n[m_FogDensity.key] >> m_FogDensity.value;
			n[m_LinearFogStart.key] >> m_LinearFogStart.value;
			n[m_LinearFogEnd.key] >> m_LinearFogEnd.value;
			read(n[m_AmbientSkyColor.key], m_AmbientSkyColor.value);
			read(n[m_AmbientEquatorColor.key], m_AmbientEquatorColor.value);
			read(n[m_AmbientGroundColor.key], m_AmbientGroundColor.value);
			n[m_AmbientIntensity.key] >> m_AmbientIntensity.value;
			n[m_AmbientMode.key] >> m_AmbientMode.value;
			read(n[m_SubtractiveShadowColor.key], m_SubtractiveShadowColor.value);
			n[m_SkyboxMaterial.key] >> m_SkyboxMaterial.value;
			n[m_HaloStrength.key] >> m_HaloStrength.value;
			n[m_FlareStrength.key] >> m_FlareStrength.value;
			n[m_FlareFadeSpeed.key] >> m_FlareFadeSpeed.value;
			n[m_HaloTexture.key] >> m_HaloTexture.value;
			n[m_SpotCookie.key] >> m_SpotCookie.value;
			n[m_DefaultReflectionMode.key] >> m_DefaultReflectionMode.value;
			n[m_DefaultReflectionResolution.key] >> m_DefaultReflectionResolution.value;
			n[m_ReflectionBounces.key] >> m_ReflectionBounces.value;
			n[m_ReflectionIntensity.key] >> m_ReflectionIntensity.value;
			n[m_CustomReflection.key] >> m_CustomReflection.value;
			n[m_Sun.key] >> m_Sun.value;
			read(n[m_IndirectSpecularColor.key], m_IndirectSpecularColor.value);
			n[m_UseRadianceAmbientProbe.key] >> m_UseRadianceAmbientProbe.value;
		}

		void RenderSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Fog.print(prefix);
			m_FogColor.print(prefix);
			m_FogMode.print(prefix);
			m_FogDensity.print(prefix);
			m_LinearFogStart.print(prefix);
			m_LinearFogEnd.print(prefix);
			m_AmbientSkyColor.print(prefix);
			m_AmbientEquatorColor.print(prefix);
			m_AmbientGroundColor.print(prefix);
			m_AmbientIntensity.print(prefix);
			m_AmbientMode.print(prefix);
			m_SubtractiveShadowColor.print(prefix);
			m_SkyboxMaterial.print(prefix);
			m_HaloStrength.print(prefix);
			m_FlareStrength.print(prefix);
			m_FlareFadeSpeed.print(prefix);
			m_HaloTexture.print(prefix);
			m_SpotCookie.print(prefix);
			m_DefaultReflectionMode.print(prefix);
			m_DefaultReflectionResolution.print(prefix);
			m_ReflectionBounces.print(prefix);
			m_ReflectionIntensity.print(prefix);
			m_CustomReflection.print(prefix);
			m_Sun.print(prefix);
			m_IndirectSpecularColor.print(prefix);
			m_UseRadianceAmbientProbe.print(prefix);
		}

		LightingSettings::LightingSettings(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void LightingSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_GIWorkflowMode.key] >> m_GIWorkflowMode.value;
			n[m_EnableBakedLightmaps.key] >> m_EnableBakedLightmaps.value;
			n[m_EnableRealtimeLightmaps.key] >> m_EnableRealtimeLightmaps.value;
			n[m_RealtimeEnvironmentLighting.key] >> m_RealtimeEnvironmentLighting.value;
			n[m_BounceScale.key] >> m_BounceScale.value;
			n[m_AlbedoBoost.key] >> m_AlbedoBoost.value;
			n[m_IndirectOutputScale.key] >> m_IndirectOutputScale.value;
			n[m_UsingShadowmask.key] >> m_UsingShadowmask.value;
			n[m_BakeBackend.key] >> m_BakeBackend.value;
			n[m_LightmapMaxSize.key] >> m_LightmapMaxSize.value;
			n[m_BakeResolution.key] >> m_BakeResolution.value;
			n[m_Padding.key] >> m_Padding.value;
			n[m_TextureCompression.key] >> m_TextureCompression.value;
			n[m_AO.key] >> m_AO.value;
			n[m_AOMaxDistance.key] >> m_AOMaxDistance.value;
			n[m_CompAOExponent.key] >> m_CompAOExponent.value;
			n[m_CompAOExponentDirect.key] >> m_CompAOExponentDirect.value;
			n[m_ExtractAO.key] >> m_ExtractAO.value;
			n[m_MixedBakeMode.key] >> m_MixedBakeMode.value;
			n[m_LightmapsBakeMode.key] >> m_LightmapsBakeMode.value;
			n[m_FilterMode.key] >> m_FilterMode.value;
			n[m_LightmapParameters.key] >> m_LightmapParameters.value;
			n[m_ExportTrainingData.key] >> m_ExportTrainingData.value;
			n[m_TrainingDataDestination.key] >> m_TrainingDataDestination.value;
			n[m_RealtimeResolution.key] >> m_RealtimeResolution.value;
			n[m_ForceWhiteAlbedo.key] >> m_ForceWhiteAlbedo.value;
			n[m_ForceUpdates.key] >> m_ForceUpdates.value;
			n[m_FinalGather.key] >> m_FinalGather.value;
			n[m_FinalGatherRayCount.key] >> m_FinalGatherRayCount.value;
			n[m_FinalGatherFiltering.key] >> m_FinalGatherFiltering.value;
			n[m_PVRCulling.key] >> m_PVRCulling.value;
			n[m_PVRSampling.key] >> m_PVRSampling.value;
			n[m_PVRDirectSampleCount.key] >> m_PVRDirectSampleCount.value;
			n[m_PVRSampleCount.key] >> m_PVRSampleCount.value;
			n[m_PVREnvironmentSampleCount.key] >> m_PVREnvironmentSampleCount.value;
			n[m_PVREnvironmentReferencePointCount.key] >> m_PVREnvironmentReferencePointCount.value;
			n[m_LightProbeSampleCountMultiplier.key] >> m_LightProbeSampleCountMultiplier.value;
			n[m_PVRBounces.key] >> m_PVRBounces.value;
			n[m_PVRMinBounces.key] >> m_PVRMinBounces.value;
			n[m_PVREnvironmentMIS.key] >> m_PVREnvironmentMIS.value;
			n[m_PVRFilteringMode.key] >> m_PVRFilteringMode.value;
			n[m_PVRDenoiserTypeDirect.key] >> m_PVRDenoiserTypeDirect.value;
			n[m_PVRDenoiserTypeIndirect.key] >> m_PVRDenoiserTypeIndirect.value;
			n[m_PVRDenoiserTypeAO.key] >> m_PVRDenoiserTypeAO.value;
			n[m_PVRFilterTypeDirect.key] >> m_PVRFilterTypeDirect.value;
			n[m_PVRFilterTypeIndirect.key] >> m_PVRFilterTypeIndirect.value;
			n[m_PVRFilterTypeAO.key] >> m_PVRFilterTypeAO.value;
			n[m_PVRFilteringGaussRadiusDirect.key] >> m_PVRFilteringGaussRadiusDirect.value;
			n[m_PVRFilteringGaussRadiusIndirect.key] >> m_PVRFilteringGaussRadiusIndirect.value;
			n[m_PVRFilteringGaussRadiusAO.key] >> m_PVRFilteringGaussRadiusAO.value;
			n[m_PVRFilteringAtrousPositionSigmaDirect.key] >> m_PVRFilteringAtrousPositionSigmaDirect.value;
			n[m_PVRFilteringAtrousPositionSigmaIndirect.key] >> m_PVRFilteringAtrousPositionSigmaIndirect.value;
			n[m_PVRFilteringAtrousPositionSigmaAO.key] >> m_PVRFilteringAtrousPositionSigmaAO.value;
		}

		void LightingSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_GIWorkflowMode.print(prefix);
			m_EnableBakedLightmaps.print(prefix);
			m_EnableRealtimeLightmaps.print(prefix);
			m_RealtimeEnvironmentLighting.print(prefix);
			m_BounceScale.print(prefix);
			m_AlbedoBoost.print(prefix);
			m_IndirectOutputScale.print(prefix);
			m_UsingShadowmask.print(prefix);
			m_BakeBackend.print(prefix);
			m_LightmapMaxSize.print(prefix);
			m_BakeResolution.print(prefix);
			m_Padding.print(prefix);
			m_TextureCompression.print(prefix);
			m_AO.print(prefix);
			m_AOMaxDistance.print(prefix);
			m_CompAOExponent.print(prefix);
			m_CompAOExponentDirect.print(prefix);
			m_ExtractAO.print(prefix);
			m_MixedBakeMode.print(prefix);
			m_LightmapsBakeMode.print(prefix);
			m_FilterMode.print(prefix);
			m_LightmapParameters.print(prefix);
			m_ExportTrainingData.print(prefix);
			m_TrainingDataDestination.print(prefix);
			m_RealtimeResolution.print(prefix);
			m_ForceWhiteAlbedo.print(prefix);
			m_ForceUpdates.print(prefix);
			m_FinalGather.print(prefix);
			m_FinalGatherRayCount.print(prefix);
			m_FinalGatherFiltering.print(prefix);
			m_PVRCulling.print(prefix);
			m_PVRSampling.print(prefix);
			m_PVRDirectSampleCount.print(prefix);
			m_PVRSampleCount.print(prefix);
			m_PVREnvironmentSampleCount.print(prefix);
			m_PVREnvironmentReferencePointCount.print(prefix);
			m_LightProbeSampleCountMultiplier.print(prefix);
			m_PVRBounces.print(prefix);
			m_PVRMinBounces.print(prefix);
			m_PVREnvironmentMIS.print(prefix);
			m_PVRFilteringMode.print(prefix);
			m_PVRDenoiserTypeDirect.print(prefix);
			m_PVRDenoiserTypeIndirect.print(prefix);
			m_PVRDenoiserTypeAO.print(prefix);
			m_PVRFilterTypeDirect.print(prefix);
			m_PVRFilterTypeIndirect.print(prefix);
			m_PVRFilterTypeAO.print(prefix);
			m_PVRFilteringGaussRadiusDirect.print(prefix);
			m_PVRFilteringGaussRadiusIndirect.print(prefix);
			m_PVRFilteringGaussRadiusAO.print(prefix);
			m_PVRFilteringAtrousPositionSigmaDirect.print(prefix);
			m_PVRFilteringAtrousPositionSigmaIndirect.print(prefix);
			m_PVRFilteringAtrousPositionSigmaAO.print(prefix);
		}

		GISettings::GISettings() : 
			Object(0, 0) 
		{}
		
		void GISettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_BounceScale.key] >> m_BounceScale.value;
			n[m_IndirectOutputScale.key] >> m_IndirectOutputScale.value;
			n[m_AlbedoBoost.key] >> m_AlbedoBoost.value;
			n[m_EnvironmentLightingMode.key] >> m_EnvironmentLightingMode.value;
			n[m_EnableBakedLightmaps.key] >> m_EnableBakedLightmaps.value;
			n[m_EnableRealtimeLightmaps.key] >> m_EnableRealtimeLightmaps.value;
		}

		void GISettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_BounceScale.print(prefix);
			m_IndirectOutputScale.print(prefix);
			m_AlbedoBoost.print(prefix);
			m_EnvironmentLightingMode.print(prefix);
			m_EnableBakedLightmaps.print(prefix);
			std::cout << "  " << m_EnableRealtimeLightmaps.key << ": " << m_EnableRealtimeLightmaps.value;
		}

		LightmapEditorSettings::LightmapEditorSettings() : 
			Object(0, 0) 
		{}

		void LightmapEditorSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Resolution.key] >> m_Resolution.value;
			n[m_BakeResolution.key] >> m_BakeResolution.value;
			n[m_AtlasSize.key] >> m_AtlasSize.value;
			n[m_AO.key] >> m_AO.value;
			n[m_AOMaxDistance.key] >> m_AOMaxDistance.value;
			n[m_CompAOExponent.key] >> m_CompAOExponent.value;
			n[m_CompAOExponentDirect.key] >> m_CompAOExponentDirect.value;
			n[m_ExtractAmbientOcclusion.key] >> m_ExtractAmbientOcclusion.value;
			n[m_Padding.key] >> m_Padding.value;
			n[m_LightmapParameters.key] >> m_LightmapParameters.value;
			n[m_LightmapsBakeMode.key] >> m_LightmapsBakeMode.value;
			n[m_TextureCompression.key] >> m_TextureCompression.value;
			n[m_FinalGather.key] >> m_FinalGather.value;
			n[m_FinalGatherFiltering.key] >> m_FinalGatherFiltering.value;
			n[m_FinalGatherRayCount.key] >> m_FinalGatherRayCount.value;
			n[m_ReflectionCompression.key] >> m_ReflectionCompression.value;
			n[m_MixedBakeMode.key] >> m_MixedBakeMode.value;
			n[m_BakeBackend.key] >> m_BakeBackend.value;
			n[m_PVRSampling.key] >> m_PVRSampling.value;
			n[m_PVRDirectSampleCount.key] >> m_PVRDirectSampleCount.value;
			n[m_PVRSampleCount.key] >> m_PVRSampleCount.value;
			n[m_PVRBounces.key] >> m_PVRBounces.value;
			n[m_PVREnvironmentSampleCount.key] >> m_PVREnvironmentSampleCount.value;
			n[m_PVREnvironmentReferencePointCount.key] >> m_PVREnvironmentReferencePointCount.value;
			n[m_PVRFilteringMode.key] >> m_PVRFilteringMode.value;
			n[m_PVRDenoiserTypeDirect.key] >> m_PVRDenoiserTypeDirect.value;
			n[m_PVRDenoiserTypeIndirect.key] >> m_PVRDenoiserTypeIndirect.value;
			n[m_PVRDenoiserTypeAO.key] >> m_PVRDenoiserTypeAO.value;
			n[m_PVRFilterTypeDirect.key] >> m_PVRFilterTypeDirect.value;
			n[m_PVRFilterTypeIndirect.key] >> m_PVRFilterTypeIndirect.value;
			n[m_PVRFilterTypeAO.key] >> m_PVRFilterTypeAO.value;
			n[m_PVREnvironmentMIS.key] >> m_PVREnvironmentMIS.value;
			n[m_PVRCulling.key] >> m_PVRCulling.value;
			n[m_PVRFilteringGaussRadiusDirect.key] >> m_PVRFilteringGaussRadiusDirect.value;
			n[m_PVRFilteringGaussRadiusIndirect.key] >> m_PVRFilteringGaussRadiusIndirect.value;
			n[m_PVRFilteringGaussRadiusAO.key] >> m_PVRFilteringGaussRadiusAO.value;
			n[m_PVRFilteringAtrousPositionSigmaDirect.key] >> m_PVRFilteringAtrousPositionSigmaDirect.value;
			n[m_PVRFilteringAtrousPositionSigmaIndirect.key] >> m_PVRFilteringAtrousPositionSigmaIndirect.value;
			n[m_PVRFilteringAtrousPositionSigmaAO.key] >> m_PVRFilteringAtrousPositionSigmaAO.value;
			n[m_ExportTrainingData.key] >> m_ExportTrainingData.value;
			n[m_TrainingDataDestination.key] >> m_TrainingDataDestination.value;
			n[m_LightProbeSampleCountMultiplier.key] >> m_LightProbeSampleCountMultiplier.value;
		}

		void LightmapEditorSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Resolution.print(prefix);
			m_BakeResolution.print(prefix);
			m_AtlasSize.print(prefix);
			m_AO.print(prefix);
			m_AOMaxDistance.print(prefix);
			m_CompAOExponent.print(prefix);
			m_CompAOExponentDirect.print(prefix);
			m_Padding.print(prefix);
			m_LightmapParameters.print(prefix);
			m_LightmapsBakeMode.print(prefix);
			m_TextureCompression.print(prefix);
			m_FinalGather.print(prefix);
			m_FinalGatherFiltering.print(prefix);
			m_FinalGatherRayCount.print(prefix);
			m_ReflectionCompression.print(prefix);
			m_MixedBakeMode.print(prefix);
			m_BakeBackend.print(prefix);
			m_PVRSampling.print(prefix);
			m_PVRDirectSampleCount.print(prefix);
			m_PVRSampleCount.print(prefix);
			m_PVRBounces.print(prefix);
			m_PVREnvironmentSampleCount.print(prefix);
			m_PVREnvironmentReferencePointCount.print(prefix);
			m_PVRFilteringMode.print(prefix);
			m_PVRDenoiserTypeDirect.print(prefix);
			m_PVRDenoiserTypeIndirect.print(prefix);
			m_PVRDenoiserTypeAO.print(prefix);
			m_PVRFilterTypeDirect.print(prefix);
			m_PVRFilterTypeIndirect.print(prefix);
			m_PVRFilterTypeAO.print(prefix);
			m_PVREnvironmentMIS.print(prefix);
			m_PVRCulling.print(prefix);
			m_PVRFilteringGaussRadiusDirect.print(prefix);
			m_PVRFilteringGaussRadiusIndirect.print(prefix);
			m_PVRFilteringGaussRadiusAO.print(prefix);
			m_PVRFilteringAtrousPositionSigmaDirect.print(prefix);
			m_PVRFilteringAtrousPositionSigmaIndirect.print(prefix);
			m_PVRFilteringAtrousPositionSigmaAO.print(prefix);
			m_ExportTrainingData.print(prefix);
			m_TrainingDataDestination.print(prefix);
			std::cout << "  " << m_LightProbeSampleCountMultiplier.key << ": " << m_LightProbeSampleCountMultiplier.value;
		}

		LightmapSettings::LightmapSettings(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void LightmapSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_GIWorkflowMode.key] >> m_GIWorkflowMode.value;
			n[m_GISettings.key] >> m_GISettings.value;
			n[m_LightmapEditorSettings.key] >> m_LightmapEditorSettings.value;
			n[m_LightingDataAsset.key] >> m_LightingDataAsset.value;
			n[m_LightingSettings.key] >> m_LightingSettings.value;
		}

		void LightmapSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_GIWorkflowMode.print(prefix);
			m_GISettings.print(prefix);
			m_LightmapEditorSettings.print(prefix);
			m_LightingDataAsset.print(prefix);
			m_LightingSettings.print(prefix);
		}

		BuildSettings::BuildSettings() :
			Object(0, 0)
		{}
			
		void BuildSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[agentTypeID.key] >> agentTypeID.value;
			n[agentRadius.key] >> agentRadius.value;
			n[agentHeight.key] >> agentHeight.value;
			n[agentSlope.key] >> agentSlope.value;
			n[agentClimb.key] >> agentClimb.value;
			n[ledgeDropHeight.key] >> ledgeDropHeight.value;
			n[maxJumpAcrossDistance.key] >> maxJumpAcrossDistance.value;
			n[minRegionArea.key] >> minRegionArea.value;
			n[manualCellSize.key] >> manualCellSize.value;
			n[cellSize.key] >> cellSize.value;
			n[manualTileSize.key] >> manualTileSize.value;
			n[tileSize.key] >> tileSize.value;
			n[accuratePlacement.key] >> accuratePlacement.value;
			n[maxJobWorkers.key] >> maxJobWorkers.value;
			n[preserveTilesOutsideBounds.key] >> preserveTilesOutsideBounds.value;
		}

		void BuildSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			agentTypeID.print(prefix);
			agentRadius.print(prefix);
			agentHeight.print(prefix);
			agentSlope.print(prefix);
			agentClimb.print(prefix);
			ledgeDropHeight.print(prefix);
			maxJumpAcrossDistance.print(prefix);
			minRegionArea.print(prefix);
			manualCellSize.print(prefix);
			cellSize.print(prefix);
			manualTileSize.print(prefix);
			tileSize.print(prefix);
			accuratePlacement.print(prefix);
			maxJobWorkers.print(prefix);
			std::cout << "  " << preserveTilesOutsideBounds.key << ": " << preserveTilesOutsideBounds.value;
		}

		NavMeshSettings::NavMeshSettings(int64 id, uint32 type) : 
			Object(id, type) 
		{}
			
		void NavMeshSettings::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_BuildSettings.key] >> m_BuildSettings.value;
			n[m_NavMeshData.key] >> m_NavMeshData.value;
		}

		void NavMeshSettings::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_BuildSettings.print(prefix);
			m_NavMeshData.print(prefix);
		}
	
		PrefabInstance::PrefabInstance(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void PrefabInstance::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Modification.key] >> m_Modification.value;
			n[m_SourcePrefab.key] >> m_SourcePrefab.value;
		}

		void PrefabInstance::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Modification.print(prefix);
			m_SourcePrefab.print(prefix);
		}

		Prefab::Prefab(int64 id, uint32 type) :
			Object(id, type)
		{

		}

		void Prefab::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_ParentPrefab.key] >> m_ParentPrefab.value;
			n[m_RootGameObject.key] >> m_RootGameObject.value;
			n[m_IsPrefabParent.key] >> m_IsPrefabParent.value;
		}

		void Prefab::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Modification.print(prefix);
			m_ParentPrefab.print(prefix);
			m_RootGameObject.print(prefix);
			m_IsPrefabParent.print(prefix);
		}

		MeshCollider::MeshCollider(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void MeshCollider::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Material.key] >> m_Material.value;
			n[m_IsTrigger.key] >> m_IsTrigger.value;
			n[m_Convex.key] >> m_Convex.value;
			n[m_CookingOptions.key] >> m_CookingOptions.value;
			n[m_Mesh.key] >> m_Mesh.value;
		}

		void MeshCollider::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Material.print(prefix);
			m_IsTrigger.print(prefix);
			m_Convex.print(prefix);
			m_CookingOptions.print(prefix);
			m_Mesh.print(prefix);
		}

		MonoBehaviour::MonoBehaviour(int64 id, uint32 type) :
			Object(id, type) 
		{}

		void MonoBehaviour::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_EditorHideFlags.key] >> m_EditorHideFlags.value;
			n[m_Script.key] >> m_Script.value;
			if (!n[m_Name.key].val_is_null())
				n[m_Name.key] >> m_Name.value;
			if (!n[m_EditorClassIdentifier.key].val_is_null())
				n[m_EditorClassIdentifier.key] >> m_EditorClassIdentifier.value;

			uint32 startIndex = n.child_pos(n[m_EditorClassIdentifier.key]) + 1;

			for (uint32 idx = startIndex; idx < n.num_children(); idx++)
			{
				// TODO: this only gets key/value pairs but there can be nested values aswell!
				auto& childNode = n.child(idx);
				if (childNode.has_val())
				{
					std::string key, value;
					c4::from_chars(childNode.key(), &key);
					c4::from_chars(childNode.val(), &value);
					scriptValues.insert(std::make_pair(key, value));
				}
			}

			//for (auto&& [k, v] : scriptValues)
			//	std::cout << k << ":" << v << std::endl;
		}

		void MonoBehaviour::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_EditorHideFlags.print(prefix);
			m_Script.print(prefix);
			m_Name.print(prefix);
			m_EditorClassIdentifier.print(prefix);
		}

		BoxCollider::BoxCollider(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void BoxCollider::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Material.key] >> m_Material.value;
			n[m_IsTrigger.key] >> m_IsTrigger.value;
			read(n[m_Size.key], m_Size.value);
			read(n[m_Center.key], m_Center.value);
		}

		void BoxCollider::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Material.print(prefix);
			m_IsTrigger.print(prefix);
			m_Size.print(prefix);
			m_Center.print(prefix);
		}

		Light::Light(int64 id, uint32 type) : 
			Object(id, type) 
		{}
		
		void Light::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Type.key] >> m_Type.value;
			n[m_Shape.key] >> m_Shape.value;
			read(n[m_Color.key], m_Color.value);
			n[m_Intensity.key] >> m_Intensity.value;
			n[m_Range.key] >> m_Range.value;
			n[m_SpotAngle.key] >> m_SpotAngle.value;
			n[m_InnerSpotAngle.key] >> m_InnerSpotAngle.value;
			n[m_CookieSize.key] >> m_CookieSize.value;
			n[m_Shadows.key] >> m_Shadows.value;
			n[m_Cookie.key] >> m_Cookie.value;
			n[m_DrawHalo.key] >> m_DrawHalo.value;
			n[m_Flare.key] >> m_Flare.value;
			n[m_RenderMode.key] >> m_RenderMode.value;
			n[m_CullingMask.key]["m_Bits"] >> m_CullingMask.value;
			n[m_RenderingLayerMask.key] >> m_RenderingLayerMask.value;
			n[m_Lightmapping.key] >> m_Lightmapping.value;
			n[m_LightShadowCasterMode.key] >> m_LightShadowCasterMode.value;
			read(n[m_AreaSize.key], m_AreaSize.value);
			n[m_BounceIntensity.key] >> m_BounceIntensity.value;
			n[m_ColorTemperature.key] >> m_ColorTemperature.value;
			n[m_UseColorTemperature.key] >> m_UseColorTemperature.value;
			read(n[m_BoundingSphereOverride.key], m_BoundingSphereOverride.value);
			n[m_UseBoundingSphereOverride.key] >> m_UseBoundingSphereOverride.value;
			n[m_UseViewFrustumForShadowCasterCull.key] >> m_UseViewFrustumForShadowCasterCull.value;
			n[m_ShadowRadius.key] >> m_ShadowRadius.value;
			n[m_ShadowAngle.key] >> m_ShadowAngle.value;
		}

		void Light::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Type.print(prefix);
			m_Shape.print(prefix);
			m_Color.print(prefix);
			m_Intensity.print(prefix);
			m_Range.print(prefix);
			m_SpotAngle.print(prefix);
			m_InnerSpotAngle.print(prefix);
			m_CookieSize.print(prefix);
			m_Shadows.print(prefix);
			m_Cookie.print(prefix);
			m_DrawHalo.print(prefix);
			m_Flare.print(prefix);
			m_RenderMode.print(prefix);
			m_CullingMask.print(prefix);
			m_RenderingLayerMask.print(prefix);
			m_Lightmapping.print(prefix);
			m_LightShadowCasterMode.print(prefix);
			m_AreaSize.print(prefix);
			m_BounceIntensity.print(prefix);
			m_ColorTemperature.print(prefix);
			m_UseColorTemperature.print(prefix);
			m_BoundingSphereOverride.print(prefix);
			m_UseBoundingSphereOverride.print(prefix);
			m_UseViewFrustumForShadowCasterCull.print(prefix);
			m_ShadowRadius.print(prefix);
			m_ShadowAngle.print(prefix);
		}

		LightProbeGroup::LightProbeGroup(int64 id, uint32 type) :
			Object(id, type) 
		{}

		void LightProbeGroup::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			for (auto& posEntry : n[m_SourcePositions.key])
			{
				glm::vec3 pos;
				read(posEntry, pos);
				m_SourcePositions.value.push_back(pos);
			}
		}

		void LightProbeGroup::print(const std::string& prefix)
		{
			Object::print(prefix);
			std::cout << m_SourcePositions.key << ":" << std::endl;
			for (auto& pos : m_SourcePositions.value)
				std::cout << "- {" << "x: " << pos.x << ", y: " << pos.y << ", z: " << pos.z << "}" << std::endl;
		}

		ReflectionProbe::ReflectionProbe(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void ReflectionProbe::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Type.key] >> m_Type.value;
			n[m_Mode.key] >> m_Mode.value;
			n[m_RefreshMode.key] >> m_RefreshMode.value;
			n[m_TimeSlicingMode.key] >> m_TimeSlicingMode.value;
			n[m_Resolution.key] >> m_Resolution.value;
			n[m_UpdateFrequency.key] >> m_UpdateFrequency.value;
			read(n[m_BoxSize.key], m_BoxSize.value);
			read(n[m_BoxOffset.key], m_BoxOffset.value);
			n[m_NearClip.key] >> m_NearClip.value;
			n[m_FarClip.key] >> m_FarClip.value;
			n[m_ShadowDistance.key] >> m_ShadowDistance.value;
			n[m_ClearFlags.key] >> m_ClearFlags.value;
			read(n[m_BackGroundColor.key], m_BackGroundColor.value);
			n[m_CullingMask.key]["m_Bits"] >> m_CullingMask.value;
			n[m_IntensityMultiplier.key] >> m_IntensityMultiplier.value;
			n[m_BlendDistance.key] >> m_BlendDistance.value;
			n[m_HDR.key] >> m_HDR.value;
			n[m_BoxProjection.key] >> m_BoxProjection.value;
			n[m_RenderDynamicObjects.key] >> m_RenderDynamicObjects.value;
			n[m_UseOcclusionCulling.key] >> m_UseOcclusionCulling.value;
			n[m_Importance.key] >> m_Importance.value;
			n[m_CustomBakedTexture.key] >> m_CustomBakedTexture.value;
		}

		void ReflectionProbe::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Type.print(prefix);
			m_Mode.print(prefix);
			m_RefreshMode.print(prefix);
			m_TimeSlicingMode.print(prefix);
			m_Resolution.print(prefix);
			m_UpdateFrequency.print(prefix);
			m_BoxSize.print(prefix);
			m_BoxOffset.print(prefix);
			m_NearClip.print(prefix);
			m_FarClip.print(prefix);
			m_ShadowDistance.print(prefix);
			m_ClearFlags.print(prefix);
			m_BackGroundColor.print(prefix);
			m_CullingMask.print(prefix);
			m_IntensityMultiplier.print(prefix);
			m_BlendDistance.print(prefix);
			m_HDR.print(prefix);
			m_BoxProjection.print(prefix);
			m_RenderDynamicObjects.print(prefix);
			m_UseOcclusionCulling.print(prefix);
			m_Importance.print(prefix);
			m_CustomBakedTexture.print(prefix);
		}

		CanvasRenderer::CanvasRenderer(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void CanvasRenderer::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_CullTransparentMesh.key] >> m_CullTransparentMesh.value;
		}

		void CanvasRenderer::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_CullTransparentMesh.print(prefix);
		}

		RectTransform::RectTransform(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void RectTransform::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			read(n[m_LocalRotation.key], m_LocalRotation.value);
			read(n[m_LocalPosition.key], m_LocalPosition.value);
			read(n[m_LocalScale.key], m_LocalScale.value);
			n[m_Children.key] >> m_Children.value;
			n[m_Father.key] >> m_Father.value;
			n[m_RootOrder.key] >> m_RootOrder.value;
			read(n[m_LocalEulerAnglesHint.key], m_LocalEulerAnglesHint.value);
			read(n[m_AnchorMin.key], m_AnchorMin.value);
			read(n[m_AnchorMax.key], m_AnchorMax.value);
			read(n[m_AnchoredPosition.key], m_AnchoredPosition.value);
			read(n[m_SizeDelta.key], m_SizeDelta.value);
			read(n[m_Pivot.key], m_Pivot.value);
		}

		void RectTransform::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_LocalRotation.print(prefix);
			m_LocalPosition.print(prefix);
			m_LocalScale.print(prefix);
			std::cout << m_Children.key << std::endl;
			for (auto& child : m_Children.value)
				std::cout << prefix << "- " << child << std::endl;
			m_Father.print(prefix);
			m_RootOrder.print(prefix);
			m_LocalEulerAnglesHint.print(prefix);
			m_AnchorMin.print(prefix);
			m_AnchorMax.print(prefix);
			m_AnchoredPosition.print(prefix);
			m_SizeDelta.print(prefix);
			m_Pivot.print(prefix);
		}

		AudioListener::AudioListener(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void AudioListener::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
		}

		void AudioListener::print(const std::string& prefix)
		{
			Object::print(prefix);
		}
	
		Animator::Animator(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void Animator::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Avatar.key] >> m_Avatar.value;
			n[m_Controller.key] >> m_Controller.value;
			n[m_CullingMode.key] >> m_CullingMode.value;
			n[m_UpdateMode.key] >> m_UpdateMode.value;
			n[m_ApplyRootMotion.key] >> m_ApplyRootMotion.value;
			n[m_LinearVelocityBlending.key] >> m_LinearVelocityBlending.value;
			if (!n[m_WarningMessage.key].val_is_null())
				n[m_WarningMessage.key] >> m_WarningMessage.value;
			n[m_HasTransformHierarchy.key] >> m_HasTransformHierarchy.value;
			n[m_AllowConstantClipSamplingOptimization.key] >> m_AllowConstantClipSamplingOptimization.value;
			if (n.has_child(m_KeepAnimatorControllerStateOnDisable.key))
				n[m_KeepAnimatorControllerStateOnDisable.key] >> m_KeepAnimatorControllerStateOnDisable.value;
		}

		void Animator::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Avatar.print(prefix);
			m_Controller.print(prefix);
			m_CullingMode.print(prefix);
			m_UpdateMode.print(prefix);
			m_ApplyRootMotion.print(prefix);
			m_LinearVelocityBlending.print(prefix);
			m_WarningMessage.print(prefix);
			m_HasTransformHierarchy.print(prefix);
			m_AllowConstantClipSamplingOptimization.print(prefix);
			m_KeepAnimatorControllerStateOnDisable.print(prefix);
		}

		Point::Point() : 
			Object(0, 0) 
		{}

		void Point::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[time.key] >> time.value;
			n[value.key] >> value.value;
			n[inSlope.key] >> inSlope.value;
			n[outSlope.key] >> outSlope.value;
			n[tangentMode.key] >> tangentMode.value;
			n[weightedMode.key] >> weightedMode.value;
			n[inWeight.key] >> inWeight.value;
			n[outWeight.key] >> outWeight.value;
		}

		void Point::print(const std::string& prefix)
		{
			Object::print(prefix);
			time.print(prefix);
			value.print(prefix);
			inSlope.print(prefix);
			outSlope.print(prefix);
			tangentMode.print(prefix);
			weightedMode.print(prefix);
			inWeight.print(prefix);
			outWeight.print(prefix);
		}

		Curve::Curve() : 
			Object(0, 0) 
		{}

		void Curve::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Curve.key] >> m_Curve.value;
			n[m_PreInfinity.key] >> m_PreInfinity.value;
			n[m_PostInfinity.key] >> m_PostInfinity.value;
			n[m_RotationOrder.key] >> m_RotationOrder.value;
		}

		void Curve::print(const std::string& prefix)
		{
			Object::print(prefix);
			std::cout << prefix << m_Curve.key << ":" << std::endl;
			for (auto& p : m_Curve.value)
				std::cout << p << std::endl;
			m_PreInfinity.print(prefix);
			m_PostInfinity.print(prefix);
			m_RotationOrder.print(prefix);
		}

		AudioSource::AudioSource(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void AudioSource::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[OutputAudioMixerGroup.key] >> OutputAudioMixerGroup.value;
			n[m_audioClip.key] >> m_audioClip.value;
			n[m_PlayOnAwake.key] >> m_PlayOnAwake.value;
			n[m_Volume.key] >> m_Volume.value;
			n[m_Pitch.key] >> m_Pitch.value;
			n[Loop.key] >> Loop.value;
			n[Mute.key] >> Mute.value;
			n[Spatialize.key] >> Spatialize.value;
			n[SpatializePostEffects.key] >> SpatializePostEffects.value;
			n[Priority.key] >> Priority.value;
			n[DopplerLevel.key] >> DopplerLevel.value;
			n[MinDistance.key] >> MinDistance.value;
			n[MaxDistance.key] >> MaxDistance.value;
			n[Pan2D.key] >> Pan2D.value;
			n[rolloffMode.key] >> rolloffMode.value;
			n[BypassEffects.key] >> BypassEffects.value;
			n[BypassListenerEffects.key] >> BypassListenerEffects.value;
			n[BypassReverbZones.key] >> BypassReverbZones.value;
			n[rolloffCustomCurve.key] >> rolloffCustomCurve.value;
			n[panLevelCustomCurve.key] >> panLevelCustomCurve.value;
			n[spreadCustomCurve.key] >> spreadCustomCurve.value;
			n[reverbZoneMixCustomCurve.key] >> reverbZoneMixCustomCurve.value;
		}

		void AudioSource::print(const std::string& prefix)
		{
			Object::print(prefix);
			OutputAudioMixerGroup.print(prefix);
			m_audioClip.print(prefix);
			m_PlayOnAwake.print(prefix);
			m_Volume.print(prefix);
			m_Pitch.print(prefix);
			Loop.print(prefix);
			Mute.print(prefix);
			Spatialize.print(prefix);
			SpatializePostEffects.print(prefix);
			Priority.print(prefix);
			DopplerLevel.print(prefix);
			MinDistance.print(prefix);
			MaxDistance.print(prefix);
			Pan2D.print(prefix);
			rolloffMode.print(prefix);
			BypassEffects.print(prefix);
			BypassListenerEffects.print(prefix);
			BypassReverbZones.print(prefix);
			rolloffCustomCurve.print(prefix);
			panLevelCustomCurve.print(prefix);
			spreadCustomCurve.print(prefix);
			reverbZoneMixCustomCurve.print(prefix);
		}

		NormalizedViewPortRect::NormalizedViewPortRect() : 
			Object(0, 0) 
		{}

		void NormalizedViewPortRect::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[x.key] >> x.value;
			n[y.key] >> y.value;
			n[width.key] >> width.value;
			n[height.key] >> height.value;
		}

		void NormalizedViewPortRect::print(const std::string& prefix)
		{
			Object::print(prefix);
			x.print(prefix);
			y.print(prefix);
			width.print(prefix);
			height.print(prefix);
		}

		Camera::Camera(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void Camera::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_ClearFlags.key] >> m_ClearFlags.value;
			read(n[m_BackGroundColor.key], m_BackGroundColor.value);
			n[m_projectionMatrixMode.key] >> m_projectionMatrixMode.value;
			n[m_GateFitMode.key] >> m_GateFitMode.value;
			n[m_FOVAxisMode.key] >> m_FOVAxisMode.value;
			read(n[m_SensorSize.key], m_SensorSize.value);
			read(n[m_LensShift.key], m_LensShift.value);
			n[m_FocalLength.key] >> m_FocalLength.value;
			n[m_NormalizedViewPortRect.key] >> m_NormalizedViewPortRect.value;
			n[near_clip_plane.key] >> near_clip_plane.value;
			n[far_clip_plane.key] >> far_clip_plane.value;
			n[field_of_view.key] >> field_of_view.value;
			n[orthographic.key] >> orthographic.value;
			n[orthographic_size.key] >> orthographic_size.value;
			n[m_Depth.key] >> m_Depth.value;
			n[m_CullingMask.key]["m_Bits"] >> m_CullingMask.value;
			n[m_RenderingPath.key] >> m_RenderingPath.value;
			n[m_TargetTexture.key] >> m_TargetTexture.value;
			n[m_TargetDisplay.key] >> m_TargetDisplay.value;
			n[m_TargetEye.key] >> m_TargetEye.value;
			n[m_HDR.key] >> m_HDR.value;
			n[m_AllowMSAA.key] >> m_AllowMSAA.value;
			n[m_AllowDynamicResolution.key] >> m_AllowDynamicResolution.value;
			n[m_ForceIntoRT.key] >> m_ForceIntoRT.value;
			n[m_OcclusionCulling.key] >> m_OcclusionCulling.value;
			n[m_StereoConvergence.key] >> m_StereoConvergence.value;
			n[m_StereoSeparation.key] >> m_StereoSeparation.value;
		}

		void Camera::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_ClearFlags.print(prefix);
			m_BackGroundColor.print(prefix);
			m_projectionMatrixMode.print(prefix);
			m_GateFitMode.print(prefix);
			m_FOVAxisMode.print(prefix);
			m_SensorSize.print(prefix);
			m_LensShift.print(prefix);
			m_FocalLength.print(prefix);
			m_NormalizedViewPortRect.print(prefix);
			near_clip_plane.print(prefix);
			far_clip_plane.print(prefix);
			field_of_view.print(prefix);
			orthographic.print(prefix);
			orthographic_size.print(prefix);
			m_Depth.print(prefix);
			m_CullingMask.print(prefix);
			m_RenderingPath.print(prefix);
			m_TargetTexture.print(prefix);
			m_TargetDisplay.print(prefix);
			m_TargetEye.print(prefix);
			m_HDR.print(prefix);
			m_AllowMSAA.print(prefix);
			m_AllowDynamicResolution.print(prefix);
			m_ForceIntoRT.print(prefix);
			m_OcclusionCulling.print(prefix);
			m_StereoConvergence.print(prefix);
			m_StereoSeparation.print(prefix);
		}

		Canvas::Canvas(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void Canvas::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_RenderMode.key] >> m_RenderMode.value;
			n[m_Camera.key] >> m_Camera.value;
			n[m_PlaneDistance.key] >> m_PlaneDistance.value;
			n[m_ReceivesEvents.key] >> m_ReceivesEvents.value;
			n[m_OverrideSorting.key] >> m_OverrideSorting.value;
			n[m_OverridePixelPerfect.key] >> m_OverridePixelPerfect.value;
			n[m_SortingBucketNormalizedSize.key] >> m_SortingBucketNormalizedSize.value;
			n[m_AdditionalShaderChannelsFlag.key] >> m_AdditionalShaderChannelsFlag.value;
			n[m_SortingLayerID.key] >> m_SortingLayerID.value;
			n[m_SortingOrder.key] >> m_SortingOrder.value;
			n[m_TargetDisplay.key] >> m_TargetDisplay.value;
		}

		void Canvas::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_RenderMode.print(prefix);
			m_Camera.print(prefix);
			m_PlaneDistance.print(prefix);
			m_PixelPerfect.print(prefix);
			m_ReceivesEvents.print(prefix);
			m_OverrideSorting.print(prefix);
			m_OverridePixelPerfect.print(prefix);
			m_SortingBucketNormalizedSize.print(prefix);
			m_AdditionalShaderChannelsFlag.print(prefix);
			m_SortingLayerID.print(prefix);
			m_SortingOrder.print(prefix);
			m_TargetDisplay.print(prefix);
		}

		CapsuleCollider::CapsuleCollider(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void CapsuleCollider::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Material.key] >> m_Material.value;
			n[m_IsTrigger.key] >> m_IsTrigger.value;
			n[m_Radius.key] >> m_Radius.value;
			n[m_Height.key] >> m_Height.value;
			n[m_Direction.key] >> m_Direction.value;
			read(n[m_Center.key], m_Center.value);
		}

		void CapsuleCollider::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Material.print(prefix);
			m_IsTrigger.print(prefix);
			m_Radius.print(prefix);
			m_Height.print(prefix);
			m_Direction.print(prefix);
			m_Center.print(prefix);
		}

		CharacterController::CharacterController(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void CharacterController::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Material.key] >> m_Material.value;
			n[m_IsTrigger.key] >> m_IsTrigger.value;
			n[m_Height.key] >> m_Height.value;
			n[m_Radius.key] >> m_Radius.value;
			n[m_SlopeLimit.key] >> m_SlopeLimit.value;
			n[m_StepOffset.key] >> m_StepOffset.value;
			n[m_SkinWidth.key] >> m_SkinWidth.value;
			n[m_MinMoveDistance.key] >> m_MinMoveDistance.value;
			read(n[m_Center.key], m_Center.value);
		}

		void CharacterController::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Material.print(prefix);
			m_IsTrigger.print(prefix);
			m_Height.print(prefix);
			m_Radius.print(prefix);
			m_SlopeLimit.print(prefix);
			m_StepOffset.print(prefix);
			m_SkinWidth.print(prefix);
			m_MinMoveDistance.print(prefix);
			m_Center.print(prefix);
		}

		ParticleSystemRenderer::ParticleSystemRenderer(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void ParticleSystemRenderer::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);

			n[m_CastShadows.key] >> m_CastShadows.value;
			n[m_ReceiveShadows.key] >> m_ReceiveShadows.value;
			n[m_DynamicOccludee.key] >> m_DynamicOccludee.value;
			n[m_MotionVectors.key] >> m_MotionVectors.value;
			n[m_LightProbeUsage.key] >> m_LightProbeUsage.value;
			n[m_ReflectionProbeUsage.key] >> m_ReflectionProbeUsage.value;
			n[m_RayTracingMode.key] >> m_RayTracingMode.value;
			n[m_RayTraceProcedural.key] >> m_RayTraceProcedural.value;
			n[m_RenderingLayerMask.key] >> m_RenderingLayerMask.value;
			n[m_RendererPriority.key] >> m_RendererPriority.value;
			n[m_Materials.key] >> m_Materials.value;
			n[m_ProbeAnchor.key] >> m_ProbeAnchor.value;
			n[m_LightProbeVolumeOverride.key] >> m_LightProbeVolumeOverride.value;
			n[m_ScaleInLightmap.key] >> m_ScaleInLightmap.value;
			n[m_ReceiveGI.key] >> m_ReceiveGI.value;
			n[m_PreserveUVs.key] >> m_PreserveUVs.value;
			n[m_IgnoreNormalsForChartDetection.key] >> m_IgnoreNormalsForChartDetection.value;
			n[m_ImportantGI.key] >> m_ImportantGI.value;
			n[m_StitchLightmapSeams.key] >> m_StitchLightmapSeams.value;
			n[m_SelectedEditorRenderState.key] >> m_SelectedEditorRenderState.value;
			n[m_MinimumChartSize.key] >> m_MinimumChartSize.value;
			n[m_AutoUVMaxDistance.key] >> m_AutoUVMaxDistance.value;
			n[m_AutoUVMaxAngle.key] >> m_AutoUVMaxAngle.value;
			n[m_LightmapParameters.key] >> m_LightmapParameters.value;
			n[m_SortingLayerID.key] >> m_SortingLayerID.value;
			n[m_SortingOrder.key] >> m_SortingOrder.value;

			n[m_RenderMode.key] >> m_RenderMode.value;
			n[m_SortMode.key] >> m_SortMode.value;
			n[m_MinParticleSize.key] >> m_MinParticleSize.value;
			n[m_MaxParticleSize.key] >> m_MaxParticleSize.value;
			n[m_CameraVelocityScale.key] >> m_CameraVelocityScale.value;
			n[m_VelocityScale.key] >> m_VelocityScale.value;
			n[m_LengthScale.key] >> m_LengthScale.value;
			n[m_SortingFudge.key] >> m_SortingFudge.value;
			n[m_NormalDirection.key] >> m_NormalDirection.value;
			n[m_ShadowBias.key] >> m_ShadowBias.value;
			n[m_RenderAlignment.key] >> m_RenderAlignment.value;
			read(n[m_Pivot.key], m_Pivot.value);
			read(n[m_Flip.key], m_Flip.value);
			n[m_UseCustomVertexStreams.key] >> m_UseCustomVertexStreams.value;
			n[m_EnableGPUInstancing.key] >> m_EnableGPUInstancing.value;
			n[m_ApplyActiveColorSpace.key] >> m_ApplyActiveColorSpace.value;
			n[m_AllowRoll.key] >> m_AllowRoll.value;
			n[m_FreeformStretching.key] >> m_FreeformStretching.value;
			n[m_RotateWithStretchDirection.key] >> m_RotateWithStretchDirection.value;
			n[m_VertexStreams.key] >> m_VertexStreams.value;
			n[m_Mesh.key] >> m_Mesh.value;
			n[m_Mesh1.key] >> m_Mesh1.value;
			n[m_Mesh2.key] >> m_Mesh2.value;
			n[m_Mesh3.key] >> m_Mesh3.value;
			n[m_MaskInteraction.key] >> m_MaskInteraction.value;
		}

		void ParticleSystemRenderer::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_CastShadows.print(prefix);
			m_ReceiveShadows.print(prefix);
			m_DynamicOccludee.print(prefix);
			m_MotionVectors.print(prefix);
			m_LightProbeUsage.print(prefix);
			m_ReflectionProbeUsage.print(prefix);
			m_RayTracingMode.print(prefix);
			m_RayTraceProcedural.print(prefix);
			m_RenderingLayerMask.print(prefix);
			m_RendererPriority.print(prefix);
			std::cout << m_Materials.key << std::endl;
			for (auto& mat : m_Materials.value)
				std::cout << "- " << mat << std::endl;
			m_ProbeAnchor.print(prefix);
			m_LightProbeVolumeOverride.print(prefix);
			m_ScaleInLightmap.print(prefix);
			m_ReceiveGI.print(prefix);
			m_PreserveUVs.print(prefix);
			m_IgnoreNormalsForChartDetection.print(prefix);
			m_ImportantGI.print(prefix);
			m_StitchLightmapSeams.print(prefix);
			m_SelectedEditorRenderState.print(prefix);
			m_MinimumChartSize.print(prefix);
			m_AutoUVMaxDistance.print(prefix);
			m_AutoUVMaxAngle.print(prefix);
			m_LightmapParameters.print(prefix);
			m_SortingLayerID.print(prefix);
			m_SortingLayer.print(prefix);
			m_SortingOrder.print(prefix);

			m_RenderMode.print(prefix);
			m_SortMode.print(prefix);
			m_MinParticleSize.print(prefix);
			m_MaxParticleSize.print(prefix);
			m_CameraVelocityScale.print(prefix);
			m_VelocityScale.print(prefix);
			m_LengthScale.print(prefix);
			m_SortingFudge.print(prefix);
			m_NormalDirection.print(prefix);
			m_ShadowBias.print(prefix);
			m_RenderAlignment.print(prefix);
			m_Pivot.print(prefix);
			m_Flip.print(prefix);
			m_UseCustomVertexStreams.print(prefix);
			m_EnableGPUInstancing.print(prefix);
			m_ApplyActiveColorSpace.print(prefix);
			m_AllowRoll.print(prefix);
			m_FreeformStretching.print(prefix);
			m_RotateWithStretchDirection.print(prefix);
			m_VertexStreams.print(prefix);
			m_Mesh.print(prefix);
			m_Mesh1.print(prefix);
			m_Mesh2.print(prefix);
			m_Mesh3.print(prefix);
			m_MaskInteraction.print(prefix);
		}

		ParticleSystem::ParticleSystem(int64 id, uint32 type) : 
			Object(id, type) 
		{}

		void ParticleSystem::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[lengthInSec.key] >> lengthInSec.value;
			n[simulationSpeed.key] >> simulationSpeed.value;
			n[stopAction.key] >> stopAction.value;
			n[cullingMode.key] >> cullingMode.value;
			n[ringBufferMode.key] >> ringBufferMode.value;
			read(n[ringBufferLoopRange.key], ringBufferLoopRange.value);
			n[looping.key] >> looping.value;
			n[prewarm.key] >> prewarm.value;
			n[playOnAwake.key] >> playOnAwake.value;
			n[useUnscaledTime.key] >> useUnscaledTime.value;
			n[autoRandomSeed.key] >> autoRandomSeed.value;
			n[useRigidbodyForVelocity.key] >> useRigidbodyForVelocity.value;
			// TODO: startDelay
			n[moveWithTransform.key] >> moveWithTransform.value;
			n[moveWithCustomTransform.key] >> moveWithCustomTransform.value;
			n[scalingMode.key] >> scalingMode.value;
			n[randomSeed.key] >> randomSeed.value;
			// TODO: modules
		}

		void ParticleSystem::print(const std::string& prefix)
		{
			Object::print(prefix);
			lengthInSec.print(prefix);
			simulationSpeed.print(prefix);
			stopAction.print(prefix);
			cullingMode.print(prefix);
			ringBufferMode.print(prefix);
			ringBufferLoopRange.print(prefix);
			looping.print(prefix);
			prewarm.print(prefix);
			playOnAwake.print(prefix);
			useUnscaledTime.print(prefix);
			autoRandomSeed.print(prefix);
			useRigidbodyForVelocity.print(prefix);
			// TODO: startDelay
			moveWithTransform.print(prefix);
			moveWithCustomTransform.print(prefix);
			scalingMode.print(prefix);
			randomSeed.print(prefix);
			// TODO: modules
		}

		Material::Material(int64 id, uint32 type) :
			Object(id, type)
		{
		}

		void Material::fromYAML(ryml::ConstNodeRef n)
		{
			Object::fromBaseYAML(n);
			n[m_Name.key] >> m_Name.value;
			n[m_Shader.key] >> m_Shader.value;
			if (!n[m_ShaderKeywords.key].val_is_null())
				n[m_ShaderKeywords.key] >> m_ShaderKeywords.value;
			n[m_LightmapFlags.key] >> m_LightmapFlags.value;
			n[m_EnableInstancingVariants.key] >> m_EnableInstancingVariants.value;
			n[m_DoubleSidedGI.key] >> m_DoubleSidedGI.value;
			n[m_CustomRenderQueue.key] >> m_CustomRenderQueue.value;
			n[disabledShaderPasses.key] >> disabledShaderPasses.value;
			n["m_SavedProperties"][m_TexEnvs.key] >> m_TexEnvs.value;
			n["m_SavedProperties"][m_Floats.key] >> m_Floats.value;
			n["m_SavedProperties"][m_Colors.key] >> m_Colors.value;
		}

		void Material::print(const std::string& prefix)
		{
			Object::print(prefix);
			m_Name.print(prefix);
			m_Shader.print(prefix);
			m_ShaderKeywords.print(prefix);
			m_LightmapFlags.print(prefix);
			m_EnableInstancingVariants.print(prefix);
			m_DoubleSidedGI.print(prefix);
			m_CustomRenderQueue.print(prefix);
			std::cout << prefix << disabledShaderPasses.key << ":" << std::endl;
			for (auto& dsp : disabledShaderPasses.value)
				std::cout << prefix << "- " << dsp << std::endl;
			std::string subPrefix = prefix + "  ";
			std::cout << subPrefix << m_TexEnvs.key << ":" << std::endl;
			for (auto& tex : m_TexEnvs.value)
				std::cout << subPrefix << "- " << tex;
			std::cout << subPrefix << m_Floats.key << ":" << std::endl;
			for (auto& f : m_Floats.value)
				std::cout << subPrefix << "- " << f;
			std::cout << subPrefix << m_Colors.key << ":" << std::endl;
			for (auto& c : m_Colors.value)
				std::cout << subPrefix << "- " << c;
		}

		bool read(ryml::ConstNodeRef n, Object* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, GISettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, LightmapEditorSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, BuildSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Curve* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Point* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, NormalizedViewPortRect* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}