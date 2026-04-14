#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#pragma once

#include "GraphicsContext.h"

namespace pr
{
	class Texture : public GraphicsRessource
	{
	public:
		Texture() {}
		virtual ~Texture() = 0 {}
		typedef std::shared_ptr<Texture> Ptr;
	protected:
		GPU::ImageParameters params;
		GPU::Image::Ptr image;
		GPU::ImageView::Ptr view;
		GPU::Sampler::Ptr sampler;
		GPU::Filter minFilter;
		GPU::Filter magFilter;
		GPU::AddressMode modeU;
		GPU::AddressMode modeV;
		GPU::AddressMode modeW;
		bool loadedOnGPU = false;
		bool genMipmaps = false;
		uint8* data = nullptr;
		uint32 size;
	private:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(uint32 width, uint32 height, GPU::Format format, uint32 levels);
		Texture2D(uint32 width, uint32 height, GPU::Format format, uint32 levels, GPU::ImageUsage usage);
		~Texture2D();

		void upload(uint8* data, uint32 size, uint32 level = 0);
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void generateMipmaps();
		void setLayout();
		void setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf);
		void setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::Image::Ptr getImage();
		GPU::ImageView::Ptr getImageView();
		GPU::ImageDescriptor::Ptr getDescriptor();
		GPU::ImageDescriptor::Ptr getDescriptor(GPU::ImageView::Ptr view);

		void createData();
		void uploadData();
		void destroyData();
		bool isLoadedOnGPU() { return loadedOnGPU; }

		typedef std::shared_ptr<Texture2D> Ptr;
		static Ptr create(uint32 width, uint32 height, GPU::Format format, uint32 levels = 1)
		{
			return std::make_shared<Texture2D>(width, height, format, levels);
		}
		static Ptr create(uint32 width, uint32 height, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
		{
			return std::make_shared<Texture2D>(width, height, format, levels, usage);
		}

	private:

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
	};

	class Texture3D : public Texture
	{
	public:
		Texture3D(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels);
		Texture3D(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels, GPU::ImageUsage usage);
		~Texture3D();

		void upload(uint8* data, uint32 size, uint32 level = 0);
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void generateMipmaps();
		void setLayout();
		void setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf);
		void setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::Image::Ptr getImage();
		GPU::ImageView::Ptr getImageView();
		GPU::ImageDescriptor::Ptr getDescriptor();
		GPU::ImageDescriptor::Ptr getDescriptor(GPU::ImageView::Ptr view);

		void createData();
		void uploadData();
		void destroyData();

		typedef std::shared_ptr<Texture3D> Ptr;
		static Ptr create(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels = 1)
		{
			return std::make_shared<Texture3D>(width, height, depth, format, levels);
		}
		static Ptr create(uint32 width, uint32 height, uint32 depth, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
		{
			return std::make_shared<Texture3D>(width, height, depth, format, levels, usage);
		}

	private:

		Texture3D(const Texture3D&) = delete;
		Texture3D& operator=(const Texture3D&) = delete;
	};

	class Texture2DArray : public Texture
	{
	public:
		Texture2DArray(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels);
		Texture2DArray(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage);
		~Texture2DArray();

		void upload(uint8* data, uint32 size);
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void generateMipmaps();
		void setLayout();
		void setLayoutShader(GPU::CommandBuffer::Ptr cmdBuf);
		void setLayoutStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::Image::Ptr getImage();
		GPU::ImageView::Ptr getImageView();
		GPU::ImageDescriptor::Ptr getDescriptor();

		void createData();
		void uploadData();
		void destroyData();

		typedef std::shared_ptr<Texture2DArray> Ptr;
		static Ptr create(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels = 1)
		{
			return std::make_shared<Texture2DArray>(width, height, layers, format, levels);
		}
		static Ptr create(uint32 width, uint32 height, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
		{
			return std::make_shared<Texture2DArray>(width, height, layers, format, levels, usage);
		}

	private:

		Texture2DArray(const Texture2DArray&) = delete;
		Texture2DArray& operator=(const Texture2DArray&) = delete;
	};

	class TextureCubeMap : public Texture
	{
	public:
		TextureCubeMap(uint32 size, GPU::Format format, uint32 levels);
		TextureCubeMap(uint32 size, GPU::Format format, uint32 levels, GPU::ImageUsage usage);
		~TextureCubeMap();

		void upload(uint8* data, uint32 size, uint32 face, uint32 level = 0);
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void setCompareMode();
		void generateMipmaps();
		void setLayout();
		GPU::Image::Ptr getImage();
		GPU::ImageView::Ptr getImageView();
		GPU::ImageDescriptor::Ptr getDescriptor();

		void createData();
		void uploadData();
		void destroyData();

		uint32 getSize()
		{
			return size;
		}

		typedef std::shared_ptr<TextureCubeMap> Ptr;
		static Ptr create(uint32 size, GPU::Format format, uint32 levels = 1)
		{
			return std::make_shared<TextureCubeMap>(size, format, levels);
		}

		static Ptr create(uint32 size, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
		{
			return std::make_shared<TextureCubeMap>(size, format, levels, usage);
		}

	private:
		uint32 size;
		TextureCubeMap(const TextureCubeMap&) = delete;
		TextureCubeMap& operator=(const TextureCubeMap&) = delete;
	};

	class TextureCubeMapArray : public Texture
	{
	public:
		TextureCubeMapArray(uint32 size, uint32 layers, GPU::Format format, uint32 levels);
		TextureCubeMapArray(uint32 size, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage);
		~TextureCubeMapArray();

		void upload(uint8* data, uint32 size, uint32 face, uint32 level = 0);
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void setCompareMode();
		void generateMipmaps();
		void setLayout();
		GPU::Image::Ptr getImage();
		GPU::ImageView::Ptr getImageView();
		GPU::ImageDescriptor::Ptr getDescriptor();

		void createData();
		void uploadData();
		void destroyData();

		typedef std::shared_ptr<TextureCubeMapArray> Ptr;
		static Ptr create(uint32 size, uint32 layers, GPU::Format format, uint32 levels = 1)
		{
			return std::make_shared<TextureCubeMapArray>(size, layers, format, levels);
		}

		static Ptr create(uint32 size, uint32 layers, GPU::Format format, uint32 levels, GPU::ImageUsage usage)
		{
			return std::make_shared<TextureCubeMapArray>(size, layers, format, levels, usage);
		}

	private:

		TextureCubeMapArray(const TextureCubeMapArray&) = delete;
		TextureCubeMapArray& operator=(const TextureCubeMapArray&) = delete;
	};
}

#endif // INCLUDED_TEXTURE