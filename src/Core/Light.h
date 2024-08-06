#ifndef INCLUDED_LIGHT
#define INCLUDED_LIGHT

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Mesh.h>
#include <glm/gtc/constants.hpp>

#include "Component.h"
#include "Transform.h"

enum class LightType : int
{
	DIRECTIONAL,
	POINT,
	SPOT,
	AREA
};

enum class LightShape : int
{
	PUNCTUAL,
	SPHERE,
	DISC,
	TUBE,
	RECTANGLE
};

struct LightUniformData
{
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 up;
	glm::vec4 left;
	glm::vec3 color;
	float intensity;
	float range;
	float angleScale;
	float angleOffset;
	float radius;
	float width;
	float height;
	int type;
	int shape;
	int iesProfile;
	int on;
	glm::vec2 padding;
	glm::mat4 worldToLight;
};

class Light : public Component
{
private:
	LightType type = LightType::POINT;
	LightShape shape = LightShape::PUNCTUAL;
	glm::vec3 linearColor = glm::vec3(1.0f);
	float lumen = 0.0f;
	float intensity = 1.0f;
	float range = -1.0f;
	float innerConeAngle = 0.0f;
	float outerConeAngle = glm::quarter_pi<float>();
	float radius = 1.0f;
	float width = 1.0f;
	float height = 1.0f;
	int iesProfile = -1;
	bool on = true;

	std::vector<glm::mat4> lightViewProjection;

	Light(const Light&) = delete;
	Light& operator=(const Light&) = delete;
public:

	static glm::vec3 lightForward; // Default forward direction of punctual lights 

	Light(LightType type);
	Light(LightType type, glm::vec3 color, float intensity, float range);
	~Light();

	void writeUniformData(LightUniformData& uniformData, Transform::Ptr transform);
	void updateLightViewProjection(FPSCamera& camera, Transform::Ptr transform);
	void setType(LightType type);
	void setShape(LightShape shape);
	void setColorLinear(glm::vec3 color);
	void setColorSRGB(glm::vec3 color);
	void setColorTemp(int temperature);
	void setLuminousPower(float lumen);
	void setLuminousIntensity(float candela);
	void setRange(float range);
	void setConeAngles(float inner, float outer);
	void setInnerAngle(float inner);
	void setOuterAngle(float outer);
	void setRadius(float radius);
	void setWidth(float width);
	void setHeight(float height);
	void setIESProfile(int index);
	void toggle();	
	LightType getType();
	glm::vec3 getColor();
	float getIntensity();
	float getLumen();
	float getRange();
	std::vector<glm::mat4> getViewProjections();
	typedef std::shared_ptr<Light> Ptr;
	static Ptr create(LightType type, glm::vec3 color, float intensity, float range)
	{
		return std::make_shared<Light>(type, color, intensity, range);
	}
	static Ptr create(LightType type)
	{
		return std::make_shared<Light>(type);
	}
};

#endif // INCLUDED_LIGHT