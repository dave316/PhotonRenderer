#include "Light.h"
#include <Graphics/Frustrum.h>

glm::vec3 pr::Light::lightForward = glm::vec3(0, 0, -1);

namespace pr
{
	Light::Light(LightType type, glm::vec3 color, float intensity, float range) :
		type(type),
		color(color),
		intensity(intensity),
		range(range)
	{
	}

	Light::~Light()
	{
	}

	void Light::setConeAngles(float inner, float outer)
	{
		innerConeAngle = inner;
		outerConeAngle = outer;
	}

	void Light::writeUniformData(LightUniformData& uniformData, Transform::Ptr transform)
	{
		glm::vec3 pos = transform->getPosition();
		glm::quat rot = transform->getRotation();
		rot = glm::normalize(rot);

		glm::vec3 dir = glm::mat3_cast(rot) * lightForward;
		float cosInner = glm::cos(innerConeAngle);
		float cosOuter = glm::cos(outerConeAngle);
		float angleScale = 1.0f / glm::max(0.001f, (cosInner - cosOuter));
		float angleOffset = -cosOuter * angleScale;

		uniformData.position = glm::vec4(pos, 0.0f);
		uniformData.direction = glm::vec4(dir, 0.0f);
		uniformData.color = glm::vec4(color, 0.0f);
		uniformData.intensity = intensity;
		uniformData.range = range;
		uniformData.angleScale = angleScale;
		uniformData.angleOffset = angleOffset;
		uniformData.type = static_cast<int>(type);
		uniformData.on = on;
		uniformData.castShadows = castShadows;
	}

	void Light::updateLightViewProjection(UserCamera& camera, Transform::Ptr transform)
	{
		if (type == LightType::DIRECTIONAL)
		{
			auto pos = transform->getPosition();
			auto rot = transform->getRotation();
			rot = glm::normalize(rot);
			auto lightDir = glm::mat3_cast(rot) * Light::lightForward;
			lightViewProjection = Math::getLightSpaceMatrices(camera, lightDir);
		}
		else
		{
			glm::vec3 pos = transform->getPosition();
			lightViewProjection = Math::creatCMViews(pos, 0.1f, range);
		}
	}

	void Light::setLuminousPower(float lumen)
	{
		this->lumen = lumen;
		const float pi = glm::pi<float>();
		switch (type)
		{
			case LightType::DIRECTIONAL: intensity = lumen; break;
			case LightType::POINT: intensity = lumen / (4.0f * pi); break;
			case LightType::SPOT: intensity = lumen / pi;	break;
		}
	}

	void Light::setColorTemp(int temp)
	{
		double T = temp;
		double T2 = T * T;
		double u = (0.860117757 + 1.54118254e-4 * T + 1.28641212e-7 * T2) / (1.0 + 8.42420235e-4 * T + 7.08145163e-7 * T2);
		double v = (0.317398726 + 4.22806245e-5 * T + 4.20481691e-8 * T2) / (1 - 2.89741816e-5 * T + 1.61456053e-7 * T2);
		double x = (3.0 * u) / (2.0 * u - 8.0 * v + 4.0);
		double y = (2.0 * v) / (2.0 * u - 8.0 * v + 4.0);
		double Y = 1.0;
		double X = x * Y / y;
		double Z = (1 - x - y) * Y / y;
		glm::vec3 XYZ = glm::vec3(X, Y, Z);
		glm::mat3 XYZ2RGB = glm::mat3(
			3.2404542, -1.5371385, -0.4985314,
			-0.9692660, 1.8760108, 0.0415560,
			0.0556434, -0.2040259, 1.057225
		);
		glm::vec3 rgbLinear = glm::transpose(XYZ2RGB) * XYZ;
		rgbLinear /= glm::max(glm::max(rgbLinear.x, rgbLinear.y), rgbLinear.z);
		color = glm::clamp(rgbLinear, 0.0f, 1.0f);
	}
}