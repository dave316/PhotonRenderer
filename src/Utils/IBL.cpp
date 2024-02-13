#include "IBL.h"

#include <Graphics/MeshPrimitives.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace IBL
{
	std::vector<glm::mat4> createCMViews(glm::vec3 position = glm::vec3(0))
	{
		std::vector<glm::mat4> VP;
		glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		VP.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		return VP;
	}

	TextureCubeMap::Ptr createCubemapFromColor(glm::vec3 color, int size, float rotation)
	{
		glClearColor(color.r, color.g, color.b, 1.0f);

		glm::vec3 position = glm::vec3(0);
		auto M = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0));
		auto VP = createCMViews(position);
		auto cubemap = TextureCubeMap::create(size, size, GL::RGB32F);
		cubemap->generateMipmaps();
		auto envFBO = Framebuffer::create(size, size);
		envFBO->addRenderTexture(GL::COLOR0, cubemap);
		envFBO->begin();
		envFBO->end();
		cubemap->generateMipmaps();
		cubemap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		return cubemap;
	}

	TextureCubeMap::Ptr convertEqui2CM(Shader::Ptr pano2cm, Texture2D::Ptr panorama, int size, float rotation, float exposure)
	{
		glm::vec3 position = glm::vec3(0);
		auto M = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0));
		auto VP = createCMViews(position);
		auto cubeMesh = MeshPrimitives::createCube(position, 1.0f);
		auto cubemap = TextureCubeMap::create(size, size, GL::RGB32F); // TODO: set format depending on use
		cubemap->generateMipmaps();
		auto envFBO = Framebuffer::create(size, size);
		envFBO->addRenderTexture(GL::COLOR0, cubemap);

		pano2cm->use();
		pano2cm->setUniform("M", M);
		pano2cm->setUniform("VP[0]", VP);
		pano2cm->setUniform("panorama", 0);
		pano2cm->setUniform("exposure", exposure);

		envFBO->checkStatus();
		envFBO->begin();
		panorama->use(0);
		cubeMesh->draw();
		envFBO->end();

		cubemap->generateMipmaps();
		cubemap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		return cubemap;
	}

	// creates a cubemap array for irradiance probes
	TextureCubeMapArray::Ptr filterLambert(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes)
	{
		if (lightProbes.empty())
			return nullptr;

		glm::vec3 position = glm::vec3(0);
		auto views = createCMViews(position);
		unsigned int numProbes = lightProbes.size();
		unsigned int faceSize = 32;
		int layerID = 0;

		auto cubeMesh = MeshPrimitives::createCube(position, 1.0f);
		auto irradianceMaps = TextureCubeMapArray::create(faceSize, faceSize, numProbes, GL::RGB16F);
		auto envFBO = Framebuffer::create(faceSize, faceSize);
		envFBO->addRenderTexture(GL::COLOR0, irradianceMaps);
		envFBO->checkStatus();
		envFBO->begin();

		filter->use();
		filter->setUniform("M", glm::mat4(1.0f));
		filter->setUniform("VP[0]", views);
		filter->setUniform("environmentMap", 0);
		filter->setUniform("filterIndex", 0);
		filter->setUniform("sampleCount", 2048);

		for (auto& lp : lightProbes)
		{
			filter->setUniform("texSize", lp->getFaceSize());
			filter->setUniform("layerID", layerID);
			lp->use(0);
			cubeMesh->draw();
			layerID++;
		}

		envFBO->end();

		return irradianceMaps;
	}

	TextureCubeMapArray::Ptr filterSpecularGGX(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes)
	{
		glm::vec3 position = glm::vec3(0);
		auto views = createCMViews(position);

		// specular probe
		filter->use();
		filter->setUniform("M", glm::mat4(1.0f));
		filter->setUniform("VP[0]", views);
		filter->setUniform("environmentMap", 0);
		filter->setUniform("filterIndex", 1);
		filter->setUniform("sampleCount", 1024);

		int numProbes = lightProbes.size();
		int faceSize = 256;
		int maxMipLevel = 8;

		auto cubeMesh = MeshPrimitives::createCube(position, 1.0f);
		auto specFBO = Framebuffer::create(faceSize, faceSize);
		auto specularMaps = TextureCubeMapArray::create(faceSize, faceSize, numProbes, GL::RGB16F);
		specularMaps->generateMipmaps();
		specularMaps->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);		

		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = faceSize * std::pow(0.5, mip);
			unsigned int mipHeight = faceSize * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			filter->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMaps, mip);
			specFBO->begin();

			int layerID = 0;
			for (auto lp : lightProbes)
			{
				filter->setUniform("texSize", lp->getFaceSize());
				filter->setUniform("layerID", layerID);
				lp->use(0);
				cubeMesh->draw();
				layerID++;
			}

			specFBO->end();
		}

		return specularMaps;
	}

	TextureCubeMapArray::Ptr filterSpecularSheen(Shader::Ptr filter, std::vector<LightProbe::Ptr> lightProbes)
	{
		glm::vec3 position = glm::vec3(0);
		auto views = createCMViews(position);

		// specular probe
		filter->use();
		filter->setUniform("M", glm::mat4(1.0f));
		filter->setUniform("VP[0]", views);
		filter->setUniform("environmentMap", 0);
		filter->setUniform("filterIndex", 2);
		filter->setUniform("sampleCount", 64);

		int numProbes = lightProbes.size();
		int faceSize = 256;
		int maxMipLevel = 8;

		auto cubeMesh = MeshPrimitives::createCube(position, 1.0f);
		auto specFBO = Framebuffer::create(faceSize, faceSize);
		auto specularMaps = TextureCubeMapArray::create(faceSize, faceSize, numProbes, GL::RGB16F);
		specularMaps->generateMipmaps();
		specularMaps->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = faceSize * std::pow(0.5, mip);
			unsigned int mipHeight = faceSize * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			filter->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMaps, mip);
			specFBO->begin();

			int layerID = 0;
			for (auto lp : lightProbes)
			{
				filter->setUniform("texSize", lp->getFaceSize());
				filter->setUniform("layerID", layerID);
				lp->use(0);
				cubeMesh->draw();
				layerID++;
			}

			specFBO->end();
		}

		return specularMaps;
	}

	void computeSHLightprobes(Scene::Ptr scene, std::map<std::string, ReflectionProbe>& reflProbes)
	{
		SHLightProbes& shProbes = scene->getLightProbes();

		auto tetrahedrons = shProbes.tetrahedras;
		auto probeCoeffs = shProbes.coeffs;
		auto positions = shProbes.positions;

		if (tetrahedrons.empty() || probeCoeffs.empty() || positions.empty())
			return;

		std::set<int> indices;
		
		int index = 0;
		for (auto [name, root] : scene->getRootEntities())
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
						glm::vec3 pos;
						if (!r->getReflName().empty())
						{
							auto name = r->getReflName();
							if (reflProbes.find(name) != reflProbes.end())
								pos = reflProbes[name].position;
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

	void computeProbeMapping(Scene::Ptr scene, std::map<std::string, ReflectionProbe>& reflProbes)
	{
		std::vector<Entity::Ptr> nodes;
		for (auto [name, rootNode] : scene->getRootEntities())
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
				if (reflProbes.find(name) != reflProbes.end())
					r->setReflectionProbe(name, reflProbes[name].index);
				else
					r->setReflectionProbe("", 0);
			}
			else
			{
				if (reflProbes.size() == 1) // no need to compute local proxy if theres is only the global probe
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
				for (auto& [name, probe] : reflProbes)
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

				//std::cout << n->getName() << " max. weight: " << maxName << " " << reflProbes[maxName].index << " " << maxWeight << std::endl;

				r->setReflectionProbe(maxName, reflProbes[maxName].index);
			}
		}
	}
}

