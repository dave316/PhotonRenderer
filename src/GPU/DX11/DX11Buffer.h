#ifndef INCLUDED_DX11BUFFER
#define INCLUDED_DX11BUFFER

#pragma once

#include <GPU/Buffer.h>
#include <GPU/DX11/DX11Platform.h>
#include <GPU/DX11/DX11Descriptor.h>

namespace DX11
{
	class Buffer : public GPU::Buffer
	{
	public:
		Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride);
		~Buffer();
		void uploadMapped(void* data);
		void uploadStaged(void* data);
		GPU::Descriptor::Ptr getDescriptor();
		uint8* getMappedPointer();
		ComPtr<ID3D11Buffer> getBuffer()
		{
			return buffer;
		}

    	typedef std::shared_ptr<Buffer> Ptr;
		static Ptr create(GPU::BufferUsage usage, uint32 size, uint32 stride)
		{
			return std::make_shared<Buffer>(usage, size, stride);
		}

	private:
		ComPtr<ID3D11Buffer> buffer;
		ComPtr<ID3D11DeviceContext> deviceContext;

		unsigned int id;
		static unsigned int globalIDCount;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	};
}

#endif // INCLUDED_DX11BUFFER