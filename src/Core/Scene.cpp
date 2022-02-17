#include "Scene.h"

#include <Graphics/Primitives.h>
#include <IO/ImageLoader.h>
#include <IO/AssimpImporter.h>

Scene::Scene(const std::string& name) :
	name(name)
{}

Scene::~Scene()
{
	for (auto&& [name, entity] : rootEntities)
	{
		auto animators = entity->getComponentsInChildren<Animator>();
		for (auto& a : animators)
			a->clear();
	}
}

void Scene::initEnvMaps(std::map<std::string, Shader::Ptr>& shaders)
{
	// TODO: put env maps in its own class & remove baking from renderer
	auto pano2cmShader = shaders["PanoToCubeMap"];
	auto irradianceShader = shaders["IBLDiffuseIrradiance"];
	auto specularShader = shaders["IBLSpecular"];
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

	unitCube = Primitives::createCube(glm::vec3(0), 1.0f);

	std::string assetPath = "../../../../assets";
	auto pano = IO::loadTextureHDR(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/Newport_Loft/Newport_Loft_Ref.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/office.hdr");

	if (pano == nullptr)
	{
		std::cout << "no panorama loaded, IBL deactivated" << std::endl;
		return;
	}

	//useSkybox = true;

	glm::vec3 position = glm::vec3(0);
	std::vector<glm::mat4> VP;

	glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	pano2cmShader->setUniform("M", glm::mat4(1.0f));
	pano2cmShader->setUniform("VP[0]", VP);
	pano2cmShader->setUniform("panorama", 0);
	pano2cmShader->use();

	glDisable(GL_DEPTH_TEST);
	{
		int size = 1024;
		cubeMap = TextureCubeMap::create(size, size, GL::RGB32F);
		cubeMap->generateMipmaps();
		cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto envFBO = Framebuffer::create(size, size);
		envFBO->addRenderTexture(GL::COLOR0, cubeMap);
		envFBO->checkStatus();
		envFBO->begin();
		pano->use(0);
		unitCube->draw();
		envFBO->end();

		cubeMap->generateMipmaps();
	}

	irradianceShader->setUniform("VP[0]", VP);
	irradianceShader->setUniform("environmentMap", 0);
	irradianceShader->use();

	{
		int size = 32;
		irradianceMap = TextureCubeMap::create(size, size, GL::RGB32F);
		auto irrFBO = Framebuffer::create(size, size);
		irrFBO->addRenderTexture(GL::COLOR0, irradianceMap);
		irrFBO->checkStatus();
		irrFBO->begin();
		cubeMap->use(0);
		unitCube->draw();
		irrFBO->end();
	}

	specularShader->setUniform("VP[0]", VP);
	specularShader->setUniform("environmentMap", 0);
	specularShader->setUniform("filterIndex", 0);
	specularShader->use();

	{
		int size = 256;
		specularMapGGX = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMapGGX->generateMipmaps();
		specularMapGGX->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 8;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMapGGX, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	specularShader->setUniform("VP[0]", VP);
	specularShader->setUniform("environmentMap", 0);
	specularShader->setUniform("filterIndex", 1);
	specularShader->use();

	{
		int size = 256;
		specularMapCharlie = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMapCharlie->generateMipmaps();
		specularMapCharlie->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 5;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMapCharlie, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	glEnable(GL_DEPTH_TEST);
}

void Scene::initLights(Shader::Ptr defaultShader)
{
	int i = 0;
	std::vector<Light::UniformData> lightData(lights.size());
	for (auto it : lights)
	{
		it.second->writeUniformData(lightData[i]);
		i++;
	}

	lightUBO.upload(lightData, GL_DYNAMIC_DRAW);
	lightUBO.bindBase(1);

	defaultShader->setUniform("numLights", (int)lights.size());

	// TODO: this has to be correctly updated, when lights are added...
	for (auto it : lights)
	{
		auto light = it.second;

		glm::vec3 pos = light->getPosition();
		glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
		std::vector<glm::mat4> VP;
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		views.push_back(VP);

		const unsigned int size = 4096;
		auto shadowMap = TextureCubeMap::create(size, size, GL::DEPTH24);
		shadowMap->setCompareMode();

		auto shadowFBO = Framebuffer::create(size, size);
		shadowFBO->addRenderTexture(GL::DEPTH, shadowMap);
		shadowFBO->checkStatus();
		shadowFBOs.push_back(shadowFBO);
	}
}

void Scene::loadModel(std::string name, std::string path)
{
	IO::GLTFImporter importer;
	auto rootEntity = importer.importModel(path);
	if (rootEntity)
	{
		auto rootTransform = rootEntity->getComponent<Transform>();
		rootEntity->update(glm::mat4(1.0f));
		rootEntities.insert(std::make_pair(name, rootEntity));
		currentModel = name;
	}

	auto modelLights = importer.getLights();
	for (int i = 0; i < modelLights.size(); i++)
	{
		std::string lightName = name + "_light_" + std::to_string(i);
		lights.insert(std::make_pair(lightName, modelLights[i]));
	}

	cameras = importer.getCameras();
	variants = importer.getVariants();
	importer.clear();
}

void Scene::addEntity(std::string name, Entity::Ptr entity)
{
	rootEntities.insert(std::make_pair(name, entity));
}

void Scene::updateAnimations(float dt)
{
	for (auto [name, rootEntity] : rootEntities)
	{
		auto animators = rootEntity->getComponentsInChildren<Animator>();
		for (auto a : animators)
			a->update(dt);

		rootEntity->update(glm::mat4(1.0f));
	}
}

void Scene::updateAnimationState(float dt)
{
	for (auto [name, e] : rootEntities)
	{
		auto animator = e->getComponent<Animator>();
		if (animator && animator->isFinished())
			animator->play();
	}
}

void Scene::switchVariant(int idx)
{
	if (!rootEntities.empty() && idx < variants.size())
	{
		auto e = rootEntities[currentModel];
		auto renderables = e->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
			r->switchMaterial(idx);
	}
}

void Scene::nextMaterial()
{
	static unsigned int materialIndex = 0;
	materialIndex = (++materialIndex) % 5;

	//if (rootEntities.find("GlamVelvetSofa") != rootEntities.end())
	{
		//auto e = rootEntitis["GlamVelvetSofa"];
		auto e = rootEntities[currentModel];
		auto renderables = e->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
			r->switchMaterial(materialIndex);
	}
}

void Scene::playAnimations()
{
	for (auto [_, e] : rootEntities)
	{
		auto animator = e->getComponent<Animator>();
		if (animator)
			animator->play();
	}
}

void Scene::stopAnimations()
{
	for (auto [_, e] : rootEntities)
	{
		auto animator = e->getComponent<Animator>();
		if (animator)
			animator->stop();
	}
}

void Scene::switchAnimations(int index)
{
	for (auto [_, e] : rootEntities)
	{
		auto animator = e->getComponent<Animator>();
		if (animator)
			animator->switchAnimation(index);
	}
}

void Scene::useIBL()
{
	irradianceMap->use(15);
	specularMapGGX->use(16);
	specularMapCharlie->use(17);

	for (int i = 0; i < shadowFBOs.size(); i++)
		shadowFBOs[i]->useTexture(GL::DEPTH, 10 + i);
}

void Scene::useSkybox()
{
	cubeMap->use(0);
}

void Scene::clear()
{
	for (auto&& [name, entity] : rootEntities)
	{
		auto animators = entity->getComponentsInChildren<Animator>();
		for (auto& a : animators)
			a->clear();
	}
	rootEntities.clear();

	lights.clear();
	views.clear();
	variants.clear();
	cameras.clear();
}

IO::GLTFCamera Scene::getCamera(int idx)
{
	return cameras[idx];
}

std::map<std::string, Entity::Ptr>& Scene::getEntities()
{
	return rootEntities;
}

std::vector<Framebuffer::Ptr>& Scene::getShadwoFBOs()
{
	return shadowFBOs;
}

std::vector<std::vector<glm::mat4>>& Scene::getViews()
{
	return views;
}

std::vector<std::string> Scene::getCameraNames()
{
	std::vector<std::string> names;
	names.push_back("MainCamera");
	for (auto cam : cameras)
		names.push_back(cam.name);
	return names;
}

std::vector<std::string> Scene::getVariantNames()
{
	return variants;
}

AABB Scene::getBoundingBox()
{
	// compute bounding box around loaded scene
	AABB sceneBBox;
	for (auto [_,entity] : rootEntities)
	{		
		auto renderables = entity->getChildrenWithComponent<Renderable>();
		for (auto e : renderables)
		{
			auto t = e->getComponent<Transform>();
			auto r = e->getComponent<Renderable>();

			AABB bbox = r->getBoundingBox();
			glm::mat4 M = t->getTransform();
			glm::vec3 minPoint = glm::vec3(M * glm::vec4(bbox.getMinPoint(), 1.0f));
			glm::vec3 maxPoint = glm::vec3(M * glm::vec4(bbox.getMaxPoint(), 1.0f));
			sceneBBox.expand(minPoint);
			sceneBBox.expand(maxPoint);
		}
	}

	return sceneBBox;
}