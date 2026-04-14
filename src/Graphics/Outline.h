#ifndef INCLUDED_OUTLINE
#define INCLUDED_OUTLINE

#pragma once

#include <Core/Renderable.h>
#include <Core/Entity.h>
#include <Core/Light.h>
#include <Core/Scene.h>

#include <Utils/IBL.h>

namespace pr
{
	class Outline
	{
	public:
		Outline();
		~Outline();

		void init(uint32 width, uint32 height);
		void initFramebuffers(uint32 width, uint32 height, pr::Texture2D::Ptr screenTex, pr::Texture2D::Ptr depthTex);
		void initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool);
		void initPipelines(GPU::DescriptorPool::Ptr descriptorPool);
		void buildCmdBuffer(GPU::CommandBuffer::Ptr cmdBuf, pr::Entity::Ptr model);
		pr::Texture2D::Ptr maskTex;
	private:
		GPU::GraphicsPipeline::Ptr unlitPipeline;
		GPU::GraphicsPipeline::Ptr unlitPipelineStencil;
		GPU::GraphicsPipeline::Ptr outlinePipeline;
		GPU::Framebuffer::Ptr maskFramebuffer;
		GPU::Framebuffer::Ptr outlineFramebuffer;
		GPU::DescriptorSet::Ptr camDescriptorSet;
		GPU::DescriptorSet::Ptr animDescriptorSet;
		GPU::DescriptorSet::Ptr morphDescriptorSet;
		GPU::DescriptorSet::Ptr outlineDS;

		//pr::Texture2D::Ptr maskTex;
		pr::Texture2D::Ptr maskDepthTex;
		pr::Primitive::Ptr unitQuad;

		uint32 width = 0;
		uint32 height = 0;
	};
}

#endif // INCLUDED_OUTLINE