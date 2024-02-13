#ifndef INCLUDED_IBL
#define INCLUDED_IBL

#pragma once

#include <Core/LightProbe.h>
#include <Core/Scene.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>

namespace IBL
{
	TextureCubeMap::Ptr createCubemapFromColor(glm::vec3 color, int size, float rotation);
	TextureCubeMap::Ptr convertEqui2CM(Shader::Ptr pano2cm, Texture2D::Ptr panorama, int size, float rotation = 0.0f, float exposure = 1.0f);
	TextureCubeMapArray::Ptr filterLambert(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes);
	TextureCubeMapArray::Ptr filterSpecularGGX(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes);
	TextureCubeMapArray::Ptr filterSpecularSheen(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes);
	void computeSHLightprobes(Scene::Ptr scene, std::map<std::string, ReflectionProbe>& reflProbes);
	void computeProbeMapping(Scene::Ptr scene, std::map<std::string, ReflectionProbe>& reflProbes);
}

#endif // INCLUDED_IBL