#include "Skin.h"

namespace pr
{
	Skin::Skin(const std::string& name) : 
		name(name), 
		skeleton(0)
	{
		auto& ctx = GraphicsContext::getInstance();
		animUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, sizeof(AnimData), 0);
	}

	void Skin::setDescriptor(GPU::DescriptorPool::Ptr descriptorPool)
	{
		descriptorSet = descriptorPool->createDescriptorSet("Animation", 1);
		descriptorSet->addDescriptor(animUBO->getDescriptor());
		descriptorSet->update();
	}

	void Skin::setSkeleton(uint32 index)
	{
		skeleton = index;
	}

	void Skin::addJoint(uint32 index, glm::mat4 ibm)
	{
		joints.push_back(index);
		inverseBindMatrices.push_back(ibm);
	}

	// computes the current joint transformations
	void Skin::computeJoints(std::vector<Entity::Ptr>& nodes)
	{
		auto& ctx = GraphicsContext::getInstance();

		jointMatrices.clear();
		normalMatrices.clear();

		auto parentNode = nodes[skeleton];

		for (uint32 i = 0; i < joints.size(); i++)
		{
			uint32 jointIndex = joints[i];
			auto node = nodes[jointIndex];
			glm::mat4 jointIBM = inverseBindMatrices[i];
			glm::mat4 nodeLocalToWorld = node->getComponent<Transform>()->getTransform();
			glm::mat4 parentWorldToLocal = glm::inverse(parentNode->getComponent<Transform>()->getTransform());
			glm::mat4 jointMatrix = nodeLocalToWorld * jointIBM;
			jointMatrix = parentWorldToLocal * jointMatrix;
			glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(jointMatrix));

			if (ctx.getCurrentAPI() == GraphicsAPI::Direct3D11)
			{
				jointMatrices.push_back(glm::transpose(jointMatrix));
				normalMatrices.push_back(glm::transpose(normalMatrix));
			}
			else
			{
				jointMatrices.push_back(jointMatrix);
				normalMatrices.push_back(normalMatrix);
			}
		}

		AnimData animData;
		for (int i = 0; i < jointMatrices.size(); i++)
		{
			animData.joints[i] = jointMatrices[i];
			animData.normals[i] = normalMatrices[i];
		}
		animUBO->uploadMapped(&animData);
	}

	void Skin::bind(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		cmdBuffer->bindDescriptorSets(pipeline, descriptorSet, 2);
	}
}