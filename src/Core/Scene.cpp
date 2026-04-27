#include "Scene.h"

#include <Core/Animator.h>
#include <Core/Renderable.h>
#include <Graphics/Intersection.h>
#include <set>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>
namespace pr
{
	Scene::Scene(const std::string& name) :
		name(name)
	{

	}

	Scene::~Scene()
	{
		for (auto root : rootNodes)
			root->clearParent();
		rootNodes.clear();
	}

	void Scene::destroy()
	{

	}

	void Scene::addRoot(pr::Entity::Ptr root)
	{
		rootNodes.push_back(root);
	}

	void Scene::addLightDesc(GPU::DescriptorSet::Ptr lightDescSet)
	{
		if (lightMaps != nullptr)
		{
			lightDescSet->addDescriptor(lightMaps->getDescriptor());
			lightDescSet->addDescriptor(directionMaps->getDescriptor());
		}
	}

	void Scene::setSkybox(pr::TextureCubeMap::Ptr skybox)
	{
		this->skybox = skybox;
	}

	void Scene::setLightMaps(pr::Texture2DArray::Ptr lightMaps)
	{
		this->lightMaps = lightMaps;
	}

	void Scene::setDirMaps(pr::Texture2DArray::Ptr dirMaps)
	{
		this->directionMaps = dirMaps;
	}

	void Scene::setSHProbes(pr::SHLightProbes& probes)
	{
		this->shProbes = probes;
	}

	void Scene::initDescriptors(GPU::DescriptorPool::Ptr descriptorPool)
	{
		for (auto root : rootNodes)
		{
			for (auto e : root->getChildrenWithComponent<Renderable>())
			{
				auto t = e->getComponent<Transform>();
				auto r = e->getComponent<Renderable>();
				r->setDescriptor(descriptorPool);
				r->update(t->getTransform());

				if (r->isSkinnedMesh())
					r->getSkin()->setDescriptor(descriptorPool);
			}
		}
	}

	void Scene::initLightProbes(ReflectionProbes& rp, std::vector<pr::TextureCubeMap::Ptr>& lightProbes)
	{

		std::vector<ReflectionProbe> reflProbes;
		//std::vector<pr::TextureCubeMap::Ptr> lightProbes;
		lightProbes.push_back(skybox);

		ReflectionProbe globalProbe;
		globalProbe.index = 0;
		reflProbes.push_back(globalProbe);

		std::map<std::string, int> names;
		std::vector<Entity::Ptr> reflectionEntities;
		for (auto entity : rootNodes)
		{
			auto probes = entity->getChildrenWithComponent<pr::LightProbe>();
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
				probe.boxMax = glm::vec4(pos + bbox.getMaxPoint(), 1.0f);
				reflProbes.push_back(probe);
				std::string name = "";
				if (names.find(p->getName()) == names.end())
				{
					names.insert(std::make_pair(p->getName(), 0));
					name = p->getName();
				}					
				else
				{
					names[p->getName()]++;
					name = p->getName() + "_" + std::to_string(names[p->getName()]);
				}					

				reflectionProbes.insert(std::make_pair(name, probe));

				reflectionEntities.push_back(p);
			}
		}

		for (auto e : reflectionEntities)
			lightProbes.push_back(e->getComponent<LightProbe>()->getCubeMap());

