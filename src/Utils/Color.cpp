#include "Color.h"

namespace Color
{
	glm::vec3 linearToSRGB(glm::vec3 linearRGB, float gamma)
	{
		return glm::pow(linearRGB, glm::vec3(1.0f / gamma));
	}

	glm::vec3 sRGBToLinear(glm::vec3 sRGB, float gamma)
	{
		return glm::pow(sRGB, glm::vec3(gamma));
	}

	glm::vec4 linearToSRGBAlpha(glm::vec4 linearRGBA, float gamma)
	{
		glm::vec3 rgb = glm::vec3(linearRGBA);
		float alpha = linearRGBA.a;
		glm::vec3 sRGB = linearToSRGB(rgb);
		return glm::vec4(sRGB, alpha);
	}

	glm::vec4 sRGBAlphaToLinear(glm::vec4 sRGBAlpha, float gamma)
	{
		glm::vec3 sRGB = glm::vec3(sRGBAlpha);
		float alpha = sRGBAlpha.a;
		glm::vec3 linearRGB = sRGBToLinear(sRGB, gamma);
		return glm::vec4(linearRGB, alpha);
	}

	glm::vec3 convertTemp2RGB(int tempInKelvin)
	{
		double T = tempInKelvin;
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
		return glm::clamp(rgbLinear, 0.0f, 1.0f);
	}
}
