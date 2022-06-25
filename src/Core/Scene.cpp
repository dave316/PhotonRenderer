#include "Scene.h"

#include <Graphics/MeshPrimitives.h>

#include <IO/AssimpImporter.h>
#include <IO/GLTFImporter.h>
#include <Physics/Intersection.h>
#include <IO/Image/ImageDecoder.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std::chrono;

Scene::Scene(const std::string& name) :
	name(name)
{
	//Channel<glm::vec3>::Ptr channel(new Channel<glm::vec3>(Interpolation::LINEAR));
}

Scene::~Scene()
{
	for (auto [name, entity] : rootEntities)
	{
		auto animators = entity->getComponentsInChildren<Animator>();
		for (auto& a : animators)
			a->clear();
		entity->clearParent();
	}
	rootEntities.clear();
	allEntities.clear();
}

void Scene::initEnvMaps(std::string fn, std::map<std::string, Shader::Ptr>& shaders)
{
	auto start = steady_clock::now();

	auto pano2cmShader = shaders["PanoToCubeMap"];
	auto iblFilterShader = shaders["IBLFilter"];
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

	unitCube = MeshPrimitives::createCube(glm::vec3(0), 1.0f);

	//std::string assetPath = "../../../../assets";
	//std::string assetPath = "C:/Users/dave316/Seafile/Assets/EnvMaps";

	int size = 1024;
	skybox = EnvironmentMap::create(size);
	irradianceMap = EnvironmentMap::create(32);
	specularMapGGX = EnvironmentMap::create(256);
	specularMapCharlie = EnvironmentMap::create(256);

	// TODO: add format to env map and saving to KTX
	//skybox->fromFile("skybox.ktx2");
	//irradianceMap->fromFile("irradianceMap.ktx2");
	//specularMapGGX->fromFile("specularMapGGX.ktx2");
	//specularMapCharlie->fromFile("specularMapCharlie.ktx2");

	glDisable(GL_DEPTH_TEST);	

	//auto pano = IO_Legacy::loadTextureHDR(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/neutral.hdr");
	//skybox->fromPanorama(pano, pano2cmShader);
 	//skybox->fromFile(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");

	//auto panoImg = IO::decodeHDRFromFile(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr", true);
	auto panoImg = IO::decodeHDRFromFile(fn, true);
	auto pano = panoImg->upload(false);
	skybox->fromPanorama(pano, pano2cmShader);
	//skybox->fromFile(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");

	iblFilterShader->use();
	iblFilterShader->setUniform("texSize", size);

	// irradiance probe
	iblFilterShader->setUniform("filterIndex", 0);
	iblFilterShader->setUniform("sampleCount", 2048);
	irradianceMap->filter(skybox, iblFilterShader);

	// specular probe
	iblFilterShader->setUniform("filterIndex", 1);
	iblFilterShader->setUniform("sampleCount", 1024);
	specularMapGGX->filterMips(skybox, iblFilterShader, 8);

	// specular sheen
	iblFilterShader->setUniform("filterIndex", 2);
	iblFilterShader->setUniform("sampleCount", 64);
	specularMapCharlie->filterMips(skybox, iblFilterShader, 8);

	glEnable(GL_DEPTH_TEST);

	//auto end = steady_clock::now();
	//auto t = duration_cast<milliseconds>(end - start).count();
	//std::cout << "baking env probes took: " << t << " ms" << std::endl;

	//IO::saveCubemapKTX("skybox.ktx2", skybox->getCubeMap());
	//IO::saveCubemapKTX("irradianceMap.ktx2", irradianceMap->getCubeMap());
	//IO::saveCubemapKTX("specularMapGGX.ktx2", specularMapGGX->getCubeMap());
	//IO::saveCubemapKTX("specularMapCharlie.ktx2", specularMapCharlie->getCubeMap());
}

void Scene::initLightProbe(EnvironmentMap::Ptr lightProbe, std::map<std::string, Shader::Ptr>& shaders)
{
	auto iblFilterShader = shaders["IBLFilter"];

	reflectionProbe = lightProbe;

	int size = 1024;
	irradianceProbe = EnvironmentMap::create(32);
	specularProbe = EnvironmentMap::create(256);

	glDisable(GL_DEPTH_TEST);
	iblFilterShader->use();
	iblFilterShader->setUniform("texSize", size);

	// irradiance probe
	iblFilterShader->setUniform("filterIndex", 0);
	iblFilterShader->setUniform("sampleCount", 2048);
	irradianceProbe->filter(lightProbe, iblFilterShader);

	// specular probe
	iblFilterShader->setUniform("filterIndex", 1);
	iblFilterShader->setUniform("sampleCount", 1024);
	specularProbe->filterMips(lightProbe, iblFilterShader, 8);

	glEnable(GL_DEPTH_TEST);
}

