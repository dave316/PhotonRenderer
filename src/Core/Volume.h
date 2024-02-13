#ifndef INCLUDED_VOLUME
#define INCLUDED_VOLUME

#pragma once

#include "Component.h"

#include <Graphics/Shader.h>

#include <glm/glm.hpp>

struct PostProcessParameters
{
	float postExposure = 0.0f;
	float bloomThreshold = 0.0f;
	float bloomIntensity = 0.5f;
	glm::vec3 bloomTint = glm::vec3(1);

	void setUniforms(Shader::Ptr shader)
	{
		shader->setUniform("manualExposure", postExposure);
		shader->setUniform("bloomIntensity", bloomIntensity);
		shader->setUniform("bloomTint", bloomTint);
	}
};

class Volume : public Component
{
private:
	bool global;
	float blendDistance;

	PostProcessParameters ppParameters;

	Volume(const Volume&) = delete;
	Volume& operator=(const Volume&) = delete;
public:
	Volume(bool global, float distance);
	void setPPP(PostProcessParameters& params);
	bool isGlobal();
	float getDistance();
	PostProcessParameters getParameters();
	typedef std::shared_ptr<Volume> Ptr;
	static Ptr create(bool global, float distance)
	{
		return std::make_shared<Volume>(global, distance);
	}
};

#endif // INCLUDED_VOLUME