#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"
#include <Graphics/Mesh.h>
#include <Graphics/Skin.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace pr
{
	enum class RenderType
	{
		Opaque,
		Transparent
	};

	class Renderable : public Component
	{
	public:
		Renderable(pr::Mesh::Ptr mesh, RenderType type = RenderType::Opaque);
		~Renderable();
		void setMesh(pr::Mesh::Ptr mesh);
		void setDescriptor(GPU::DescriptorPool::Ptr descriptorPool);
		void update(glm::mat4 modelMatrix);
		void render(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);
		void renderDepth(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);
		void setSkin(pr::Skin::Ptr skin);
		void setType(RenderType type);
		void setPriority(uint32 priority);
		void setDiffuseMode(int mode);
		void setLightMapIndex(int mode);
		void setLightMapST(glm::vec2 offsect, glm::vec2 scale);
		void setReflectionProbe(std::string name, int index);
		void setProbeSH9(std::vector<glm::vec3>& sh9);
		bool isSkinnedMesh();
		bool hasMorphtargets();
		bool isTransmissive();
		bool isEnabled() { return enabled; }
		void setCurrentWeights(std::vector<float> weights);
		pr::Skin::Ptr getSkin();
		Box getBoundingBox();
		pr::Mesh::Ptr getMesh();
		uint32 getNumPrimitives();
		uint32 getNumVariants();
		void switchVariant(uint32 index);
		void setEnabled(bool enabled);
		std::string getShaderName();
		RenderType getType();
		uint32 getPriority();
		glm::vec2 getLMOffset();
		glm::vec2 getLMScale();
		int getDiffuseMode();
		int getLMIndex();
		int getRPIndex();
		std::string getReflName();

		struct UniformData
		{
			glm::mat4 M;
			glm::mat4 N;
			glm::vec4 weights[2];
			int animMode = 0;
			int numMorphTargets = 0;
			int irradianceMode = 0;
			int lightMapIndex = -1;
			glm::vec4 lightMapST = glm::vec4(0);
			glm::vec4 sh[9];
			int reflectionProbeIndex = 0;
			int paddint[3];
		};


		typedef std::shared_ptr<Renderable> Ptr;
		static Ptr create(pr::Mesh::Ptr mesh)
		{
			return std::make_shared<Renderable>(mesh);
		}

	private:
		pr::Mesh::Ptr mesh = nullptr;
		pr::Skin::Ptr skin = nullptr;
		GPU::DescriptorSet::Ptr descriptorSet;
		GPU::Buffer::Ptr modelUBO = nullptr;
		std::vector<float> morphWeights;
		bool enabled = true;
		bool castShadow = true;
		bool receiveShadow = true;

		int diffuseMode = 0; // 0 - lightprobe(cubemap), 1 - lightprobe(SH), 2 - lightmap
		int lightMapIndex = -1;
		glm::vec2 lightMapOffset = glm::vec2(0);
		glm::vec2 lightMapScale = glm::vec2(0);
		std::string reflName = "";
		int specularProbeIndex = 0;
		std::vector<glm::vec3> sh9;

		RenderType type;
		uint32 priority;
	};
}

#endif // INCLUDED_RENDERABLE