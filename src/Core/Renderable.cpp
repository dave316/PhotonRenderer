#include "Renderable.h"
#include <Graphics/GraphicsContext.h>
namespace pr
{
	Renderable::Renderable(pr::Mesh::Ptr mesh, RenderType type) : 
		mesh(mesh),
		type(type),
		priority(0)
	{
		if (mesh->hasMorphTargets())
			morphWeights = mesh->getWeights();
		
		auto& subMeshes = mesh->getSubMeshes();
		for (auto& s : subMeshes)
		{
			auto mat = s.material;
			if (mat->isTransmissive())
				this->type = RenderType::Transparent;
			if (mat->isTransparent())
				priority = 1;
		}

		//auto& ctx = GraphicsContext::getInstance();
		//modelUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(UniformData), 0);
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::setMesh(pr::Mesh::Ptr mesh)
	{
		this->mesh = mesh;

		auto& subMeshes = mesh->getSubMeshes();
		for (auto& s : subMeshes)
		{
			auto mat = s.material;
			if (mat->isTransmissive())
				type = RenderType::Transparent;
		}
	}

	void Renderable::setDescriptor(GPU::DescriptorPool::Ptr descriptorPool)
	{
		auto& ctx = GraphicsContext::getInstance();
		modelUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(UniformData), 0);

		descriptorSet = descriptorPool->createDescriptorSet("Model", 1);
		descriptorSet->addDescriptor(modelUBO->getDescriptor());
		descriptorSet->update();

		mesh->setDescriptor(descriptorPool);
	}

	void Renderable::update(glm::mat4 modelMatrix)
	{
		UniformData model;
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			model.M = glm::transpose(modelMatrix);
			model.N = glm::inverse(glm::mat3(modelMatrix));
		}
		else
		{
			model.M = modelMatrix;
			model.N = glm::inverseTranspose(glm::mat3(modelMatrix));
		}

		if (isSkinnedMesh())
			model.animMode = 1;
		else if (hasMorphtargets())
			model.animMode = 2;
		else
			model.animMode = 0;

		model.numMorphTargets = static_cast<int>(morphWeights.size());
		for (int i = 0; i < morphWeights.size(); i++)
			model.weights[i / 4][i % 4] = morphWeights[i];

		model.irradianceMode = diffuseMode;
		model.lightMapIndex = lightMapIndex;
		model.lightMapST = glm::vec4(lightMapOffset, lightMapScale);
		model.reflectionProbeIndex = specularProbeIndex;
		for (int i = 0; i < sh9.size(); i++)
			model.sh[i] = glm::vec4(sh9[i], 0.0f);

		modelUBO->uploadMapped(&model);
	}

	void Renderable::render(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		if (enabled)
		{
			cmdBuffer->bindDescriptorSets(pipeline, descriptorSet, 1);
			if (skin)
				skin->bind(cmdBuffer, pipeline);
			mesh->draw(cmdBuffer, pipeline);
		}
	}

	void Renderable::renderDepth(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		if (enabled)
		{
			cmdBuffer->bindDescriptorSets(pipeline, descriptorSet, 1);
			if (skin)
				skin->bind(cmdBuffer, pipeline);
			mesh->drawDepth(cmdBuffer, pipeline);
		}
	}

	void Renderable::setSkin(pr::Skin::Ptr skin)
	{
		this->skin = skin;
	}

	void Renderable::setType(RenderType type)
	{
		this->type = type;
	}

	void Renderable::setPriority(uint32 priority)
	{
		this->priority = priority;
	}

	void Renderable::setDiffuseMode(int mode)
	{
		diffuseMode = mode;
	}

	void Renderable::setLightMapIndex(int index)
	{
		lightMapIndex = index;
	}

	void Renderable::setLightMapST(glm::vec2 offsect, glm::vec2 scale)
	{
		lightMapOffset = offsect;
		lightMapScale = scale;
	}

	void Renderable::setReflectionProbe(std::string name, int index)
	{
		reflName = name;
		specularProbeIndex = index;
	}

	void Renderable::setProbeSH9(std::vector<glm::vec3>& sh9)
	{
		this->sh9 = sh9;
	}

	bool Renderable::isSkinnedMesh()
	{
		return (skin != nullptr);
	}

	bool Renderable::hasMorphtargets()
	{
		return Renderable::mesh->hasMorphTargets();
	}

	bool Renderable::isTransmissive() // TODO: this only works if all submeshes are transparent
	{
		return mesh->isTransmissive();
	}

	void Renderable::setCurrentWeights(std::vector<float> weights)
	{
		morphWeights = weights;
	}

	pr::Skin::Ptr Renderable::getSkin()
	{
		return skin;
	}

	Box Renderable::getBoundingBox()
	{
		return mesh->getBoundingBox();
	}

	pr::Mesh::Ptr Renderable::getMesh()
	{
		return mesh;
	}

	uint32 Renderable::getNumPrimitives()
	{
		return mesh->numPrimitives();
	}

	uint32 Renderable::getNumVariants()
	{
		return mesh->getNumVariants();
	}

	void Renderable::switchVariant(uint32 index)
	{
		mesh->switchVariant(index);
	}

	void Renderable::setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}

	std::string Renderable::getShaderName()
	{
		return mesh->getShaderName();
	}

	uint32 Renderable::getPriority()
	{
		return priority;
	}

	RenderType Renderable::getType()
	{
		return type;
	}

	glm::vec2 Renderable::getLMOffset()
	{
		return lightMapOffset;
	}

	glm::vec2 Renderable::getLMScale()
	{
		return lightMapScale;
	}

	int Renderable::getDiffuseMode()
	{
		return diffuseMode;
	}

	int Renderable::getLMIndex()
	{
		return lightMapIndex;
	}

	int Renderable::getRPIndex()
	{
		return specularProbeIndex;
	}

	std::string Renderable::getReflName()
	{
		return reflName;
	}
}
