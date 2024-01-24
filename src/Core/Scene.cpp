#include "Scene.h"

#include <Math/Intersection.h>
#include <Math/Frustrum.h>

#include <IO/GLTFImporter.h>

Scene::Scene(const std::string& name) :
	name(name)
{

}

Scene::~Scene()
{
	clear();
}

void Scene::addRootEntity(std::string name, Entity::Ptr entity)
{
	if (rootEntities.find(name) != rootEntities.end())
	{
		std::cout << "error: root entity " << name << " already exists!" << std::endl;
	}
	else
	{
		//std::cout << "adding root entity: " << name << std::endl;
		rootEntities.insert(std::make_pair(name, entity));
		currentModel = name;
	}

	for (auto r : entity->getComponentsInChildren<Renderable>())
	{
		if(r->getType() == RenderType::TRANSPARENT)
			useTransmission = true;
	}

	numLights = 0;
	for (auto [_, root] : rootEntities)
	{
		auto lights = root->getComponentsInChildren<Light>();
		numLights += lights.size();
	}

	entity->getAllNodes(allEntities);
}

void Scene::addRootEntity(Entity::Ptr entity)
{
	addRootEntity(entity->getName(), entity);
}

void Scene::addLightMaps(Texture2DArray::Ptr lightmaps)
{
	this->lightMaps = lightmaps;
}

void Scene::addDirectionMaps(Texture2DArray::Ptr dirMaps)
{
	this->directionMaps = dirMaps;
}

void Scene::setIESProfile(Texture2DArray::Ptr iesProfiles)
{
	this->iesProfiles = iesProfiles;
}

void Scene::setSkybox(Skybox& skybox)
{
	this->skybox = skybox;
}

void Scene::setLightProbes(SHLightProbes& lightProbes)
{
	this->lightProbes = lightProbes;
}

void Scene::removeRootEntity(std::string name)
{
	rootEntities.erase(name);
}

bool Scene::hasTransmission()
{
	return useTransmission;
}

