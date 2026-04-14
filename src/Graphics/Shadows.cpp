#include "Shadows.h"
#include <Utils/IBL.h>
namespace pr
{
	Shadows::Shadows()
	{

	}
	Shadows::~Shadows()
	{

	}

	void Shadows::init()
	{
		auto& ctx = GraphicsContext::getInstance();

		uint32 size = 4096;
		csmShadowMap = Texture2DArray::create(size, size, 4, GPU::Format::DEPTH32, 1, GPU::ImageUsage::DepthStencilAttachment | GPU::ImageUsage::Sampled);
		csmShadowMap->setAddressMode(GPU::AddressMode::ClampToBorder);
		csmShadowMap->setFilter(GPU::Filter::Nearest, GPU::Filter::Nearest);
		csmShadowMap->setLayout();

		csmFBO = ctx.createFramebuffer(size, size, 4, true, true);
		csmFBO->addAttachment(csmShadowMap->getImageView());
		csmFBO->createFramebuffer();

		csmCmdBuf = ctx.allocateCommandBuffer();

		csmViewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CSMViews), 0);
		csmDataUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(CSMData), 0);

		size = 4096;
		omniShadowMap = TextureCubeMapArray::create(size, 1, GPU::Format::DEPTH32, 1, GPU::ImageUsage::DepthStencilAttachment | GPU::ImageUsage::Sampled);
		omniShadowMap->setAddressMode(GPU::AddressMode::ClampToEdge);
		omniShadowMap->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
		omniShadowMap->setCompareMode();
		omniShadowMap->setLayout();
		
		omniViewsUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(OMNIViews), 0);
		omniDataUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(OMNIData), 0);

		auto image = omniShadowMap->getImage();
		auto fbo = ctx.createFramebuffer(size, size, 6, true, true);
		auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(0, 0, 1, 6));
		fbo->addAttachment(view);
		fbo->createFramebuffer();
		omniShadowFBOs.push_back(fbo);
		omniShadowViews.push_back(view);

		unitCube = createCube(glm::vec3(0), 1.0f);
		unitQuad = createScreenQuad();
	}

	void Shadows::initPipelines(GPU::DescriptorPool::Ptr descriptorPool)
	{
		auto& ctx = GraphicsContext::getInstance();
		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[2].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[2].index = 1;
			vertexInputDescription.inputAttributes[3].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[4].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model", "Animation", "Morph", "CSM", "MaterialShadow" };

			shadowCSMPipeline = ctx.createGraphicsPipeline(csmFBO, "CSM", 1);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					shadowCSMPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCSM.vert"), GPU::ShaderStage::Vertex);
					shadowCSMPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCSM.geom"), GPU::ShaderStage::Geometry);
					shadowCSMPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCSM.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string vsCode, gsCode, psCode;
					std::string shaderPath = "../../../../cache/shaders/cso";
					loadBinary(shaderPath + "/DepthCSM.vs.cso", vsCode);
					loadBinary(shaderPath + "/DepthCSM.gs.cso", gsCode);
					loadBinary(shaderPath + "/DepthCSM.ps.cso", psCode);
					shadowCSMPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					shadowCSMPipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
					shadowCSMPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					shadowCSMPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCSM.vert.spv"), GPU::ShaderStage::Vertex);
					shadowCSMPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCSM.geom.spv"), GPU::ShaderStage::Geometry);
					shadowCSMPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCSM.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			shadowCSMPipeline->setVertexInputDescripton(vertexInputDescription);
			shadowCSMPipeline->setLayout(descriptorPool, setLayouts);
			shadowCSMPipeline->createProgram();
		}
		{
			GPU::VertexDescription vertexInputDescription;
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = sizeof(Vertex);
			vertexInputDescription.inputRate = GPU::VertexInputeRate::Vertex;
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(0, 0, GPU::VertexAttribFormat::Vector3F, offsetof(Vertex, position)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(3, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord0)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(4, 0, GPU::VertexAttribFormat::Vector2F, offsetof(Vertex, texCoord1)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(6, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, joints)));
			vertexInputDescription.inputAttributes.push_back(GPU::VertexInputAttribute(7, 0, GPU::VertexAttribFormat::Vector4F, offsetof(Vertex, weights)));

			vertexInputDescription.inputAttributes[0].name = "POSITION";
			vertexInputDescription.inputAttributes[1].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[2].name = "TEXCOORD";
			vertexInputDescription.inputAttributes[2].index = 1;
			vertexInputDescription.inputAttributes[3].name = "BLENDINDICES";
			vertexInputDescription.inputAttributes[4].name = "BLENDWEIGHT";

			std::vector<std::string> setLayouts = { "Camera", "Model",  "Animation", "Morph", "OMNIViews", "MaterialShadow", "OMNILight" };

			shadowOMNIPipeline = ctx.createGraphicsPipeline(omniShadowFBOs[0], "OMNI", 1);
			shadowOMNIPipeline->setWindingOrder(1);

			switch (ctx.getCurrentAPI())
			{
				case pr::GraphicsAPI::OpenGL:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					std::string versionStr = "#version 460 core\n";
					std::string defineStr = "#define USE_OPENGL\n";
					std::string prefix = versionStr + defineStr;
					shadowOMNIPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCubemap.vert"), GPU::ShaderStage::Vertex);
					shadowOMNIPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCubemap.geom"), GPU::ShaderStage::Geometry);
					shadowOMNIPipeline->addShaderStage(prefix + loadExpanded(shaderPath + "/Shadows/DepthCubemap.frag"), GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Direct3D11:
				{
					std::string vsCode, gsCode, psCode;
					std::string shaderPath = "../../../../cache/shaders/cso";
					loadBinary(shaderPath + "/DepthCubemap.vs.cso", vsCode);
					loadBinary(shaderPath + "/DepthCubemap.gs.cso", gsCode);
					loadBinary(shaderPath + "/DepthCubemap.ps.cso", psCode);
					shadowOMNIPipeline->addShaderStage(vsCode, GPU::ShaderStage::Vertex);
					shadowOMNIPipeline->addShaderStage(gsCode, GPU::ShaderStage::Geometry);
					shadowOMNIPipeline->addShaderStage(psCode, GPU::ShaderStage::Fragment);
					break;
				}
				case pr::GraphicsAPI::Vulkan:
				{
					std::string shaderPath = "../../../../src/Shaders/GLSL";
					shadowOMNIPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCubemap.vert.spv"), GPU::ShaderStage::Vertex);
					shadowOMNIPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCubemap.geom.spv"), GPU::ShaderStage::Geometry);
					shadowOMNIPipeline->addShaderStage(loadTxtFile(shaderPath + "/Shadows/DepthCubemap.frag.spv"), GPU::ShaderStage::Fragment);
					break;
				}
			}

			shadowOMNIPipeline->setVertexInputDescripton(vertexInputDescription);
			shadowOMNIPipeline->setLayout(descriptorPool, setLayouts);
			shadowOMNIPipeline->createProgram();
		}
	}

	void Shadows::initDescriptorSets(GPU::DescriptorPool::Ptr descriptorPool)
	{
		camDescriptorSet = descriptorPool->createDescriptorSet("Camera", 1);
		camDescriptorSet->update();

		animDescriptorSet = descriptorPool->createDescriptorSet("Animation", 1);
		animDescriptorSet->update();

		morphDescriptorSet = descriptorPool->createDescriptorSet("Morph", 1);
		morphDescriptorSet->update();

		std::vector<GPU::DescriptorSetLayoutBinding> bindings = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry }
		};
		descriptorPool->addDescriptorSetLayout("CSM", bindings);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings1 = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Geometry }
		};
		descriptorPool->addDescriptorSetLayout("OMNIViews", bindings1);

		std::vector<GPU::DescriptorSetLayoutBinding> bindings2 = {
			{ 0, GPU::DescriptorType::UniformBuffer, 1, GPU::ShaderStage::Fragment }
		};
		descriptorPool->addDescriptorSetLayout("OMNILight", bindings2);

		descriptorSetShadow = descriptorPool->createDescriptorSet("CSM", 1);
		descriptorSetShadow->addDescriptor(csmViewsUBO->getDescriptor());
		descriptorSetShadow->update();

		descriptorSetOmniViews = descriptorPool->createDescriptorSet("OMNIViews", 1);
		descriptorSetOmniViews->addDescriptor(omniViewsUBO->getDescriptor());
		descriptorSetOmniViews->update();

		descriptorSetOmniLight = descriptorPool->createDescriptorSet("OMNILight", 1);
		descriptorSetOmniLight->addDescriptor(omniDataUBO->getDescriptor());
		descriptorSetOmniLight->update();
	}

	void Shadows::updateLights(pr::Scene::Ptr scene)
	{
		auto& ctx = GraphicsContext::getInstance();
		std::vector<pr::Light::Ptr> lights;
		for (auto entity : scene->getRootNodes())
		{
			auto lightsEntity = entity->getChildrenWithComponent<Light>();
			for (auto lightEntity : lightsEntity)
			{
				auto t = lightEntity->getComponent<pr::Transform>();
				auto l = lightEntity->getComponent<pr::Light>();
				if (l->getType() == LightType::POINT)
				{
					lights.push_back(l);
				}
			}
		}

		uint32 size = 4096;
		omniShadowMap = TextureCubeMapArray::create(size, (uint32)(lights.size() > 0 ? lights.size() : 1), GPU::Format::DEPTH32, 1, GPU::ImageUsage::DepthStencilAttachment | GPU::ImageUsage::Sampled);
		omniShadowMap->setAddressMode(GPU::AddressMode::ClampToEdge);
		omniShadowMap->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
		omniShadowMap->setCompareMode();
		omniShadowMap->setLayout();

		omniShadowFBOs.clear();
		omniShadowViews.clear();

		auto image = omniShadowMap->getImage();
		for (int i = 0; i < lights.size(); i++)
		{
			auto fbo = ctx.createFramebuffer(size, size, 6, true, true);
			auto view = image->createImageView(GPU::ViewType::ViewCubeMap, GPU::SubResourceRange(0, i * 6, 1, 6));
			fbo->addAttachment(view);
			fbo->createFramebuffer();
			omniShadowFBOs.push_back(fbo);
			omniShadowViews.push_back(view);
		}
	}

	void Shadows::addDesc(GPU::DescriptorSet::Ptr lightDescSet)
	{
		lightDescSet->addDescriptor(csmDataUBO->getDescriptor());
		lightDescSet->addDescriptor(omniShadowMap->getDescriptor());
		lightDescSet->addDescriptor(csmShadowMap->getDescriptor());
	}

	void Shadows::updateShadowsCSM(uint32 frameIndex, pr::Scene::Ptr scene)
	{
		auto& ctx = GraphicsContext::getInstance();
		std::vector<glm::mat4> lightSpaceMatrices(4);
		int numDirLights = 0;
		for (auto root : scene->getRootNodes())
		{
			for (auto lightEntity : root->getChildrenWithComponent<pr::Light>())
			{
				auto l = lightEntity->getComponent<pr::Light>();
				if (l->getType() == LightType::DIRECTIONAL)
				{
					lightSpaceMatrices = l->getViewProjections();
					numDirLights++;
				}					
			}
		}

		if (numDirLights == 0)
			return;

		CSMViews views;
		for (int i = 0; i < lightSpaceMatrices.size(); i++)
			views.VP[i] = lightSpaceMatrices[i];
	
		csmViewsUBO->uploadMapped(&views);

		float zFar = 500.0f;
		std::vector<float> csmLevels =
		{
			zFar / 50.0f,
			zFar / 25.0f,
			zFar / 10.0f,
			0.0
		};

		CSMData csmData;
		for (int i = 0; i < 4; i++)
		{
			csmData.lightSpaceMatrices[i] = lightSpaceMatrices[i];
			csmData.cascadePlaneDistance[i] = csmLevels[i];
		}
		csmData.cascadeCount = 3;
		csmDataUBO->uploadMapped(&csmData);

		csmCmdBuf->flush();
	}

	void Shadows::updateShadowsOMNI(pr::Scene::Ptr scene)
	{
		auto& ctx = GraphicsContext::getInstance();
		uint32 size = 4096;
		std::vector<glm::vec3> lightPositions;
		std::vector<pr::Light::Ptr> lights;
		for (auto entity : scene->getRootNodes())
		{
			auto lightsEntity = entity->getChildrenWithComponent<Light>();
			for (auto lightEntity : lightsEntity)
			{
				auto t = lightEntity->getComponent<pr::Transform>();
				auto l = lightEntity->getComponent<pr::Light>();
				if (l->getType() == LightType::POINT)
				{
					lights.push_back(l);
					lightPositions.push_back(t->getPosition());
				}
			}
		}

		for (int i = 0; i < lights.size(); i++)
		{
			OMNIViews views;
			auto VPs = lights[i]->getViewProjections();
			for (int j = 0; j < 6; j++)
			{
				if (ctx.getCurrentAPI() == GraphicsAPI::Direct3D11)
					views.VP[j] = glm::transpose(VPs[j]);
				else
					views.VP[j] = VPs[j];
				views.lightIndex = i;
			}
			omniViewsUBO->uploadMapped(&views);

			OMNIData omniData;
			omniData.position = glm::vec4(lightPositions[i], 0.0f);
			omniData.range = lights[i]->getRange();
			omniDataUBO->uploadMapped(&omniData);

			omniCmdBufs[i]->flush();
		}
	}

	void Shadows::buildCmdShadowsCSM(pr::Scene::Ptr scene)
	{
		uint32 size = 4096;
		csmCmdBuf->begin();
		csmCmdBuf->setViewport(0.0f, 0.0f, (float)size, (float)size);
		csmCmdBuf->setScissor(0, 0, size, size);
		csmCmdBuf->beginRenderPass(csmFBO);
		csmCmdBuf->setCullMode(0);
		csmCmdBuf->bindPipeline(shadowCSMPipeline);
		csmCmdBuf->bindDescriptorSets(shadowCSMPipeline, camDescriptorSet, 0);
		csmCmdBuf->bindDescriptorSets(shadowCSMPipeline, animDescriptorSet, 2);
		csmCmdBuf->bindDescriptorSets(shadowCSMPipeline, morphDescriptorSet, 3);
		csmCmdBuf->bindDescriptorSets(shadowCSMPipeline, descriptorSetShadow, 4);

		auto opaqueNodes = scene->getOpaqueEntities();
		for (auto&& [shaderName, renderQueue] : opaqueNodes)
		{
			for (auto e : renderQueue)
			{
				if (e->isActive())
				{
					auto r = e->getComponent<Renderable>();
					r->renderDepth(csmCmdBuf, shadowCSMPipeline);
				}
			}
		}

		csmCmdBuf->endRenderPass();
		csmCmdBuf->end();
	}

	void Shadows::buildCmdShadowsOMNI(pr::Scene::Ptr scene)
	{
		uint32 size = 4096;
		omniCmdBufs.clear();

		auto& ctx = GraphicsContext::getInstance();

		for (int i = 0; i < omniShadowFBOs.size(); i++)
		{
			auto cmdBuf = ctx.allocateCommandBuffer();
			cmdBuf->begin();
			cmdBuf->setViewport(0.0f, 0.0f, (float)size, (float)size);
			cmdBuf->setScissor(0, 0, size, size);
			cmdBuf->beginRenderPass(omniShadowFBOs[i]);
			cmdBuf->setCullMode(0);
			cmdBuf->bindPipeline(shadowOMNIPipeline);
			cmdBuf->bindDescriptorSets(shadowOMNIPipeline, camDescriptorSet, 0);
			cmdBuf->bindDescriptorSets(shadowOMNIPipeline, animDescriptorSet, 2);
			cmdBuf->bindDescriptorSets(shadowOMNIPipeline, morphDescriptorSet, 3);
			cmdBuf->bindDescriptorSets(shadowOMNIPipeline, descriptorSetOmniViews, 4);
			cmdBuf->bindDescriptorSets(shadowOMNIPipeline, descriptorSetOmniLight, 5);

			auto opaqueNodes = scene->getOpaqueEntities();
			for (auto&& [shaderName, renderQueue] : opaqueNodes)
			{
				for (auto e : renderQueue)
				{
					if (e->isActive())
					{
						auto r = e->getComponent<Renderable>();
						r->renderDepth(cmdBuf, shadowOMNIPipeline);
					}
				}
			}

			cmdBuf->endRenderPass();
			cmdBuf->end();

			omniCmdBufs.push_back(cmdBuf);
		}
	}
}