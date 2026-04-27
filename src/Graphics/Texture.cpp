#include "Texture.h"

namespace pr
{
	Texture2D::Texture2D(uint32 width, uint32 height, GPU::Format format, uint32 levels)
	{
		auto& ctx = GraphicsContext::getInstance();

		params.type = GPU::ViewType::View2D;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = 1;
		params.levels = levels;
		params.usage = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;

		//image = ctx.createImage(params);
		//view = image->createImageView();
		//sampler = ctx.createSampler(levels);
	}

	Texture2D::Texture2D(uint32 width, uint32 height, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
	{
		auto& ctx = GraphicsContext::getInstance();

		params.type = GPU::ViewType::View2D;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = 1;
		params.levels = levels;
		params.usage = usage;

		//image = ctx.createImage(params);
		//view = image->createImageView();
		//sampler = ctx.createSampler(levels);
	}

	Texture2D::~Texture2D() {}

	void Texture2D::upload(uint8* data, uint32 size, uint32 level)
	{
		this->data = new uint8[size];
		this->size = size;
		std::memcpy(this->data, data, size);
		//auto& ctx = GraphicsContext::getInstance();
		//auto cmdBuf = ctx.allocateCommandBuffer();
		//image->uploadData(cmdBuf, data, size, 0, level);
	}

	void Texture2D::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		//sampler->setAddressMode(modeU, modeV, modeW);
		this->modeU = modeU;
		this->modeV = modeV;
		this->modeW = modeW;
	}

	void Texture2D::setAddressMode(GPU::AddressMode mode)
	{
		//sampler->setAddressMode(mode);
		setAddressMode(mode, mode, mode);
	}

