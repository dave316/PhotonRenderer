#include "Light.h"

Light::Light(int type, glm::vec3 color, float intensity, float range) :
	type(type), color(color), intensity(intensity), range(range)
{

}

Light::~Light()
{

}

void Light::writeUniformData(UniformData& uniformData)
{
	uniformData.position = glm::vec4(position, 1.0f);
	uniformData.direction = glm::vec4(direction, 1.0f);
	uniformData.color = glm::vec4(color, 1.0f);
	uniformData.intensity = intensity;
	uniformData.range = range;
	uniformData.innterConeAngle = innerConeAngle;
	uniformData.outerConeAngle = outerConeAngle;
	uniformData.type = type;
}

void Light::draw()
{

}

void Light::setPostion(glm::vec3 position)
{
	this->position = position;
}

void Light::setDirection(glm::vec3 direction)
{
	this->direction = direction;
}

void Light::setConeAngles(float inner, float outer)
{
	this->innerConeAngle = inner;
	this->outerConeAngle = outer;
}