#include "Renderer.h"
#include <fstream>
#include <IO/FileIO.h>
#include <IO/ImageLoader.h>
#include <Utils/IBL.h>

namespace pr
{
	void Renderer::init(Window::Ptr window, GPU::Swapchain::Ptr swapchain)
	{
  		auto& context = GraphicsContext::getInstance();

		descriptorPool = context.createDescriptorPool();

		// TODO: resize framebuffers when window size changes
		width = window->getWidth();
		height = window->getHeight();
		
		initFramebuffers();
		initDescriptorLayouts();
		initPipelines();

		if (swapchain)
			postProcessor.init(width, height, descriptorPool, swapchain->getFramebuffer(0));
		else
			postProcessor.init(width, height, descriptorPool, finalFramebuffer);

		for (int i = 0; i < 2; i++)
			commandBuffers.push_back(context.allocateCommandBuffer());
	}

	void Renderer::initFramebuffers()
	{
		auto& ctx = GraphicsContext::getInstance();
		{
			screenTex = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
			screenTex->setAddressMode(GPU::AddressMode::ClampToEdge);
			screenTex->createData();
			screenTex->uploadData();

			uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1;
			grabTex = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, levels, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled | GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst);
			grabTex->setAddressMode(GPU::AddressMode::ClampToEdge);
			grabTex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
			grabTex->createData();
			grabTex->uploadData();

			brightTex = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
			brightTex->setAddressMode(GPU::AddressMode::ClampToEdge);
			brightTex->createData();
			brightTex->uploadData();

			depthTex = pr::Texture2D::create(width, height, GPU::Format::D24_S8, 1, GPU::ImageUsage::DepthStencilAttachment);
			depthTex->createData();

			finalTex = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
			finalTex->createData();

			auto cmdBuf = ctx.allocateCommandBuffer();
			cmdBuf->begin();
			screenTex->setLayoutShader(cmdBuf);
			grabTex->setLayoutShader(cmdBuf);
			brightTex->setLayoutShader(cmdBuf);
			finalTex->setLayoutShader(cmdBuf);
			cmdBuf->end();
			cmdBuf->flush();

			grabView = grabTex->getImage()->createImageView(GPU::ViewType::View2D, GPU::SubResourceRange(0, 0, 1, 1));
			
			offscreenFramebuffer = ctx.createFramebuffer(width, height, 1, true, true);
			offscreenFramebuffer->addAttachment(screenTex->getImageView());
			offscreenFramebuffer->addAttachment(grabView);
			offscreenFramebuffer->addAttachment(brightTex->getImageView());
			offscreenFramebuffer->addAttachment(depthTex->getImageView());
			offscreenFramebuffer->createFramebuffer();
		}
		{
			offscreenFramebuffer2 = ctx.createFramebuffer(width, height, 1, true, false);
			offscreenFramebuffer2->addAttachment(screenTex->getImageView());
			offscreenFramebuffer2->addAttachment(depthTex->getImageView());
			offscreenFramebuffer2->createFramebuffer();
		}

		{
			finalFramebuffer = ctx.createFramebuffer(width, height, 1, true, true);
			finalFramebuffer->addAttachment(finalTex->getImageView());
			finalFramebuffer->createFramebuffer();
		}

