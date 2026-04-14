#include "Outline.h"

namespace pr
{
	Outline::Outline()
	{

	}
	Outline::~Outline()
	{

	}

	void Outline::init(uint32 width, uint32 height)
	{
		this->width = width;
		this->height = height;
		unitQuad = createScreenQuad();
	}

	void Outline::initFramebuffers(uint32 width, uint32 height, pr::Texture2D::Ptr screenTex, pr::Texture2D::Ptr depthTex)
	{
		maskTex = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		maskTex->setAddressMode(GPU::AddressMode::ClampToEdge);
		maskTex->createData();
		maskTex->uploadData();

		maskDepthTex = pr::Texture2D::create(width, height, GPU::Format::D24_S8, 1, GPU::ImageUsage::DepthStencilAttachment);
		maskDepthTex->createData();

		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		maskTex->setLayoutShader(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();

		maskFramebuffer = ctx.createFramebuffer(width, height, 1, true, true);
		maskFramebuffer->addAttachment(maskTex->getImageView());
		maskFramebuffer->addAttachment(maskDepthTex->getImageView());
		maskFramebuffer->createFramebuffer();

		outlineFramebuffer = ctx.createFramebuffer(width, height, 1, true, false);
		outlineFramebuffer->addAttachment(screenTex->getImageView());
		outlineFramebuffer->addAttachment(depthTex->getImageView());
		outlineFramebuffer->createFramebuffer();
	}

	void Outline::initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool)
	{
		camDescriptorSet = descriptorPool->createDescriptorSet("Camera", 1);
		camDescriptorSet->update();

		animDescriptorSet = descriptorPool->createDescriptorSet("Animation", 1);
		animDescriptorSet->update();

		morphDescriptorSet = descriptorPool->createDescriptorSet("Morph", 1);
		morphDescriptorSet->update();

		outlineDS = descriptorPool->createDescriptorSet("Outline", 1);
		outlineDS->addDescriptor(maskTex->getDescriptor());
		outlineDS->update();
	}

