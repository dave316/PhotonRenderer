#ifndef INCLUDED_UNITYCANVASRENDERER
#define INCLUDED_UNITYCANVASRENDERER

#pragma once

#include "Component.h"

namespace Unity
{
	class CanvasRenderer : public Component
	{
	public:
		CanvasRenderer();
		~CanvasRenderer();

		typedef std::shared_ptr<CanvasRenderer> Ptr;
		static Ptr create()
		{
			return std::make_shared<CanvasRenderer>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYCANVASRENDERER