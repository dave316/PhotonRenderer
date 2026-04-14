#include "GLSampler.h"

namespace GL
{
	GLenum getWrap(GPU::AddressMode mode)
	{
		GLenum wrap = GL_REPEAT;
		switch (mode)
		{
			case GPU::AddressMode::Repeat:			wrap = GL_REPEAT; break;
			case GPU::AddressMode::MirroredRepeat:	wrap = GL_MIRRORED_REPEAT; break;
			case GPU::AddressMode::ClampToEdge:		wrap = GL_CLAMP_TO_EDGE; break;
			case GPU::AddressMode::ClampToBorder:	wrap = GL_CLAMP_TO_BORDER; break;
		}
		return wrap;
	}

	GLenum getFilter(GPU::Filter filter)
	{
		GLenum glFilter = GL_LINEAR;
		switch (filter)
		{
			case GPU::Filter::Nearest:				glFilter = GL_NEAREST; break;
			case GPU::Filter::Linear:				glFilter = GL_LINEAR; break;
			case GPU::Filter::NearestMipmapNearest:	glFilter = GL_NEAREST_MIPMAP_NEAREST; break;
			case GPU::Filter::NearestMipmapLinear:	glFilter = GL_NEAREST_MIPMAP_LINEAR; break;
			case GPU::Filter::LinearMipmapNearest:	glFilter = GL_LINEAR_MIPMAP_NEAREST; break;
			case GPU::Filter::LinearMipmapLinear:	glFilter = GL_LINEAR_MIPMAP_LINEAR; break;
		}
		return glFilter;
	}

	GLint getCompareFunc(GPU::CompareOp op)
	{
		GLint glFunc = GL_NEVER;
		switch (op)
		{
			case GPU::CompareOp::Never:				glFunc = GL_NEVER; break;
			case GPU::CompareOp::Less:				glFunc = GL_LESS; break;
			case GPU::CompareOp::Equal:				glFunc = GL_EQUAL; break;
			case GPU::CompareOp::LessOrEqual:		glFunc = GL_LEQUAL; break;
			case GPU::CompareOp::Greater:			glFunc = GL_GREATER; break;
			case GPU::CompareOp::NotEqual:			glFunc = GL_NOTEQUAL; break;
			case GPU::CompareOp::GreaterOrEqual:	glFunc = GL_GEQUAL; break;
			case GPU::CompareOp::Always:			glFunc = GL_ALWAYS; break;
		}
		return glFunc;
	}

	Sampler::Sampler(uint32 levels) : GPU::Sampler(levels)
	{
		glGenSamplers(1, &sampler);
	}

	Sampler::~Sampler()
	{
		glDeleteSamplers(1, &sampler);
	}

	void Sampler::setAddressMode(GPU::AddressMode modeS, GPU::AddressMode modeT, GPU::AddressMode modeR)
	{
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, getWrap(modeS));
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, getWrap(modeT));
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, getWrap(modeR));
	}

	void Sampler::setAddressMode(GPU::AddressMode mode)
	{
		setAddressMode(mode, mode, mode);
	}

	void Sampler::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, getFilter(minFilter));
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, getFilter(magFilter));
	}

	void Sampler::setCompareMode(bool enable)
	{
		if (enable)
			glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	}

	void Sampler::setCompareOp(GPU::CompareOp op)
	{
		glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, getCompareFunc(op));
	}

	GLuint Sampler::getSampler()
	{
		return sampler;
	}
}