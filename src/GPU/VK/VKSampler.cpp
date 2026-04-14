#include "VKSampler.h"
#include "VKDevice.h"
namespace VK
{
	vk::SamplerAddressMode getAddressMode(GPU::AddressMode addressMode)
	{
		vk::SamplerAddressMode samplerAddressMode;
		switch (addressMode)
		{
			case GPU::AddressMode::Repeat:			samplerAddressMode = vk::SamplerAddressMode::eRepeat; break;
			case GPU::AddressMode::MirroredRepeat:	samplerAddressMode = vk::SamplerAddressMode::eMirroredRepeat; break;
			case GPU::AddressMode::ClampToEdge:		samplerAddressMode = vk::SamplerAddressMode::eClampToEdge; break;
			case GPU::AddressMode::ClampToBorder:	samplerAddressMode = vk::SamplerAddressMode::eClampToBorder; break;
		}
		return samplerAddressMode;
	}

	vk::CompareOp getCompareOp(GPU::CompareOp op)
	{
		vk::CompareOp compareOp;
		switch (op)
		{
			case GPU::CompareOp::Never:				compareOp = vk::CompareOp::eNever; break;
			case GPU::CompareOp::Less:				compareOp = vk::CompareOp::eLess; break;
			case GPU::CompareOp::Equal:				compareOp = vk::CompareOp::eEqual; break;
			case GPU::CompareOp::LessOrEqual:		compareOp = vk::CompareOp::eLessOrEqual; break;
			case GPU::CompareOp::Greater:			compareOp = vk::CompareOp::eGreater; break;
			case GPU::CompareOp::NotEqual:			compareOp = vk::CompareOp::eNotEqual; break;
			case GPU::CompareOp::GreaterOrEqual:	compareOp = vk::CompareOp::eGreaterOrEqual; break;
			case GPU::CompareOp::Always:			compareOp = vk::CompareOp::eAlways; break;
		}
		return compareOp;
	}

	Sampler::Sampler(uint32 levels) :
		device(Device::getInstance().getDevice()),
		GPU::Sampler(levels)
	{
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = vk::CompareOp::eNever;
		samplerInfo.compareEnable = false;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(levels);
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.anisotropyEnable = false;
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		sampler = device.createSampler(samplerInfo);
	}

	Sampler::~Sampler()
	{
		device.destroySampler(sampler);
	}

	void Sampler::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		samplerInfo.addressModeU = getAddressMode(modeU);
		samplerInfo.addressModeV = getAddressMode(modeV);
		samplerInfo.addressModeW = getAddressMode(modeW);

		device.destroySampler(sampler);
		sampler = device.createSampler(samplerInfo);
	}

	void Sampler::setAddressMode(GPU::AddressMode mode)
	{
		setAddressMode(mode, mode, mode);
	}

	void Sampler::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		if (magFilter == GPU::Filter::Linear)
			samplerInfo.magFilter = vk::Filter::eLinear;
		else if (magFilter == GPU::Filter::Nearest)
			samplerInfo.magFilter = vk::Filter::eNearest;

		if (minFilter == GPU::Filter::Linear ||
			minFilter == GPU::Filter::LinearMipmapNearest ||
			minFilter == GPU::Filter::LinearMipmapLinear)
			samplerInfo.minFilter = vk::Filter::eLinear;
		else if (minFilter == GPU::Filter::Nearest ||
			minFilter == GPU::Filter::NearestMipmapNearest ||
			minFilter == GPU::Filter::NearestMipmapLinear)
			samplerInfo.minFilter = vk::Filter::eNearest;

		if (minFilter == GPU::Filter::NearestMipmapNearest ||
			minFilter == GPU::Filter::LinearMipmapNearest)
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		else if (minFilter == GPU::Filter::NearestMipmapLinear ||
			minFilter == GPU::Filter::LinearMipmapLinear)
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

		device.destroySampler(sampler);
		sampler = device.createSampler(samplerInfo);
	}

	void Sampler::setCompareMode(bool enable)
	{
		samplerInfo.compareEnable = enable;

		device.destroySampler(sampler);
		sampler = device.createSampler(samplerInfo);
	}

	void Sampler::setCompareOp(GPU::CompareOp op)
	{
		samplerInfo.compareOp = getCompareOp(op);

		device.destroySampler(sampler);
		sampler = device.createSampler(samplerInfo);
	}

	vk::Sampler Sampler::getSampler()
	{
		return sampler;
	}
}
