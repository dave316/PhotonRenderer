#ifndef INCLUDED_GUI
#define INCLUDED_GUI

#pragma once

#ifdef WITH_IMGUI

#include <Graphics/Mesh.h>
#include <imgui_impl_win32.h>

namespace pr
{
	class GUI
	{
	public:
		GUI(Window::Ptr window);
		~GUI();

		void prepare(GPU::DescriptorPool::Ptr descriptorPool, int numImages);
		void preparePipeline(GPU::Swapchain::Ptr swapchain);
		bool update();
		void buildCmd(GPU::Swapchain::Ptr swapchain);
		void buildCmdViewport();
		void buildCmd2(GPU::Swapchain::Ptr swapchain);
		void addTexture(uint32 texID, pr::Texture2D::Ptr texture);
		void removeTexture(uint32 texID);
		void render();
		GPU::CommandBuffer::Ptr getCommandBuffer(int index) { return commandBuffers[index]; }

		typedef std::shared_ptr<GUI> Ptr;
		static Ptr create(Window::Ptr window)
		{
			return std::make_shared<GUI>(window);
		}

	private:
		
		//GPU::Buffer::Ptr vertexBuffer;
		//GPU::Buffer::Ptr indexBuffer;
		std::vector<GPU::Buffer::Ptr> vertexBuffers;
		std::vector<GPU::Buffer::Ptr> indexBuffers;
		std::map<uint32, GPU::DescriptorSet::Ptr> texDescriptorSets;
		std::vector<GPU::CommandBuffer::Ptr> commandBuffers;
		GPU::DescriptorPool::Ptr descriptorPool;
		GPU::GraphicsPipeline::Ptr guiPipeline;
		pr::Texture2D::Ptr fontTexture;

		struct PushConstants
		{
			glm::mat4 othoProjection;
		} pushConstants;
		GPU::BackendData backendData;

		float scale = 1.0f;
		uint32 vertexCount = 0;
		uint32 indexCount = 0;
	};
}

#endif

#endif // INCLUDED_GUI