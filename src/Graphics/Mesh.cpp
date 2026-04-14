#include "Mesh.h"

namespace pr
{
	Mesh::Mesh(const std::string& name) : name(name)
	{
	}

	void Mesh::addVariant(std::string variant)
	{
		variants.push_back(variant);
	}

	void Mesh::addSubMesh(SubMesh subMesh)
	{
		subMeshes.push_back(subMesh);
	}

	void Mesh::setMorphWeights(std::vector<float>& weights)
	{
		this->weights = weights;
	}

	void Mesh::flipWindingOrder()
	{
		for (auto subMesh : subMeshes)
			subMesh.primitive->flipWindingOrder();
	}

	void Mesh::draw(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		for (auto subMesh : subMeshes)
		{
			auto mat = subMesh.material;

			// TODO: There is a problem when primitives have different materials because
			// now the shader is set for the whole mesh! It would be better to extract
			// the primitives/materials and group/sort according to shader/material!
			//if (pipeline->getPipelineName().compare(mat->getShaderName()) == 0)
			//if (mat)
			{
				if (mat->isDoubleSided())
					cmdBuffer->setCullMode(0);

				mat->bindMainMat(cmdBuffer, pipeline);
				subMesh.primitive->bind(cmdBuffer, pipeline);
				subMesh.primitive->draw(cmdBuffer);

				if (mat->isDoubleSided())
					cmdBuffer->setCullMode(2);
			}
		}
	}

	void Mesh::drawDepth(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		for (auto subMesh : subMeshes)
		{
			auto mat = subMesh.material;

			if (mat->isDoubleSided())
				cmdBuffer->setCullMode(0);

			mat->bindShadowMat(cmdBuffer, pipeline);
			subMesh.primitive->bind(cmdBuffer, pipeline);
			subMesh.primitive->draw(cmdBuffer);

			if (mat->isDoubleSided())
				cmdBuffer->setCullMode(2);
		}
	}

	void Mesh::setDescriptor(GPU::DescriptorPool::Ptr descriptorPool)
	{
		for (auto subMesh : subMeshes)
		{
			subMesh.primitive->update(descriptorPool);
			if (subMesh.material)
				subMesh.material->update(descriptorPool);

			for (auto v : subMesh.variants)
				v->update(descriptorPool);
		}
	}

	void Mesh::setMaterial(unsigned int index, Material::Ptr material)
	{
		if (index < subMeshes.size())
			subMeshes[index].material = material;
	}

	bool Mesh::hasMorphTargets()
	{
		return !weights.empty();
	}

	bool Mesh::isTransmissive()
	{
		for (auto subMesh : subMeshes)
		{
			if (subMesh.material->isTransmissive())
				return true;
		}
		return false;
	}

	std::vector<float> Mesh::getWeights()
	{
		return weights;
	}

	std::vector<std::string> Mesh::getVariants()
	{
		return variants;
	}

	Box Mesh::getBoundingBox()
	{
		Box boundingBox;
		for (auto subMesh : subMeshes)
			boundingBox.expand(subMesh.primitive->getBoundingBox());
		return boundingBox;
	}

	uint32 Mesh::numPrimitives()
	{
		return static_cast<uint32>(subMeshes.size());
	}

	uint32 Mesh::getNumVariants()
	{
		return static_cast<uint32>(subMeshes[0].variants.size());
	}

	void Mesh::switchVariant(uint32 index)
	{
		for (auto& subMesh : subMeshes)
		{
			if (index < subMesh.variants.size())
				subMesh.material = subMesh.variants[index];
		}
	}

	std::string Mesh::getShaderName()
	{
		return subMeshes[0].material->getShaderName();
	}
}