#ifndef INCLUDED_DX11FRAMEBUFFER
#define INCLUDED_DX11FRAMEBUFFER

#pragma once

#include <GPU/Framebuffer.h>
#include <GPU/DX11/DX11ImageView.h>
#include <GPU/DX11/DX11Image.h>
#include <GPU/DX11/DX11Platform.h>
#include <Platform/Types.h>
#include <vector>
namespace DX11
{
	class Framebuffer : public GPU::Framebuffer
	{
	public:
		Framebuffer(uint32 width, uint32 height, bool clear);
		~Framebuffer();
		void addAttachment(GPU::Image::Ptr image);
		void addAttachment(GPU::ImageView::Ptr imageView);
		void addAttachment(ComPtr<ID3D11Texture2D> texture);
		void createFramebuffer();
		void bindRenderTarget();
		void unbindRenderTarget();
		void clearRenderTarget();
		void setClearColor(glm::vec4 color);
		uint32 getWidth();
		uint32 getHeight();
		glm::vec4 getClearColor();
		bool clearOnLoad() { return clear; }

		typedef std::shared_ptr<Framebuffer> Ptr;
		static Ptr create(uint32 width, uint32 height, bool clear)
		{
			return std::make_shared<Framebuffer>(width, height, clear);
		}

	private:
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;
		std::vector<ID3D11RenderTargetView*> renderTargets;
		ComPtr<ID3D11DepthStencilView> depthStencilView;

		uint32 width;
		uint32 height;
		glm::vec4 clearColor = glm::vec4(0,0,0,1);
		bool clear = true;

		unsigned int id;
		static unsigned int globalIDCount;

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
	};
}

#endif // INCLUDED_DX11FRAMEBUFFER