#include "Scene.h"

#include <Graphics/MeshPrimitives.h>
#include <IO/ImageLoader.h>
#include <IO/AssimpImporter.h>

#include <Physics/Intersection.h>

#include <glm/gtx/matrix_decompose.hpp>

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

	unitCube = MeshPrimitives::createCube(glm::vec3(0), 1.0f);

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

void Scene::initLights(std::map<std::string, Shader::Ptr>& shaders)
{
	//auto lightEntity = Entity::create("light");
	//lightEntity->addComponent(Light::create(LightType::POINT, glm::vec3(1), 10.0f, 10.0f));
	//lightEntity->getComponent<Transform>()->setPosition(glm::vec3(0, 1, 0));
	//rootEntities.insert(std::make_pair("light", lightEntity));

	std::vector<Light::UniformData> lightData;
	float maxIntensity = 0.0;
	for (auto [name, entity] : rootEntities)
	{
		auto lightEntities = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightEntities)
		{
			auto t = lightEntity->getComponent<Transform>();
			auto l = lightEntity->getComponent<Light>();
			glm::vec3 color = l->getColor();
			float intensity = l->getIntensity();
			float maxComponent = intensity * glm::max(glm::max(color.r, color.g), color.b);
			maxIntensity = glm::max(maxIntensity, maxComponent);

			Light::UniformData data;
			l->writeUniformData(data, t);
			lightData.push_back(data);
		}
	}

	float value = glm::min(maxIntensity, 10000.0f);
	float factor = value / maxIntensity;

	lightUBO.upload(lightData, GL_DYNAMIC_DRAW);
	lightUBO.bindBase(1);

	for (auto [_, s] : shaders)
	{
		s->setUniform("numLights", (int)lightData.size());
		s->setUniform("apertureFactor", factor);
		s->setUniform("pq", usePerceptualQuantization);
	}		

	views.clear();
	for (auto [name, entity] : rootEntities)
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			glm::mat4 M = lightEntity->getComponent<Transform>()->getTransform();
			glm::vec3 skew;
			glm::vec4 persp;
			glm::vec3 pos;
			glm::vec3 scale;
			glm::quat rot;
			glm::decompose(M, scale, rot, pos, skew, persp);

			glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
			std::vector<glm::mat4> VP;
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			VP.push_back(P * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
			views.push_back(VP);
		}
	}
}

void Scene::initShadowMaps()
{
	for (auto [name, entity] : rootEntities)
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			const unsigned int size = 1024;
			auto shadowMap = TextureCubeMap::create(size, size, GL::DEPTH16);
			shadowMap->setCompareMode();

			auto shadowFBO = Framebuffer::create(size, size);
			shadowFBO->addRenderTexture(GL::DEPTH, shadowMap);
			shadowFBO->checkStatus();
			shadowFBOs.push_back(shadowFBO);
		}
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

	usePerceptualQuantization |= importer.needsPQ;

	auto models = rootEntity->getComponentsInChildren<Renderable>();
	for (auto r : models)
	{
		if (r->useBlending() || r->isTransmissive())
			useTransmission = true;
	}
	rootEntity->getAllNodes(allEntities);

	cameras = importer.getCameras();
	variants = importer.getVariants();
	importer.clear();

	//auto renderables = rootEntity->getChildrenWithComponent<Renderable>();
	//for (auto e : renderables)
	//{
	//	auto t = e->getComponent<Transform>();
	//	auto r = e->getComponent<Renderable>();

	//	glm::mat4 M = t->getTransform();
	//	AABB bbox = r->getBoundingBox();

	//	boundingBoxes.push_back(MeshPrimitives::createLineBox(bbox.getCenter(), bbox.getSize()));
	//	boxMat.push_back(M);
	//}
}

void Scene::addEntity(std::string name, Entity::Ptr entity)
{
	rootEntities.insert(std::make_pair(name, entity));
}

