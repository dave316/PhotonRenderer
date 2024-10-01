#include "Renderer.h"
#include "MeshPrimitives.h"

#include <Core/Animator.h>
#include <Core/Lod.h>
#include <Core/ParticleSystem.h>

#include <IO/Image/ImageDecoder.h>
#include <IO/ShaderLoader.h>

#include <Math/Frustrum.h>
#include <Utils/IBL.h>

#include <glm/gtx/matrix_decompose.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

Renderer::Renderer(unsigned int width, unsigned int height) :
	width(width), 
	height(height)
{

}

Renderer::~Renderer()
{

}

bool Renderer::init()
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glDepthFunc(GL_LEQUAL);
	glPointSize(5.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, width, height);

	unitCube = MeshPrimitives::createCube(glm::vec3(0), 1.0f);
	screenQuad = MeshPrimitives::createQuad(glm::vec3(0.0f), 2.0f);

	initShaders();
	initLUT();
	initFBOs();

	cameraUBO.bindBase(0);
	lightUBO.bindBase(1);
	modelUBO.bindBase(2);
	reflUBO.bindBase(3);

	return true;
}

void Renderer::initLUT()
{
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];
	int size = 512;
	auto brdfFBO = Framebuffer::create(size, size);
	brdfLUT = Texture2D::create(size, size, GL::RGBA16F);
	brdfLUT->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
	brdfFBO->addRenderTexture(GL::COLOR0, brdfLUT);
	brdfFBO->begin();
	integrateBRDFShader->use();
	screenQuad->draw();
	brdfFBO->end();
}

void Renderer::initFBOs()
{
	int w = width;
	int h = height;

	screenTex = Texture2D::create(w, h, GL::RGBA32F);
	refrTex = Texture2D::create(w, h, GL::RGBA32F);
	refrTex->generateMipmaps();
	refrTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	bloomTex = Texture2D::create(w, h, GL::RGBA32F);
	bloomTex->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);

	screenFBO = Framebuffer::create(w, h);
	screenFBO->addRenderTexture(GL::COLOR0, screenTex);
	screenFBO->addRenderTexture(GL::COLOR1, refrTex);
	screenFBO->addRenderTexture(GL::COLOR2, bloomTex);
	screenFBO->addRenderBuffer(GL::DEPTH_STENCIL, GL::D24_S8);

	postTex = Texture2D::create(w, h, GL::RGBA8);
	postFBO = Framebuffer::create(w, h);
	postFBO->addRenderTexture(GL::COLOR0, postTex);

	outlineTex = Texture2D::create(w, h, GL::RGBA8);
	outlineTex->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
	outlineFBO = Framebuffer::create(w, h);
	outlineFBO->addRenderTexture(GL::COLOR0, outlineTex);
	outlineFBO->addRenderBuffer(GL::DEPTH, GL::DEPTH24);

	bloomBlurTex = Texture2D::create(w * 0.5f, h * 0.5f, GL::RGBA32F);
	bloomBlurTex->generateMipmaps();
	bloomBlurTex->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
	bloomBlurTex->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	bloomFBO = Framebuffer::create(w * 0.5f, h * 0.5f);
}

