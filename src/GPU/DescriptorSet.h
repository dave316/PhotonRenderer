#ifndef INCLUDED_DESCRIPTORSET
#define INCLUDED_DESCRIPTORSET

#pragma once

#include "Descriptor.h"
#include <memory>
#include <string>
#include <vector>

namespace GPU
{
	class DescriptorSet
	{
	public:
		DescriptorSet() {}
		virtual ~DescriptorSet() = 0 {}
		virtual void update() = 0;
		virtual void updateVariable() = 0;
		virtual void addDescriptor(Descriptor::Ptr descriptor) = 0;

		typedef std::shared_ptr<DescriptorSet> Ptr;
	protected:
		std::vector<Descriptor::Ptr> descriptors;
	private:

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}

#endif // INCLUDED_DESCRIPTORSET