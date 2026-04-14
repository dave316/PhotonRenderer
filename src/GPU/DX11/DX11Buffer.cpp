#include "DX11Buffer.h"
#include "DX11Device.h"
#include "DX11Enums.h"
#include <iostream>
#include <string>

unsigned int DX11::Buffer::globalIDCount = 0;

namespace DX11
{
	D3D11_BIND_FLAG getBufferTarget(GPU::BufferUsage usage)
	{
		D3D11_BIND_FLAG target = D3D11_BIND_VERTEX_BUFFER;
		if (usage & GPU::BufferUsage::VertexBuffer)
			target = D3D11_BIND_VERTEX_BUFFER;
		else if (usage & GPU::BufferUsage::IndexBuffer)
			target = D3D11_BIND_INDEX_BUFFER;
		else if (usage & GPU::BufferUsage::UniformBuffer)
			target = D3D11_BIND_CONSTANT_BUFFER;
		return target;
	}

	Buffer::Buffer(GPU::BufferUsage usage, uint32 size, uint32 stride) :
		GPU::Buffer(usage, size, stride)
	{
		auto device = Device::getInstance().getDevice();
		deviceContext = Device::getInstance().getDeviceContext();

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = size;
		bufferDesc.BindFlags = getBufferTarget(usage);
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = stride;

		HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &buffer);
		if (FAILED(result))
		{
			std::cout << "error creatind DX11 buffer!" << std::endl;
		}

		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created buffer " << std::to_string(id) << std::endl;
	}

	Buffer::~Buffer()
	{
		//std::cout << "destroyed buffer " << std::to_string(id) << std::endl;
	}

	void Buffer::uploadMapped(void* data)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = deviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		std::memcpy(mappedResource.pData, data, size);
		deviceContext->Unmap(buffer.Get(), 0);
	}

	void Buffer::uploadStaged(void* data)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = deviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		std::memcpy(mappedResource.pData, data, size);
		deviceContext->Unmap(buffer.Get(), 0);
	}

	GPU::Descriptor::Ptr Buffer::getDescriptor()
	{
		return BufferDescriptor::create(buffer);
	}

	uint8* Buffer::getMappedPointer()
	{
		return nullptr;
	}
}