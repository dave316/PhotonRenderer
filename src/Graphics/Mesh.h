#ifndef INCLUDED_MESH
#define INCLUDED_MESH

#pragma once

#include "Material.h"
#include "Primitive.h"

namespace pr
{
	struct SubMesh
	{
		Primitive::Ptr primitive;
		Material::Ptr material;
		std::vector<Material::Ptr> variants;
	};

	class Asset
	{
		std::string name;
		std::string uri;
		std::string type; // image, shader, material, mesh,
	};

	class Mesh
	{
	public:
		Mesh(const std::string& name);
		void addSubMesh(SubMesh subMesh);
		void addVariant(std::string name);
		void setMorphWeights(std::vector<float>& weights);
		void flipWindingOrder();
		void draw(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);
		void drawDepth(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);
		void setDescriptor(GPU::DescriptorPool::Ptr descriptorPool);
		void setMaterial(unsigned int index, pr::Material::Ptr material);
		bool hasMorphTargets();
		bool isTransmissive();
		std::vector<float> getWeights();
		std::vector<std::string> getVariants();
		Box getBoundingBox();
		uint32 numPrimitives();
		uint32 getNumVariants();
		void switchVariant(uint32 index);
		std::string& getName() {
			return name;
		}
		std::string getShaderName();
		std::vector<SubMesh>& getSubMeshes()
		{
			return subMeshes;
		}

		typedef std::shared_ptr<Mesh> Ptr;
		static Ptr create(const std::string& name)
		{
			return std::make_shared<Mesh>(name);
		}

	private:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		std::string name;
		std::vector<std::string> variants;
		std::vector<SubMesh> subMeshes;
		std::vector<float> weights;
	};
}

#endif // INCLUDED_PRIMITIVE