void Renderer::initShaders()
{
	std::string shaderPath = "../../../../src/Shaders";
	auto shaderList = IO::loadShadersFromPath(shaderPath);
	for (auto s : shaderList)
		shaders.insert(std::pair(s->getName(), s));

	auto mat = getDefaultMaterial();
	mat->addProperty("material.metallicFactor", 0.0f);

	for (auto [name, shader] : shaders)
	{
		if (name.substr(0, 7).compare("Default") == 0)
		{
			shader->setUniform("screenTex", 10);
			shader->setUniform("irradianceMaps", 11);
			shader->setUniform("specularMapsGGX", 12);
			shader->setUniform("specularMapsSheen", 13);
			shader->setUniform("brdfLUT", 14);
			shader->setUniform("useIBL", true);
			shader->setUniform("shadowCascades", 15); 
			shader->setUniform("shadowMaps", 16);
			shader->setUniform("lightMaps", 17);
			shader->setUniform("directionMaps", 18);
			shader->setUniform("iesProfile", 19);
			shader->setUniform("morphTargets", 20);
			shader->setUniform("accumFogTex", 21);
			shader->setUniform("inScatteringTex", 22);
			shader->setUniform("fogMaterialTex", 23);
			
			mat->setUniforms(shader);
		}
	}

	defaultShader = shaders["Default"];

	shaders["Depth"]->setUniform("morphTargets", 20);
	shaders["DepthCubemap"]->setUniform("morphTargets", 20);

	skyboxShader = shaders["Skybox"];
	skyboxShader->setUniform("envMap", 0);

	unlitShader = shaders["Unlit"];
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("morphTargets", 20);
	unlitShader->setUniform("useGammaEncoding", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("tex", 0);

	outlineShader = shaders["Outline"];
	outlineShader->setUniform("inputTexture", 0);

	postProcessShader = shaders["PostProcess"];
	postProcessShader->setUniform("linearRGBTex", 0);
	postProcessShader->setUniform("linearBloomTex", 1);
	postProcessShader->setUniform("useCameraExposure", false);
	postProcessShader->setUniform("manualExposure", 0.0f);

	auto volumeScatter = shaders["VolumeScatter"];
	volumeScatter->setUniform("irradianceMaps", 11);
	volumeScatter->setUniform("shadowCascades", 15);
	volumeScatter->setUniform("shadowMaps", 16);
	volumeScatter->setUniform("fogMaterialTex", 0);

	auto volumeAccum = shaders["VolumeAccum"];
	volumeScatter->setUniform("inScatteringTex", 0);
}

void Renderer::initLights(Scene::Ptr scene, FPSCamera& camera)
{
	std::vector<LightUniformData> lightData;
	float maxIntensity = 0.0;
	for (auto [name, entity] : scene->getRootEntities())
	{
		auto lightEntities = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightEntities)
		{
			auto t = lightEntity->getComponent<Transform>();
			auto l = lightEntity->getComponent<Light>();

			LightUniformData data;
			l->writeUniformData(data, t);
			lightData.push_back(data);

			l->updateLightViewProjection(camera, t);
		}
	}

	lightUBO.upload(lightData, GL_DYNAMIC_DRAW);

	for (auto [name, entity] : scene->getRootEntities())
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			auto l = lightEntity->getComponent<Light>();
			if (l->getType() == LightType::DIRECTIONAL)
			{
				float zFar = camera.getZFar();
				std::vector<float> csmLevels =
				{
					zFar / 10.0f,
					zFar / 5.0f,
					zFar / 2.0f,
				};

				for (auto& [_, s] : shaders)
				{
					s->setUniform("lightSpaceMatrices[0]", l->getViewProjections());
					s->setUniform("cascadePlaneDistance[0]", csmLevels);
					s->setUniform("cascadeCount", (int)csmLevels.size());
				}
			}
		}
	}
}

void Renderer::initShadows(Scene::Ptr scene)
{
	int numPointLights = 0;
	for (auto [name, entity] : scene->getRootEntities())
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			// TODO: now there is only one directional light + CSMs. Should there be more?
			auto l = lightEntity->getComponent<Light>();
			if (l->getType() == LightType::DIRECTIONAL)
			{
				const unsigned int size = 4096;
				csmShadowMap = Texture2DArray::create(size, size, 4, GL::DEPTH24);
				csmShadowMap->setFilter(GL::NEAREST, GL::NEAREST);
				csmShadowMap->setWrap(GL::CLAMP_TO_BORDER, GL::CLAMP_TO_BORDER);
				csmShadowMap->bind();
				GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

				csmShadowFBO = Framebuffer::create(size, size);
				csmShadowFBO->addRenderTexture(GL::DEPTH, csmShadowMap);
				csmShadowFBO->checkStatus();
			}
			else
			{
				numPointLights++;
			}
		}
	}

	if (numPointLights > 0)
	{
		const unsigned int size = 1024;
		omniShadowMap = TextureCubeMapArray::create(size, size, numPointLights, GL::DEPTH24);
		omniShadowMap->setCompareMode();
		{
			omniShadowFBO = Framebuffer::create(size, size);
			omniShadowFBO->addRenderTexture(GL::DEPTH, omniShadowMap);
			omniShadowFBO->checkStatus();
		}
	}
}

