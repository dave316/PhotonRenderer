#ifndef INCLUDED_SKIN
#define INCLUDED_SKIN

#pragma once

#include <Core/Entity.h>
#include <Graphics/Texture.h>
#include <Platform/Types.h>
#include <glm/gtc/matrix_inverse.hpp>

namespace pr
{
	struct AnimData
	{
		glm::mat4 joints[32];
		glm::mat4 normals[32];

		AnimData()
		{
			for (int i = 0; i < 32; i++)
			{
				joints[i] = glm::mat4(1.0f);
				normals[i] = glm::mat4(1.0f);
			}
		}
	};

	class Skin
	{
	public:
		Skin(const std::string& name);

		void setDescriptor(GPU::DescriptorPool::Ptr descriptorPool);
		void setSkeleton(uint32 index);
		void addJoint(uint32 index, glm::mat4 ibm);

		// computes the current joint transformations
		void computeJoints(std::vector<Entity::Ptr>& nodes);
		void bind(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);

		typedef std::shared_ptr<Skin> Ptr;
		static Ptr create(const std::string& name)
		{
			return std::make_shared<Skin>(name);
		}

	private:
		std::string name;
		std::vector<uint32> joints;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<glm::mat4> jointMatrices;
		std::vector<glm::mat3> normalMatrices;
		uint32 skeleton;

		GPU::Buffer::Ptr animUBO;
		GPU::DescriptorSet::Ptr descriptorSet;

		Skin(const Skin&) = delete;
		Skin& operator=(const Skin&) = delete;
	};
}

#endif // INCLUDED_SKIN