#ifndef INCLUDED_DX11DEVICE
#define INCLUDED_DX11DEVICE

#pragma once

#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	class Device
	{
	public:
		Device();
		~Device();
		void init();
		void destroy();

		static Device& getInstance()
		{
			static Device instance;
			return instance;
		}

		ComPtr<ID3D11Device> getDevice();
		ComPtr<ID3D11DeviceContext> getDeviceContext();

	private:
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;
	};
}

#endif // INCLUDED_DX11DEVICE