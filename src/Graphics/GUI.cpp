#ifdef WITH_IMGUI
#include "GUI.h"
#include <Utils/IBL.h>
#include <Platform/Win32/Win32Window.h>
#include <Graphics/GraphicsContext.h>

namespace pr
{
	static void renderWindow(ImGuiViewport* viewport, void*)
	{
		auto& ctx = GraphicsContext::getInstance();

		ImDrawData* imDrawData = viewport->DrawData;
		if (!imDrawData)
			return;

		uint32 vboSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		uint32 iboSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if (vboSize == 0 || iboSize == 0)
			return;

		std::vector<GPU::Buffer::Ptr> vertexBuffers;
		std::vector<GPU::Buffer::Ptr> indexBuffers;
		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			auto vbo = ctx.createBuffer(GPU::BufferUsage::VertexBuffer | GPU::BufferUsage::TransferDst, cmdList->VtxBuffer.Size * sizeof(ImDrawVert), sizeof(ImDrawVert));
			auto ibo = ctx.createBuffer(GPU::BufferUsage::IndexBuffer | GPU::BufferUsage::TransferDst, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx), sizeof(ImDrawIdx));
			vbo->uploadStaged(cmdList->VtxBuffer.Data);
			ibo->uploadStaged(cmdList->IdxBuffer.Data);
			vertexBuffers.push_back(vbo);
			indexBuffers.push_back(ibo);
		}

		auto cmdBuf = ctx.allocateCommandBuffer();

		uint32 width = static_cast<uint32>(imDrawData->DisplaySize.x);
		uint32 height = static_cast<uint32>(imDrawData->DisplaySize.y);

		cmdBuf->begin();
		cmdBuf->setCullMode(0);
		cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		cmdBuf->setScissor(0, 0, width, height);
		//cmdBuf->beginRenderPass(outputFBO);

		uint32 vertexOffset = 0;
		uint32 indexOffset = 0;

		if (!imDrawData || imDrawData->CmdListsCount == 0)
			return;

		ImGuiIO& io = ImGui::GetIO();
		GPU::BackendData* backendData = (GPU::BackendData*)io.BackendRendererUserData;
		cmdBuf->bindPipeline(backendData->guiPipeline);

		ImVec2 clipOff = imDrawData->DisplayPos;
		float L = imDrawData->DisplayPos.x;
		float R = imDrawData->DisplayPos.x + imDrawData->DisplaySize.x;
		float T = imDrawData->DisplayPos.y;
		float B = imDrawData->DisplayPos.y + imDrawData->DisplaySize.y;

		glm::mat4 orhtoProjection = {
			{ 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f, 0.0f },
			{ (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f }
		};

		if (ctx.getCurrentAPI() == GraphicsAPI::Direct3D11)
			backendData->pushConstants.othoProjection = glm::transpose(orhtoProjection);
		else
			backendData->pushConstants.othoProjection = orhtoProjection;
		cmdBuf->pushConstants(backendData->guiPipeline, GPU::ShaderStage::Vertex, 0, sizeof(GPU::BackendData::PushConstants), &backendData->pushConstants);

		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			cmdBuf->bindVertexBuffers(vertexBuffers[i]);
			cmdBuf->bindIndexBuffers(indexBuffers[i], GPU::IndexType::uint16);

			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];
				ImVec2 clipMin(pcmd->ClipRect.x - clipOff.x, pcmd->ClipRect.y - clipOff.y);
				ImVec2 clipMax(pcmd->ClipRect.z - clipOff.x, pcmd->ClipRect.w - clipOff.y);
				if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
					continue;

				int32 x = static_cast<int32>(clipMin.x);
				int32 y = static_cast<int32>(clipMin.y);
				if (ctx.getCurrentAPI() == GraphicsAPI::OpenGL)
					y = static_cast<int32>((float)height - clipMax.y);
				uint32 cw = static_cast<uint32>(clipMax.x - clipMin.x);
				uint32 ch = static_cast<uint32>(clipMax.y - clipMin.y);
				cmdBuf->setScissor(x, y, cw, ch);

				auto texID = static_cast<uint32>(pcmd->GetTexID());
				if (backendData->texDescriptorSets.find(texID) != backendData->texDescriptorSets.end())
					cmdBuf->bindDescriptorSets(backendData->guiPipeline, backendData->texDescriptorSets[texID], 0);
				else
					std::cout << "error: descriporSet for texID: " << texID << " not found!" << std::endl;

				cmdBuf->drawIndexed(pcmd->ElemCount, pcmd->IdxOffset, pcmd->VtxOffset);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmdList->VtxBuffer.Size;
		}

		//cmdBuf->endRenderPass();
		cmdBuf->end();
		cmdBuf->flush();
	}



	GUI::GUI(Window::Ptr window)
	{
		auto win32Window = std::dynamic_pointer_cast<Win32Window>(window);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_InitForOpenGL(win32Window->getWindowHandle());

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.FontGlobalScale = scale;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

		auto& ctx = GraphicsContext::getInstance();
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			if (ctx.getCurrentAPI() == GraphicsAPI::OpenGL)
			{
				ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
				platformIO.Renderer_CreateWindow = GL::Context::createWindow;
				platformIO.Renderer_RenderWindow = renderWindow;
				platformIO.Renderer_DestroyWindow = GL::Context::destroyWindow;
				platformIO.Platform_SwapBuffers = GL::Context::swapBuffers;
				platformIO.Platform_RenderWindow = GL::Context::platformWindow;
			}
			//else if (ctx.getCurrentAPI() == GraphicsAPI::Direct3D11)
			//{
			//	ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
			//	platformIO.Renderer_CreateWindow = DX11::Context::createWindow;
			//	platformIO.Renderer_DestroyWindow = DX11::Context::destroyWindow;
			//	platformIO.Renderer_SetWindowSize = DX11::Context::setWindowSize;
			//	platformIO.Renderer_RenderWindow = renderWindow;
			//	platformIO.Platform_SwapBuffers = DX11::Context::swapBuffers;
			//	platformIO.Platform_RenderWindow = DX11::Context::platformRenderWindow;
			//}
		}

		backendData.context = ctx.getContext();
		io.BackendRendererUserData = &backendData;
	}

	GUI::~GUI()
	{
		//vertexBuffer->unmap();
		//indexBuffer->unmap();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.BackendRendererUserData = NULL;
		
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void GUI::prepare(GPU::DescriptorPool::Ptr descriptorPool, int numImages)
	{
		this->descriptorPool = descriptorPool;

		ImGuiIO& io = ImGui::GetIO();
		std::string assetPath = "../../../../assets";
		std::string fn = assetPath + "/Roboto-Medium.ttf";
		io.Fonts->AddFontFromFileTTF(fn.c_str(), 20.0f * scale);

		uint8* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		uint32 uploadSize = texWidth * texHeight * 4 * sizeof(uint8);
		fontTexture = pr::Texture2D::create(texWidth, texHeight, GPU::Format::RGBA8);
		fontTexture->createData();
		fontTexture->upload(fontData, uploadSize);
		fontTexture->uploadData();

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		descriptorPool->addDescriptorSetLayout("GUI", bindings);

		auto descriptorSetGUI = descriptorPool->createDescriptorSet("GUI", 1);
		descriptorSetGUI->addDescriptor(fontTexture->getDescriptor());
		descriptorSetGUI->update();

		texDescriptorSets.insert(std::make_pair(0, descriptorSetGUI));

		auto& ctx = GraphicsContext::getInstance();
		for (int i = 0; i < numImages; i++)
			commandBuffers.push_back(ctx.allocateCommandBuffer());

		backendData.texDescriptorSets = texDescriptorSets;

		//auto& ctx = GraphicContex::getInstance();
		//vertexBuffer = ctx.createBuffer(GPU::BufferUsage::VertexBuffer | GPU::BufferUsage::TransferDst, 1, sizeof(ImDrawVert));
		//indexBuffer = ctx.createBuffer(GPU::BufferUsage::IndexBuffer | GPU::BufferUsage::TransferDst, 1, sizeof(ImDrawVert));
	}

	void GUI::preparePipeline(GPU::Swapchain::Ptr swapchain)
	{
		std::string shaderPath = "../../../../src/Shaders/GLSL";
		auto& ctx = GraphicsContext::getInstance();
		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(ImDrawVert);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector2F, offsetof(ImDrawVert, pos)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector2F, offsetof(ImDrawVert, uv)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vectur4UC, offsetof(ImDrawVert, col)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[2].name = "COLOR";

			std::vector<GPU::PushConstant> pushConstants;
			pushConstants.push_back(GPU::PushConstant(GPU::ShaderStage::Vertex, 0, sizeof(PushConstants)));

			std::vector<std::string> setLayouts = { "GUI" };
			guiPipeline = ctx.createGraphicsPipeline(swapchain->getFramebuffer(0), "GUI", 1);
			guiPipeline->setCullMode(0);
			guiPipeline->setDepthTest(false, false);
			guiPipeline->setBlending(true);
			guiPipeline->setScissorTest(true);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					guiPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/gui.vert"), GPU::ShaderStage::Vertex);
					guiPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/gui.frag"), GPU::ShaderStage::Fragment);					
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string vsCode, psCode;
					loadBinary(shaderPath + "/GUI.vs.cso", vsCode);
					loadBinary(shaderPath + "/GUI.ps.cso", psCode);
					guiPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					guiPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					guiPipeline->addShaderStage(loadTxtFile(shaderPath + "/Utils/gui.vert.spv"), GPU::ShaderStage::Vertex);
					guiPipeline->addShaderStage(loadTxtFile(shaderPath + "/Utils/gui.frag.spv"), GPU::ShaderStage::Fragment);					break;
				}
			}

			guiPipeline->setVertexInputDescripton(vertexInputDescription);
			guiPipeline->setLayout(descriptorPool, setLayouts, pushConstants);
			guiPipeline->createProgram();
		}

		backendData.guiPipeline = guiPipeline;
	}

	bool GUI::update()
	{
		bool updateCmdBuffers = false;

		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData)
			return false;

		uint32 vboSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		uint32 iboSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		
		if (vboSize == 0 || iboSize == 0)
			return false;

		vertexBuffers.clear();
		indexBuffers.clear();
		auto& ctx = GraphicsContext::getInstance();
		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			auto vbo = ctx.createBuffer(GPU::BufferUsage::VertexBuffer | GPU::BufferUsage::TransferDst, cmdList->VtxBuffer.Size * sizeof(ImDrawVert), sizeof(ImDrawVert));
			auto ibo = ctx.createBuffer(GPU::BufferUsage::IndexBuffer | GPU::BufferUsage::TransferDst, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx), sizeof(ImDrawIdx));
			vbo->uploadStaged(cmdList->VtxBuffer.Data);
			ibo->uploadStaged(cmdList->IdxBuffer.Data);
			vertexBuffers.push_back(vbo);
			indexBuffers.push_back(ibo);
		}

		//auto& ctx = GraphicContex::getInstance();
		//if (vertexCount != imDrawData->TotalVtxCount)
		//{
		//	//std::cout << "resize vbo!" << std::endl;
		//	vertexBuffer->unmap();
		//	vertexBuffer->destroy();
		//	vertexBuffer->init(GPU::BufferUsage::VertexBuffer | GPU::BufferUsage::TransferDst, vboSize, sizeof(ImDrawVert));
		//	vertexCount = imDrawData->TotalVtxCount;
		//	vertexBuffer->map();
		//	updateCmdBuffers = true;
		//}

		//if (indexCount != imDrawData->TotalIdxCount)
		//{
		//	//std::cout << "resize ibo!" << std::endl;
		//	indexBuffer->unmap();
		//	indexBuffer->destroy();
		//	indexBuffer->init(GPU::BufferUsage::IndexBuffer | GPU::BufferUsage::TransferDst, iboSize, sizeof(ImDrawIdx));
		//	indexCount = imDrawData->TotalIdxCount;
		//	indexBuffer->map();
		//	updateCmdBuffers = true;
		//}

		//ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer->getMappedPointer();
		//ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer->getMappedPointer();

		//for (int i = 0; i < imDrawData->CmdListsCount; i++)
		//{
		//	const ImDrawList* cmdList = imDrawData->CmdLists[i];
		//	std::memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
		//	std::memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
		//	vtxDst += cmdList->VtxBuffer.Size;
		//	idxDst += cmdList->IdxBuffer.Size;
		//}

		//vertexBuffer->flush(std::numeric_limits<uint64_t>::max(), 0);
		//indexBuffer->flush(std::numeric_limits<uint64_t>::max(), 0);

		return true;
	}

	void GUI::buildCmd(GPU::Swapchain::Ptr swapchain)
	{
		for (int i = 0; i < commandBuffers.size(); i++)
		{
			auto cmdBuf = commandBuffers[i];
			auto outputFBO = swapchain->getFramebuffer(i);

			ImDrawData* imDrawData = ImGui::GetDrawData();
			uint32 vertexOffset = 0;
			uint32 indexOffset = 0;

			if (!imDrawData || imDrawData->CmdListsCount == 0)
				return;

			uint32 width = outputFBO->getWidth();
			uint32 height = outputFBO->getHeight();

			cmdBuf->begin();
			cmdBuf->setCullMode(0);
			cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
			cmdBuf->setScissor(0, 0, width, height);
			cmdBuf->beginRenderPass(outputFBO);

			ImVec2 clipOff = imDrawData->DisplayPos;
			ImGuiIO& io = ImGui::GetIO();

			cmdBuf->bindPipeline(guiPipeline);

			float L = imDrawData->DisplayPos.x;
			float R = imDrawData->DisplayPos.x + imDrawData->DisplaySize.x;
			float T = imDrawData->DisplayPos.y;
			float B = imDrawData->DisplayPos.y + imDrawData->DisplaySize.y;

			glm::mat4 orhtoProjection = {
				{ 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
				{ 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
				{ 0.0f, 0.0f, -1.0f, 0.0f },
				{ (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f }
			};

			auto& ctx = GraphicsContext::getInstance();
			if (ctx.getCurrentAPI() == GraphicsAPI::Direct3D11)
				pushConstants.othoProjection = glm::transpose(orhtoProjection);
			else
				pushConstants.othoProjection = orhtoProjection;
			cmdBuf->pushConstants(guiPipeline, GPU::ShaderStage::Vertex, 0, sizeof(PushConstants), &pushConstants);
			
			for (int i = 0; i < imDrawData->CmdListsCount; i++)
			{
				cmdBuf->bindVertexBuffers(vertexBuffers[i]);
				cmdBuf->bindIndexBuffers(indexBuffers[i], GPU::IndexType::uint16);

				const ImDrawList* cmdList = imDrawData->CmdLists[i];
				for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];
					ImVec2 clipMin(pcmd->ClipRect.x - clipOff.x, pcmd->ClipRect.y - clipOff.y);
					ImVec2 clipMax(pcmd->ClipRect.z - clipOff.x, pcmd->ClipRect.w - clipOff.y);
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
						continue;

					int32 x = static_cast<int32>(clipMin.x);
					int32 y = static_cast<int32>(clipMin.y);
					if (ctx.getCurrentAPI() == GraphicsAPI::OpenGL)
						y = static_cast<int32>((float)height - clipMax.y);
					uint32 cw = static_cast<uint32>(clipMax.x - clipMin.x);
					uint32 ch = static_cast<uint32>(clipMax.y - clipMin.y);
					cmdBuf->setScissor(x, y, cw, ch);

					auto texID = static_cast<uint32>(pcmd->GetTexID());
					//std::cout << "tex ID: " << texID << std::endl;
					if (texDescriptorSets.find(texID) != texDescriptorSets.end())
						cmdBuf->bindDescriptorSets(guiPipeline, texDescriptorSets[texID], 0);
					else
						std::cout << "error: descriporSet for texID: " << texID << " not found!" << std::endl;

					cmdBuf->drawIndexed(pcmd->ElemCount, pcmd->IdxOffset, pcmd->VtxOffset);
					indexOffset += pcmd->ElemCount;
				}
				vertexOffset += cmdList->VtxBuffer.Size;
			}

			//		cmdBuf->bindVertexBuffers(vertexBuffer);
			//		cmdBuf->bindIndexBuffers(indexBuffer, GPU::IndexType::uint16);
			//		
			//		for (int i = 0; i < imDrawData->CmdListsCount; i++)
			//		{
			//			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			//			for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
			//			{
			//				const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];
			//				int offsetX = std::max((int32)(pcmd->ClipRect.x), 0);
			//#ifdef USE_VULKAN
			//				int offsetY = std::max((int32)(pcmd->ClipRect.y), 0);
			//#else
			//				int offsetY = 900 - (int32)(pcmd->ClipRect.w); // TODO: get window size
			//#endif
			//				uint32 width = (uint32)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			//				uint32 height = (uint32)(pcmd->ClipRect.w - pcmd->ClipRect.y);
			//
			//				cmdBuf->setScissor(offsetX, offsetY, width, height);
			//				cmdBuf->drawIndexed(pcmd->ElemCount, indexOffset, vertexOffset);
			//				indexOffset += pcmd->ElemCount;
			//			}
			//			vertexOffset += cmdList->VtxBuffer.Size;
			//		}

			cmdBuf->endRenderPass();
			cmdBuf->end();
		}
	}

	void GUI::buildCmdViewport()
	{
	}

	void GUI::buildCmd2(GPU::Swapchain::Ptr swapchain)
	{
		for (int i = 0; i < commandBuffers.size(); i++)
		{
			auto cmdBuf = commandBuffers[i];
			auto outputFBO = swapchain->getFramebuffer(i);

			uint32 width = outputFBO->getWidth();
			uint32 height = outputFBO->getHeight();

			cmdBuf->begin();
			cmdBuf->setCullMode(0);
			cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
			cmdBuf->setScissor(0, 0, width, height);
			cmdBuf->beginRenderPass(outputFBO);

			cmdBuf->endRenderPass();
			cmdBuf->end();
		}
	}

	void GUI::addTexture(uint32 texID, pr::Texture2D::Ptr texture)
	{
		auto ds = descriptorPool->createDescriptorSet("GUI", 1);
		ds->addDescriptor(texture->getDescriptor());
		ds->update();

		texDescriptorSets.insert(std::make_pair(texID, ds));
		backendData.texDescriptorSets.insert(std::make_pair(texID, ds));
	}

	void GUI::removeTexture(uint32 texID)
	{
		if (texDescriptorSets.find(texID) != texDescriptorSets.end())
			texDescriptorSets.erase(texID);
		else
			std::cout << "error: GUI descriptor ID " << texID << " does not exist!" << std::endl;
	}

	void GUI::render()
	{
		commandBuffers[0]->flush();
	}
}
#endif