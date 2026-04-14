#ifndef INCLUDED_UNITYLIGHT
#define INCLUDED_UNITYLIGHT

#pragma once

#include "Component.h"
#include <Types/ObjectTypes.h>
#include <Types/DataTypes.h>

namespace Unity
{
	class Light : public Component
	{
	public:
		Light();
		~Light();
		void fromYAML(YAML::Light* light);
		uint32 getType() { return type; }
		uint32 getShape() { return shape; }
		glm::vec3 getColor() { return color; }
		float getIntensity() {
			return intensity;
		}
		float getRange() {
			return range;
		}

		typedef std::shared_ptr<Light> Ptr;
		static Ptr create()
		{
			return std::make_shared<Light>();
		}
	private:
		uint32 type;
		uint32 shape;
		glm::vec3 color;
		float intensity;
		float range;
	};
}

#endif // INCLUDED_UNITYLIGHT