#ifndef INCLUDED_GLIMAGEVIEW
#define INCLUDED_GLIMAGEVIEW

#pragma once

#include <GPU/ImageView.h>
#include <GPU/GL/GLPlatform.h>

namespace GL
{
	GLenum getTarget(GPU::ViewType viewType);
	GLenum getFormat(GPU::Format format);

	class ImageView : public GPU::ImageView
	{
	public:
		ImageView(GLuint parentTexture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange);
		~ImageView();
		GLuint getTexture();

		typedef std::shared_ptr<ImageView> Ptr;
		static Ptr create(GLuint texture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange)
		{
			return std::make_shared<ImageView>(texture, type, format, subRange);
		}

	private:
		GLuint texture;
		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
	};
}

#endif // INCLUDED_GLIMAGEVIEW