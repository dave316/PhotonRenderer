#include "Scatter.h"
#include <Utils/IBL.h>

namespace pr
{
	float burleySetup(float radius, float albedo)
	{
		const float s = 1.9f - albedo + 3.5f * ((albedo - 0.8f) * (albedo - 0.8f));
		const float l = 0.25f * glm::one_over_pi<float>() * radius;
		return l / s;
	}

	float burleySample(float d, float xRand)
	{
		xRand *= 0.9963790093708328f;
		const float tolerance = 1e-6f;
		const uint32 maxIterationCount = 10;
		float r = 15.0f;
		if (xRand <= 0.9)
			r = glm::exp(xRand * xRand * 2.4f) - 1.0f;
		for (uint32 i = 0; i < maxIterationCount; i++)
		{
			const float expR3 = glm::exp(-r / 3.0f);
			const float expR = expR3 * expR3 * expR3;
			const float f = 1.0f - 0.25f * expR - 0.75f * expR3 - xRand;
			const float f_ = 0.25f * expR + 0.25f * expR3;
			if (glm::abs(f) < tolerance || f_ == 0.0f)
				break;
			r = r - f / f_;
			r = glm::max(r, 0.0f);
		}

		return r * d;
	}

	float burleyEval(float d, float r)
	{
		if (r >= 16.0f * d)
			return 0.0f;

		const float expR3 = glm::exp(-r / 3.0f * d);
		const float expR = expR3 * expR3 * expR3;
		return (expR * expR3) / (8.0f * glm::pi<float>() * d);
	}

	float burleyPdf(float d, float r)
	{
		return burleyEval(d, r) / 0.9963790093708328f;
	}

	void computeScatterSamples(ScatterData& scatter)
	{
		const float d = burleySetup(1.0f, 1.0f);
		const float randU = 0.5f; // TODO: compute actual random values [0-1]
		const float randV = 0.5f; // TODO: compute actual random values [0-1]
		float minRadius = 1.0f;
		const float goldenAngle = glm::pi<float>() * (3.0f - glm::sqrt(5.0f));
		for (int i = 0; i < ScatterSamplesCount; i++)
		{
			const float theta = goldenAngle * i + glm::pi<float>() * 2.0f * randU;
			const float x = (randV + i) / ScatterSamplesCount;
			const float r = burleySample(d, x);
			minRadius = glm::min(minRadius, r);
			scatter.scatterSamples[i] = glm::vec4(theta, r, burleyPdf(d, r), 0.0f);
		}

		scatter.minRadius = glm::max(minRadius, 0.00001f);
	}

	Scatter::Scatter()
	{

	}
	Scatter::~Scatter()
	{

	}

	void Scatter::init(uint32 width, uint32 height)
	{
		this->width = width;
		this->height = height;

		auto& ctx = GraphicsContext::getInstance();

		ScatterData scatter;
		computeScatterSamples(scatter);
		scatterUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(ScatterData), 0);
		scatterUBO->uploadMapped(&scatter);
				
