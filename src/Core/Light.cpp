#include "Light.h"

#include <glm/gtx/matrix_decompose.hpp>

Light::Light(int type, glm::vec3 color, float intensity, float range) :
	type(type), color(color), intensity(intensity), range(range)
{

}

Light::~Light()
{

}

void Light::writeUniformData(UniformData& uniformData, Transform::Ptr transform)
{
	glm::mat4 M = transform->getTransform();
	glm::vec3 skew;
	glm::vec4 persp;
	glm::vec3 pos;
	glm::vec3 scale;
	glm::quat rot;
	glm::decompose(M, scale, rot, pos, skew, persp);
	rot = glm::normalize(rot);
	glm::vec3 dir = glm::mat3_cast(rot) * glm::vec3(0, 0, -1);

	uniformData.position = glm::vec4(pos, 1.0f);
	uniformData.direction = glm::vec4(dir, 1.0f);
	uniformData.color = glm::vec4(color, 1.0f);
	uniformData.intensity = intensity;
	uniformData.range = range;
	uniformData.innterConeAngle = innerConeAngle;
	uniformData.outerConeAngle = outerConeAngle;
	uniformData.type = type;
}

//void Light::draw()
//{
//	// TODO: create mesh based on light type for debugging
//}

//void Light::setPostion(glm::vec3 position)
//{
//	this->position = position;
//}
//
//void Light::setDirection(glm::vec3 direction)
//{
//	this->direction = direction;
//}

void Light::setConeAngles(float inner, float outer)
{
	this->innerConeAngle = inner;
	this->outerConeAngle = outer;
}

//glm::vec3 Light::getPosition()
//{
//	return position;
//}