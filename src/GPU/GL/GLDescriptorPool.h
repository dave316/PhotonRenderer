#ifndef INCLUDED_GLDESCRIPTORPOOL
#define INCLUDED_GLDESCRIPTORPOOL

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/GL/GLDescriptorSet.h>

namespace GL
{
	class DescriptorPool : public GPU::DescriptorPool
	{
	public:
		DescriptorPool();
		~DescriptorPool();
		void addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& bindings);
		GPU::DescriptorSet::Ptr createDescriptorSet(std::string name, uint32 count);
		std::vector<GPU::DescriptorSetLayoutBinding> getLayout(std::string name);

		typedef std::shared_ptr<DescriptorPool> Ptr;
		static Ptr create()
		{
			return std::make_shared<DescriptorPool>();
		}

	private:
		std::map<std::string, std::vector<GPU::DescriptorSetLayoutBinding>> setBindings;
	
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
	};
}

#endif // INCLUDED_GLDESCRIPTORPOOL