void Scene::updateAnimations(float dt)
{
	for (auto [name, rootEntity] : rootEntities)
	{
		auto animators = rootEntity->getComponentsInChildren<Animator>();
		for (auto a : animators)
			a->update(dt);

		rootEntity->update(glm::mat4(1.0f));

		//auto cameraEntites = rootEntity->getChildrenWithComponent<Camera>();
		//for (auto e : cameraEntites)
		//{
		//	auto t = e->getComponent<Transform>();
		//	auto cam = e->getComponent<Camera>();

		//	std::string name = cam->getName();
		//	glm::mat4 T = t->getTransform();
		//	cameras[name].V = glm::inverse(T);
		//	cameras[name].P = cam->getProjection();
		//	cameras[name].pos = T * glm::vec4(0, 0, 0, 1);
		//}
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

void Scene::switchVariant(int index)
{
	if (!rootEntities.empty())
	{
		auto e = rootEntities[currentModel];
		auto renderables = e->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
			r->switchMaterial(index);
	}
}

void Scene::select(Entity::Ptr e)
{
	selectedModel = e;
}

void Scene::unselect()
{
	selectedModel = nullptr;
}

int Scene::getNumLights()
{
	return numLights;
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
}

PostProcessParameters Scene::getCurrentProfile(glm::vec3 cameraPosition)
{
	PostProcessParameters globalPPP;
	for (auto [_, rootNodes] : rootEntities)
	{
		for (auto v : rootNodes->getComponentsInChildren<Volume>())
		{
			if (v->isGlobal())
				globalPPP = v->getParameters();
		}

		for (auto e : rootNodes->getChildrenWithComponent<Volume>())
		{
			auto c = std::dynamic_pointer_cast<BoxCollider>(e->getComponent<Collider>());
			auto t = e->getComponent<Transform>();
			auto v = e->getComponent<Volume>();

			if (!v->isGlobal())
			{
				auto localToWorld = t->getTransform();
				auto worldToLocal = glm::inverse(localToWorld);
				auto camPosLocal = glm::vec3(worldToLocal * glm::vec4(cameraPosition, 1.0f));
				Box bbox = c->getAABB();
				auto volumeCenterLocal = bbox.getCenter();

				if (bbox.isInside(camPosLocal))
					return v->getParameters();

				Ray r(camPosLocal, glm::normalize(volumeCenterLocal - camPosLocal));
				glm::vec3 hitPoint;
				if (c->rayTest(r, hitPoint))
				{
					glm::vec3 hitPointWorld = glm::vec3(localToWorld * glm::vec4(hitPoint, 1.0f));
					float distance = glm::distance(cameraPosition, hitPointWorld);
					if (distance < v->getDistance())
					{
						auto localPPP = v->getParameters();
						float weight = distance / v->getDistance();

						PostProcessParameters blendedPPP;
						blendedPPP.postExposure = glm::mix(localPPP.postExposure, globalPPP.postExposure, weight);
						blendedPPP.bloomThreshold = glm::mix(localPPP.bloomThreshold, globalPPP.bloomThreshold, weight);
						blendedPPP.bloomIntensity = glm::mix(localPPP.bloomIntensity, globalPPP.bloomIntensity, weight);
						blendedPPP.bloomTint = glm::mix(localPPP.bloomTint, globalPPP.bloomTint, weight);
						return blendedPPP;
					}
				}
			}
		}
	}

	return globalPPP;
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

void Scene::checkWindingOrder()
{
	for (auto [name, root] : rootEntities)
	{
		root->update(glm::mat4(1.0f));
		auto entities = root->getChildrenWithComponent<Renderable>();
		for (auto e : entities)
		{
			auto t = e->getComponent<Transform>();
			glm::vec3 scale = t->getScale();
			float sign = glm::sign(scale.x * scale.y * scale.z);
			if (sign < 0)
			{
				auto r = e->getComponent<Renderable>();
				auto mesh = r->getMesh();

				auto newRend = Renderable::create();
				newRend->setLightMapST(r->getLMOffset(), r->getLMScale());
				newRend->setLightMapIndex(r->getLMIndex());
				newRend->setReflectionProbe(r->getReflName(), r->getRPIndex());
				newRend->setDiffuseMode(r->getDiffuseMode());

				auto newMesh = Mesh::create(mesh->getName());
				for (auto m : mesh->getSubMeshes())
				{
					auto surface = m.primitive->getSurface();
					surface.flipWindingOrder();
					
					SubMesh s;
					s.primitive = Primitive::create(m.primitive->getName(), surface, 4);
					s.primitive->setBoundingBox(m.primitive->getBoundingBox().getMinPoint(), m.primitive->getBoundingBox().getMaxPoint());
					s.material = m.material;
					newMesh->addSubMesh(s);
				}

				newRend->setMesh(newMesh);
				e->addComponent(newRend);
			}
		}
	}
}

std::map<std::string, Entity::Ptr> Scene::getRootEntities()
{
	return rootEntities;
}

std::map<std::string, std::vector<Renderable::Ptr>> Scene::batchOpaqueInstances()
{
	// TODO: check if opaque meshes and no skinned/animated nodes
	struct InstanceMesh
	{
		SubMesh subMesh;
		std::vector<glm::mat4> transformations;
		std::vector<IBLUniformData> iblData;
	};
	std::map<unsigned int, InstanceMesh> meshInstances;
	for (auto [name, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto t = m->getComponent<Transform>();
			auto r = m->getComponent<Renderable>();

			if (m->isActive() && r->isEnabled())
			{
				auto mesh = r->getMesh();

				for (auto s : mesh->getSubMeshes())
				{
					auto id = s.primitive->getID();
					if (meshInstances.find(id) == meshInstances.end())
					{
						InstanceMesh inst;
						inst.subMesh = s;
						meshInstances.insert(std::make_pair(id, inst));
					}

					meshInstances[id].transformations.push_back(t->getTransform());

					IBLUniformData data;
					data.diffuseMode = r->getDiffuseMode();
					data.specularProbeIndex = r->getRPIndex();
					data.lightMapIndex = r->getLMIndex();
					data.lightMapST = glm::vec4(r->getLMOffset(), r->getLMScale());
					auto sh9 = r->getSH9();
					for (int i = 0; i < sh9.size(); i++)
						data.sh[i] = glm::vec4(sh9[i], 0.0f);

					meshInstances[id].iblData.push_back(data);
				}
			}
		}
	}

	std::vector<IBLUniformData> iblData;
	std::map<std::string, std::vector<Renderable::Ptr>> renderQueue;
	for (auto [id, inst] : meshInstances)
	{
		//std::cout << "Prim ID: " << id << ", name: " << inst.subMesh.primitive->getName() << ", #transform: " << inst.transformations.size() << std::endl;

		// TODO: check winding order!
		SubMesh s;
		s.primitive = inst.subMesh.primitive;
		s.material = inst.subMesh.material;

		GLuint matrixBuffer;
		glGenBuffers(1, &matrixBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
		glBufferData(GL_ARRAY_BUFFER, inst.transformations.size() * sizeof(glm::mat4), inst.transformations.data(), GL_STATIC_DRAW);

		GLuint vaoID = s.primitive->getVaoID();
		glBindVertexArray(vaoID);
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(11);
		glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);
		glVertexAttribDivisor(11, 1);

		glBindVertexArray(0);

		int offset = iblData.size();
		for (auto ibl : inst.iblData)
			iblData.push_back(ibl);

		s.primitive->setInstances(inst.transformations.size());

		auto mesh = Mesh::create("mesh");
		mesh->addSubMesh(s);

		auto r = Renderable::create();
		r->setMesh(mesh);
		r->setOffset(offset);

		auto shaderName = s.material->getShader();
		if (renderQueue.find(shaderName) == renderQueue.end())
			renderQueue.insert(std::make_pair(shaderName, std::vector<Renderable::Ptr>()));
		renderQueue[shaderName].push_back(r);
	}

	iblUBO.bindBase(4);
	iblUBO.upload(iblData, GL_DYNAMIC_DRAW);

	return renderQueue;
}

std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> Scene::getOpaqueEntitiesCullFrustrum(FPSCamera& camera)
{
	Math::Frustrum frustrum(camera);

	std::map<int, std::map<std::string, std::vector<Entity::Ptr>>> mapping;
	for (auto [name, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto t = m->getComponent<Transform>();
			auto r = m->getComponent<Renderable>();
			if (r->isEnabled() && r->getType() == RenderType::OPAQUE)
			{
				unsigned int p = r->getPriority();
				if (mapping.find(p) == mapping.end())
					mapping.insert(std::make_pair(p, std::map<std::string, std::vector<Entity::Ptr>>()));

				auto mesh = r->getMesh();
				auto subMeshes = mesh->getSubMeshes();
				std::set<std::string> usedShader;
				for (auto s : subMeshes)
				{
					Box bbox = s.primitive->getBoundingBox();
					glm::mat4 M = t->getTransform();

					if (frustrum.isInside(bbox, M))
					{
						auto mat = s.material;
						std::string shaderName = mat->getShader();
						if (usedShader.find(shaderName) != usedShader.end())
							continue;						

						auto& shaderMapping = mapping[p];
						if (shaderMapping.find(shaderName) == shaderMapping.end())
							shaderMapping.insert(std::make_pair(shaderName, std::vector<Entity::Ptr>()));
						shaderMapping[shaderName].push_back(m);
						usedShader.insert(shaderName);
					}
				}
			}
		}
	}

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> renderQueue;
	for (auto [_, shaderMapping] : mapping)
		for (auto [name, models] : shaderMapping)
			renderQueue.push_back(std::make_pair(name, models));

	return renderQueue;
}

std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> Scene::getOpaqueEntities()
{
	std::map<int, std::map<std::string, std::vector<Entity::Ptr>>> mapping;
	for (auto [name, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto r = m->getComponent<Renderable>();
			if (r->isEnabled() && r->getType() == RenderType::OPAQUE)
			{
				unsigned int p = r->getPriority();
				if (mapping.find(p) == mapping.end())
					mapping.insert(std::make_pair(p, std::map<std::string, std::vector<Entity::Ptr>>()));

				auto mesh = r->getMesh();
				auto subMeshes = mesh->getSubMeshes();
				std::set<std::string> usedShader;
				for (auto s : subMeshes)
				{
					auto mat = s.material;
					std::string shaderName = mat->getShader();
					if (usedShader.find(shaderName) != usedShader.end())
						continue;
					
					auto& shaderMapping = mapping[p];
					if (shaderMapping.find(shaderName) == shaderMapping.end())
						shaderMapping.insert(std::make_pair(shaderName, std::vector<Entity::Ptr>()));
					shaderMapping[shaderName].push_back(m);
					usedShader.insert(shaderName);
				}
			}
		}
	}

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> renderQueue;
	for (auto [_, shaderMapping] : mapping)
		for (auto [name, models] : shaderMapping)
			renderQueue.push_back(std::make_pair(name, models));

	return renderQueue;
}

std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> Scene::getTransparentEntities()
{
	std::map<int, std::map<std::string, std::vector<Entity::Ptr>>> mapping;
	for (auto [name, entity] : rootEntities)
	{
		auto models = entity->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			auto r = m->getComponent<Renderable>();
			if (r->isEnabled() && r->getType() == RenderType::TRANSPARENT)
			{
				unsigned int p = r->getPriority();
				if (mapping.find(p) == mapping.end())
					mapping.insert(std::make_pair(p, std::map<std::string, std::vector<Entity::Ptr>>()));

				auto mesh = r->getMesh();
				auto subMeshes = mesh->getSubMeshes();
				std::set<std::string> usedShader;
				for (auto s : subMeshes)
				{
					auto mat = s.material;
					std::string shaderName = mat->getShader();
					if (usedShader.find(shaderName) != usedShader.end())
						continue;

					auto& shaderMapping = mapping[p];
					if (shaderMapping.find(shaderName) == shaderMapping.end())
						shaderMapping.insert(std::make_pair(shaderName, std::vector<Entity::Ptr>()));
					shaderMapping[shaderName].push_back(m);
					usedShader.insert(shaderName);
				}
			}
		}
	}

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> renderQueue;
	for (auto [_, shaderMapping] : mapping)
	{
		for (auto it = shaderMapping.rbegin(); it != shaderMapping.rend(); ++it)
		{
			auto name = it->first;
			auto models = it->second;
			renderQueue.push_back(std::make_pair(name, models));
		}
	}
	
	return renderQueue;
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

			Box bbox = r->getBoundingBox();
			glm::mat4 M = t->getTransform();
			glm::mat4 M_I = glm::inverse(t->getTransform());
			glm::vec3 startModel = glm::vec3(M_I * glm::vec4(start, 1.0f));
			glm::vec3 endModel = glm::vec3(M_I * glm::vec4(end, 1.0f));

			Ray ray(startModel, glm::normalize(endModel - startModel));
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

Entity::Ptr Scene::getNode(int id)
{
	return allEntities[id];
}

Entity::Ptr Scene::getCurrentModel()
{
	return selectedModel;
}

Box Scene::getBoundingBox()
{
	Box sceneBox;
	for (auto [_, entity] : rootEntities)
	{
		auto renderables = entity->getChildrenWithComponent<Renderable>();
		for (auto e : renderables)
		{
			auto t = e->getComponent<Transform>();
			auto r = e->getComponent<Renderable>();

			Box bbox = r->getBoundingBox();
			glm::mat4 M = t->getTransform();
			glm::vec3 minPoint = glm::vec3(M * glm::vec4(bbox.getMinPoint(), 1.0f));
			glm::vec3 maxPoint = glm::vec3(M * glm::vec4(bbox.getMaxPoint(), 1.0f));
			sceneBox.expand(minPoint);
			sceneBox.expand(maxPoint);
		}
	}

	return sceneBox;
}

Texture2DArray::Ptr Scene::getLightMaps()
{
	return lightMaps;
}

Texture2DArray::Ptr Scene::getDirectionMaps()
{
	return directionMaps;
}

Texture2DArray::Ptr Scene::getIESProfiles()
{
	return iesProfiles;
}

SHLightProbes& Scene::getLightProbes()
{
	return lightProbes;
}

Skybox& Scene::getSkybox()
{
	return skybox;
}