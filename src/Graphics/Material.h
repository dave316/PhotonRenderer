#ifndef INCLUDED_MATERIAL
#define INCLUDED_MATERIAL

#pragma once

#include "Texture.h"

#include <GL/GLProgram.h>

#include <vector>

#include <glm/glm.hpp>

class IProperty
{
	
};

class Material
{
	//std::string name;
	std::vector<Texture2D::Ptr> textures;
	glm::vec3 color;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
public:
	Material();
	~Material();

	void setColor(glm::vec3& color);
	void addTexture(Texture2D::Ptr texture);
	void setUniforms(GL::Program& program);

	typedef std::shared_ptr<Material> Ptr;
	static Ptr create()
	{
		return std::make_shared<Material>();
	}
};

#endif // INCLUDED_MATERIAL