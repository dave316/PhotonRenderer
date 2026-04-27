#include "IBL.h"

#include <Graphics/GraphicsContext.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void loadBinary(std::string fileName, std::string& buffer)
{
	std::ifstream file(fileName, std::ios::binary);
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		unsigned int size = (unsigned int)file.tellg();
		file.seekg(0, std::ios::beg);

		buffer.resize(size);

		file.read(&buffer[0], size);
		file.close();
	}
	else
	{
		std::cout << "could not open file " << fileName << std::endl;
	}
}

std::string loadTxtFile(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::binary);
	std::stringstream ss;

	if (file.is_open())
	{
		ss << file.rdbuf();
	}
	else
	{
		std::cout << "could not open file " << fileName << std::endl;
	}
	return ss.str();
}

std::string loadExpanded(const std::string& fileName)
{
	std::string code = loadTxtFile(fileName);
	std::stringstream is(code);
	std::string line;
	std::string expandedCode = "";
	std::getline(is, line);

	while (std::getline(is, line))
	{
		if (!line.empty() && line.at(0) == '#')
		{
			size_t index = line.find_first_of(" ");
			std::string directive = line.substr(0, index);
			if (directive.compare("#include") == 0)
			{
				size_t start = line.find_first_of("\"") + 1;
				size_t end = line.find_last_of("\"");
				size_t index = fileName.find_last_of("/");
				std::string subFile = line.substr(start, end - start);
				std::string includeFile = fileName.substr(0, index) + "/" + subFile;
				std::string includeCode = loadTxtFile(includeFile);
				expandedCode += includeCode;
			}
			else
			{
				expandedCode += line + "\n";
			}
		}
		else
		{
			expandedCode += line + "\n";
		}
	}

	expandedCode += '\0';

	return expandedCode;
}

