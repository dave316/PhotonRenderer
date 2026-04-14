#ifndef INCLUDED_SCATTER
#define INCLUDED_SCATTER

#pragma once

#include <Core/Renderable.h>
#include <Core/Entity.h>
#include <Core/Light.h>
#include <Core/Scene.h>

namespace pr
{
	const uint32 ScatterSamplesCount = 55;
	struct ScatterData
	{
		glm::vec4 scatterSamples[ScatterSamplesCount];
		float minRadius = 0.0f;
		float padding[3];
	};

	class Scatter
	{
	public:
		Scatter();
		~Scatter();

		void init(uint32 width, uint32 height);
		void initFramebuffers(uint32 width, uint32 height);
		void initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool);
		void initPipelines(GPU::DescriptorPool::Ptr descriptorPool);
		void buildCmdBuffer(pr::Scene::Ptr scene, std::map<uint32, GPU::DescriptorSet::Ptr> descriptorSets);
		void flush();
		GPU::DescriptorSet::Ptr getDescriptorSet() { return scatterDescriptorSet; }

	private:
		GPU::GraphicsPipeline::Ptr scatterPipeline;
		GPU::Framebuffer::Ptr scatterFramebuffer;
		GPU::CommandBuffer::Ptr scatterCmdBuf;
		GPU::DescriptorSet::Ptr scatterDescriptorSet;
		GPU::Buffer::Ptr scatterUBO;
		
		pr::Texture2D::Ptr scatterFrontTexture;
		pr::Texture2D::Ptr scatterDepthTexture;

		uint32 width = 0;
		uint32 height = 0;
	};
}

#endif // INCLUDED_SCATTER