#ifndef INCLUDED_UNITYLIGHTPROBEGROUP
#define INCLUDED_UNITYLIGHTPROBEGROUP

#pragma once

#include "Component.h"

namespace Unity
{
	class LightProbeGroup : public Component
	{
	public:
		LightProbeGroup();
		~LightProbeGroup();

		typedef std::shared_ptr<LightProbeGroup> Ptr;
		static Ptr create()
		{
			return std::make_shared<LightProbeGroup>();
		}
	private:

	};
}

#endif // INCLUDED_UNITYLIGHTPROBEGROUP