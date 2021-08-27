#ifndef INCLUDED_LIGHT
#define INCLUDED_LIGHT

#pragma once

#include <Graphics/Mesh.h>

class Light
{
public:
	glm::vec3 position;
	glm::vec3 color;

	Light(const Light&) = delete;
	Light& operator=(const Light&) = delete;
public:

	Light(glm::vec3 position, glm::vec3 color);
	~Light();

	struct UniformData
	{
		glm::vec4 position;
		glm::vec4 color;
	};

	void writeUniformData(UniformData& uniformData);
	void draw();
	typedef std::shared_ptr<Light> Ptr;
	static Ptr create(glm::vec3 position, glm::vec3 color)
	{
		return std::make_shared<Light>(position, color);
	}
};

#endif // INCLUDED_LIGHT