		//ReflectionProbes rp;
		for (int i = 0; i < reflProbes.size(); i++)
			rp.probes[i] = reflProbes[i];
	}

	void Scene::checkWindingOrder()
	{
		for (auto root : rootNodes)
		{
			root->update(glm::mat4(1.0f));
			for (auto e : root->getChildrenWithComponent<Renderable>())
			{
				auto t = e->getComponent<Transform>();
				glm::vec3 scale = t->getScale();
				float sign = glm::sign(scale.x * scale.y * scale.z);
				if (sign < 0)
				{
					auto r = e->getComponent<Renderable>();
					auto mesh = r->getMesh();
					auto meshCopy = pr::Mesh::create(mesh->getName());

					for (auto m : mesh->getSubMeshes())
					{
						auto surface = m.primitive->getSurface();
						surface.flipWindingOrder();

						SubMesh s;
						s.primitive = pr::Primitive::create(m.primitive->getName(), surface, GPU::Topology::Triangles);
						s.primitive->createData();
						s.primitive->uploadData();
						s.material = m.material;
						meshCopy->addSubMesh(s);
					}

					auto newRend = pr::Renderable::create(meshCopy);
					newRend->setLightMapST(r->getLMOffset(), r->getLMScale());
					newRend->setLightMapIndex(r->getLMIndex());
					newRend->setDiffuseMode(r->getDiffuseMode());
					e->addComponent(newRend);
				}
			}
		}
	}

	void Scene::computeSHLightprobes()
	{
		auto tetrahedrons = shProbes.tetrahedras;
		auto probeCoeffs = shProbes.coeffs;
		auto positions = shProbes.positions;

		if (tetrahedrons.empty() || probeCoeffs.empty() || positions.empty())
			return;

		std::set<int> indices;

		int index = 0;
		for (auto root : rootNodes)
		{
			auto rendEnts = root->getChildrenWithComponent<Renderable>();
			for (auto e : rendEnts)
			{
				auto r = e->getComponent<Renderable>();
				auto mesh = r->getMesh();
				if (e->isActive() && r->isEnabled())
				{
					if (r->getDiffuseMode() == 2)
					{
						//std::cout << "mesh " << e->getName() << " uses lightmapping." << std::endl;
					}
					else
					{
						//std::cout << "mesh " << e->getName() << " uses lightprobes." << std::endl;
						glm::vec3 pos = glm::vec3(0);
						if (!r->getReflName().empty())
						{
							auto name = r->getReflName();
							if (reflectionProbes.find(name) != reflectionProbes.end())
								pos = reflectionProbes[name].position;
						}
						else
						{
							auto M = e->getComponent<Transform>()->getTransform();
							Box meshbox;
							for (auto s : mesh->getSubMeshes())
							{
								auto surf = s.primitive->getSurface();
								for (auto v : surf.vertices)
								{
									glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0f));
									meshbox.expand(pos);
								}
							}
							pos = meshbox.getCenter();
						}

						IO::Unity::SH9 probe;
						auto p = glm::vec3(-pos.x, pos.y, pos.z);
						for (int i = 0; i < tetrahedrons.size(); i++)
						{
							auto& t = tetrahedrons[i];
							if (t.indices[3] < 0)
								continue;

							auto p3 = positions[t.indices[3]];
							auto R = glm::transpose(glm::mat3(t.matrix));
							auto bc = R * (p - p3);
							float a = bc.x;
							float b = bc.y;
							float c = bc.z;
							float d = 1.0f - a - b - c;

							if (a >= 0 && b >= 0 && c >= 0 && d >= 0) // point is inside tetrahedra
							{
								indices.insert(i);

								auto p0 = positions[t.indices[0]];
								auto p1 = positions[t.indices[1]];
								auto p2 = positions[t.indices[2]];

								auto& sh0 = probeCoeffs[t.indices[0]].coefficients;
								auto& sh1 = probeCoeffs[t.indices[1]].coefficients;
								auto& sh2 = probeCoeffs[t.indices[2]].coefficients;
								auto& sh3 = probeCoeffs[t.indices[3]].coefficients;
								for (int i = 0; i < 27; i++)
									probe.coefficients[i] = sh0[i] * a + sh1[i] * b + sh2[i] * c + sh3[i] * d;

								//std::cout << "probe idx: " << pIdx << " tetrahedron: " << i << std::endl;
								break;
							}
						}

						std::vector<glm::vec3> sh(9);
						for (int k = 0; k < 9; k++)
						{
							sh[k].r = probe.coefficients[k];
							sh[k].g = probe.coefficients[k + 9];
							sh[k].b = probe.coefficients[k + 18];
						}

						r->setDiffuseMode(1);
						r->setProbeSH9(sh);

						//SubMesh m;
						//m.primitive = MeshPrimitives::createUVSphere(pos, 0.1f, 16, 16);
						//m.material = getDefaultMaterial();
						//m.material->addProperty("material.baseColorFactor", glm::vec4(1, 1, 1, 1));
						//m.material->addProperty("material.roughnessFactor", 1.0f);
						//m.material->addProperty("material.metallicFactor", 0.0f);

						//auto meshLP = Mesh::create("lightprobe");
						//meshLP->addSubMesh(m);

						//auto lpEntity = Entity::create("lightprobe_" + e->getName(), nullptr);
						//auto lpRend = lpEntity->addComponent<Renderable>();
						//lpRend->setMesh(meshLP);
						//lpRend->setDiffuseMode(1);
						//lpRend->setProbeSH9(sh);
						//scene->addRootEntity("mesh_lightprobe_" + std::to_string(index), lpEntity);
						//index++;
					}
				}
			}
		}
	}

	void Scene::computeProbeMapping()
	{
		std::vector<Entity::Ptr> nodes;
		//auto rootNodes = scene->getRootNodes();
		for (auto rootNode : rootNodes)
		{
			auto rendNodes = rootNode->getChildrenWithComponent<Renderable>();
			for (auto e : rendNodes)
				nodes.push_back(e);
		}

		for (auto n : nodes)
		{
			auto r = n->getComponent<Renderable>();
			auto t = n->getComponent<Transform>();
			//auto meshBox = r->getBoundingBox();
			auto M = t->getTransform();
			auto mesh = r->getMesh();
			Box meshbox;
			for (auto s : mesh->getSubMeshes())
			{
				auto surf = s.primitive->getSurface();
				for (auto v : surf.vertices)
				{
					glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0f));
					meshbox.expand(pos);
				}
			}

			if (r->getRPIndex() < 0)
			{
				auto name = r->getReflName();
				if (reflectionProbes.find(name) != reflectionProbes.end())
					r->setReflectionProbe(name, reflectionProbes[name].index);
				else
					r->setReflectionProbe("", 0);
			}
			else
			{
				if (reflectionProbes.size() == 1) // no need to compute local proxy if theres is only the global probe
					continue;

				auto M = t->getTransform();
				//auto bmin = glm::vec3(M * glm::vec4(meshBox.getMinPoint(), 1.0f));
				//auto bmax = glm::vec3(M * glm::vec4(meshBox.getMaxPoint(), 1.0f));
				auto mmin = meshbox.getMinPoint();
				auto mmax = meshbox.getMaxPoint();
				glm::vec3 size = mmax - mmin;
				float eps = std::numeric_limits<float>::epsilon();
				if (size.x == 0) size.x = eps;
				if (size.y == 0) size.y = eps;
				if (size.z == 0) size.z = eps;
				float meshVolume = size.x * size.y * size.z;
				//std::cout << n->getName() << " mesh volume: " << meshVolume << std::endl;
				//std::cout << n->getName() << " mesh size: " << size.x << " " << size.y << " " << size.z << std::endl;

				//std::cout << n->getName() << std::endl;
				std::vector<std::pair<std::string, float>> weights;
				for (auto& [name, probe] : reflectionProbes)
				{
					glm::vec3 bmin = probe.boxMin;
					glm::vec3 bmax = probe.boxMax;
					if ((mmax.x >= bmin.x && bmax.x >= mmin.x) &&
						(mmax.y >= bmin.y && bmax.y >= mmin.y) &&
						(mmax.z >= bmin.z && bmax.z >= mmin.z))
					{
						glm::vec3 imin = glm::max(bmin, mmin);
						glm::vec3 imax = glm::min(bmax, mmax);
						glm::vec3 size = imax - imin;
						if (size.x == 0) size.x = eps;
						if (size.y == 0) size.y = eps;
						if (size.z == 0) size.z = eps;
						float intersectionVolume = size.x * size.y * size.z;
						float ratio = intersectionVolume / meshVolume * 100.0f;
						weights.push_back(std::make_pair(name, ratio));
						//std::cout << env.name << " ratio: " << ratio << std::endl;
					}
				}

				// TODO: set max. probes to be blended / recompute weights / implement weighted blending
				//std::cout << n->getName() << " blend weights " << std::endl;
				float maxWeight = 0.0f;
				std::string maxName;
				float sum = 0.0f;
				for (auto [_, w] : weights)
					sum += w;
				for (auto [n, w] : weights)
				{
					float reweighted = w * 100.0f / sum;
					//std::cout << envMapData[idx].name << " weight: " << reweighted << std::endl;
					if (reweighted > maxWeight)
					{
						maxWeight = reweighted;
						maxName = n;
					}
				}

				//std::cout << n->getName() << " max. weight: " << maxName << " " << reflectionProbes[maxName].index << " " << maxWeight << std::endl;

				r->setReflectionProbe(maxName, reflectionProbes[maxName].index);
			}
		}
	}

	void Scene::update(float dt)
	{
		for (auto root : rootNodes)
		{
			auto a = root->getComponent<Animator>();
			if (a)
				a->update(dt);

			root->update(glm::mat4(1.0f));

			if (a)
			{
				for (auto e : root->getChildrenWithComponent<Renderable>())
				{
					auto r = e->getComponent<Renderable>();
					if (r->hasMorphtargets())
					{
						r->setCurrentWeights(a->getWeights());
					}

					if (r->isSkinnedMesh())
					{
						auto nodes = a->getNodes();
						auto skin = r->getSkin();
						skin->computeJoints(nodes);
					}
				}
			}

			for (auto e : root->getChildrenWithComponent<Renderable>())
			{
				auto t = e->getComponent<Transform>();
				auto r = e->getComponent<Renderable>();
				r->update(t->getTransform());
			}
		}
	}

	void Scene::switchVariant(int index)
	{
		//if (numVariants == 0)
		//	return;

		//currentVariant = (++currentVariant) % numVariants;
		//auto rootNodes = scene->getRootNodes();
		for (auto root : rootNodes)
			for (auto r : root->getComponentsInChildren<Renderable>())
				r->switchVariant(index);
	}

	void Scene::switchAnimation(int index)
	{
		//if (numAnimations == 0)
		//	return;

		//currentAnimation = (++currentAnimation) % numAnimations;

		//auto rootNodes = scene->getRootNodes();
		for (auto root : rootNodes)
		{
			auto a = root->getComponent<Animator>();
			if (a)
				a->switchAnimation(index);
		}
	}

	Box Scene::getBoundingBox()
	{
		Box sceneBox;

		//auto rootNodes = scene->getRootNodes();
		for (auto root : rootNodes)
		{
			for (auto e : root->getChildrenWithComponent<Renderable>())
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

	std::vector<Entity::Ptr> Scene::selectModelsRaycast(glm::vec3 start, glm::vec3 end)
	{
		std::map<float, Entity::Ptr> hitEntities;
		for (auto entity : rootNodes)
		{
			if (!entity->isActive())
				continue;

			for (auto e : entity->getChildrenWithComponent<Renderable>())
			{
				if (!e->isActive())
					continue;

				auto t = e->getComponent<Transform>();
				auto r = e->getComponent<Renderable>();

				auto bbox = r->getBoundingBox();
				auto M = t->getTransform();
				auto M_I = glm::inverse(M);
				auto startModel = glm::vec3(M_I * glm::vec4(start, 1.0));
				auto endModel = glm::vec3(M_I * glm::vec4(end, 1.0));
				auto direction = glm::normalize(endModel - startModel);
				Ray ray(startModel, direction);
				glm::vec3 hitPoint;
				if (Intersection::rayBoxIntersection(ray, bbox, hitPoint))
				{
					glm::vec3 h = glm::vec3(M * glm::vec4(hitPoint, 1.0));
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
			// TODO: so this is a hack to get the root node of the hit models
			Entity::Ptr entity = entities[0];
			while (entity->getParent() != nullptr)
				entity = entity->getParent();
			if (entity->getID() != entities[0]->getID())
				entities.insert(entities.begin(), entity);
		}

		return entities;
	}

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> Scene::getOpaqueEntities()
	{
		std::map<int, std::map<std::string, std::vector<Entity::Ptr>>> mapping;
		for (auto entity : rootNodes)
		{
			std::string name = entity->getName();
			auto models = entity->getChildrenWithComponent<Renderable>(true);
			for (auto m : models)
			{
				auto r = m->getComponent<Renderable>();
				if (r->isEnabled() && r->getType() == RenderType::Opaque)
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
						if (mat) // TODO: add pink debug material when it is missing
						{
							std::string shaderName = mat->getShaderName();
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

	std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> Scene::getTransparentEntities()
	{
		std::map<int, std::map<std::string, std::vector<Entity::Ptr>>> mapping;
		for (auto entity : rootNodes)
		{
			auto models = entity->getChildrenWithComponent<Renderable>(true);
			for (auto m : models)
			{
				auto r = m->getComponent<Renderable>();
				if (r->isEnabled() && r->getType() == RenderType::Transparent)
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
						std::string shaderName = mat->getShaderName();
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

	std::vector<pr::Entity::Ptr> Scene::getRootNodes()
	{
		return rootNodes;
	}

	pr::TextureCubeMap::Ptr Scene::getSkybox()
	{
		return skybox;
	}
}