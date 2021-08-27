#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 color) :
	position(position), color(color)
{

}

Light::~Light()
{

}

void Light::writeUniformData(UniformData& uniformData)
{
	uniformData.position = glm::vec4(position, 1.0f);
	uniformData.color = glm::vec4(color, 1.0f);
}

void Light::draw()
{

}