		outline.initFramebuffers(width, height, screenTex, depthTex);
		scatter.initFramebuffers(width, height);
	}

	void Renderer::initDescriptorLayouts()
	{
		{ // camera descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			descriptorPool->addDescriptorSetLayout("Camera", bindings);
		}

		{ // model descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Model", bindings);
		}

		{ // animation descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex));
			descriptorPool->addDescriptorSetLayout("Animation", bindings);
		}

		{ // morph descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Vertex));
			descriptorPool->addDescriptorSetLayout("Morph", bindings);
		}

		{ // material descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 12, GPU::ShaderStage::Fragment));
			bindings[1].variableCount = true;
			descriptorPool->addDescriptorSetLayout("Material", bindings);
		}

		{ // material descriptor set (shadows)
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings[1].variableCount = true;
			descriptorPool->addDescriptorSetLayout("MaterialShadow", bindings);
		}

		{ // skybox descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Skybox", bindings);
		}

		{ // IBL descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(3, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(4, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(5, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("IBL", bindings);
		}

		{ // light descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(3, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(4, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(5, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment | GPU::ShaderStage::Compute));
			descriptorPool->addDescriptorSetLayout("Light", bindings);
		}

		{ // volume descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Volume", bindings);
		}

		{ // final descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Final", bindings);
		}

		{ // outline descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Outline", bindings);
		}

		{ // scatter descriptor set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("Scatter", bindings);
		}
	}

	void Renderer::initDescriptorSets()
	{
		descriptorSetCamera = descriptorPool->createDescriptorSet("Camera", 1);
		descriptorSetCamera->addDescriptor(cameraUBO->getDescriptor());
		descriptorSetCamera->update();

		// skybox set
		descriptorSetSkybox = descriptorPool->createDescriptorSet("Skybox", 1);
		descriptorSetSkybox->addDescriptor(cameraUBO->getDescriptor());
		descriptorSetSkybox->addDescriptor(skyboxUBO->getDescriptor());
		descriptorSetSkybox->addDescriptor(skybox->getDescriptor());
		descriptorSetSkybox->update();

		// IBL set
		descriptorSetIBL = descriptorPool->createDescriptorSet("IBL", 1);
		descriptorSetIBL->addDescriptor(reflectionProbeUBO->getDescriptor());
		descriptorSetIBL->addDescriptor(irradianceMap->getDescriptor());
		descriptorSetIBL->addDescriptor(reflectionMaps->getDescriptor());
		descriptorSetIBL->addDescriptor(prefilteredMapCharlie->getDescriptor());
		descriptorSetIBL->addDescriptor(brdfLUT->getDescriptor());
		descriptorSetIBL->addDescriptor(grabTex->getDescriptor());
		descriptorSetIBL->update();

		animDescriptorSet = descriptorPool->createDescriptorSet("Animation", 1);
		animDescriptorSet->update();

		morphDescriptorSet = descriptorPool->createDescriptorSet("Morph", 1);
		morphDescriptorSet->update();
	}

	void Renderer::initPipelines()
	{
		auto& ctx = GraphicsContext::getInstance();

		std::string shaderPath = "";
		std::vector<std::string> filenames;
		switch (ctx.getCurrentAPI())
		{
			case GraphicsAPI::Direct3D11:
			{
				shaderPath = "../../../../cache/shaders/cso";
				filenames = IO::getAllFileNames(shaderPath, ".cso");
				break;
			}
			case GraphicsAPI::OpenGL:
			{
				shaderPath = "../../../../src/Shaders/GLSL/Generated";
				filenames = IO::getAllFileNames(shaderPath, ".vert");
				break;
			}
			case GraphicsAPI::Vulkan:
			{
				shaderPath = "../../../../src/Shaders/GLSL/Generated";
				filenames = IO::getAllFileNames(shaderPath, ".vert");
				shaderPath = "../../../../cache/shaders/spv";
				break;
			}
		}

		for (auto fn : filenames)
		{
			int index = static_cast<int>(fn.find_first_of('.'));
			std::string shaderName = fn.substr(0, index);
			if (shaderName.substr(0, 6).compare("Volume") == 0)
				continue;

			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(5, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, tangent)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;
			vertexInputDescription.inputAttributes[5].name = "TANGENT";
			vertexInputDescription.inputAttributes[6].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[7].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material", "IBL", "Light", "Volume", "Scatter" };
			GPU::GraphicsPipeline::Ptr pipeline;

			// TODO: add alpha blending to opaque pass for now
			if (shaderName.find("Transmission") != std::string::npos)
			{
				pipeline = ctx.createGraphicsPipeline(offscreenFramebuffer2, shaderName, 1);
				pipeline->setBlending(true);
			}
			else
			{
				pipeline = ctx.createGraphicsPipeline(offscreenFramebuffer, shaderName, 3);
			}

			GraphicsAPI api = ctx.getCurrentAPI();
			switch (api)
			{
				case GraphicsAPI::OpenGL:
				{
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/" + shaderName + ".vert"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/" + shaderName + ".frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Direct3D11:
				{
					std::string vsCode, psCode;
					loadBinary(shaderPath + "/" + shaderName + ".vs.cso", vsCode);
					loadBinary(shaderPath + "/" + shaderName + ".ps.cso", psCode);
					pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Vulkan:
				{
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/" + shaderName + ".vert.spv"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/" + shaderName + ".frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			pipeline->setVertexInputDescripton(vertexInputDescription);
			pipeline->setLayout(descriptorPool, setLayouts);
			pipeline->createProgram();

			pipelines.insert(std::make_pair(shaderName, pipeline));
		}

		{
			std::string shaderName = "UnityDefaultTransparency";
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(5, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, tangent)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;
			vertexInputDescription.inputAttributes[5].name = "TANGENT";
			vertexInputDescription.inputAttributes[6].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[7].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material", "IBL", "Light", "Volume", "Scatter" };
			GPU::GraphicsPipeline::Ptr pipeline = ctx.createGraphicsPipeline(offscreenFramebuffer2, shaderName, 1);
			pipeline->setBlending(true);

			GraphicsAPI api = ctx.getCurrentAPI();
			switch (api)
			{
				case GraphicsAPI::OpenGL:
				{
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/UnityDefault.vert"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/UnityDefault.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string vsCode, psCode;
					loadBinary(shaderPath + "/UnityDefault.vs.cso", vsCode);
					loadBinary(shaderPath + "/UnityDefault.ps.cso", psCode);
					pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Vulkan:
				{
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/UnityDefault.vert.spv"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/UnityDefault.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			pipeline->setVertexInputDescripton(vertexInputDescription);
			pipeline->setLayout(descriptorPool, setLayouts);
			pipeline->createProgram();

			pipelines.insert(std::make_pair(shaderName, pipeline));
		}

		{
			std::string shaderName = "UnitySpecGlossTransparency";
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(5, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, tangent)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;
			vertexInputDescription.inputAttributes[5].name = "TANGENT";
			vertexInputDescription.inputAttributes[6].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[7].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material", "IBL", "Light", "Volume", "Scatter" };
			GPU::GraphicsPipeline::Ptr pipeline = ctx.createGraphicsPipeline(offscreenFramebuffer2, shaderName, 1);
			pipeline->setBlending(true);

			GraphicsAPI api = ctx.getCurrentAPI();
			switch (api)
			{
				case GraphicsAPI::OpenGL:
				{
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/UnitySpecGloss.vert"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/UnitySpecGloss.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string vsCode, psCode;
					loadBinary(shaderPath + "/UnitySpecGloss.vs.cso", vsCode);
					loadBinary(shaderPath + "/UnitySpecGloss.ps.cso", psCode);
					pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Vulkan:
				{
					std::cout << "compiling shader " << shaderName << std::endl;
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/UnitySpecGloss.vert.spv"), GPU::ShaderStage::Vertex);
					pipeline->addShaderStage(loadTxtFile(shaderPath + "/UnitySpecGloss.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			pipeline->setVertexInputDescripton(vertexInputDescription);
			pipeline->setLayout(descriptorPool, setLayouts);
			pipeline->createProgram();

			pipelines.insert(std::make_pair(shaderName, pipeline));
		}	

		{
			std::string shaderName = "Skybox";

			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes[0].name = "POSITION";

			std::vector<std::string> setLayouts = { "Skybox" };
			skyboxPipeline = ctx.createGraphicsPipeline(offscreenFramebuffer, "Skybox", 3);
			skyboxPipeline->setCullMode(1);

			GraphicsAPI api = ctx.getCurrentAPI();
			switch (api)
			{
				case GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					skyboxPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Skybox.vert"), GPU::ShaderStage::Vertex);
					skyboxPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Skybox.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Direct3D11:
				{
					std::string shaderPath = "../../../../cache/shaders/cso";
					std::string vsCode, psCode;
					loadBinary(shaderPath + "/Skybox.vs.cso", vsCode);
					loadBinary(shaderPath + "/Skybox.ps.cso", psCode);
					skyboxPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					skyboxPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../cache/shaders/spv";
					skyboxPipeline->addShaderStage(loadTxtFile(shaderPath + "/Skybox.vert.spv"), GPU::ShaderStage::Vertex);
					skyboxPipeline->addShaderStage(loadTxtFile(shaderPath + "/Skybox.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			skyboxPipeline->setVertexInputDescripton(vertexInputDescription);
			skyboxPipeline->setLayout(descriptorPool, setLayouts);
			skyboxPipeline->createProgram();
		}

		outline.initPipelines(descriptorPool);
		scatter.initPipelines(descriptorPool);

	}

	void Renderer::initScene(UserCamera& userCamera, Scene::Ptr scene)
	{
		std::vector<LightUniformData> lightData;
		for (auto root : scene->getRootNodes())
		{
			for (auto e : root->getChildrenWithComponent<pr::Light>())
			{
				auto t = e->getComponent<pr::Transform>();
				auto l = e->getComponent<pr::Light>();

				LightUniformData data;
				l->writeUniformData(data, t);
				lightData.push_back(data);

				l->updateLightViewProjection(userCamera, t);
			}
		}

		Lights lights;
		for (int i = 0; i < lightData.size(); i++)
			lights.lightData[i] = lightData[i];
		lights.numLights = (int)lightData.size();
		lightUBO->uploadMapped(&lights);

		scene->initDescriptors(descriptorPool);

		//shadows.initDescriptorSets(descriptorPool);
		//shadows.prepare(userCamera, scene);
		//shadows.updateShadowsCSM(0, scene);
		//shadows.updateShadowsOMNI(scene);

		//// light set
		////for (int i = 0; i < 3; i++)
		//{
		//	descriptorSetLight = descriptorPool->createDescriptorSet("Light", 1);
		//	descriptorSetLight->addDescriptor(lightUBO->getDescriptor());
		//	shadows.addDesc(descriptorSetLight);
		//	scene->addLightDesc(descriptorSetLight);
		//	descriptorSetLight->update();
		//}
	}

	void Renderer::resize(uint32 width, uint32 height)
	{

	}

	void Renderer::prepare(UserCamera& userCamera, pr::Scene::Ptr scene)
	{
		auto& ctx = GraphicsContext::getInstance();

		std::vector<LightUniformData> lightData;
		for (auto root : scene->getRootNodes())
		{
			for (auto e : root->getChildrenWithComponent<pr::Light>())
			{
				auto t = e->getComponent<pr::Transform>();
				auto l = e->getComponent<pr::Light>();

				LightUniformData data;
				l->writeUniformData(data, t);
				lightData.push_back(data);

				l->updateLightViewProjection(userCamera, t);
			}
		}

		Lights lights;
		for (int i = 0; i < lightData.size(); i++)
			lights.lightData[i] = lightData[i];
		lights.numLights = (int)lightData.size();
		lightUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(Lights), 0);
		lightUBO->uploadMapped(&lights);

		skyboxUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(Skybox), 0);
		skyboxData.index = 0;
		skyboxData.lod = 0;
		skyboxUBO->uploadMapped(&skyboxData);

		cameraUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CameraData), 0);
		cameraUBO->uploadMapped((uint8*)&camera);

		unitCube = createCube(glm::vec3(0), 1.0f);
		unitQuad = createScreenQuad();

		ReflectionProbes rp;
		std::vector<pr::TextureCubeMap::Ptr> lightProbes;
		scene->initLightProbes(rp, lightProbes);

		skybox = scene->getSkybox();

		reflectionProbeUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(ReflectionProbes), 0);
		reflectionProbeUBO->uploadMapped(&rp);

		brdfLUT = IBL::generateBRDFLUT(512);
		irradianceMap = IBL::generateIrradianceMap(skybox, 32);
		reflectionMaps = IBL::generatePrefilteredMaps(lightProbes, 256, 8);
		prefilteredMapCharlie = IBL::generatePrefilteredMapCharlie(skybox, 256, 4);

		initDescriptorSets();
		postProcessor.initDescriptorSets(screenTex, brightTex);
		scene->initDescriptors(descriptorPool);
		scene->update(0.0f);
		//scene->computeSHLightprobes();
		scene->computeProbeMapping();

		shadows.init();
		shadows.initDescriptorSets(descriptorPool);
		shadows.initPipelines(descriptorPool);
		shadows.updateLights(scene);
		shadows.buildCmdShadowsOMNI(scene);
		shadows.buildCmdShadowsCSM(scene);
		shadows.updateShadowsCSM(0, scene);
		shadows.updateShadowsOMNI(scene);

		// light set
		descriptorSetLight = descriptorPool->createDescriptorSet("Light", 1);
		descriptorSetLight->addDescriptor(lightUBO->getDescriptor());
		shadows.addDesc(descriptorSetLight);
		scene->addLightDesc(descriptorSetLight);
		descriptorSetLight->update();

		volumes.initDescriptorSets(descriptorPool, descriptorSetCamera, descriptorSetLight);
		volumes.initFogVolumes(scene);

		descriptorSetVolume = descriptorPool->createDescriptorSet("Volume", 1);
		volumes.addDesc(descriptorSetVolume);
		descriptorSetVolume->update();

		outline.init(width, height);
		outline.initDescriptorSets(descriptorPool);

		scatter.init(width, height);
		scatter.initDescriptorSets(descriptorPool);
	}

	void Renderer::buildCmdBuffer(pr::Scene::Ptr scene, GPU::Swapchain::Ptr swapchain)
	{
		auto opaqueNodes = scene->getOpaqueEntities();
		auto transparentNodes = scene->getTransparentEntities();

		for (int i = 0; i < commandBuffers.size(); i++)
		{
			auto cmdBuf = commandBuffers[i];

			cmdBuf->begin();
			cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
			cmdBuf->setScissor(0, 0, width, height);
			cmdBuf->beginRenderPass(offscreenFramebuffer);

			// opaque forward pass
			cmdBuf->setCullMode(2);
			for (auto&& [shaderName, renderQueue] : opaqueNodes)
			{
				GPU::GraphicsPipeline::Ptr pipeline = nullptr;
				if (pipelines.find(shaderName) != pipelines.end())
					pipeline = pipelines[shaderName];
				else
				{
					std::cout << "could not find pipeline with name " << shaderName << std::endl;
					pipeline = pipelines["Default"];
				}

				cmdBuf->bindPipeline(pipeline);
				cmdBuf->bindDescriptorSets(pipeline, descriptorSetCamera, 0);
				cmdBuf->bindDescriptorSets(pipeline, animDescriptorSet, 2);
				cmdBuf->bindDescriptorSets(pipeline, morphDescriptorSet, 3);
				cmdBuf->bindDescriptorSets(pipeline, descriptorSetIBL, 5);
				cmdBuf->bindDescriptorSets(pipeline, descriptorSetLight, 6);
				cmdBuf->bindDescriptorSets(pipeline, descriptorSetVolume, 7);
				cmdBuf->bindDescriptorSets(pipeline, scatter.getDescriptorSet(), 8);

				for (auto e : renderQueue)
				{
					if (e->isActive())
					{
						auto r = e->getComponent<Renderable>();
						r->render(cmdBuf, pipeline);
					}
				}
			}

			// skybox
			cmdBuf->setCullMode(1);
			cmdBuf->bindPipeline(skyboxPipeline);
			cmdBuf->bindDescriptorSets(skyboxPipeline, descriptorSetSkybox, 0);
			unitCube->draw(cmdBuf);

			cmdBuf->endRenderPass();

			// transparent forward pass
			if (!transparentNodes.empty())
			{
				if (GraphicsContext::getInstance().getCurrentAPI() == GraphicsAPI::Direct3D11)
					grabTex->generateMipmaps();
				else
					grabTex->getImage()->generateMipmaps(cmdBuf);

				cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
				cmdBuf->setScissor(0, 0, width, height);
				cmdBuf->beginRenderPass(offscreenFramebuffer2);
				cmdBuf->setCullMode(2);

				for (auto it = transparentNodes.begin(); it != transparentNodes.end(); ++it)
				{
					auto shaderName = it->first;
					auto renderQueue = it->second;
					GPU::GraphicsPipeline::Ptr pipeline = nullptr;
					if (pipelines.find(shaderName) != pipelines.end())
						pipeline = pipelines[shaderName];
					else
					{
						std::cout << "could not find pipeline with name " << shaderName << std::endl;
						pipeline = pipelines["Default"];
					}						

					cmdBuf->bindPipeline(pipeline);
					cmdBuf->bindDescriptorSets(pipeline, descriptorSetCamera, 0);
					cmdBuf->bindDescriptorSets(pipeline, animDescriptorSet, 2);
					cmdBuf->bindDescriptorSets(pipeline, morphDescriptorSet, 3);
					cmdBuf->bindDescriptorSets(pipeline, descriptorSetIBL, 5);
					cmdBuf->bindDescriptorSets(pipeline, descriptorSetLight, 6);
					cmdBuf->bindDescriptorSets(pipeline, descriptorSetVolume, 7);
					cmdBuf->bindDescriptorSets(pipeline, scatter.getDescriptorSet(), 8);

					for (auto e : renderQueue)
					{
						if (e->isActive())
						{
							auto r = e->getComponent<Renderable>();
							r->render(cmdBuf, pipeline);
						}
					}
				}

				cmdBuf->endRenderPass();
			}

			auto currentModel = scene->getCurrentModel();
			if (currentModel)
			{
				auto parent = currentModel->getParent();
				bool activeSubTree = true;
				while (parent != nullptr)
				{
					if (!parent->isActive())
					{
						activeSubTree = false;
						break;
					}						
					parent = parent->getParent();
				}					

				if(currentModel->isActive() && activeSubTree)
					outline.buildCmdBuffer(cmdBuf, currentModel);
			}				

			if (swapchain)
				postProcessor.buildCmdForward(cmdBuf, swapchain->getFramebuffer(i));
			else
				postProcessor.buildCmdForward(cmdBuf, finalFramebuffer);

			cmdBuf->end();
		}
	}

	void Renderer::buildScatterCmdBuffer(pr::Scene::Ptr scene)
	{
		std::map<uint32, GPU::DescriptorSet::Ptr> descriptorSets;
		descriptorSets[0] = descriptorSetCamera;
		descriptorSets[2] = animDescriptorSet;
		descriptorSets[3] = morphDescriptorSet;
		descriptorSets[5] = descriptorSetIBL;
		descriptorSets[6] = descriptorSetLight;
		scatter.buildCmdBuffer(scene, descriptorSets);
	}

	void Renderer::buildShadowCmdBuffer(pr::Scene::Ptr scene)
	{
		shadows.buildCmdShadowsCSM(scene);
		shadows.buildCmdShadowsOMNI(scene);
	}

	void Renderer::addLights(pr::Scene::Ptr scene)
	{
		shadows.updateLights(scene);
		descriptorSetLight = descriptorPool->createDescriptorSet("Light", 1);
		descriptorSetLight->addDescriptor(lightUBO->getDescriptor());
		shadows.addDesc(descriptorSetLight);
		scene->addLightDesc(descriptorSetLight);
		descriptorSetLight->update();

		volumes.initDescriptorSets(descriptorPool, descriptorSetCamera, descriptorSetLight);
		volumes.buildCmdVolumes();
	}

	void Renderer::updateLights(UserCamera& userCamera, pr::Scene::Ptr scene)
	{
		std::vector<LightUniformData> lightData;
		for (auto root : scene->getRootNodes())
		{
			for (auto e : root->getChildrenWithComponent<pr::Light>())
			{
				auto t = e->getComponent<pr::Transform>();
				auto l = e->getComponent<pr::Light>();

				LightUniformData data;
				l->writeUniformData(data, t);
				lightData.push_back(data);

				l->updateLightViewProjection(userCamera, t);
			}
		}

		Lights lights;
		for (int i = 0; i < lightData.size(); i++)
			lights.lightData[i] = lightData[i];
		lights.numLights = (int)lightData.size();
		lightUBO->uploadMapped(&lights);
	}

	void Renderer::updateCamera(Scene::Ptr scene, UserCamera& userCamera, float time, int debugChannel)
	{
		//userCamera.setAspect((float)width / (float)height);
		camera.VP = userCamera.getViewProjectionMatrix();
		camera.VP_I = glm::inverse(camera.VP);
		camera.P = userCamera.getProjectionMatrix();
		camera.P_I = glm::inverse(camera.P);
		camera.V = userCamera.getViewMatrix();
		camera.V_I = glm::inverse(camera.V);
		camera.position = glm::vec4(userCamera.getPosition(), 0.0f);
		camera.time = glm::vec4(time);
		camera.time.w = (float)debugChannel;
		camera.projParams = glm::vec4(1);
		camera.zNear = userCamera.getZNear();
		camera.zFar = userCamera.getZFar();
		camera.scale = 1.0f / log2(userCamera.getZFar() / userCamera.getZNear());
		camera.bias = -(log2(userCamera.getZNear()) * camera.scale);

		if (GraphicsContext::getInstance().getCurrentAPI() == GraphicsAPI::Direct3D11)
		{
			camera.VP = glm::transpose(camera.VP);
			camera.VP_I = glm::transpose(camera.VP_I);
			camera.P = glm::transpose(camera.P);
			camera.P_I = glm::transpose(camera.P_I);
			camera.V = glm::transpose(camera.V);
			camera.V_I = glm::transpose(camera.V_I);
		}

		for (auto root : scene->getRootNodes())
		{
			for (auto e : root->getChildrenWithComponent<pr::Light>())
			{
				auto t = e->getComponent<pr::Transform>();
				auto l = e->getComponent<pr::Light>();
				l->updateLightViewProjection(userCamera, t);
			}
		}		

		cameraUBO->uploadMapped((uint8*)&camera);
		updated = true;
	}

	void Renderer::updateCamera(Scene::Ptr scene, glm::mat4 P, glm::mat4 V, glm::vec3 pos, float time, int debugChannel)
	{
		camera.VP = P * V;
		camera.VP_I = glm::inverse(camera.VP);
		camera.P = P;
		camera.P_I = glm::inverse(camera.P);
		camera.V = V;
		camera.V_I = glm::inverse(camera.V);
		camera.position = glm::vec4(pos, 0.0f);
		camera.time = glm::vec4(time);
		camera.time.w = (float)debugChannel;
		camera.projParams = glm::vec4(1);
		camera.zNear = 0.1f;
		camera.zFar = 1000.0f;
		camera.scale = 1.0f / log2(camera.zFar / camera.zNear);
		camera.bias = -(log2(camera.zNear) * camera.scale);

		if (GraphicsContext::getInstance().getCurrentAPI() == GraphicsAPI::Direct3D11)
		{
			camera.VP = glm::transpose(camera.VP);
			camera.VP_I = glm::transpose(camera.VP_I);
			camera.P = glm::transpose(camera.P);
			camera.P_I = glm::transpose(camera.P_I);
			camera.V = glm::transpose(camera.V);
			camera.V_I = glm::transpose(camera.V_I);
		}

		cameraUBO->uploadMapped((uint8*)&camera);
		updated = true;
	}

	void Renderer::updateShadows(pr::Scene::Ptr scene)
	{
		shadows.updateShadowsCSM(0, scene);
		shadows.updateShadowsOMNI(scene);
	}

	void Renderer::updatePost(Post& post)
	{
		postProcessor.updatePost(post);
	}

	void Renderer::renderToTexture(pr::Scene::Ptr scene)
	{
		if (updated)
		{
			shadows.updateShadowsCSM(0, scene);
			shadows.updateShadowsOMNI(scene);
			scatter.flush();			
			volumes.updateVolumes(scene);
			updated = false;
		}
	}
}
