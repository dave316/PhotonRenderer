#include "Light.h"

#include <Math/Frustrum.h>
#include <Utils/Color.h>

glm::vec3 Light::lightForward = glm::vec3(0, 0, -1);

Light::Light(LightType type) :
	type(type)
{

}

Light::Light(LightType type, glm::vec3 color, float intensity, float range) :
	type(type), 
	linearColor(color), 
	intensity(intensity), 
	range(range),
	on(true)
{

}

Light::~Light()
{

}

void Light::writeUniformData(LightUniformData& uniformData, Transform::Ptr transform)
{
	glm::vec3 pos = transform->getPosition();
	glm::quat rot = transform->getRotation();
	rot = glm::normalize(rot);

	// TODO: these can be retrived from the model matrix
	glm::vec3 dir = glm::mat3_cast(rot) * lightForward;
	glm::vec3 up = glm::mat3_cast(rot) * glm::vec3(0,1,0);
	glm::vec3 left = glm::mat3_cast(rot) * glm::vec3(-1,0,0);

	float cosInner = cos(innerConeAngle);
	float cosOuter = cos(outerConeAngle);
	float angleScale = 1.0f / glm::max(0.001f, (cosInner - cosOuter));
	float angleOffset = -cosOuter * angleScale;

	uniformData.position = glm::vec4(pos, 1.0f);
	uniformData.direction = glm::vec4(dir, 1.0f);
	uniformData.up = glm::vec4(up, 1.0f);
	uniformData.left = glm::vec4(left, 1.0f);
	uniformData.color = linearColor;
	uniformData.intensity = intensity;
	uniformData.range = range;
	uniformData.angleScale = angleScale;
	uniformData.angleOffset = angleOffset;
	uniformData.radius = radius;
	uniformData.width = width;
	uniformData.height = height;
	uniformData.type = static_cast<int>(type);
	uniformData.shape = static_cast<int>(shape);
	uniformData.iesProfile = iesProfile;
	uniformData.on = on;
	uniformData.worldToLight = transform->getTransform();
}

void Light::updateLightViewProjection(FPSCamera& camera, Transform::Ptr transform)
{
	if (type == LightType::DIRECTIONAL)
	{
		auto pos = transform->getPosition();
		auto rot = transform->getRotation();
		rot = glm::normalize(rot);
		auto lightDir = glm::mat3_cast(rot) * Light::lightForward;
		lightViewProjection = Math::getLightSpaceMatrices(camera, lightDir);
		
		//float zFar = camera.getZFar();
		//std::vector<float> csmLevels =
		//{
		//	zFar / 10.0f,
		//	zFar / 5.0f,
		//	zFar / 2.0f,
		//};

		//for (auto& [_, s] : shaders)
		//{
		//	s->setUniform("lightSpaceMatrices[0]", lightSpaceMatrices);
		//	s->setUniform("cascadePlaneDistance[0]", csmLevels);
		//	s->setUniform("cascadeCount", (int)csmLevels.size());
		//}
	}
	else
	{
		glm::vec3 pos = transform->getPosition();
		lightViewProjection = Math::creatCMViews(pos, 0.1f, range);
	}
}

void Light::setType(LightType type)
{
	this->type = type;
}

void Light::setShape(LightShape shape)
{
	if (shape != LightShape::PUNCTUAL)
	{
		this->type = LightType::AREA;
		this->shape = shape;
	}	
}

void Light::setColorLinear(glm::vec3 color)
{
	this->linearColor = color;
}

void Light::setColorSRGB(glm::vec3 color)
{
	this->linearColor = Color::sRGBToLinear(color);
}

void Light::setColorTemp(int temperature)
{
	this->linearColor = Color::convertTemp2RGB(temperature);
}

void Light::setLuminousPower(float lumen)
{
	this->lumen = lumen;
	const float pi = glm::pi<float>();
	switch (type)
	{
		case LightType::DIRECTIONAL: break;
		case LightType::POINT: intensity = lumen / (4.0f * pi); break;
		case LightType::SPOT: intensity = lumen / pi;	break;
		case LightType::AREA:
		{
			switch (shape)
			{
				case LightShape::SPHERE: intensity = lumen / (4.0f * pi); break;
				case LightShape::DISC: intensity = lumen / pi; break;
				case LightShape::TUBE: intensity = lumen / (2.0f * pi * radius * width); break;
				case LightShape::RECTANGLE: intensity = lumen / (width * height); break;
			}
		}
		break;
	}
}

void Light::setLuminousIntensity(float candela)
{
	intensity = candela;
}

void Light::setRange(float range)
{
	this->range = range;
}

void Light::setConeAngles(float inner, float outer)
{
	this->innerConeAngle = inner;
	this->outerConeAngle = outer;
}

void Light::setInnerAngle(float inner)
{
	this->innerConeAngle = inner;
}

void Light::setOuterAngle(float outer)
{
	this->outerConeAngle = outer;
}

void Light::setRadius(float radius)
{
	this->radius = radius;
}

void Light::setWidth(float width)
{
	this->width = width;
}

void Light::setHeight(float height)
{
	this->height = height;
}

void Light::setIESProfile(int index)
{
	iesProfile = index;
}

void Light::toggle()
{
	on = !on;
}

LightType Light::getType()
{
	return type;
}

glm::vec3 Light::getColor()
{
	return linearColor;
}

float Light::getIntensity()
{
	return intensity;
}

float Light::getLumen()
{
	return lumen;
}

float Light::getRange()
{
	return range;
}

std::vector<glm::mat4> Light::getViewProjections()
{
	return lightViewProjection;
}