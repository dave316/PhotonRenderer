#ifndef INCLUDED_GLSAMPLER
#define INCLUDED_GLSAMPLER

#pragma once

#include <GPU/Sampler.h>
#include <GPU/GL/GLPlatform.h>

namespace GL
{
	GLenum getWrap(GPU::AddressMode mode);
	GLenum getFilter(GPU::Filter filter);
	GLint getCompareFunc(GPU::CompareOp op);

	class Sampler : public GPU::Sampler
	{
	public:
		Sampler(uint32 levels);
		~Sampler();
		void setAddressMode(GPU::AddressMode modeS, GPU::AddressMode modeT, GPU::AddressMode modeR);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void setCompareMode(bool enable);
		void setCompareOp(GPU::CompareOp op);
		GLuint getSampler();

		typedef std::shared_ptr<Sampler> Ptr;
		static Ptr create(uint32 levels)
		{
			return std::make_shared<Sampler>(levels);
		}
	private:
		GLuint sampler;
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
	};
}

#endif // INCLUDED_GLSAMPLER