		scatterCmdBuf = ctx.allocateCommandBuffer();
	}

	void Scatter::initFramebuffers(uint32 width, uint32 height)
	{
		auto& ctx = GraphicsContext::getInstance();
		scatterFrontTexture = pr::Texture2D::create(width, height, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		scatterFrontTexture->createData();
		scatterFrontTexture->setFilter(GPU::Filter::Nearest, GPU::Filter::Nearest);
		scatterFrontTexture->setAddressMode(GPU::AddressMode::ClampToEdge);

		scatterDepthTexture = pr::Texture2D::create(width, height, GPU::Format::DEPTH32, 1, GPU::ImageUsage::DepthStencilAttachment | GPU::ImageUsage::Sampled);
		scatterDepthTexture->createData();
		scatterDepthTexture->setFilter(GPU::Filter::Nearest, GPU::Filter::Nearest);
		scatterDepthTexture->setAddressMode(GPU::AddressMode::ClampToEdge);

		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		scatterFrontTexture->setLayoutShader(cmdBuf);
		scatterDepthTexture->setLayoutShader(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();

		scatterFramebuffer = ctx.createFramebuffer(width, height, 1, true, true);
		scatterFramebuffer->addAttachment(scatterFrontTexture->getImageView());
		scatterFramebuffer->addAttachment(scatterDepthTexture->getImageView());
		scatterFramebuffer->createFramebuffer();
	}

	void Scatter::initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool)
	{
		scatterDescriptorSet = descriptorPool->createDescriptorSet("Scatter", 1);
		scatterDescriptorSet->addDescriptor(scatterUBO->getDescriptor());
		scatterDescriptorSet->addDescriptor(scatterFrontTexture->getDescriptor());
		scatterDescriptorSet->addDescriptor(scatterDepthTexture->getDescriptor());
		scatterDescriptorSet->update();
	}

	void Scatter::initPipelines(GPU::DescriptorPool::Ptr descriptorPool)
	{
		auto& ctx = GraphicsContext::getInstance();
		std::string shaderName = "DefaultScatter";
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

		std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "Material", "IBL", "Light" };
		scatterPipeline = ctx.createGraphicsPipeline(scatterFramebuffer, shaderName, 1);

		GraphicsAPI api = ctx.getCurrentAPI();
		switch (api)
		{
		case GraphicsAPI::OpenGL:
		{
			std::string shaderPath = "../../../../src/Shaders/GLSL";
			std::string versionStr = "#version 460 core\n";
			std::string defineStr = "#define USE_OPENGL\n";
			std::string prefix = versionStr + defineStr;
			std::cout << "compiling shader " << shaderName << std::endl;
			scatterPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/" + shaderName + ".vert"), GPU::ShaderStage::Vertex);
			scatterPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/" + shaderName + ".frag"), GPU::ShaderStage::Fragment);
			break;
		}
		case GraphicsAPI::Direct3D11:
		{
			std::string shaderPath = "../../../../cache/shaders/cso";
			std::string vsCode, psCode;
			loadBinary(shaderPath + "/" + shaderName + ".vs.cso", vsCode);
			loadBinary(shaderPath + "/" + shaderName + ".ps.cso", psCode);
			scatterPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
			scatterPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			std::string shaderPath = "../../../../src/Shaders/GLSL";
			std::cout << "compiling shader " << shaderName << std::endl;
			scatterPipeline->addShaderStage(loadTxtFile(shaderPath + "/" + shaderName + ".vert.spv"), GPU::ShaderStage::Vertex);
			scatterPipeline->addShaderStage(loadTxtFile(shaderPath + "/" + shaderName + ".frag.spv"), GPU::ShaderStage::Fragment);
			break;
		}
		}

		scatterPipeline->setVertexInputDescripton(vertexInputDescription);
		scatterPipeline->setLayout(descriptorPool, setLayouts);
		scatterPipeline->createProgram();
	}

	void Scatter::buildCmdBuffer(pr::Scene::Ptr scene, std::map<uint32, GPU::DescriptorSet::Ptr> descriptorSets)
	{		
		// TODO: only get nodes with volume scatter material
		auto opaqueNodes = scene->getOpaqueEntities();

		scatterCmdBuf->begin();
		scatterCmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
		scatterCmdBuf->setScissor(0, 0, width, height);
		scatterCmdBuf->beginRenderPass(scatterFramebuffer);

		// opaque forward pass
		scatterCmdBuf->setCullMode(2);
		for (auto&& [shaderName, renderQueue] : opaqueNodes)
		{
			scatterCmdBuf->bindPipeline(scatterPipeline);
			for (auto&& [setIndex, descriptorSet] : descriptorSets)
				scatterCmdBuf->bindDescriptorSets(scatterPipeline, descriptorSet, setIndex);
			for (auto e : renderQueue)
			{
				if (e->isActive())
				{
					auto r = e->getComponent<Renderable>();
					r->render(scatterCmdBuf, scatterPipeline);
				}
			}
		}

		scatterCmdBuf->endRenderPass();
		scatterCmdBuf->end();
	}

	void Scatter::flush()
	{
		scatterCmdBuf->flush();
	}
}
