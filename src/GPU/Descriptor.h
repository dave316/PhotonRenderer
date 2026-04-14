#ifndef INCLUDED_DESCRIPTOR
#define INCLUDED_DESCRIPTOR

#pragma once

#include <memory>

namespace GPU
{
	class Descriptor
	{
	public:
		Descriptor() {}
		virtual ~Descriptor() = 0 {}
		typedef std::shared_ptr<Descriptor> Ptr;
	};

	class BufferDescriptor : public Descriptor
	{
	public:
		BufferDescriptor() {}
		virtual ~BufferDescriptor() = 0 {}
		typedef std::shared_ptr<BufferDescriptor> Ptr;
	};

	class ImageDescriptor : public Descriptor
	{
	public:
		ImageDescriptor() {}
		virtual ~ImageDescriptor() = 0 {}
		typedef std::shared_ptr<ImageDescriptor> Ptr;
	};

	class ImageDescriptorArray : public Descriptor
	{
	public:
		ImageDescriptorArray() {}
		virtual ~ImageDescriptorArray() = 0 {}
		typedef std::shared_ptr<ImageDescriptorArray> Ptr;
	};
}

#endif // INCLUDED_DESCRIPTOR