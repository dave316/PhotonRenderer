#include "Volumes.h"
#include <Graphics/GraphicsContext.h>
#include <Utils/IBL.h>

namespace pr
{
	Volumes::Volumes()
	{

	}

	Volumes::~Volumes()
	{

	}

	void Volumes::initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool, GPU::DescriptorSet::Ptr descriptorSetCamera, GPU::DescriptorSet::Ptr descriptorSetLight)
	{
		this->descriptorPool = descriptorPool;
		this->descriptorSetCamera = descriptorSetCamera;
		this->descriptorSetLight = descriptorSetLight;
	}

	void Volumes::initFogVolumes(Scene::Ptr scene)
	{
		scatteringExtinction = Texture3D::create(160, 90, 128, GPU::Format::RGBA16F, 1, GPU::ImageUsage::Storage | GPU::ImageUsage::Sampled);
		scatteringExtinction->setAddressMode(GPU::AddressMode::ClampToEdge);
		scatteringExtinction->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);

		emissivePhase = Texture3D::create(160, 90, 128, GPU::Format::RGBA16F, 1, GPU::ImageUsage::Storage | GPU::ImageUsage::Sampled);
		emissivePhase->setAddressMode(GPU::AddressMode::ClampToEdge);
		emissivePhase->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);

		inScatteringVolume = Texture3D::create(160, 90, 128, GPU::Format::RGBA16F, 1, GPU::ImageUsage::Storage | GPU::ImageUsage::Sampled);
		inScatteringVolume->setAddressMode(GPU::AddressMode::ClampToEdge);
		inScatteringVolume->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);

		acumFogVolume = Texture3D::create(160, 90, 128, GPU::Format::RGBA16F, 1, GPU::ImageUsage::Storage | GPU::ImageUsage::Sampled);
		acumFogVolume->setAddressMode(GPU::AddressMode::ClampToEdge);
		acumFogVolume->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);

		auto& ctx = GraphicsContext::getInstance();

		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		scatteringExtinction->setLayoutStorage(cmdBuf);
		emissivePhase->setLayoutStorage(cmdBuf);
		inScatteringVolume->setLayoutStorage(cmdBuf);
		acumFogVolume->setLayoutStorage(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();

		struct Volume
		{
			glm::vec4 scattering = glm::vec4(1);
			float absorption = 0.1f;
			float phase = 0.0f;
			float time = 0;
			int padding;
		};
		Volume volume;

		//struct VolumeData
		//{
		//	FogVolumeUniformData volumes[10];
		//	int numVolumes;
		//	int padding[3];
		//};

		volumeMaterialUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(VolumeData), 0);
		std::vector<Texture3D::Ptr> densityTextures;
		std::vector<FogVolumeUniformData> fogVolumeData;
		for (auto root : scene->getRootNodes())
		{
			auto volumes = root->getChildrenWithComponent<FogVolume>();
			if (volumes.size() > 0)
			{
				auto e = volumes[0];
				auto t = e->getComponent<Transform>();
				auto v = e->getComponent<FogVolume>();

				auto tex = v->getDensityTex();
				int texIndex = -1;
				if (tex)
				{
					texIndex = static_cast<int>(densityTextures.size());
					densityTextures.push_back(v->getDensityTex());
				}

				FogVolumeUniformData fogData;
				v->writeUniformData(fogData, t);
				fogData.densityTexIndex = texIndex;
				fogVolumeData.push_back(fogData);
			}
		}

		VolumeData data;
		for (int i = 0; i < fogVolumeData.size(); i++)
			data.volumes[i] = fogVolumeData[i];
		data.numVolumes = static_cast<int>(fogVolumeData.size());
		volumeMaterialUBO->uploadMapped(&data);

		{
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 10, GPU::ShaderStage::Compute));
			bindings[1].variableCount = true;
			descriptorPool->addDescriptorSetLayout("PM", bindings);

			descriptorSetVolumePM = descriptorPool->createDescriptorSet("PM", static_cast<uint32>(densityTextures.size()));
			descriptorSetVolumePM->addDescriptor(volumeMaterialUBO->getDescriptor());
			for (int i = 0; i < densityTextures.size(); i++)
				descriptorSetVolumePM->addDescriptor(densityTextures[i]->getDescriptor());
			descriptorSetVolumePM->updateVariable();
		}

		{
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::StorageImage, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::StorageImage, 1, GPU::ShaderStage::Compute));

			descriptorPool->addDescriptorSetLayout("VolumeMaterial", bindings);

			descriptorSetVolumeMaterial = descriptorPool->createDescriptorSet("VolumeMaterial", 1);
			descriptorSetVolumeMaterial->addDescriptor(scatteringExtinction->getDescriptor());
			descriptorSetVolumeMaterial->addDescriptor(emissivePhase->getDescriptor());
			descriptorSetVolumeMaterial->update();
		}

		{
			volumeScatterUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(Volume), 0);
			volumeScatterUBO->uploadMapped(&volume);

			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(3, GPU::DescriptorType::StorageImage, 1, GPU::ShaderStage::Compute));

			descriptorPool->addDescriptorSetLayout("VolumeScatter", bindings);

			descriptorSetVolumeScatter = descriptorPool->createDescriptorSet("VolumeScatter", 1);
			descriptorSetVolumeScatter->addDescriptor(volumeScatterUBO->getDescriptor());
			descriptorSetVolumeScatter->addDescriptor(scatteringExtinction->getDescriptor());
			descriptorSetVolumeScatter->addDescriptor(emissivePhase->getDescriptor());
			descriptorSetVolumeScatter->addDescriptor(inScatteringVolume->getDescriptor());
			descriptorSetVolumeScatter->update();
		}

		{
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::StorageImage, 1, GPU::ShaderStage::Compute));

			descriptorPool->addDescriptorSetLayout("VolumeAccum", bindings);

			descriptorSetVolumeAccum = descriptorPool->createDescriptorSet("VolumeAccum", 1);
			descriptorSetVolumeAccum->addDescriptor(inScatteringVolume->getDescriptor());
			descriptorSetVolumeAccum->addDescriptor(acumFogVolume->getDescriptor());
			descriptorSetVolumeAccum->update();
		}

		{
			std::vector<std::string> setLayouts = { "Camera", "VolumeMaterial", "PM" };

			materialPipeline = ctx.createComputePipeline("VolumeMaterial");
			materialPipeline->setLayout(descriptorPool, setLayouts);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					materialPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Volume/VolumeMaterial.comp"), GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string csCode;
					loadBinary(shaderPath + "/VolumeMaterial.cs.cso", csCode);
					materialPipeline->addShaderStage(csCode, GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					materialPipeline->addShaderStage(loadTxtFile(shaderPath + "/Volume/VolumeMaterial.comp.spv"), GPU::ShaderStage::Compute);
					break;
				}
			}
			materialPipeline->createProgram();
		}

		{
			std::vector<std::string> setLayouts = { "Camera", "Light", "VolumeScatter" };
			scatterPipeline = ctx.createComputePipeline("VolumeScatter");
			scatterPipeline->setLayout(descriptorPool, setLayouts);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					scatterPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Volume/VolumeScatter.comp"), GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string csCode;
					loadBinary(shaderPath + "/VolumeScatter.cs.cso", csCode);
					scatterPipeline->addShaderStage(csCode, GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					scatterPipeline->addShaderStage(loadTxtFile(shaderPath + "/Volume/VolumeScatter.comp.spv"), GPU::ShaderStage::Compute);
					break;
				}
			}

			scatterPipeline->createProgram();
		}

		{
			std::vector<std::string> setLayouts = { "Camera", "VolumeAccum" };
			accumPipeline = ctx.createComputePipeline("VolumeAccum");
			accumPipeline->setLayout(descriptorPool, setLayouts);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					accumPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Volume/VolumeAccum.comp"), GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string csCode;
					loadBinary(shaderPath + "/VolumeAccum.cs.cso", csCode);
					accumPipeline->addShaderStage(csCode, GPU::ShaderStage::Compute);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					accumPipeline->addShaderStage(loadTxtFile(shaderPath + "/Volume/VolumeAccum.comp.spv"), GPU::ShaderStage::Compute);
					break;
				}
			}

			accumPipeline->createProgram();
		}

		buildCmdVolumes();
	}

	void Volumes::addDesc(GPU::DescriptorSet::Ptr volumeDesc)
	{
		volumeDesc->addDescriptor(scatteringExtinction->getDescriptor());
		volumeDesc->addDescriptor(acumFogVolume->getDescriptor());
	}

	void Volumes::updateVolumes(Scene::Ptr scene)
	{
		std::vector<Texture3D::Ptr> densityTextures;
		std::vector<FogVolumeUniformData> fogVolumeData;
		for (auto root : scene->getRootNodes())
		{
			auto volumes = root->getChildrenWithComponent<FogVolume>();
			if (volumes.size() > 0)
			{
				auto e = volumes[0];
				auto t = e->getComponent<Transform>();
				auto v = e->getComponent<FogVolume>();

				auto tex = v->getDensityTex();
				int texIndex = -1;
				if (tex)
				{
					texIndex = static_cast<int>(densityTextures.size());
					densityTextures.push_back(v->getDensityTex());
				}

				FogVolumeUniformData fogData;
				v->writeUniformData(fogData, t);
				fogData.densityTexIndex = texIndex;
				fogVolumeData.push_back(fogData);
			}
		}

		VolumeData data;
		for (int i = 0; i < fogVolumeData.size(); i++)
			data.volumes[i] = fogVolumeData[i];
		data.numVolumes = static_cast<int>(fogVolumeData.size());
		volumeMaterialUBO->uploadMapped(&data);

		// TODO: add a way to add/remove descriptors from a set since creating new ones everytime is not good...
		descriptorSetVolumePM = descriptorPool->createDescriptorSet("PM", static_cast<uint32>(densityTextures.size()));
		descriptorSetVolumePM->addDescriptor(volumeMaterialUBO->getDescriptor());
		for (int i = 0; i < densityTextures.size(); i++)
			descriptorSetVolumePM->addDescriptor(densityTextures[i]->getDescriptor());
		descriptorSetVolumePM->updateVariable();

		buildCmdVolumes();
		volumeCmd->flush();
	}

	void Volumes::buildCmdVolumes()
	{
		auto& ctx = GraphicsContext::getInstance();
		volumeCmd = ctx.allocateCommandBuffer();
		volumeCmd->begin();
		volumeCmd->bindPipeline(materialPipeline);
		volumeCmd->bindDescriptorSets(materialPipeline, descriptorSetCamera, 0);
		volumeCmd->bindDescriptorSets(materialPipeline, descriptorSetVolumeMaterial, 1);
		volumeCmd->bindDescriptorSets(materialPipeline, descriptorSetVolumePM, 2);
		volumeCmd->dispatchCompute(160, 90, 128);
		volumeCmd->pipelineBarrier();

		volumeCmd->bindPipeline(scatterPipeline);
		volumeCmd->bindDescriptorSets(scatterPipeline, descriptorSetCamera, 0);
		volumeCmd->bindDescriptorSets(scatterPipeline, descriptorSetLight, 1);
		volumeCmd->bindDescriptorSets(scatterPipeline, descriptorSetVolumeScatter, 2);
		volumeCmd->dispatchCompute(160, 90, 128);
		volumeCmd->pipelineBarrier();

		volumeCmd->bindPipeline(accumPipeline);
		volumeCmd->bindDescriptorSets(accumPipeline, descriptorSetCamera, 0);
		volumeCmd->bindDescriptorSets(accumPipeline, descriptorSetVolumeAccum, 1);
		volumeCmd->dispatchCompute(160, 90, 1);
		volumeCmd->pipelineBarrier();
		volumeCmd->end();
	}
}