	void Texture2D::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		//sampler->setFilter(minFilter, magFilter);
		this->minFilter = minFilter;
		this->magFilter = magFilter;
	}

	void Texture2D::generateMipmaps()
	{
		genMipmaps = true;
		//if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		//{
		//	auto dxImageView = std::dynamic_pointer_cast<DX11::ImageView>(view);
		//	dxImageView->generateMipmaps();
		//}
		//else
		//{
		//	auto& ctx = GraphicsContext::getInstance();
		//	auto cmdBuf = ctx.allocateCommandBuffer();
		//	cmdBuf->begin();
		//	image->generateMipmaps(cmdBuf);
		//	cmdBuf->end();
		//	cmdBuf->flush();
		//}
	}

	void Texture2D::setLayout()
	{
		image->setImageLayout();
	}

	void Texture2D::setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionShader(cmdBuf);
	}

	void Texture2D::setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionStorage(cmdBuf);
	}

	GPU::Image::Ptr Texture2D::getImage()
	{
		return image;
	}

	GPU::ImageView::Ptr Texture2D::getImageView()
	{
		return view;
	}

	GPU::ImageDescriptor::Ptr Texture2D::getDescriptor()
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	GPU::ImageDescriptor::Ptr Texture2D::getDescriptor(GPU::ImageView::Ptr view)
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	void Texture2D::createData()
	{
		auto& ctx = GraphicsContext::getInstance();
		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(params.levels);

		loadedOnGPU = true;
	}

	void Texture2D::uploadData()
	{
		auto& ctx = GraphicsContext::getInstance();

		if (data != nullptr)
		{
			auto cmdBuf = ctx.allocateCommandBuffer();
			image->uploadData(cmdBuf, data, size, 0, 0);
		}

		if (genMipmaps)
		{
#ifdef GPU_BACKEND_DX11
			if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
			{
				auto dxImageView = std::dynamic_pointer_cast<DX11::ImageView>(view);
				dxImageView->generateMipmaps();
			}
			else
#endif
			{
				auto& ctx = GraphicsContext::getInstance();
				auto cmdBuf = ctx.allocateCommandBuffer();
				cmdBuf->begin();
				image->generateMipmaps(cmdBuf);
				cmdBuf->end();
				cmdBuf->flush();
			}
		}

		sampler->setFilter(minFilter, magFilter);
		sampler->setAddressMode(modeU, modeV, modeW);
	}

	void Texture2D::destroyData()
	{
		loadedOnGPU = false;
	}

	Texture3D::Texture3D(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::View3D;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, depth);
		params.layers = 1;
		params.levels = levels;
		params.usage = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	Texture3D::Texture3D(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::View3D;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, depth);
		params.layers = 1;
		params.levels = levels;
		params.usage = usage;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	Texture3D::~Texture3D()
	{

	}

	void Texture3D::upload(uint8* data, uint32 size, uint32 level)
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		image->uploadArray(cmdBuf, data, size);
	}

	void Texture3D::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		sampler->setAddressMode(modeU, modeV, modeW);
	}

	void Texture3D::setAddressMode(GPU::AddressMode mode)
	{
		sampler->setAddressMode(mode);
	}

	void Texture3D::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		sampler->setFilter(minFilter, magFilter);
	}

	void Texture3D::generateMipmaps()
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		image->generateMipmaps(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();
	}

	void Texture3D::setLayout()
	{
		image->setImageLayout();
	}

	void Texture3D::setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionShader(cmdBuf);
	}

	void Texture3D::setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionStorage(cmdBuf);
	}

	GPU::Image::Ptr Texture3D::getImage()
	{
		return image;
	}

	GPU::ImageView::Ptr Texture3D::getImageView()
	{
		return view;
	}

	GPU::ImageDescriptor::Ptr Texture3D::getDescriptor()
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	GPU::ImageDescriptor::Ptr Texture3D::getDescriptor(GPU::ImageView::Ptr view)
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	void Texture3D::createData()
	{

	}

	void Texture3D::uploadData()
	{

	}

	void Texture3D::destroyData()
	{

	}

	Texture2DArray::Texture2DArray(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::View2DArray;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = layers;
		params.levels = levels;
		params.usage = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	Texture2DArray::Texture2DArray(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::View2DArray;
		params.format = format;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = layers;
		params.levels = levels;
		params.usage = usage;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	Texture2DArray::~Texture2DArray() {}

	void Texture2DArray::upload(uint8* data, uint32 size)
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		image->uploadArray(cmdBuf, data, size);
	}

	void Texture2DArray::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		sampler->setAddressMode(modeU, modeV, modeW);
	}

	void Texture2DArray::setAddressMode(GPU::AddressMode mode)
	{
		sampler->setAddressMode(mode);
	}

	void Texture2DArray::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		sampler->setFilter(minFilter, magFilter);
	}

	void Texture2DArray::generateMipmaps()
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		image->generateMipmaps(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();
	}

	void Texture2DArray::setLayout()
	{
		image->setImageLayout();
	}

	void Texture2DArray::setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionShader(cmdBuf);
	}

	void Texture2DArray::setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{
		image->layoutTransitionStorage(cmdBuf);
	}

	GPU::Image::Ptr Texture2DArray::getImage()
	{
		return image;
	}

	GPU::ImageView::Ptr Texture2DArray::getImageView()
	{
		return view;
	}

	GPU::ImageDescriptor::Ptr Texture2DArray::getDescriptor()
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	void Texture2DArray::createData()
	{

	}

	void Texture2DArray::uploadData()
	{

	}

	void Texture2DArray::destroyData()
	{

	}

	TextureCubeMap::TextureCubeMap(uint32 size, GPU::Format format, uint32 levels) :
		size(size)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::ViewCubeMap;
		params.format = format;
		params.extent = GPU::Extent3D(size, size, 1);
		params.layers = 6;
		params.levels = levels;
		params.usage = GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	TextureCubeMap::TextureCubeMap(uint32 size, GPU::Format format, uint32 levels, GPU::ImageUsage usage) :
		size(size)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::ViewCubeMap;
		params.format = format;
		params.extent = GPU::Extent3D(size, size, 1);
		params.layers = 6;
		params.levels = levels;
		params.usage = usage;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	TextureCubeMap::~TextureCubeMap() {}

	void TextureCubeMap::upload(uint8* data, uint32 size, uint32 face, uint32 level)
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		image->uploadData(cmdBuf, data, size, face, level);
	}

	void TextureCubeMap::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		sampler->setAddressMode(modeU, modeV, modeW);
	}

	void TextureCubeMap::setAddressMode(GPU::AddressMode mode)
	{
		sampler->setAddressMode(mode);
	}

	void TextureCubeMap::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		sampler->setFilter(minFilter, magFilter);
	}

	void TextureCubeMap::setCompareMode()
	{
		sampler->setCompareMode(true);
		sampler->setCompareOp(GPU::CompareOp::LessOrEqual);
	}

	void TextureCubeMap::generateMipmaps()
	{
#ifdef GPU_BACKEND_DX11
		if (pr::GraphicsContext::getInstance().getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
		{
			auto dxImageView = std::dynamic_pointer_cast<DX11::ImageView>(view);
			dxImageView->generateMipmaps();
		}
		else
#endif
		{
			auto& ctx = GraphicsContext::getInstance();
			auto cmdBuf = ctx.allocateCommandBuffer();
			cmdBuf->begin();
			image->generateMipmaps(cmdBuf);
			cmdBuf->end();
			cmdBuf->flush();
		}
	}

	void TextureCubeMap::setLayout()
	{
		image->setImageLayout();
	}

	GPU::Image::Ptr TextureCubeMap::getImage()
	{
		return image;
	}

	GPU::ImageView::Ptr TextureCubeMap::getImageView()
	{
		return view;
	}

	GPU::ImageDescriptor::Ptr TextureCubeMap::getDescriptor()
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	void TextureCubeMap::createData()
	{

	}

	void TextureCubeMap::uploadData()
	{

	}

	void TextureCubeMap::destroyData()
	{

	}

	TextureCubeMapArray::TextureCubeMapArray(uint32 size, uint32 layers, GPU::Format format, uint32 levels)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::ViewCubeMapArray;
		params.format = format;
		params.extent = GPU::Extent3D(size, size, 1);
		params.layers = 6 * layers;
		params.levels = levels;
		params.usage = GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	TextureCubeMapArray::TextureCubeMapArray(uint32 size, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
	{
		auto& ctx = GraphicsContext::getInstance();

		GPU::ImageParameters params;
		params.type = GPU::ViewType::ViewCubeMapArray;
		params.format = format;
		params.extent = GPU::Extent3D(size, size, 1);
		params.layers = 6 * layers;
		params.levels = levels;
		params.usage = usage;

		image = ctx.createImage(params);
		view = image->createImageView();
		sampler = ctx.createSampler(levels);
	}

	TextureCubeMapArray::~TextureCubeMapArray()
	{

	}

	void TextureCubeMapArray::upload(uint8* data, uint32 size, uint32 face, uint32 level)
	{

	}

	void TextureCubeMapArray::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		sampler->setAddressMode(modeU, modeV, modeW);
	}

	void TextureCubeMapArray::setAddressMode(GPU::AddressMode mode)
	{
		sampler->setAddressMode(mode);
	}

	void TextureCubeMapArray::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		sampler->setFilter(minFilter, magFilter);
	}

	void TextureCubeMapArray::setCompareMode()
	{
		sampler->setCompareMode(true);
		sampler->setCompareOp(GPU::CompareOp::LessOrEqual);
	}

	void TextureCubeMapArray::generateMipmaps()
	{
		auto& ctx = GraphicsContext::getInstance();
		auto cmdBuf = ctx.allocateCommandBuffer();
		cmdBuf->begin();
		image->generateMipmaps(cmdBuf);
		cmdBuf->end();
		cmdBuf->flush();
	}

	void TextureCubeMapArray::setLayout()
	{
		image->setImageLayout();
	}

	GPU::Image::Ptr TextureCubeMapArray::getImage()
	{
		return image;
	}

	GPU::ImageView::Ptr TextureCubeMapArray::getImageView()
	{
		return view;
	}

	GPU::ImageDescriptor::Ptr TextureCubeMapArray::getDescriptor()
	{
		auto& ctx = GraphicsContext::getInstance();
		return ctx.createImageDescriptor(image, view, sampler);
	}

	void TextureCubeMapArray::createData()
	{

	}

	void TextureCubeMapArray::uploadData()
	{

	}

	void TextureCubeMapArray::destroyData()
	{

	}
}