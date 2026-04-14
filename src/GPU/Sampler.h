#ifndef INCLUDED_SAMPLER
#define INCLUDED_SAMPLER

#pragma once

#include <memory>
#include <Platform/Types.h>

namespace GPU
{
	enum class Filter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapNearest,
		LinearMipmapLinear,
	};

	enum class AddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	enum class CompareOp
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	class Sampler
	{
	public:
		Sampler(uint32 levels) {}
		virtual ~Sampler() = 0 {}
		virtual void setAddressMode(AddressMode modeS, AddressMode modeT, AddressMode modeR) = 0;
		virtual void setAddressMode(AddressMode mode) = 0;
		virtual void setFilter(Filter minFilter, Filter magFilter) = 0;
		virtual void setCompareMode(bool enable) = 0;
		virtual void setCompareOp(CompareOp op) = 0;
		
		typedef std::shared_ptr<Sampler> Ptr;
	private:
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
	};
}

#endif // INCLUDED_SAMPLER