void Renderer::initLightProbes(Scene::Ptr scene)
{
	glDisable(GL_CULL_FACE);

	auto pano2cm = shaders["PanoToCubeMap"];
	auto iblFilterShader = shaders["IBLFilter"];

	auto& skyboxInfo = scene->getSkybox(); // TODO: check what format skybox is in (cross, pano, cubemap etc.)
	if (skyboxInfo.texture)
		skybox = IBL::convertEqui2CM(pano2cm, skyboxInfo.texture, 1024, skyboxInfo.rotation, skyboxInfo.exposure);
	else
		skybox = IBL::createCubemapFromColor(skyboxInfo.color * skyboxInfo.exposure, 1024, skyboxInfo.rotation); // TODO: just use the color in the shader, no need for a cubemap...

	std::vector<ReflectionProbe> reflProbes;
	std::vector<LightProbe::Ptr> lightProbes;
	lightProbes.push_back(LightProbe::create(skybox, Box()));

	ReflectionProbe globalProbe;
	globalProbe.index = 0;
	reflProbes.push_back(globalProbe);

	std::vector<Entity::Ptr> reflectionEntities;
	for (auto [name, entity] : scene->getRootEntities())
	{
		auto probes = entity->getChildrenWithComponent<LightProbe>();
		for (int i = 0; i < probes.size(); i++)
		{
			auto p = probes[i];

			auto t = p->getComponent<Transform>();
			auto l = p->getComponent<LightProbe>();
			auto bbox = l->getboundingBox();
			auto pos = t->getPosition();

			ReflectionProbe probe;
			probe.index = i + 1; // index 0 is the skybox/global probe
			probe.position = glm::vec4(pos, 1.0f);
			probe.boxMin = glm::vec4(pos + bbox.getMinPoint(), 1.0f);
			probe.boxMax = pos + bbox.getMaxPoint();
			reflProbes.push_back(probe);
			reflectionProbes.insert(std::make_pair(p->getName(), probe));

			reflectionEntities.push_back(p);
		}
	}

	for (auto e : reflectionEntities)
		lightProbes.push_back(e->getComponent<LightProbe>());

	reflUBO.upload(reflProbes);

	glDisable(GL_DEPTH_TEST);

	irradianceMaps = IBL::filterLambert(iblFilterShader, lightProbes);
	specularMapsGGX = IBL::filterSpecularGGX(iblFilterShader, lightProbes);
	specularMapsSheen = IBL::filterSpecularSheen(iblFilterShader, lightProbes); // TODO: only create if needed (sheen material present)

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Renderer::initFogVolumes(Scene::Ptr scene)
{
	int s = 256;
	fogMaterialVolume = Texture3D::create(s, s, s, GL::R16F);
	float* buffer = new float[s * s * s];
	for (int i = 0; i < s; i++)
	{
		for (int j = 0; j < s; j++)
		{
			for (int k = 0; k < s; k++)
			{
				int c = 1;
				float x = static_cast<float>(i) / 64.0f;
				float y = static_cast<float>(j) / 64.0f;
				float z = static_cast<float>(k) / 64.0f;

				int o;
				float frequency = 1.0f;
				float amplitude = 1.0f;
				float sum = 0.0f;

				for (o = 0; o < 8; o++) {
					float r = stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 4, 4, 4, (unsigned char)o) * amplitude;
					sum += (float)fabs(r);
					frequency *= 2.0f;
					amplitude *= 0.5f;
				}
				buffer[k * s * s * c + j * s * c + i * c + 0] = sum;
			}
		}
	}
	fogMaterialVolume->upload(buffer);
	delete[] buffer;

	inScatteringVolume = Texture3D::create(160, 90, 64, GL::RGBA16F);
	inScatteringVolume->setFilter(GL::LINEAR, GL::LINEAR);
	inScatteringVolume->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);

	acumFogVolume = Texture3D::create(160, 90, 64, GL::RGBA16F);
	acumFogVolume->setFilter(GL::LINEAR, GL::LINEAR);
	acumFogVolume->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
}