void Scene::addLight(std::string name)
{
	glm::vec3 lightColor = glm::vec3(1.0f);

	auto lightEntity = Entity::create(name, nullptr);
	lightEntity->addComponent(Light::create(LightType::POINT, lightColor, 10.0f, 10.0f));
	//lightEntity->getComponent<Transform>()->setScale(glm::vec3(0.05f));

	auto r = Renderable::create();
	RenderPrimitive prim;
	prim.mesh = MeshPrimitives::createSphere(glm::vec3(0), 0.05f, 32, 32);
	prim.mesh->setBoundingBox(glm::vec3(-0.05), glm::vec3(0.05));
	prim.materials.push_back(getDefaultMaterial());
	prim.materials[0]->addProperty("material.baseColorFactor", glm::vec4(lightColor, 1.0));
	prim.materials[0]->addProperty("material.unlit", true);
	r->addPrimitive(prim);
	lightEntity->addComponent(r);

	rootEntities.insert(std::make_pair(lightEntity->getName(), lightEntity));
	lightEntity->getAllNodes(allEntities);
}

void Scene::removeRootEntity(std::string name)
{
	rootEntities.erase(name);
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

void Scene::renderBoxes(Shader::Ptr shader)
{
	glEnable(GL_LINE_WIDTH);
	glLineWidth(1.0f);

	shader->use();
	shader->setUniform("useTex", false);
	shader->setUniform("orthoProjection", false);
	shader->setUniform("solidColor", glm::vec3(1.0f, 0.5f, 0.0f));
	for (int i = 0; i < boundingBoxes.size(); i++)
	{
		shader->setUniform("M", boxMat[i]);
		boundingBoxes[i]->draw();
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

	//lights.clear();
	views.clear();
	variants.clear();
	cameras.clear();
}

void Scene::updateBoxes()
{
	boundingBoxes.clear();
	boxMat.clear();

	for (auto [_, rootEntity] : rootEntities)
	{
		auto renderables = rootEntity->getChildrenWithComponent<Renderable>();
		for (auto e : renderables)
		{
			auto t = e->getComponent<Transform>();
			auto r = e->getComponent<Renderable>();
			glm::mat4 M = t->getTransform();
			AABB bbox = r->getBoundingBox();
			boundingBoxes.push_back(MeshPrimitives::createLineBox(bbox.getCenter(), bbox.getSize()));
			boxMat.push_back(M);
		}
	}
}

void Scene::selectBox(Entity::Ptr e)
{
	boundingBoxes.clear();
	boxMat.clear();

	auto t = e->getComponent<Transform>();
	auto r = e->getComponent<Renderable>();
	if (r != nullptr)
	{
		glm::mat4 M = t->getTransform();
		AABB bbox = r->getBoundingBox();
		boundingBoxes.push_back(MeshPrimitives::createLineBox(bbox.getCenter(), bbox.getSize()));
		boxMat.push_back(M);
	}
	else
	{
		glm::mat4 M = t->getTransform();
		AABB selectedBox;
		auto renderEntities = e->getChildrenWithComponent<Renderable>();
		for (auto e : renderEntities)
		{
			auto meshT = e->getComponent<Transform>();
			auto meshR = e->getComponent<Renderable>();
			AABB bbox = meshR->getBoundingBox();
			glm::mat4 local2world = meshT->getTransform();

			auto points = bbox.getPoints();
			for (auto p : points)
			{
				p = glm::vec3(local2world * glm::vec4(p, 1.0f));
				p = glm::vec3(glm::inverse(M) * glm::vec4(p, 1.0f));
				selectedBox.expand(p);
			}
		}

		boundingBoxes.push_back(MeshPrimitives::createLineBox(selectedBox.getCenter(), selectedBox.getSize()));
		boxMat.push_back(M);
	}

	selectedModel = e;
}

void Scene::unselect()
{
	boundingBoxes.clear();
	boxMat.clear();

	selectedModel = nullptr;
}

bool Scene::hasTransmission()
{
	return useTransmission;
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

Entity::Ptr Scene::getRootNode(std::string name)
{
	if (rootEntities.find(name) != rootEntities.end())
		return rootEntities[name];
	return nullptr;
}

Entity::Ptr Scene::getCurrentModel()
{
	return selectedModel;
}

Entity::Ptr Scene::getNode(int id)
{
	return allEntities[id];
}

Entity::Ptr Scene::selectModelRaycast(glm::vec3 start, glm::vec3 end)
{
	Entity::Ptr nearestEntity = nullptr;
	float minDist = std::numeric_limits<float>::max();
	int hitCount = 0;

	for (auto [name, entity] : rootEntities)
	{
		//AABB modelBox; // TODO: This could precomputed for root nodes, or select AABB for each submesh
		auto renderables = entity->getChildrenWithComponent<Renderable>();
		for (auto e : renderables)
		{
			auto t = e->getComponent<Transform>();
			auto r = e->getComponent<Renderable>();

			AABB bbox = r->getBoundingBox();
			glm::mat4 M = t->getTransform();
			glm::mat4 M_I = glm::inverse(t->getTransform());
			glm::vec3 startModel = glm::vec3(M_I * glm::vec4(start, 1.0f));
			glm::vec3 endModel = glm::vec3(M_I * glm::vec4(end, 1.0f));

			Ray ray;
			ray.origin = startModel;
			ray.direction = glm::normalize(endModel - startModel);
			ray.dirInv = 1.0f / ray.direction;

			glm::vec3 hitpoint;
			if (Intersection::rayBoxIntersection(ray, bbox, hitpoint))
			{
				hitCount++;
				glm::vec3 h = glm::vec3(M * glm::vec4(hitpoint, 1.0f));
				float dist = glm::distance(h, start);
				if (dist < minDist)
				{
					minDist = dist;
					nearestEntity = e;
				}
			}
		}
	}

	std::cout << "hit " << hitCount << " meshes" << std::endl;
	//if (rootEntities.find(nearestModel) != rootEntities.end())
	//	return rootEntities[nearestModel];

	return nearestEntity;
}

std::vector<Entity::Ptr> Scene::selectModelsRaycast(glm::vec3 start, glm::vec3 end)
{
	std::map<float, Entity::Ptr> hitEntities;
	float minDist = std::numeric_limits<float>::max();
	int hitCount = 0;

	for (auto [name, entity] : rootEntities)
	{
		//AABB modelBox; // TODO: This could precomputed for root nodes, or select AABB for each submesh
		auto renderables = entity->getChildrenWithComponent<Renderable>();
		for (auto e : renderables)
		{
			auto t = e->getComponent<Transform>();
			auto r = e->getComponent<Renderable>();

			AABB bbox = r->getBoundingBox();
			glm::mat4 M = t->getTransform();
			glm::mat4 M_I = glm::inverse(t->getTransform());
			glm::vec3 startModel = glm::vec3(M_I * glm::vec4(start, 1.0f));
			glm::vec3 endModel = glm::vec3(M_I * glm::vec4(end, 1.0f));

			Ray ray;
			ray.origin = startModel;
			ray.direction = glm::normalize(endModel - startModel);
			ray.dirInv = 1.0f / ray.direction;

			glm::vec3 hitpoint;
			if (Intersection::rayBoxIntersection(ray, bbox, hitpoint))
			{
				hitCount++;
				glm::vec3 h = glm::vec3(M * glm::vec4(hitpoint, 1.0f));
				float dist = glm::distance(h, start);
				hitEntities.insert(std::make_pair(dist, e));
			}
		}
	}

	std::vector<Entity::Ptr> entities;
	for (auto [_, e] : hitEntities)
		entities.push_back(e);

	return entities;
}