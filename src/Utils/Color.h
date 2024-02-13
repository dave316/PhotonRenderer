#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

#pragma once

#include <glm/glm.hpp>

namespace Color
{
	glm::vec3 linearToSRGB(glm::vec3 linearColor, float gamma = 2.2f);
	glm::vec3 sRGBToLinear(glm::vec3 linearColor, float gamma = 2.2f);
	glm::vec4 linearToSRGBAlpha(glm::vec4 sRGBAlpha, float gamma = 2.2f);
	glm::vec4 sRGBAlphaToLinear(glm::vec4 sRGBAlpha, float gamma = 2.2f);
	glm::vec3 convertTemp2RGB(int tempInKelvin);
}

#endif // INCLUDED_COLOR