void Renderer::prepare(Scene::Ptr scene)
{
	scene->updateAnimations(0.0f);

	for (auto [_, root] : scene->getRootEntities())
	{
		for (auto entity : root->getChildrenWithComponent<Lod>())
		{
			auto t = entity->getComponent<Transform>();
			auto lod = entity->getComponent<Lod>();
			glm::vec3 scale = t->getScale();
			float maxScale = glm::max(glm::max(scale.x, scale.y), scale.z);
			lod->computeDistances(camera.getFov(), maxScale);
		}
	}

	initLightProbes(scene);

	IBL::computeSHLightprobes(scene, reflectionProbes);
	IBL::computeProbeMapping(scene, reflectionProbes);

	//auto renderQueue = scene->batchOpaqueInstances();
	//renderer.setRenderQueue(renderQueue);
	initLights(scene, camera);
	initShadows(scene);
	//initFogVolumes(scene);
	updateShadows(scene);

	setLights(scene->getNumLights());

	//scene->useLightMaps();

	lightMaps = scene->getLightMaps();
	directionMaps = scene->getDirectionMaps();
	iesProfiles = scene->getIESProfiles();
}

void Renderer::resize(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;

	initFBOs();
}

void Renderer::updateCamera(FPSCamera& camera)
{
	CameraUniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);

	float scale = 1.0 / log2(camera.getZFar() / camera.getZNear());
	float bias = -(log2(camera.getZNear()) * scale);

	for (auto [_, shader] : shaders)
	{
		shader->setUniform("scale", scale);
		shader->setUniform("bias", bias);
	}

	this->camera = camera;
}

void Renderer::updateCamera(FPSCamera& camera, float dt)
{
	CameraUniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);

	this->camera = camera;
}

