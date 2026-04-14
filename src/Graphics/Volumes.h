#ifndef INCLUDED_VOLUMES
#define INCLUDED_VOLUMES

#pragma once

#include <Core/Renderable.h>
#include <Core/Entity.h>
#include <Core/Light.h>
#include <Core/Scene.h>
#include <Core/FogVolume.h>
namespace pr
{
	struct VolumeData
	{
		FogVolumeUniformData volumes[10];
		int numVolumes;
		int padding[3];
	};

	class Volumes
	{
	public:
		Volumes();
		~Volumes();

		void initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool, GPU::DescriptorSet::Ptr descriptorSetCamera, GPU::DescriptorSet::Ptr descriptorSetLight);
		void initFogVolumes(Scene::Ptr scene);
		void addDesc(GPU::DescriptorSet::Ptr volumeDesc);
		void updateVolumes(Scene::Ptr scene);
		void buildCmdVolumes();

	private:
		GPU::DescriptorPool::Ptr descriptorPool;
		GPU::DescriptorSet::Ptr descriptorSetCamera;
		GPU::DescriptorSet::Ptr descriptorSetLight;
		GPU::DescriptorSet::Ptr descriptorSetVolumePM;
		GPU::DescriptorSet::Ptr descriptorSetVolumeMaterial;
		GPU::DescriptorSet::Ptr descriptorSetVolumeScatter;
		GPU::DescriptorSet::Ptr descriptorSetVolumeAccum;
		GPU::Buffer::Ptr volumeMaterialUBO;
		GPU::Buffer::Ptr volumeScatterUBO;

		// Fog Volumes
		pr::Texture3D::Ptr scatteringExtinction;
		pr::Texture3D::Ptr emissivePhase;
		pr::Texture3D::Ptr inScatteringVolume;
		pr::Texture3D::Ptr acumFogVolume;
		GPU::ComputePipeline::Ptr materialPipeline;
		GPU::ComputePipeline::Ptr scatterPipeline;
		GPU::ComputePipeline::Ptr accumPipeline;
		GPU::CommandBuffer::Ptr volumeCmd;
	};
}

#endif // INCLUDED_VOLUMES