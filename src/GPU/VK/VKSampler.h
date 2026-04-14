#ifndef INCLUDED_VKSAMPLER
#define INCLUDED_VKSAMPLER

#pragma once

#include <GPU/Sampler.h>
#include <Platform/Types.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	vk::SamplerAddressMode getAddressMode(GPU::AddressMode addressMode);
	vk::CompareOp getCompareOp(GPU::CompareOp op);

	class Sampler : public GPU::Sampler
	{
	public:
		Sampler(uint32 levels);
		~Sampler();
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void setCompareMode(bool enable);
		void setCompareOp(GPU::CompareOp op);
		vk::Sampler getSampler();

		typedef std::shared_ptr<Sampler> Ptr;
		static Ptr create(uint32 levels)
		{
			return std::make_shared<Sampler>(levels);
		}

	private:
		vk::Device device;
		vk::Sampler sampler;
		vk::SamplerCreateInfo samplerInfo;
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
	};
}

#endif // INCLUDED_VKSAMPLER