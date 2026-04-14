#include "GLDescriptorSet.h"
#include <iostream>
namespace GL
{
	DescriptorSet::DescriptorSet(std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb) :
		layoutName(layoutName),
		dslb(dslb)
	{
	}

	DescriptorSet::~DescriptorSet()
	{}

	void DescriptorSet::update()
	{

	}

	void DescriptorSet::updateVariable()
	{

	}

	void DescriptorSet::bind(std::vector<int> bindings)
	{
		for (int i = 0; i < descriptors.size(); i++)
		{
			auto desc = descriptors[i];
			GPU::DescriptorSetLayoutBinding layoutBinding;
			if (i < dslb.size())
				layoutBinding = dslb[i];
			else
				layoutBinding = dslb[dslb.size() - 1]; // TODO: Quick fix for array descriptors...
			if (std::dynamic_pointer_cast<BufferDescriptor>(desc))
			{
				auto glBufferDesc = std::dynamic_pointer_cast<BufferDescriptor>(desc);
				GLuint buffer = glBufferDesc->getBuffer();
				glBindBufferBase(GL_UNIFORM_BUFFER, bindings[i], buffer);
			}
			if (std::dynamic_pointer_cast<ImageDescriptor>(desc))
			{
				if (layoutBinding.descriptorType == GPU::DescriptorType::CombinedImageSampler)
				{
					auto glImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(desc);
					GLenum target = glImageDesc->getTarget();
					GLuint texture = glImageDesc->getTexture();
					GLuint sampler = glImageDesc->getSampler();
					glActiveTexture(GL_TEXTURE0 + bindings[i]);
					glBindTexture(target, texture);
					glBindSampler(bindings[i], sampler);
				}
				else
				{
					auto glImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(desc);
					GLenum target = glImageDesc->getTarget();
					GLuint texture = glImageDesc->getTexture();
					GLuint sampler = glImageDesc->getSampler();
					glActiveTexture(GL_TEXTURE0 + bindings[i]);
					glBindTexture(target, texture);
					glBindImageTexture(bindings[i], texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
					//std::cout << "unknow image descriptor type: " << (int)layoutBinding.descriptorType << std::endl;
				}
			}
		}
	}

	std::string DescriptorSet::getLayoutName()
	{
		return layoutName;
	}
}