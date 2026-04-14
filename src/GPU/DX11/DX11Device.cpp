#include "DX11Device.h"
#include <iostream>
#include <dxgi1_6.h>
namespace DX11
{
	Device::Device()
	{

	}
	Device::~Device()
	{

	}

	void Device::init()
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		UINT flags = 0;
		//UINT flags = D3D11_CREATE_DEVICE_DEBUG;
		HRESULT result = D3D11CreateDevice(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&device,
			NULL,
			&deviceContext
		);
		if (FAILED(result)) {
			std::cout << "error creating DX11 device!" << std::endl;
		}
	}

	void Device::destroy()
	{
		std::cout << "Device ref. count: " << device.Reset() << std::endl;
		std::cout << "Device context ref. count: " << deviceContext.Reset() << std::endl;
	}

	ComPtr<ID3D11Device> Device::getDevice()
	{
		return device;
	}
	ComPtr<ID3D11DeviceContext> Device::getDeviceContext()
	{
		return deviceContext;
	}
}