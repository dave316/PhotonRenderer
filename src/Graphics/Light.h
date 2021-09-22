#ifndef INCLUDED_LIGHT
#define INCLUDED_LIGHT

#pragma once

#include <Graphics/Mesh.h>
#include <glm/gtc/constants.hpp>

enum LightType
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

class Light
{
private:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0,0,-1);
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float range = -1.0f;
	float innerConeAngle = 0.0f;
	float outerConeAngle = glm::quarter_pi<float>();
	int type;

	Light(const Light&) = delete;
	Light& operator=(const Light&) = delete;
public:

	Light(int type, glm::vec3 color, float intensity, float range);
	~Light();

	struct UniformData
	{
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec3 color;
		float intensity;
		float range;
		float innterConeAngle;
		float outerConeAngle;
		int type;
	};

	void writeUniformData(UniformData& uniformData);
	void draw();
	void setPostion(glm::vec3 position);
	void setDirection(glm::vec3 direction);
	void setConeAngles(float inner, float outer);
	glm::vec3 getPosition();
	typedef std::shared_ptr<Light> Ptr;
	static Ptr create(int type, glm::vec3 color, float intensity, float range)
	{
		return std::make_shared<Light>(type, color, intensity, range);
	}
};

#endif // INCLUDED_LIGHT