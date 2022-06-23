#ifndef INCLUDED_ENVIRONMENTMAP
#define INCLUDED_ENVIRONMENTMAP

#pragma once

#include "Framebuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include <glm/glm.hpp>

class EnvironmentMap
{
	Primitive::Ptr cubeMesh;
	TextureCubeMap::Ptr cubeMap;
	
	unsigned int faceSize;
	
public:
	std::vector<glm::mat4> views;
	typedef std::shared_ptr<EnvironmentMap> Ptr;

	EnvironmentMap(unsigned int faceSize);
	void setViews(glm::vec3 position);
	void fromPanorama(Texture2D::Ptr panorama, Shader::Ptr pano2cm);
	void fromFile(const std::string& filename);
	void fromTex(TextureCubeMap::Ptr tex);
	void filter(EnvironmentMap::Ptr source, Shader::Ptr filter);
	void filterMips(EnvironmentMap::Ptr source, Shader::Ptr filter, unsigned int maxMipLevel);
	void use(GLuint unit);
	TextureCubeMap::Ptr getCubeMap();

	static Ptr create(unsigned int faceSize)
	{
		return std::make_shared<EnvironmentMap>(faceSize);
	}
};

#endif // INCLUDED_ENVIRONMENTMAP