	void Outline::initPipelines(GPU::DescriptorPool::Ptr descriptorPool)
	{
		auto& ctx = GraphicsContext::getInstance();
		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));
			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[3].index = 1;
			vertexInputDescription.inputAttributes[4].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[5].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material" };
			unlitPipeline = ctx.createGraphicsPipeline(maskFramebuffer, "Unlit", 1);

			std::cout << "compiling Unlit shader" << std::endl;
			switch (ctx.getCurrentAPI())
			{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				unlitPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Unlit.vert"), GPU::ShaderStage::Vertex);
				unlitPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Unlit.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/Unlit.vs.cso", vsCode);
				loadBinary(shaderPath + "/Unlit.ps.cso", psCode);
				unlitPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				unlitPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				unlitPipeline->addShaderStage(loadTxtFile(shaderPath + "/Unlit.vert.spv"), GPU::ShaderStage::Vertex);
				unlitPipeline->addShaderStage(loadTxtFile(shaderPath + "/Unlit.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
			}

			unlitPipeline->setVertexInputDescripton(vertexInputDescription);
			unlitPipeline->setLayout(descriptorPool, setLayouts);
			unlitPipeline->createProgram();
		}

		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(1, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, color)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));
			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "COLOR";
			vertexInputDescription.inputAttributes[2].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[3].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[3].index = 1;
			vertexInputDescription.inputAttributes[4].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[5].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material" };
			unlitPipelineStencil = ctx.createGraphicsPipeline(outlineFramebuffer, "Unlit", 1);
			unlitPipelineStencil->setDepthTest(true, false);
			unlitPipelineStencil->setStencilTest(true, 0xFF, 1, GPU::CompareOp::Always);
			unlitPipelineStencil->setColorMask(false, false, false, false);

			switch (ctx.getCurrentAPI())
			{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				unlitPipelineStencil->addShaderStage(prefix + loadExpanded(shaderPath + "/Unlit.vert"), GPU::ShaderStage::Vertex);
				unlitPipelineStencil->addShaderStage(prefix + loadExpanded(shaderPath + "/Unlit.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/Unlit.vs.cso", vsCode);
				loadBinary(shaderPath + "/Unlit.ps.cso", psCode);
				unlitPipelineStencil->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				unlitPipelineStencil->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				unlitPipelineStencil->addShaderStage(loadTxtFile(shaderPath + "/Unlit.vert.spv"), GPU::ShaderStage::Vertex);
				unlitPipelineStencil->addShaderStage(loadTxtFile(shaderPath + "/Unlit.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
			}

			unlitPipelineStencil->setVertexInputDescripton(vertexInputDescription);
			unlitPipelineStencil->setLayout(descriptorPool, setLayouts);
			unlitPipelineStencil->createProgram();
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

			std::vector<std::string> setLayouts = { "Outline" };
			outlinePipeline = ctx.createGraphicsPipeline(outlineFramebuffer, "Outline", 1);
			outlinePipeline->setCullMode(0);
			outlinePipeline->setDepthTest(false, false);
			outlinePipeline->setStencilTest(true, 0x00, 1, GPU::CompareOp::NotEqual);

			switch (ctx.getCurrentAPI())
			{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				outlinePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Outline.vert"), GPU::ShaderStage::Vertex);
				outlinePipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Outline.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/Outline.vs.cso", vsCode);
				loadBinary(shaderPath + "/Outline.ps.cso", psCode);
				outlinePipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				outlinePipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				outlinePipeline->addShaderStage(loadTxtFile(shaderPath + "/Outline.vert.spv"), GPU::ShaderStage::Vertex);
				outlinePipeline->addShaderStage(loadTxtFile(shaderPath + "/Outline.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
			}

			outlinePipeline->setVertexInputDescripton(vertexInputDescription);
			outlinePipeline->setLayout(descriptorPool, setLayouts);
			outlinePipeline->createProgram();
		}
	}

	void Outline::buildCmdBuffer(GPU::CommandBuffer::Ptr cmdBuf, pr::Entity::Ptr model)
	{
		glm::vec3 color = glm::vec3(1.0f, 0.533f, 0.0f);
		cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		cmdBuf->setScissor(0, 0, width, height);
		cmdBuf->beginRenderPass(maskFramebuffer);
		cmdBuf->setCullMode(0);
		cmdBuf->bindPipeline(unlitPipeline);
		cmdBuf->bindDescriptorSets(unlitPipeline, camDescriptorSet, 0);
		cmdBuf->bindDescriptorSets(unlitPipeline, animDescriptorSet, 2);
		cmdBuf->bindDescriptorSets(unlitPipeline, morphDescriptorSet, 3);

		auto models = model->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			if (m->isActive())
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();
				r->render(cmdBuf, unlitPipeline);
			}
		}

		cmdBuf->endRenderPass();

		cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		cmdBuf->setScissor(0, 0, width, height);
		cmdBuf->beginRenderPass(outlineFramebuffer);
		cmdBuf->setCullMode(0);
		cmdBuf->bindPipeline(unlitPipelineStencil);
		cmdBuf->bindDescriptorSets(unlitPipelineStencil, camDescriptorSet, 0);
		cmdBuf->bindDescriptorSets(unlitPipelineStencil, animDescriptorSet, 2);
		cmdBuf->bindDescriptorSets(unlitPipelineStencil, morphDescriptorSet, 3);
		for (auto m : models)
		{
			if (m->isActive())
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();
				r->render(cmdBuf, unlitPipelineStencil);
			}
		}

		cmdBuf->bindPipeline(outlinePipeline);
		cmdBuf->bindDescriptorSets(outlinePipeline, outlineDS, 0);
		unitQuad->draw(cmdBuf);

		cmdBuf->endRenderPass();
	}
}