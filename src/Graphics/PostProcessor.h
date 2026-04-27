#ifndef INCLUDED_POSTPROCESSOR
#define INCLUDED_POSTPROCESSOR

#pragma once

#include <Graphics/FPSCamera.h>
#include <Core/Renderable.h>
#include <Core/Entity.h>
#include <Core/Light.h>
#include <iostream>

#include <Graphics/Shadows.h>
#include <Graphics/Volumes.h>

namespace pr
{
	struct Post
	{
		int toneMappingMode = 7;
		float manualExposure = 0.0f;
		int applyBloom = 1;
		float bloomIntensity = 1.0f;
		glm::vec3 bloomTint = glm::vec3(1);
		float padding;
	};

	class PostProcessor
	{
	public:
		PostProcessor();
		~PostProcessor();

		void init(uint32 width, uint32 height, GPU::DescriptorPool::Ptr descriptorPool, GPU::Framebuffer::Ptr outFBO);
		void initFramebuffers();
		void initPipelines(GPU::Framebuffer::Ptr finalFBO);
		void initDescriptorLayouts(GPU::DescriptorPool::Ptr descriptorPool);
		void initDescriptorSets(pr::Texture2D::Ptr screenTex, pr::Texture2D::Ptr brightTex);
		void buildCmdForward(GPU::CommandBuffer::Ptr cmdBuf, GPU::Framebuffer::Ptr outFBO);
		void buildCmdPost(GPU::CommandBuffer::Ptr cmdBuf, GPU::Framebuffer::Ptr outFBO);
		void updatePost(Post& post);

		struct UpSampleParams
		{
			float filterRadius;
			int mipLevel;
			int padding[2];
		};

		struct DownSampleParams
		{
			int width;
			int height;
			int mipLevel;
			int padding;
		};

	private:
		GPU::GraphicsPipeline::Ptr postProcessPipeline;
		GPU::GraphicsPipeline::Ptr upSamplePipeline;
		GPU::GraphicsPipeline::Ptr downSamplePipeline;
		GPU::DescriptorPool::Ptr descriptorPool;
		GPU::DescriptorSet::Ptr descriptorSetPostProcess;
		GPU::DescriptorSet::Ptr bloomDescriptorSet;
		GPU::DescriptorSet::Ptr upSampleDescriptorSets[4];
		GPU::DescriptorSet::Ptr downSampleDescriptorSets[4];
		GPU::ImageView::Ptr grabView;
		std::vector<GPU::ImageView::Ptr> upSampleViews;
		std::vector<GPU::ImageView::Ptr> downSampleViews;

		GPU::Buffer::Ptr postUBO;

		struct Bloom
		{
			std::vector<GPU::ImageView::Ptr> imageViews;
			std::vector<GPU::Framebuffer::Ptr> framebuffers;
		} bloom;

		pr::Texture2D::Ptr bloomBlurTex;

		// helper meshes
		pr::Primitive::Ptr unitQuad;

		uint32 width;
		uint32 height;
	};
}

#endif // INCLUDED_POSTPROCESSOR