void Scene::initLights(std::map<std::string, Shader::Ptr>& shaders)
{
	//auto lightEntity = Entity::create("light", nullptr);
	//lightEntity->addComponent(Light::create(LightType::POINT, glm::vec3(1), 10.0f, 10.0f));
	//lightEntity->getComponent<Transform>()->setPosition(glm::vec3(0, 1, 1));
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

	//for (auto& [_, s] : shaders)
	//{
	//	s->setUniform("numLights", (int)lightData.size());
	//	s->setUniform("apertureFactor", factor);
	//	s->setUniform("pq", usePerceptualQuantization);
	//}

	views.clear();
	for (auto [name, entity] : rootEntities)
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			auto l = lightEntity->getComponent<Light>();
			if (l->getType() == LightType::DIRECTIONAL)
			{
				glm::mat4 M = lightEntity->getComponent<Transform>()->getTransform();
				glm::vec3 skew;
				glm::vec4 persp;
				glm::vec3 pos;
				glm::vec3 scale;
				glm::quat rot;
				glm::decompose(M, scale, rot, pos, skew, persp);
				rot = glm::normalize(rot);
				glm::vec3 dir = glm::mat3_cast(rot) * glm::vec3(0, 0, -1);
				glm::mat4 V = glm::lookAt(pos - 2.0f*dir, pos + dir, glm::vec3(0, 1, 0));
				if (glm::length(pos) > 0)
					V = glm::inverse(M);

				// TODO: compute optimal ortho frustrum based on scene extents
				AABB bbox = getBoundingBox();
				glm::vec3 size = bbox.getSize();
				float maxExtent = glm::max(glm::max(size.x, size.y), size.z);
				//std::cout << "max extent: " << maxExtent << std::endl;
				glm::mat4 P = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
				std::vector<glm::mat4> VP;
				VP.push_back(P * V);
				views.push_back(VP);
			}
			else
			{
				glm::mat4 M = lightEntity->getComponent<Transform>()->getTransform();
				glm::vec3 skew;
				glm::vec4 persp;
				glm::vec3 pos;
				glm::vec3 scale;
				glm::quat rot;
				glm::decompose(M, scale, rot, pos, skew, persp);

				//std::cout << "light pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

				glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
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
}

void Scene::initShadowMaps()
{
	for (auto [name, entity] : rootEntities)
	{
		auto lightsEntity = entity->getChildrenWithComponent<Light>();
		for (auto lightEntity : lightsEntity)
		{
			const unsigned int size = 2048;
			auto l = lightEntity->getComponent<Light>();
			if (l->getType() == LightType::DIRECTIONAL)
			{
				auto shadowMap = Texture2D::create(size, size, GL::DEPTH16);
				shadowMap->setFilter(GL::NEAREST, GL::NEAREST);
				shadowMap->setWrap(GL::CLAMP_TO_BORDER, GL::CLAMP_TO_BORDER);
				shadowMap->bind();
				GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

				auto shadowFBO = Framebuffer::create(size, size);
				shadowFBO->addRenderTexture(GL::DEPTH, shadowMap);
				shadowFBO->checkStatus();
				shadowFBOs.push_back(shadowFBO);
			}
			else
			{				
				auto shadowMap = TextureCubeMap::create(size, size, GL::DEPTH16);
				shadowMap->setCompareMode();

				auto shadowFBO = Framebuffer::create(size, size);
				shadowFBO->addRenderTexture(GL::DEPTH, shadowMap);
				shadowFBO->checkStatus();
				shadowFBOs.push_back(shadowFBO);
			}
		}
	}
}

bool Scene::loadModelGLTF(std::string name, std::string path)
{
	modelInfo.clear();
	renderInfo.clear();

	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(path);
	if (rootEntity)
	{
		auto rootTransform = rootEntity->getComponent<Transform>();
		rootEntity->update(glm::mat4(1.0f));
		rootEntities.insert(std::make_pair(name, rootEntity));
		currentModel = name;

		modelInfo = importer.getGeneralStats();
		renderInfo = importer.getRenderingStats();
	}
	else
	{
		// TODO: better error handling if model could not be loaded
		return false;
	}
	//usePerceptualQuantization |= importer.needsPQ;

	auto models = rootEntity->getComponentsInChildren<Renderable>();
	for (auto r : models)
	{
		if (r->useBlending() || r->isTransmissive())
			useTransmission = true;
	}
	rootEntity->getAllNodes(allEntities);

	auto cameraEntites = rootEntity->getChildrenWithComponent<Camera>();
	for (auto e : cameraEntites)
	{
		auto t = e->getComponent<Transform>();
		auto cam = e->getComponent<Camera>();

		std::string name = cam->getName();
		glm::mat4 T = t->getTransform();
		glm::mat4 V = glm::inverse(T);
		glm::mat4 P = cam->getProjection();
		glm::vec3 pos = T * glm::vec4(0, 0, 0, 1);
		cameras[name] = CameraInfo(P, V, pos);
	}

	//cameras = importer.getCameras();
	//variants = importer.getVariants();
	//importer.clear();

	AABB worldBox;
	auto renderables = rootEntity->getChildrenWithComponent<Renderable>();
	for (auto e : renderables)
	{
		auto t = e->getComponent<Transform>();
		auto r = e->getComponent<Renderable>();

		glm::mat4 M = t->getTransform();
		AABB bbox = r->getBoundingBox();
		glm::vec3 minPoint = glm::vec3(M * glm::vec4(bbox.getMinPoint(), 1.0f));
		glm::vec3 maxPoint = glm::vec3(M * glm::vec4(bbox.getMaxPoint(), 1.0f));
		worldBox.expand(minPoint);
		worldBox.expand(maxPoint);
		//boundingBoxes.push_back(MeshPrimitives::createLineBox(bbox.getCenter(), bbox.getSize()));
		//boxMat.push_back(M);
	}
	glm::vec3 s = worldBox.getSize();
	//std::cout << "model size: " << s.x << " " << s.y << " " << s.z << std::endl;

	//switchAnimations(1);

	return true;
}

#ifdef WITH_ASSIMP
bool Scene::loadModelASSIMP(std::string name, std::string path)
{
	IO::AssimpImporter importer;
	auto model = importer.importModel(path);
	if (model == nullptr)
		return false;
	addEntity(name, model);
	return true;
}
#endif

void Scene::addEntity(std::string name, Entity::Ptr entity)
{
	rootEntities.insert(std::make_pair(name, entity));
	currentModel = name;
	auto models = entity->getComponentsInChildren<Renderable>();
	for (auto r : models)
	{
		if (r->useBlending() || r->isTransmissive())
			useTransmission = true;
	}
	entity->getAllNodes(allEntities);
}

void Scene::addLight(std::string name)
{
	glm::vec3 lightColor = glm::vec3(1.0f,0.2,0.1);
	//glm::vec3 lightColor = glm::vec3(1.0f);

	auto lightEntity = Entity::create(name, nullptr);
	//lightEntity->addComponent(Light::create(LightType::SPOT, lightColor, 20.0f, 25.0f));
	lightEntity->addComponent(Light::create(LightType::DIRECTIONAL, lightColor, 2.0f, 10.0f));
	//lightEntity->addComponent(Light::create(LightType::POINT, lightColor, 20.0f, 10.0f));
	//lightEntity->getComponent<Transform>()->setLocalRotation(glm::angleAxis(glm::radians(45.0f), glm::vec3(-1, 0, 0)));
	//lightEntity->getComponent<Transform>()->setLocalPosition(glm::vec3(0,2,0));
	//lightEntity->getComponent<Light>()->setConeAngles(glm::pi<float>() / 8.0f, glm::pi<float>() / 4.0f);

	auto r = Renderable::create();
	//Mesh mesh;
	float radius = 0.05f;
	auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
	prim->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	auto mat = getDefaultMaterial();
	mat->addProperty("material.baseColorFactor", glm::vec4(lightColor, 1.0));
	mat->addProperty("material.unlit", true);
	prim->setMaterial(mat);
	auto mesh = Mesh::create("Sphere");
	mesh->addPrimitive(prim);
	//mesh->addMaterial(mat);
	r->setMesh(mesh);

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

		auto cameraEntites = rootEntity->getChildrenWithComponent<Camera>();
		for (auto e : cameraEntites)
		{
			auto t = e->getComponent<Transform>();
			auto cam = e->getComponent<Camera>();

			std::string name = cam->getName();
			glm::mat4 T = t->getTransform();
			cameras[name].V = glm::inverse(T);
			cameras[name].P = cam->getProjection();
			cameras[name].pos = T * glm::vec4(0, 0, 0, 1);
		}
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
	if (!rootEntities.empty())
	{
		auto e = rootEntities[currentModel];
		auto renderables = e->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
			r->switchMaterial(idx);
	}
}

void Scene::nextMaterial()
{
	//static unsigned int materialIndex = 0;
	//materialIndex = (++materialIndex) % 5;

	////if (rootEntities.find("GlamVelvetSofa") != rootEntities.end())
	//{
	//	//auto e = rootEntitis["GlamVelvetSofa"];
	//	auto e = rootEntities[currentModel];
	//	auto renderables = e->getComponentsInChildren<Renderable>();
	//	for (auto r : renderables)
	//		r->switchMaterial(materialIndex);
	//}
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
	if (reflectionProbe == nullptr)
	{
		irradianceMap->use(15);
		specularMapGGX->use(16);
		specularMapCharlie->use(17);
	}
	else
	{
		irradianceProbe->use(15);
		specularProbe->use(16);
	}

	for (int i = 0; i < views.size(); i++)
	{
		if (views[i].size() == 1)
		{
			shadowFBOs[i]->useTexture(GL::DEPTH, 19);
		}
		else
		{
			// TODO: find next free slot in omni shadow maps
			shadowFBOs[i]->useTexture(GL::DEPTH, 8 + i);
		}
	}
	//for (int i = 0; i < shadowFBOs.size(); i++)
	//	shadowFBOs[i]->useTexture(GL::DEPTH, 10 + i);
}

void Scene::useSkybox()
{
	//if (reflectionProbe == nullptr)
		skybox->use(0);
	//else
	//	reflectionProbe->use(0);

}

void Scene::clear()
{
	for (auto [name, entity] : rootEntities)
	{
		auto animators = entity->getComponentsInChildren<Animator>();
		for (auto& a : animators)
			a->clear();
		entity->clearParent();
	}
	rootEntities.clear();
	allEntities.clear();
	cameras.clear();
	views.clear();
	shadowFBOs.clear();
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

int Scene::numLights()
{
	return lightUBO.size();
}

std::map<std::string, int> Scene::getModelInfo()
{
	return modelInfo;
}

std::map<std::string, int> Scene::getRenderInfo()
{
	return renderInfo;
}

CameraInfo Scene::getCameraInfo(std::string name)
{
	if (cameras.find(name) != cameras.end())
		return cameras[name];
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
	for (auto [name,_] : cameras)
		names.push_back(name);
	return names;
}

std::vector<std::string> Scene::getVariantNames()
{
	if (!rootEntities.empty())
	{
		auto renderables = rootEntities[currentModel]->getComponentsInChildren<Renderable>();
		auto mesh = renderables[0]->getMesh();
		return mesh->getVariants();
	}	
	return std::vector<std::string>();
}

std::vector<std::string> Scene::getAnimations()
{
	std::vector<std::string> names;
	if (!rootEntities.empty())
	{
		auto animator = rootEntities[currentModel]->getComponent<Animator>();
		if (animator)
			names = animator->getAnimationNames();
	}
	return names;
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

	if (!entities.empty())
	{
		Entity::Ptr entitiy = entities[0];
		while (entitiy->getParent() != nullptr)
			entitiy = entitiy->getParent();
		if (entitiy->getID() != entities[0]->getID())
			entities.insert(entities.begin(), entitiy);
	}

	return entities;
}

std::map<std::string, Entity::Ptr> Scene::getRootEntities()
{
	return rootEntities;
}

std::map<std::string, std::vector<Entity::Ptr>> Scene::getOpaqueEntities()
{
	std::map<std::string, std::vector<Entity::Ptr>> mapping;
	for (auto [_, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto r = m->getComponent<Renderable>();
			auto mesh = r->getMesh();
			auto primitives = mesh->getPrimitives();
			auto mat = primitives[0]->getMaterial();
			if (!mat->isTransmissive() && !mat->useBlending())
			{
				std::string shaderName = mat->getShader();
				if (mapping.find(shaderName) == mapping.end())
					mapping.insert(std::make_pair(shaderName, std::vector<Entity::Ptr>()));
				mapping[shaderName].push_back(m);
			}
		}
	}

	//for (auto [name, entities] : mapping)
	//	std::cout << "shader " << name << ", entities: " << entities.size() << std::endl;
	return mapping;
}

std::map<std::string, std::vector<Entity::Ptr>> Scene::getTransparentEntities()
{
	std::map<std::string, std::vector<Entity::Ptr>> mapping;
	for (auto [_, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto r = m->getComponent<Renderable>();
			auto mesh = r->getMesh();
			auto primitives = mesh->getPrimitives();
			auto mat = primitives[0]->getMaterial();
			if (mat->isTransmissive() || mat->useBlending())
			{
				std::string shaderName = mat->getShader();
				if (mapping.find(shaderName) == mapping.end())
					mapping.insert(std::make_pair(shaderName, std::vector<Entity::Ptr>()));
				mapping[shaderName].push_back(m);
			}
		}
	}

	//for (auto [name, entities] : mapping)
	//	std::cout << "shader " << name << ", entities: " << entities.size() << std::endl;
	return mapping;
}