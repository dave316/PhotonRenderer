#ifndef INCLUDED_GLDESCRIPTORSET
#define INCLUDED_GLDESCRIPTORSET

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/GL/GLDescriptor.h>

namespace GL
{
	class DescriptorSet : public GPU::DescriptorSet
	{
	public:
		DescriptorSet(std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb);
		~DescriptorSet();

		void update();
		void updateVariable();
		void addDescriptor(GPU::Descriptor::Ptr descriptor)
		{
			descriptors.push_back(descriptor);
		}
		void bind(std::vector<int> bindings);
		std::string getLayoutName();

		typedef std::shared_ptr<DescriptorSet> Ptr;
		static Ptr create(std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb)
		{
			return std::make_shared<DescriptorSet>(layoutName, dslb);
		}

	private:
		std::string layoutName;
		std::vector<GPU::DescriptorSetLayoutBinding> dslb;

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}

#endif // INCLUDED_GLDESCRIPTORSET
