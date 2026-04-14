#ifndef INCLUDED_SHADOWS
#define INCLUDED_SHADOWS

#pragma once

#include <Core/Renderable.h>
#include <Core/Entity.h>
#include <Core/Light.h>
#include <Core/Scene.h>

namespace pr
{

	struct CSMViews
	{
		glm::mat4 VP[4];
	};

	struct CSMData
	{
		glm::mat4 lightSpaceMatrices[4];
		glm::vec4 cascadePlaneDistance;
		int cascadeCount;
		int padding[3];
	};

	struct OMNIViews
	{
		glm::mat4 VP[6];
		int lightIndex;
		int padding[3];
	};

	struct OMNIData
	{
		glm::vec4 position;
		float range;
		int padding[3];
	};

	class Shadows
	{
	public:
		Shadows();
		~Shadows();

		void init();
		void initPipelines(GPU::DescriptorPool::Ptr descriptorPool);
		void initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool);
		void updateLights(pr::Scene::Ptr);
		void addDesc(GPU::DescriptorSet::Ptr lightDescSet);
		void updateShadowsCSM(uint32 frameIndex, pr::Scene::Ptr scene);
		void updateShadowsOMNI(pr::Scene::Ptr scene);
		void buildCmdShadowsCSM(pr::Scene::Ptr scene);
		void buildCmdShadowsOMNI(pr::Scene::Ptr scene);
		pr::Texture2DArray::Ptr getShadowMap()
		{
			return csmShadowMap;
		}
	private:
		GPU::DescriptorSet::Ptr descriptorSetShadow;
		GPU::DescriptorSet::Ptr descriptorSetOmniViews;
		GPU::DescriptorSet::Ptr descriptorSetOmniLight;
		GPU::DescriptorSet::Ptr camDescriptorSet;
		GPU::DescriptorSet::Ptr animDescriptorSet;
		GPU::DescriptorSet::Ptr morphDescriptorSet;
		GPU::Buffer::Ptr csmViewsUBO;
		GPU::Buffer::Ptr csmDataUBO;
		GPU::Buffer::Ptr omniViewsUBO;
		GPU::Buffer::Ptr omniDataUBO;

		// Shadows CSM
		pr::Texture2DArray::Ptr csmShadowMap;
		GPU::Framebuffer::Ptr csmFBO;
		GPU::CommandBuffer::Ptr csmCmdBuf;
		GPU::GraphicsPipeline::Ptr shadowCSMPipeline;

		// Shadows OMNI
		pr::TextureCubeMapArray::Ptr omniShadowMap;
		std::vector<GPU::Framebuffer::Ptr> omniShadowFBOs;
		std::vector<GPU::ImageView::Ptr> omniShadowViews;
		std::vector<GPU::CommandBuffer::Ptr> omniCmdBufs;
		GPU::GraphicsPipeline::Ptr shadowOMNIPipeline;

		// helper meshes
		pr::Primitive::Ptr unitQuad;
		pr::Primitive::Ptr unitCube;
	};
}

#endif // INCLUDED_SHADOWS