void Renderer::updateCamera(glm::mat4 P, glm::mat4 V, glm::vec3 pos)
{
	CameraUniformData cameraData;
	cameraData.VP = P * V;
	cameraData.V = V;
	cameraData.P = P;
	cameraData.position = glm::vec4(pos, 0.0f);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::updatePostProcess(PostProcessParameters& params)
{
	params.setUniforms(postProcessShader);
	float maxLuminance = params.bloomThreshold;
	for (auto [_, shader] : shaders)
		shader->setUniform("maxLuminance", maxLuminance);
	shaders["Skybox"]->setUniform("maxLuminance", maxLuminance);
}

void Renderer::updateShadows(Scene::Ptr scene)
{
	auto depthShader = shaders["Depth"];
	auto depthCMShader = shaders["DepthCubemap"];
	auto depthCSMShader = shaders["DepthCSM"];

	int numLights = 0;
	std::vector<int> pointLightIdx;
	std::vector<Light::Ptr> pointLights;
	for (auto [_, root] : scene->getRootEntities())
	{
		for (auto lightEntity : root->getChildrenWithComponent<Light>())
		{
			auto l = lightEntity->getComponent<Light>();
			if (l->getType() == LightType::DIRECTIONAL)
			{
				csmShadowFBO->begin();
				glCullFace(GL_FRONT);
				depthCSMShader->use();
				depthCSMShader->setUniform("VP[0]", l->getViewProjections());
				renderScene(scene, depthCSMShader, false);
				csmShadowFBO->end();
				glCullFace(GL_BACK);
			}
			else
			{
				pointLights.push_back(l);
				pointLightIdx.push_back(numLights);
			}
			numLights++;
		}
	}

	if (!pointLights.empty())
	{
		omniShadowFBO->begin();
		//glCullFace(GL_FRONT);
		depthCMShader->use();
		for (int i = 0; i < pointLights.size(); i++)
		{
			auto l = pointLights[i];
			auto idx = pointLightIdx[i];
			depthCMShader->setUniform("lightIndex", idx);
			depthCMShader->setUniform("VP[0]", l->getViewProjections());
			renderScene(scene, depthCMShader, false);
		}
		omniShadowFBO->end();
		//glCullFace(GL_BACK);
	}
}

void Renderer::updateVolumes(Scene::Ptr scene)
{
	// TODO: sample local pm volumes
	// TODO: accumulate inscattering and extinction


	//GLuint timerQuery;
	//glGenQueries(1, &timerQuery);
	//glBeginQuery(GL_TIME_ELAPSED, timerQuery);

	inScatteringVolume->bind();
	glBindImageTexture(0, inScatteringVolume->getID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	auto vScatterShader = getShader("VolumeScatter");
	vScatterShader->use();
	fogMaterialVolume->use(0);
	glDispatchCompute(160, 90, 64);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	acumFogVolume->bind();
	glBindImageTexture(0, acumFogVolume->getID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	auto vAccumShader = getShader("VolumeAccum");
	vAccumShader->use();
	inScatteringVolume->use(0);
	glDispatchCompute(160, 90, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//glEndQuery(GL_TIME_ELAPSED);
	//auto end = high_resolution_clock::now();
	//auto t = duration_cast<milliseconds>(end - start).count();
	//std::cout << "inscattering time: " << t << " ms" << std::endl;

	//int done = 0;
	//while (!done)
	//	glGetQueryObjectiv(timerQuery, GL_QUERY_RESULT_AVAILABLE, &done);

	//GLuint64 elapsedTime;
	//glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsedTime);

	//double inScatter = (float)elapsedTime / 1000000.0;
	//inScatteringTime += inScatter;
	//numFramesScattering++;

	//std::cout << "inscattering time: " << (float)elapsedTime / 1000000.0 << " ms" << std::endl;
}

void Renderer::setBloom(bool useBloom)
{
	this->useBloom = useBloom;
}

void Renderer::setIBL(bool useIBL)
{
	for (auto [_, shader] : shaders)
		shader->setUniform("useIBL", useIBL);
}

void Renderer::setLights(int numLights)
{
	for (auto& [_, s] : shaders)
		s->setUniform("numLights", numLights);
}

void Renderer::setDebugChannel(int channel)
{
	for (auto& [_, s] : shaders)
		s->setUniform("debugChannel", channel);
}

void Renderer::setTonemappingOp(int index)
{
	for (auto& [_, s] : shaders)
		s->setUniform("toneMappingMode", index);
}

void Renderer::setRenderQueue(std::map<std::string, std::vector<Renderable::Ptr>> renderQueue)
{
	this->renderQueue = renderQueue;
}

void Renderer::updateTime(float dt)
{
	for (auto& [_, s] : shaders)
	{
		s->setUniform("deltaTime", dt);
		s->setUniform("material.time", dt);
		s->setUniform("vertexWind.time", dt);
	}
}

void Renderer::clear()
{
	//views.clear();
	//shadowFBOs.clear();
}

void Renderer::renderBatchedScene(Scene::Ptr scene, Shader::Ptr defaultShader)
{
	for (auto&& [shaderName, renderList] : renderQueue)
	{
		Shader::Ptr shader = nullptr;
		if (defaultShader != nullptr)
			shader = defaultShader;
		else
			shader = shaders[shaderName];
		shader->use();

		for (auto r : renderList)
		{
			shader->setUniform("numMorphTargets", 0);
			r->render(shader, defaultShader != nullptr);
		}
	}
}

void Renderer::renderScene(Scene::Ptr scene, Shader::Ptr defShader, bool transmission)
{
	//if(!renderQueue.empty())
	//{
	//	if(!transmission)
	//		renderBatchedScene(scene, defShader);
	//	return;
	//}

	for (auto [_, root] : scene->getRootEntities())
	{
		for (auto entity : root->getChildrenWithComponent<Lod>())
		{
			auto t = entity->getComponent<Transform>();
			auto lod = entity->getComponent<Lod>();
			lod->selectLod(camera.getPosition(), t->getTransform());
		}
	}

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> renderQueue;
	if (transmission)
		renderQueue = scene->getTransparentEntities();
	else
		renderQueue = scene->getOpaqueEntities();
		//renderQueue = scene->getOpaqueEntitiesCullFrustrum(camera);
	
	for (auto it = renderQueue.begin(); it != renderQueue.end(); ++it)
	{
		auto name = it->first;
		auto models = it->second;

		Shader::Ptr shader;
		if (defShader)
			shader = defShader;
		else
			shader = shaders[name];

		shader->use();
		for (auto m : models)
		{
			if (!m->isActive())
				continue;

			auto r = m->getComponent<Renderable>();
			auto t = m->getComponent<Transform>();
			Entity::Ptr p = m;
			while (p->getParent() != nullptr)
				p = p->getParent();

			auto animator = p->getComponent<Animator>();

			if (r->useMorphTargets())
			{
				std::vector<float> weights;
				if (animator)
					weights = animator->getWeights();
				else
					weights = r->getWeights();
				shader->setUniform("numMorphTargets", (int)weights.size());
				if (!weights.empty())
					shader->setUniform("morphWeights[0]", weights);
			}
			else
			{
				shader->setUniform("numMorphTargets", 0);
			}

			int animMode = 0;
			if (r->isSkinnedMesh())
			{
				animMode = 1;

				auto nodes = animator->getNodes();
				auto skin = r->getSkin();
				skin->computeJoints(nodes);
				auto boneTransforms = skin->getBoneTransform();
				auto normalTransforms = skin->getNormalTransform();
				shader->setUniform("hasAnimations", true);
				shader->setUniform("bones[0]", boneTransforms);
				shader->setUniform("normals[0]", normalTransforms);
			}
			else
			{
				shader->setUniform("hasAnimations", false);
			}

			ModelUniformData data;
			data.M = t->getTransform();
			data.N = t->getNormalMatrix();
			data.animMode = animMode;
			modelUBO.upload(&data, 1);
			r->render(shader, defShader != nullptr);
		}
	}
}

void Renderer::renderOutline(Entity::Ptr entity)
{
	glStencilMask(0x00);

	glm::vec3 color = glm::vec3(1.0f, 0.533f, 0.0f);

	outlineFBO->begin();
	unlitShader->use();
	unlitShader->setUniform("useTex", false);
	unlitShader->setUniform("orthoProjection", false);
	unlitShader->setUniform("skipEmptyFragments", false);
	unlitShader->setUniform("solidColor", glm::pow(color, glm::vec3(2.2)));

	Entity::Ptr p = entity;
	while (p->getParent() != nullptr)
		p = p->getParent();

	auto animator = p->getComponent<Animator>();
	auto models = entity->getChildrenWithComponent<Renderable>();
	for (auto m : models)
	{
		auto r = m->getComponent<Renderable>();
		auto t = m->getComponent<Transform>();

		if (r->useMorphTargets())
		{
			std::vector<float> weights;
			if (animator)
				weights = animator->getWeights();
			else
				weights = r->getWeights();
			unlitShader->setUniform("numMorphTargets", (int)weights.size());
			if (!weights.empty())
				unlitShader->setUniform("morphWeights[0]", weights);
		}
		else
		{
			unlitShader->setUniform("numMorphTargets", 0);
		}

		if (r->isSkinnedMesh())
		{
			auto nodes = animator->getNodes();
			auto skin = r->getSkin();
			skin->computeJoints(nodes); // TODO: this should be done on animation update, not each frame...
			auto boneTransforms = skin->getBoneTransform();
			auto normalTransforms = skin->getNormalTransform();
			unlitShader->setUniform("hasAnimations", true);
			unlitShader->setUniform("bones[0]", boneTransforms);
			unlitShader->setUniform("normals[0]", normalTransforms);
		}
		else
		{
			unlitShader->setUniform("hasAnimations", false);
		}

		t->setUniforms(unlitShader);
		r->render(unlitShader, true);
	}
	outlineFBO->end();

	screenFBO->bindDraw();

	// Draw selected model into stencil buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glDepthMask(0x00);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	for (auto m : models)
	{
		auto r = m->getComponent<Renderable>();
		auto t = m->getComponent<Transform>();
		t->setUniforms(unlitShader);
		r->render(unlitShader, true);
	}

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDepthMask(0xFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_DEPTH_TEST);

	outlineShader->use();
	outlineFBO->useTexture(GL::COLOR0, 0);
	screenQuad->draw();

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::renderToScreen(Scene::Ptr scene)
{
	auto screenTex = renderForward(scene);

	if (useBloom)
	{
		auto downSampleShader = shaders["DownSample"];
		downSampleShader->setUniform("inputTexture", 0);

		auto upSampleShader = shaders["DownSample"];
		upSampleShader->setUniform("inputTexture", 0);
		upSampleShader->setUniform("filterRadius", 0.005f);

		bloomTex->use(0);
		downSampleShader->use();
		int maxMipLevel = 4;
		int w = width * 0.5f;
		int h = height * 0.5f;
		downSampleShader->setUniform("mipLevel", 0);
		downSampleShader->setUniform("inputResolution", glm::vec2(w, h));
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = w * std::pow(0.5, mip);
			unsigned int mipHeight = h * std::pow(0.5, mip);
			
			bloomFBO->resize(mipWidth, mipHeight);
			bloomFBO->addRenderTexture(GL::COLOR0, bloomBlurTex, mip);
			bloomFBO->begin();

			screenQuad->draw();

			bloomFBO->end();

			bloomBlurTex->use(0);

			downSampleShader->setUniform("mipLevel", (int)mip);
			downSampleShader->setUniform("inputResolution", glm::vec2(mipWidth, mipHeight));
		}

		upSampleShader->use();
		for (unsigned int mip = maxMipLevel - 1; mip > 0; mip--)
		{
			upSampleShader->setUniform("mipLevel", (int)mip);

			unsigned int mipWidth = w * std::pow(0.5, mip - 1);
			unsigned int mipHeight = h * std::pow(0.5, mip - 1);

			bloomFBO->resize(mipWidth, mipHeight);
			bloomFBO->addRenderTexture(GL::COLOR0, bloomBlurTex, mip - 1);
			bloomFBO->begin();

			screenQuad->draw();

			bloomFBO->end();
		}
	}

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screenTex->use(0);
	bloomBlurTex->use(1);
	postProcessShader->use();
	postProcessShader->setUniform("useBloom", useBloom);
	screenQuad->draw();
}

Texture2D::Ptr Renderer::renderToTexture(Scene::Ptr scene)
{
	auto mainTex = renderForward(scene);

	screenFBO->bind();
	auto selectedModel = scene->getCurrentModel();
	if (selectedModel != nullptr)
		renderOutline(selectedModel);
	screenFBO->end();

	postFBO->begin();
	screenTex->use(0);
	unlitShader->setUniform("M", glm::mat4(1.0f));
	unlitShader->setUniform("orthoProjection", true);
	unlitShader->setUniform("useGammaEncoding", true);
	unlitShader->setUniform("useTex", true);
	unlitShader->setUniform("numMorphTargets", 0);
	unlitShader->setUniform("hasAnimations", false);
	unlitShader->use();
	screenQuad->draw();
	postFBO->end();

	return postTex;
}

Texture2D::Ptr Renderer::renderForward(Scene::Ptr scene)
{
	// TODO: put tex units elsewhere, they only need to be set once
	irradianceMaps->use(11);
	specularMapsGGX->use(12);
	specularMapsSheen->use(13);
	brdfLUT->use(14);

	if (csmShadowMap)
		csmShadowMap->use(15);
	if (omniShadowMap)
		omniShadowMap->use(16);
	if (lightMaps)
		lightMaps->use(17);
	if (directionMaps)
		directionMaps->use(18);
	if (iesProfiles)
		iesProfiles->use(19);
	if (acumFogVolume)
		acumFogVolume->use(21);
	if (inScatteringVolume)
		inScatteringVolume->use(22);
	if (inScatteringVolume)
		inScatteringVolume->use(23);

	glStencilMask(0xFF);
	screenFBO->begin();
	glStencilMask(0x00);
	//if (useSkybox)
	{
		glCullFace(GL_FRONT);
		skyboxShader->use();
		//scene->useSkybox();
		skybox->use(0);
		unitCube->draw();
		glCullFace(GL_BACK);
	}

	renderScene(scene, nullptr, false);
	//renderBatchedScene(scene, nullptr);

	for (auto [_, root] : scene->getRootEntities())
	{
		for (auto ps : root->getComponentsInChildren<ParticleSystem>())
		{
			ps->renderParticles();
		}
	}

	if (scene->hasTransmission())
	{
		// This is quite expensive... find a better way to do this!
		refrTex->generateMipmaps();
		refrTex->use(10);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	renderScene(scene, nullptr, true);

	screenFBO->end();

	return screenTex;
}

Shader::Ptr Renderer::getShader(std::string name)
{
	if (shaders.find(name) != shaders.end())
		return shaders[name];
	else
		return nullptr;
} 