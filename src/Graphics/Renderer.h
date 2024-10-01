#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/Framebuffer.h>
#include <Graphics/FPSCamera.h>

#include <Core/Light.h>
#include <Core/LightProbe.h>
#include <Core/Renderable.h>
#include <Core/Scene.h>

#include <glm/glm.hpp>

class Renderer
{
private:
	Shader::Ptr defaultShader;
	Shader::Ptr postProcessShader;
	Shader::Ptr skyboxShader;
	Shader::Ptr unlitShader;
	Shader::Ptr outlineShader;

	std::map<std::string, Shader::Ptr> shaders;
	GL::UniformBuffer<CameraUniformData> cameraUBO;
	GL::UniformBuffer<ModelUniformData> modelUBO;
	GL::UniformBuffer<LightUniformData> lightUBO;
	GL::UniformBuffer<ReflectionProbe> reflUBO;

	Framebuffer::Ptr screenFBO;
	Framebuffer::Ptr postFBO;
	Framebuffer::Ptr outlineFBO;
	Framebuffer::Ptr bloomFBO;
	
	// screen maps
	Texture2D::Ptr screenTex;
	Texture2D::Ptr refrTex;
	Texture2D::Ptr bloomTex;
	Texture2D::Ptr postTex;
	Texture2D::Ptr outlineTex;
	Texture2D::Ptr bloomBlurTex;

	Primitive::Ptr screenQuad;
	Primitive::Ptr unitCube;

	FPSCamera camera;

	// IBL Maps
	TextureCubeMap::Ptr skybox;
	TextureCubeMapArray::Ptr irradianceMaps;
	TextureCubeMapArray::Ptr specularMapsGGX;
	TextureCubeMapArray::Ptr specularMapsSheen;
	Texture2D::Ptr brdfLUT;

	// Lightmaps
	Texture2DArray::Ptr lightMaps;
	Texture2DArray::Ptr directionMaps;
	Texture2DArray::Ptr iesProfiles;

	// Shadowmaps
	Texture2DArray::Ptr csmShadowMap;
	TextureCubeMapArray::Ptr omniShadowMap;
	Framebuffer::Ptr csmShadowFBO;
	Framebuffer::Ptr omniShadowFBO;

	// Fog volumes
	Texture3D::Ptr fogMaterialVolume;
	Texture3D::Ptr inScatteringVolume;
	Texture3D::Ptr acumFogVolume;

	std::map<std::string, std::vector<Renderable::Ptr>> renderQueue;

	unsigned int width;
	unsigned int height;

	bool useBloom = true;

	std::map<std::string, ReflectionProbe> reflectionProbes;

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
public:
	
	Renderer(unsigned int width, unsigned int height);
	~Renderer();

	bool init();
	void initLUT();
	void initFBOs();
	void initShaders();
	void initLights(Scene::Ptr scene, FPSCamera& camera);
	void initShadows(Scene::Ptr scene);
	void initLightProbes(Scene::Ptr scene);
	void initFogVolumes(Scene::Ptr scene);
	void prepare(Scene::Ptr scene);
	void resize(unsigned int width, unsigned int height);
	void updateCamera(FPSCamera& camera);
	void updateCamera(FPSCamera& camera, float dt);
	void updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos);
	void updatePostProcess(PostProcessParameters& params);
	void updateShadows(Scene::Ptr scene);
	void updateVolumes(Scene::Ptr scene);
	void setBloom(bool useBloom);
	void setIBL(bool useIBL);
	void setLights(int numLights);
	void setDebugChannel(int channel);
	void setTonemappingOp(int index);
	void setRenderQueue(std::map<std::string, std::vector<Renderable::Ptr>> renderQueue);
	void updateTime(float dt);
	void clear();
	void renderScene(Scene::Ptr scene, Shader::Ptr defShader, bool transmission);
	void renderBatchedScene(Scene::Ptr scene, Shader::Ptr shader);
	void renderToScreen(Scene::Ptr scene);
	void renderOutline(Entity::Ptr entity);
	Texture2D::Ptr renderToTexture(Scene::Ptr scene);
	Texture2D::Ptr renderForward(Scene::Ptr scene);
	Shader::Ptr getShader(std::string name);
	std::map<std::string, ReflectionProbe> getEnvMapData()
	{
		return reflectionProbes;
	}

	// TODO: put somewhere else...
	//std::vector<glm::vec4> getFrustrumCorners(glm::mat4& VP);
	//std::vector<glm::mat4> getLightSpaceMatrices(FPSCamera& camera, glm::vec3 lightDir);
	//glm::mat4 getLightSpaceMatrix(FPSCamera& camera, glm::vec3& lightDir, float zNear, float zFar);
};

#endif // INCLUDED_RENDERER