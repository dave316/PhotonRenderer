#ifndef INCLUDED_LIGHT
#define INCLUDED_LIGHT

#pragma once

#include "Component.h"
#include "Transform.h"
#include <Graphics/FPSCamera.h>
#include <Graphics/Mesh.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pr
{
	enum class LightType : int
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

	struct LightUniformData
	{
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 color;
		float intensity;
		float range;
		float angleScale;
		float angleOffset;
		int type;
		int on;
		int castShadows;
		uint32 padding;
	};

	class Light : public Component
	{
	public:
		static glm::vec3 lightForward;

		Light(LightType type = LightType::POINT, glm::vec3 color = glm::vec3(1), float intensity = 1.0f, float range = -1.0);
		~Light();
		void setConeAngles(float inner, float outer);
		void writeUniformData(LightUniformData& uniformData, Transform::Ptr transform);
		void updateLightViewProjection(UserCamera& camera, Transform::Ptr transform);
		void toggle() { on = !on; }
		void setLightType(LightType type) { this->type = type; }
		void setColorLinear(glm::vec3 color) { this->color = color; }
		void setRange(float range) {
			this->range = range;
		}
		void setLuminousPower(float lumen);
		void setColorTemp(int temp);
		void setCastShadows(bool cashShadows) { this->castShadows = cashShadows;  }
		float getLumen() { return lumen; }
		float getRange() { return range; }
		LightType getType() { return type; }
		glm::vec3 getColor() { return color; }
		std::vector<glm::mat4> getViewProjections() { return lightViewProjection; }

		typedef std::shared_ptr<Light> Ptr;
		static Ptr create(LightType type, glm::vec3 color, float intensity, float range)
		{
			return std::make_shared<Light>(type, color, intensity, range);
		}

	private:
		LightType type = LightType::POINT;
		glm::vec3 color = glm::vec3(1);
		float lumen = 0.0f;
		float intensity = 1.0f;
		float range = -1.0f;
		float innerConeAngle = 0.0f;
		float outerConeAngle = glm::quarter_pi<float>();
		bool on = true;
		bool castShadows = true;

		std::vector<glm::mat4> lightViewProjection;
	};
}

#endif // INCLUDED_LIGHT