namespace IBL
{
	std::vector<glm::mat4> createCMViews(glm::vec3 position)
	{
		std::vector<glm::mat4> VP;
		glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));
		}
		else
		{
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		}
		return VP;
	}

	pr::Texture2D::Ptr generateBRDFLUT(uint32 dim)
	{
		//std::string shaderPath = "../../../../src/Shaders";
		auto unitQuad = createScreenQuad();
		auto brdfLUT = pr::Texture2D::create(dim, dim, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		brdfLUT->setAddressMode(GPU::AddressMode::ClampToEdge);
		brdfLUT->createData();
		brdfLUT->uploadData();
		brdfLUT->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto fbo = ctx.createFramebuffer(dim, dim, 1, true, true);
		fbo->addAttachment(brdfLUT->getImageView());
		fbo->createFramebuffer();

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

		auto descriptorPool = ctx.createDescriptorPool();
		auto pipeline = ctx.createGraphicsPipeline(fbo, "BRDFLUT", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLIntegrateBRDF.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLIntegrateBRDF.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/IBLIntegrateBRDF.vs.cso", vsCode);
				loadBinary(shaderPath + "/IBLIntegrateBRDF.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLIntegrateBRDF.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLIntegrateBRDF.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, {});
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		cmdBuf->beginRenderPass(fbo);
		cmdBuf->setViewport(0.0f, 0.0f, (float)dim, (float)dim);
		cmdBuf->setScissor(0, 0, dim, dim);
		cmdBuf->setCullMode(0);
		cmdBuf->bindPipeline(pipeline);
		unitQuad->draw(cmdBuf);
		cmdBuf->endRenderPass();
		cmdBuf->end();
		cmdBuf->flush();

		return brdfLUT;
	}

	pr::TextureCubeMap::Ptr convertEqui2CM(pr::Texture2D::Ptr pano, uint32 faceSize, float rotation)
	{		//std::string shaderPath = "../../../../src/Shaders";
		uint32 levels = static_cast<uint32>(std::floor(std::log2(std::max(faceSize, faceSize)))) + 1;
		auto unitCube = createCube(glm::vec3(0), 1.0f);
		auto cubeMap = pr::TextureCubeMap::create(faceSize, GPU::Format::RGBA16F, levels, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled | GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst);
		cubeMap->createData();
		cubeMap->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto fbo = ctx.createFramebuffer(faceSize, faceSize, 6, true, true);
		auto image = cubeMap->getImage();
		auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(0, 0, 1, 6));
		fbo->addAttachment(view);
		fbo->createFramebuffer();

		Model model;
		model.localToWorld = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0));
		auto modelUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(Model), 0);
		modelUBO->uploadMapped(&model);

		auto views = createCMViews();
		CubeViews cubeViews;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = glm::transpose(views[i]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = views[i];
		}
		cubeViews.layerID = 0;

		auto viewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CubeViews), 0);
		viewsUBO->uploadMapped(&cubeViews);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Vertex },
			{ 1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry },
			{ 2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		auto descriptorPool = ctx.createDescriptorPool();
		descriptorPool->addDescriptorSetLayout("Pano2CM", bindings);

		auto descriptorSet = descriptorPool->createDescriptorSet("Pano2CM", 1);
		descriptorSet->addDescriptor(modelUBO->getDescriptor());
		descriptorSet->addDescriptor(viewsUBO->getDescriptor());
		descriptorSet->addDescriptor(pano->getDescriptor());
		descriptorSet->update();

		GPU::VertexDescription vertexInputDescription;
		vertexInputDescription.binding = 0;
		vertexInputDescription.stride = sizeof(Vertex);
		vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
		vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
		vertexInputDescription.inputAttributes[0].name = "POSITION";

		std::vector<std::string> setLayouts = { "Pano2CM" };

		auto pipeline = ctx.createGraphicsPipeline(fbo, "Pano2CM", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/PanoToCubeMap.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/PanoToCubeMap.geom"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/PanoToCubeMap.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, gsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/PanoToCubemap.vs.cso", vsCode);
				loadBinary(shaderPath + "/PanoToCubemap.gs.cso", gsCode);
				loadBinary(shaderPath + "/PanoToCubemap.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/PanoToCubeMap.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/PanoToCubeMap.geom.spv"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/PanoToCubeMap.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, setLayouts);
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		cmdBuf->setViewport(0.0f, 0.0f, (float)faceSize, (float)faceSize);
		cmdBuf->setScissor(0, 0, faceSize, faceSize);
		cmdBuf->setCullMode(0);
		cmdBuf->beginRenderPass(fbo);
		cmdBuf->bindPipeline(pipeline);
		cmdBuf->bindDescriptorSets(pipeline, descriptorSet, 0);
		unitCube->draw(cmdBuf);
		cmdBuf->endRenderPass();
		cmdBuf->end();
		cmdBuf->flush();

		cubeMap->generateMipmaps();
		cubeMap->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);

		return cubeMap;
	}

	pr::TextureCubeMap::Ptr generateIrradianceMap(pr::TextureCubeMap::Ptr lightProbe, uint32 dim)
	{
		//std::string shaderPath = "../../../../src/Shaders";
		auto unitCube = createCube(glm::vec3(0), 1.0f);
		auto irradianceMap = pr::TextureCubeMap::create(dim, GPU::Format::RGBA16F, 1, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		irradianceMap->setAddressMode(GPU::AddressMode::ClampToEdge);
		irradianceMap->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto fbo = ctx.createFramebuffer(dim, dim, 6, true, true);
		fbo->addAttachment(irradianceMap->getImageView());
		fbo->createFramebuffer();

		auto views = createCMViews();
		CubeViews cubeViews;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = glm::transpose(views[i]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = views[i];
		}
		cubeViews.layerID = 0;

		auto viewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CubeViews), 0);
		viewsUBO->uploadMapped(&cubeViews);

		FilterParameters params;
		params.roughness = 0.0f;
		params.sampleCount = 2048;
		params.texSize = 1024;
		params.filterIndex = 0;

		auto paramsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(FilterParameters), 0);
		paramsUBO->uploadMapped(&params);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry },
			{ 1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment },
			{ 2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		auto descriptorPool = ctx.createDescriptorPool();
		descriptorPool->addDescriptorSetLayout("IBLFilter", bindings);

		auto descriptorSet = descriptorPool->createDescriptorSet("IBLFilter", 1);
		descriptorSet->addDescriptor(viewsUBO->getDescriptor());
		descriptorSet->addDescriptor(paramsUBO->getDescriptor());
		descriptorSet->addDescriptor(lightProbe->getDescriptor());
		descriptorSet->update();

		GPU::VertexDescription vertexInputDescription;
		vertexInputDescription.binding = 0;
		vertexInputDescription.stride = sizeof(Vertex);
		vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
		vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
		vertexInputDescription.inputAttributes[0].name = "POSITION";

		std::vector<std::string> setLayouts = { "IBLFilter" };

		auto pipeline = ctx.createGraphicsPipeline(fbo, "IBLFilter", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.geom"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, gsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/IBLFilter.vs.cso", vsCode);
				loadBinary(shaderPath + "/IBLFilter.gs.cso", gsCode);
				loadBinary(shaderPath + "/IBLFilter.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.geom.spv"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, setLayouts);
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		cmdBuf->setViewport(0.0f, 0.0f, (float)dim, (float)dim);
		cmdBuf->setScissor(0, 0, dim, dim);
		cmdBuf->setCullMode(0);
		cmdBuf->beginRenderPass(fbo);
		cmdBuf->bindPipeline(pipeline);
		cmdBuf->bindDescriptorSets(pipeline, descriptorSet, 0);
		unitCube->draw(cmdBuf);
		cmdBuf->endRenderPass();
		cmdBuf->end();
		cmdBuf->flush();

		return irradianceMap;
	}

	pr::TextureCubeMap::Ptr generatePrefilteredMap(pr::TextureCubeMap::Ptr lightProbe, uint32 dim, uint32 levels)
	{
		//std::string shaderPath = "../../../../src/Shaders";
		auto unitCube = createCube(glm::vec3(0), 1.0f);
		auto prefilteredMap = pr::TextureCubeMap::create(dim, GPU::Format::RGBA16F, levels, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		prefilteredMap->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
		prefilteredMap->setAddressMode(GPU::AddressMode::ClampToEdge);
		prefilteredMap->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto image = prefilteredMap->getImage();
		std::vector<GPU::ImageView::Ptr> imageViews;
		std::vector<GPU::Framebuffer::Ptr> framebuffers;
		for (uint32 m = 0; m < levels; m++)
		{
			uint32 width = dim >> m;
			uint32 height = dim >> m;

			auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(m, 0, 1, 6));
			imageViews.push_back(view);

			auto fbo = ctx.createFramebuffer(width, height, 6, true, true);
			fbo->addAttachment(view);
			fbo->createFramebuffer();
			framebuffers.push_back(fbo);
		}

		auto views = createCMViews();
		CubeViews cubeViews;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = glm::transpose(views[i]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = views[i];
		}
		cubeViews.layerID = 0;

		auto viewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CubeViews), 0);
		viewsUBO->uploadMapped(&cubeViews);

		FilterParameters params;
		params.roughness = 0.0f;
		params.sampleCount = 1024;
		params.texSize = 1024;
		params.filterIndex = 1;

		auto paramsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(FilterParameters), 0);
		paramsUBO->uploadMapped(&params);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry },
			{ 1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment },
			{ 2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		auto descriptorPool = ctx.createDescriptorPool();
		descriptorPool->addDescriptorSetLayout("IBLFilter", bindings);

		auto descriptorSet = descriptorPool->createDescriptorSet("IBLFilter", 1);
		descriptorSet->addDescriptor(viewsUBO->getDescriptor());
		descriptorSet->addDescriptor(paramsUBO->getDescriptor());
		descriptorSet->addDescriptor(lightProbe->getDescriptor());
		descriptorSet->update();

		GPU::VertexDescription vertexInputDescription;
		vertexInputDescription.binding = 0;
		vertexInputDescription.stride = sizeof(Vertex);
		vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
		vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
		vertexInputDescription.inputAttributes[0].name = "POSITION";

		std::vector<std::string> setLayouts = { "IBLFilter" };
		auto pipeline = ctx.createGraphicsPipeline(framebuffers[0], "IBLFilter", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.geom"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, gsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/IBLFilter.vs.cso", vsCode);
				loadBinary(shaderPath + "/IBLFilter.gs.cso", gsCode);
				loadBinary(shaderPath + "/IBLFilter.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.geom.spv"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, setLayouts);
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		for (uint32 m = 0; m < levels; m++)
		{
			params.roughness = (float)m / (float)(levels - 1);
			paramsUBO->uploadMapped(&params);

			cmdBuf->begin();
			uint32 width = static_cast<uint32>(dim * std::pow(0.5f, m));
			uint32 height = static_cast<uint32>(dim * std::pow(0.5f, m));
			cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
			cmdBuf->setScissor(0, 0, width, height);
			cmdBuf->setCullMode(0);
			cmdBuf->beginRenderPass(framebuffers[m]);
			cmdBuf->bindPipeline(pipeline);
			cmdBuf->bindDescriptorSets(pipeline, descriptorSet, 0);

			unitCube->draw(cmdBuf);

			cmdBuf->endRenderPass();
			cmdBuf->end();
			cmdBuf->flush();
		}

		return prefilteredMap;
	}

	pr::TextureCubeMapArray::Ptr generatePrefilteredMaps(std::vector<pr::TextureCubeMap::Ptr> lightProbes, uint32 dim, uint32 levels)
	{
		auto unitCube = createCube(glm::vec3(0), 1.0f);
		auto prefilteredMaps = pr::TextureCubeMapArray::create(dim, (uint32)lightProbes.size(), GPU::Format::RGBA16F, levels, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		prefilteredMaps->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
		prefilteredMaps->setAddressMode(GPU::AddressMode::ClampToEdge);
		prefilteredMaps->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto image = prefilteredMaps->getImage();
		std::vector<std::vector<GPU::ImageView::Ptr>> imageViews;
		std::vector<std::vector<GPU::Framebuffer::Ptr>> framebuffers;
		for (uint32 m = 0; m < levels; m++)
		{
			uint32 width = dim >> m;
			uint32 height = dim >> m;

			std::vector<GPU::ImageView::Ptr> views;
			std::vector<GPU::Framebuffer::Ptr> fbos;
			for (int l = 0; l < lightProbes.size(); l++)
			{
				auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(m, l * 6, 1, 6));
				views.push_back(view);
				
				auto fbo = ctx.createFramebuffer(width, height, 6, true, true);
				fbo->addAttachment(view);
				fbo->createFramebuffer();
				fbos.push_back(fbo);
			}
			imageViews.push_back(views);
			framebuffers.push_back(fbos);
		}

		auto views = createCMViews();
		CubeViews cubeViews;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = glm::transpose(views[i]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = views[i];
		}
		cubeViews.layerID = 0;

		auto viewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CubeViews), 0);
		viewsUBO->uploadMapped(&cubeViews);

		FilterParameters params;
		params.roughness = 0.0f;
		params.sampleCount = 1024;
		params.texSize = 0;
		params.filterIndex = 1;

		auto paramsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(FilterParameters), 0);
		paramsUBO->uploadMapped(&params);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry },
			{ 1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment },
			{ 2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		auto descriptorPool = ctx.createDescriptorPool();
		descriptorPool->addDescriptorSetLayout("IBLFilter", bindings);

		std::vector<GPU::DescriptorSet::Ptr> descriptorSets;
		for (int i = 0; i < lightProbes.size(); i++)
		{
			auto descriptorSet = descriptorPool->createDescriptorSet("IBLFilter", 1);
			descriptorSet->addDescriptor(viewsUBO->getDescriptor());
			descriptorSet->addDescriptor(paramsUBO->getDescriptor());
			descriptorSet->addDescriptor(lightProbes[i]->getDescriptor());
			descriptorSet->update();
			descriptorSets.push_back(descriptorSet);
		}

		GPU::VertexDescription vertexInputDescription;
		vertexInputDescription.binding = 0;
		vertexInputDescription.stride = sizeof(Vertex);
		vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
		vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
		vertexInputDescription.inputAttributes[0].name = "POSITION";

		std::vector<std::string> setLayouts = { "IBLFilter" };
		auto pipeline = ctx.createGraphicsPipeline(framebuffers[0][0], "IBLFilter", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.geom"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, gsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/IBLFilter.vs.cso", vsCode);
				loadBinary(shaderPath + "/IBLFilter.gs.cso", gsCode);
				loadBinary(shaderPath + "/IBLFilter.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.geom.spv"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, setLayouts);
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		for (uint32 m = 0; m < levels; m++)
		{
			for (uint32 l = 0; l < lightProbes.size(); l++)
			{
				params.roughness = (float)m / (float)(levels - 1);
				params.texSize = lightProbes[l]->getSize();
				paramsUBO->uploadMapped(&params);

				cubeViews.layerID = l;
				viewsUBO->uploadMapped(&cubeViews);

				cmdBuf->begin();
				uint32 width = static_cast<uint32>(dim * std::pow(0.5f, m));
				uint32 height = static_cast<uint32>(dim * std::pow(0.5f, m));
				cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
				cmdBuf->setScissor(0, 0, width, height);
				cmdBuf->setCullMode(0);
				cmdBuf->beginRenderPass(framebuffers[m][l]);
				cmdBuf->bindPipeline(pipeline);
				cmdBuf->bindDescriptorSets(pipeline, descriptorSets[l], 0);

				unitCube->draw(cmdBuf);

				cmdBuf->endRenderPass();
				cmdBuf->end();
				cmdBuf->flush();
			}
		}

 		return prefilteredMaps;
	}

	pr::TextureCubeMap::Ptr generatePrefilteredMapCharlie(pr::TextureCubeMap::Ptr lightProbe, uint32 dim, uint32 levels)
	{
		//std::string shaderPath = "../../../../src/Shaders";
		auto unitCube = createCube(glm::vec3(0), 1.0f);
		auto prefilteredMapCharlie = pr::TextureCubeMap::create(dim, GPU::Format::RGBA16F, levels, GPU::ImageUsage::ColorAttachment | GPU::ImageUsage::Sampled);
		prefilteredMapCharlie->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
		prefilteredMapCharlie->setAddressMode(GPU::AddressMode::ClampToEdge);
		prefilteredMapCharlie->setLayout();

		auto& ctx = pr::GraphicsContext::getInstance();
		//auto renderPass = ctx.createRenderPass(1, true, false, false);
		auto image = prefilteredMapCharlie->getImage();
		std::vector<GPU::ImageView::Ptr> imageViews;
		std::vector<GPU::Framebuffer::Ptr> framebuffers;
		for (uint32 m = 0; m < levels; m++)
		{
			uint32 width = dim >> m;
			uint32 height = dim >> m;

			auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(m, 0, 1, 6));
			imageViews.push_back(view);

			auto fbo = ctx.createFramebuffer(width, height, 6, true, true);
			fbo->addAttachment(view);
			fbo->createFramebuffer();
			framebuffers.push_back(fbo);
		}

		auto views = createCMViews();
		CubeViews cubeViews;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = glm::transpose(views[i]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				cubeViews.VP[i] = views[i];
		}
		cubeViews.layerID = 0;

		auto viewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CubeViews), 0);
		viewsUBO->uploadMapped(&cubeViews);

		FilterParameters params;
		params.roughness = 0.0f;
		params.sampleCount = 64;
		params.texSize = 1024;
		params.filterIndex = 2;

		auto paramsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(FilterParameters), 0);
		paramsUBO->uploadMapped(&params);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry },
			{ 1, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment },
			{ 2, GPU::DescriptorType::CombinedImageSampler, 1, GPU::ShaderStage::Fragment }
		};

		auto descriptorPool = ctx.createDescriptorPool();
		descriptorPool->addDescriptorSetLayout("IBLFilter", bindings);

		auto descriptorSet = descriptorPool->createDescriptorSet("IBLFilter", 1);
		descriptorSet->addDescriptor(viewsUBO->getDescriptor());
		descriptorSet->addDescriptor(paramsUBO->getDescriptor());
		descriptorSet->addDescriptor(lightProbe->getDescriptor());
		descriptorSet->update();

		GPU::VertexDescription vertexInputDescription;
		vertexInputDescription.binding = 0;
		vertexInputDescription.stride = sizeof(Vertex);
		vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
		vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
		vertexInputDescription.inputAttributes[0].name = "POSITION";

		std::vector<std::string> setLayouts = { "IBLFilter" };
		auto pipeline = ctx.createGraphicsPipeline(framebuffers[0], "IBLFilter", 1);
		pipeline->setDepthTest(false, false);
		pipeline->setBlending(false);
		pipeline->setCullMode(0);

		switch (ctx.getCurrentAPI())
		{
			case pr::GraphicsAPI::OpenGL:
			{
				std::string shaderPath = "../../../../src/Shaders/GLSL";
				std::string versionStr = "#version 460 core\n";
				std::string defineStr = "#define USE_OPENGL\n";
				std::string prefix = versionStr + defineStr;
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.vert"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.geom"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/IBL/IBLFilter.frag"), GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Direct3D11:
			{
				std::string vsCode, gsCode, psCode;
				std::string shaderPath = "../../../../cache/shaders/cso";
				loadBinary(shaderPath + "/IBLFilter.vs.cso", vsCode);
				loadBinary(shaderPath + "/IBLFilter.gs.cso", gsCode);
				loadBinary(shaderPath + "/IBLFilter.ps.cso", psCode);
				pipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
				break;
			}
			case pr::GraphicsAPI::Vulkan:
			{
				std::string shaderPath = "../../../../cache/shaders/spv";
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.vert.spv"), GPU::ShaderStage::Vertex);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.geom.spv"), GPU::ShaderStage::Geometry);
				pipeline->addShaderStage(loadTxtFile(shaderPath + "/IBLFilter.frag.spv"), GPU::ShaderStage::Fragment);
				break;
			}
		}

		pipeline->setVertexInputDescripton(vertexInputDescription);
		pipeline->setLayout(descriptorPool, setLayouts);
		pipeline->createProgram();

		auto cmdBuf = ctx.allocateCommandBuffer();
		for (uint32 m = 0; m < levels; m++)
		{
			params.roughness = (float)m / (float)(levels - 1);
			paramsUBO->uploadMapped(&params);

			cmdBuf->begin();
			uint32 width = static_cast<uint32>(dim * std::pow(0.5f, m));
			uint32 height = static_cast<uint32>(dim * std::pow(0.5f, m));
			cmdBuf->setViewport(0.0f, 0.0f, (float)width, (float)height);
			cmdBuf->setScissor(0, 0, width, height);
			cmdBuf->setCullMode(0);
			cmdBuf->beginRenderPass(framebuffers[m]);
			cmdBuf->bindPipeline(pipeline);
			cmdBuf->bindDescriptorSets(pipeline, descriptorSet, 0);

			unitCube->draw(cmdBuf);

			cmdBuf->endRenderPass();
			cmdBuf->end();
			cmdBuf->flush();
		}

		return prefilteredMapCharlie;
	}
}
