#include "PostProcessor.h"
#include <Utils/IBL.h>
namespace pr
{
	PostProcessor::PostProcessor() :
		width(0), height(0)
	{

	}

	PostProcessor::~PostProcessor()
	{

	}

	void PostProcessor::init(uint32 width, uint32 height, GPU::DescriptorPool::Ptr descriptorPool, GPU::Framebuffer::Ptr outFBO)
	{
		this->width = width;
		this->height = height;
		this->descriptorPool = descriptorPool;

		auto& ctx = GraphicsContext::getInstance();

		initFramebuffers();
		initDescriptorLayouts(descriptorPool);
		initPipelines(outFBO);

		unitQuad = createScreenQuad();

		Post post;
		postUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(Post), 0);
		postUBO->uploadMapped(&post);
	}

	void PostProcessor::initFramebuffers()
	{
		uint32 maxMipLevel = 4;

		bloomBlurTex = pr::Texture2D::create(width / 2, height / 2, GPU::Format::RGBA16F, maxMipLevel, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		bloomBlurTex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
		bloomBlurTex->setAddressMode(GPU::AddressMode::ClampToEdge);
		bloomBlurTex->createData();
		bloomBlurTex->uploadData();

		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		bloomBlurTex->setLayoutShader(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();

		uint32 w = width / 2;
		uint32 h = height / 2;

		bloom.imageViews.clear();
		bloom.framebuffers.clear();
		auto image = bloomBlurTex->getImage();
		for (uint32 m = 0; m < maxMipLevel; m++)
		{
			uint32 mipWidth = static_cast<uint32>(w * std::pow(0.5f, m));
			uint32 mipHeight = static_cast<uint32>(h * std::pow(0.5f, m));

			auto view = image->createImageView(GPU::ViewType::View2D, GPU::SubResourceRange(m, 0, 1, 1));
			bloom.imageViews.push_back(view);

			auto fbo = ctx.createFramebuffer(mipWidth, mipHeight, 1, true, true);
			fbo->setClearColor(glm::vec4(0, 1, 0, 1));
			fbo->addAttachment(view);
			fbo->createFramebuffer();
			bloom.framebuffers.push_back(fbo);
		}
	}

	void PostProcessor::initDescriptorLayouts(GPU::DescriptorPool::Ptr descriptorPool)
	{
		this->descriptorPool = descriptorPool;

		{ // post process set
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(1, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			bindings.push_back(GPU::DescriptorSetLayoutBinding(2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("PostProcess", bindings);
		}

		{ // up sample
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("UpSample", bindings);
		}

		{ // down sample
			std::vector<GPU::DescriptorSetLayoutBinding> bindings;
			bindings.push_back(GPU::DescriptorSetLayoutBinding(0, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment));
			descriptorPool->addDescriptorSetLayout("DownSample", bindings);
		}
	}

	void PostProcessor::initDescriptorSets(pr::Texture2D::Ptr screenTex, pr::Texture2D::Ptr brightTex)
	{
		// post process set
		descriptorSetPostProcess = descriptorPool->createDescriptorSet("PostProcess", 1);
		descriptorSetPostProcess->addDescriptor(postUBO->getDescriptor());
		descriptorSetPostProcess->addDescriptor(screenTex->getDescriptor());
		descriptorSetPostProcess->addDescriptor(bloomBlurTex->getDescriptor());
		descriptorSetPostProcess->update();

		// up sample
		upSampleViews.clear();
		auto image = bloomBlurTex->getImage();
		for (int i = 0; i < 4; i++)
		{
			auto view = image->createImageView(GPU::ViewType::View2D, GPU::SubResourceRange(i, 0, 1, 1));
			upSampleViews.push_back(view);

			upSampleDescriptorSets[i] = descriptorPool->createDescriptorSet("UpSample", 1);
			upSampleDescriptorSets[i]->addDescriptor(bloomBlurTex->getDescriptor(view));
			upSampleDescriptorSets[i]->update();
		}

		// down sample
		bloomDescriptorSet = descriptorPool->createDescriptorSet("DownSample", 1);
		bloomDescriptorSet->addDescriptor(brightTex->getDescriptor());
		bloomDescriptorSet->update();

		downSampleViews.clear();
		for (int i = 0; i < 4; i++)
		{
			auto view = image->createImageView(GPU::ViewType::View2D, GPU::SubResourceRange(i, 0, 1, 1));
			downSampleViews.push_back(view);

			downSampleDescriptorSets[i] = descriptorPool->createDescriptorSet("DownSample", 1);
			downSampleDescriptorSets[i]->addDescriptor(bloomBlurTex->getDescriptor(view));
			downSampleDescriptorSets[i]->update();
		}
	}

	void PostProcessor::initPipelines(GPU::Framebuffer::Ptr finalFBO)
	{
		auto& ctx = GraphicsContext::getInstance();
		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;

			std::vector<std::string> setLayouts = { "PostProcess" };
			postProcessPipeline = ctx.createGraphicsPipeline(finalFBO, "PostProcess", 1);
			postProcessPipeline->setCullMode(0);
			postProcessPipeline->setBlending(false);
			postProcessPipeline->setDepthTest(false, false);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					postProcessPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/PostProcessing/PostProcess.vert"), GPU::ShaderStage::Vertex);
					postProcessPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/PostProcessing/PostProcess.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string vsCode, psCode;
					std::string shaderPath = "../../../../cache/shaders/cso";
					loadBinary(shaderPath + "/PostProcess.vs.cso", vsCode);
					loadBinary(shaderPath + "/PostProcess.ps.cso", psCode);
					postProcessPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					postProcessPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../cache/shaders/spv";
					postProcessPipeline->addShaderStage(loadTxtFile(shaderPath + "/PostProcess.vert.spv"), GPU::ShaderStage::Vertex);
					postProcessPipeline->addShaderStage(loadTxtFile(shaderPath + "/PostProcess.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			postProcessPipeline->setVertexInputDescripton(vertexInputDescription);
			postProcessPipeline->setLayout(descriptorPool, setLayouts);
			postProcessPipeline->createProgram();
		}

		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;

			std::vector<GPU::PushConstant> pushConstants;
			pushConstants.push_back(GPU::PushConstant(GPU::ShaderStage::Fragment, 0, sizeof(UpSampleParams)));

			std::vector<std::string> setLayouts = { "UpSample" };
			upSamplePipeline = ctx.createGraphicsPipeline(bloom.framebuffers[0], "UpSample", 1);
			upSamplePipeline->setCullMode(0);
			upSamplePipeline->setBlending(false);
			upSamplePipeline->setDepthTest(false, false);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					upSamplePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/UpSample.vert"), GPU::ShaderStage::Vertex);
					upSamplePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/UpSample.frag"), GPU::ShaderStage::Fragment);

					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string vsCode, psCode;
					std::string shaderPath = "../../../../cache/shaders/cso";
					loadBinary(shaderPath + "/UpSample.vs.cso", vsCode);
					loadBinary(shaderPath + "/UpSample.ps.cso", psCode);
					upSamplePipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					upSamplePipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../cache/shaders/spv";
					upSamplePipeline->addShaderStage(loadTxtFile(shaderPath + "/UpSample.vert.spv"), GPU::ShaderStage::Vertex);
					upSamplePipeline->addShaderStage(loadTxtFile(shaderPath + "/UpSample.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			upSamplePipeline->setVertexInputDescripton(vertexInputDescription);
			upSamplePipeline->setLayout(descriptorPool, setLayouts, pushConstants);
			upSamplePipeline->createProgram();
		}

		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(2, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, normal)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "NORMAL";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[4].index = 1;

			std::vector<GPU::PushConstant> pushConstants;
			pushConstants.push_back(GPU::PushConstant(GPU::ShaderStage::Fragment, 0, sizeof(DownSampleParams)));

			std::vector<std::string> setLayouts = { "DownSample" };
			downSamplePipeline = ctx.createGraphicsPipeline(bloom.framebuffers[0], "DownSample", 1);
			downSamplePipeline->setCullMode(0);
			downSamplePipeline->setBlending(false);
			downSamplePipeline->setDepthTest(false, false);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					downSamplePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/DownSample.vert"), GPU::ShaderStage::Vertex);
					downSamplePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Utils/DownSample.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string vsCode, psCode;
					std::string shaderPath = "../../../../cache/shaders/cso";
					loadBinary(shaderPath + "/DownSample.vs.cso", vsCode);
					loadBinary(shaderPath + "/DownSample.ps.cso", psCode);
					downSamplePipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					downSamplePipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../cache/shaders/spv";
					downSamplePipeline->addShaderStage(loadTxtFile(shaderPath + "/DownSample.vert.spv"), GPU::ShaderStage::Vertex);
					downSamplePipeline->addShaderStage(loadTxtFile(shaderPath + "/DownSample.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			downSamplePipeline->setVertexInputDescripton(vertexInputDescription);
			downSamplePipeline->setLayout(descriptorPool, setLayouts, pushConstants);
			downSamplePipeline->createProgram();
		}
	}

	void PostProcessor::buildCmdForward(GPU::CommandBuffer::Ptr cmdBuf, GPU::Framebuffer::Ptr outFBO)
	{
		cmdBuf->setCullMode(0);

		uint32 maxMipLevel = 4;
		uint32 w = width / 2;
		uint32 h = height / 2;

		DownSampleParams downSampleParams;
		downSampleParams.mipLevel = 0;
		downSampleParams.width = width;
		downSampleParams.height = height;

		uint32 mipWidth = w;
		uint32 mipHeight = h;

		cmdBuf->setViewport(0.0f, 0.0f, (float)mipWidth, (float)mipHeight);
		cmdBuf->setScissor(0, 0, mipWidth, mipHeight);
		cmdBuf->beginRenderPass(bloom.framebuffers[0]);
		cmdBuf->bindPipeline(downSamplePipeline);
		cmdBuf->pushConstants(downSamplePipeline, GPU::ShaderStage::Fragment, 0, sizeof(DownSampleParams), &downSampleParams);
		cmdBuf->bindDescriptorSets(downSamplePipeline, bloomDescriptorSet, 0);

		unitQuad->draw(cmdBuf);

		cmdBuf->endRenderPass();

		for (uint32 m = 1; m < maxMipLevel; m++)
		{
			uint32 mipWidth = static_cast<uint32>(w * std::pow(0.5, m));
			uint32 mipHeight = static_cast<uint32>(h * std::pow(0.5, m));

			downSampleParams.mipLevel = m - 1;
			downSampleParams.width = mipWidth;
			downSampleParams.height = mipHeight;

			cmdBuf->setViewport(0, 0, (float)mipWidth, (float)mipHeight);
			cmdBuf->setScissor(0, 0, mipWidth, mipHeight);
			cmdBuf->beginRenderPass(bloom.framebuffers[m]);
			cmdBuf->bindPipeline(downSamplePipeline);
			cmdBuf->pushConstants(downSamplePipeline, GPU::ShaderStage::Fragment, 0, sizeof(DownSampleParams), &downSampleParams);
			cmdBuf->bindDescriptorSets(downSamplePipeline, downSampleDescriptorSets[m - 1], 0);

			unitQuad->draw(cmdBuf);

			cmdBuf->endRenderPass();
		}

		UpSampleParams upSampleParams;
		upSampleParams.filterRadius = 0.005f;

		for (uint32 m = maxMipLevel - 1; m > 0; m--)
		{
			uint32 mipWidth = static_cast<uint32>(w * std::pow(0.5, m - 1));
			uint32 mipHeight = static_cast<uint32>(h * std::pow(0.5, m - 1));

			upSampleParams.mipLevel = m;

			cmdBuf->setViewport(0, 0, (float)mipWidth, (float)mipHeight);
			cmdBuf->setScissor(0, 0, mipWidth, mipHeight);
			cmdBuf->beginRenderPass(bloom.framebuffers[m - 1]);
			cmdBuf->bindPipeline(upSamplePipeline);
			cmdBuf->pushConstants(upSamplePipeline, GPU::ShaderStage::Fragment, 0, sizeof(UpSampleParams), &upSampleParams);
			cmdBuf->bindDescriptorSets(upSamplePipeline, upSampleDescriptorSets[m], 0);

			unitQuad->draw(cmdBuf);

			cmdBuf->endRenderPass();
		}

		// post process pass
		cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		cmdBuf->setScissor(0, 0, width, height);
		cmdBuf->beginRenderPass(outFBO);

		cmdBuf->bindPipeline(postProcessPipeline);
		cmdBuf->bindDescriptorSets(postProcessPipeline, descriptorSetPostProcess, 0);

		unitQuad->draw(cmdBuf);

		cmdBuf->endRenderPass();
	}

	void PostProcessor::buildCmdPost(GPU::CommandBuffer::Ptr cmdBuf, GPU::Framebuffer::Ptr outFBO)
	{
		cmdBuf->setCullMode(0);
		cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		cmdBuf->setScissor(0, 0, width, height);

		cmdBuf->bindPipeline(postProcessPipeline);
		cmdBuf->bindDescriptorSets(postProcessPipeline, descriptorSetPostProcess, 0);

		unitQuad->draw(cmdBuf);
	}

	void PostProcessor::updatePost(Post& post)
	{
		postUBO->uploadMapped(&post);
	}
}