#include "Renderer.h"

#include <Graphics/Framebuffer.h>
#include <Graphics/Primitives.h>
#include <Graphics/Shader.h>

#include <IO/GLTFImporter.h>
#include <IO/ImageLoader.h>
#include <IO/ShaderLoader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace json = rapidjson;

void extern debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		std::cout << std::hex << std::endl;
		std::cout << "OpenGL error: type=" << type << " severity: " << severity << " message: " << message << std::endl;
	}
}

AABB::AABB()
{
	float maxVal = std::numeric_limits<float>::max();
	float minVal = -maxVal;

	minPoint = glm::vec3(maxVal);
	maxPoint = glm::vec3(minVal);
}

AABB::AABB(glm::vec3& minPoint, glm::vec3& maxPoint) :
	minPoint(minPoint),
	maxPoint(maxPoint)
{

}

glm::vec3 AABB::getMinPoint() const
{
	return minPoint;
}

glm::vec3 AABB::getMaxPoint() const
{
	return maxPoint;
}

void AABB::expand(const glm::vec3& point)
{
	maxPoint = glm::max(maxPoint, point);
	minPoint = glm::min(minPoint, point);
}

void AABB::expand(const Triangle& tri)
{
	expand(tri.v0);
	expand(tri.v1);
	expand(tri.v2);
}

void AABB::expand(const AABB& box)
{
	expand(box.getMinPoint());
	expand(box.getMaxPoint());
}

float AABB::radius()
{
	glm::vec3 diff = maxPoint - minPoint;
	return glm::sqrt(glm::dot(diff, diff) * 0.25f);
}

glm::vec3 AABB::getCenter()
{
	return (maxPoint + minPoint) / 2.0f;
}

glm::vec3 AABB::getSize()
{
	return (maxPoint - minPoint);
}

std::vector<glm::vec3> AABB::getPoints()
{
	std::vector<glm::vec3> points;
	points.push_back(minPoint);
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, maxPoint.y, minPoint.z));
	points.push_back(maxPoint);
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, maxPoint.z));
	return points;
}

Renderer::Renderer(unsigned int width, unsigned int height)
{
}

bool Renderer::init()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, 1280, 720);
	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(debugCallback, 0);

	initShader();
	initEnvMaps();

	//std::string assetPath = "../assets";
	std::string assetPath = "../../assets";
	std::string path = assetPath + "/glTF-Sample-Models/2.0";
	//std::string name = "Corset";
	//std::cout << "loading model " << name << std::endl;
	//std::string fn = name + "/glTF/" + name + ".gltf";

	//std::vector<std::string> names;
	////names.push_back("BrainStem");
	////names.push_back("CesiumMan");
	//names.push_back("Fox");
	////names.push_back("RiggedFigure");
	////names.push_back("RiggedSimple");

	//////int i = 3;
	//for(int i = 0 ; i < names.size(); i++)
	//{
	//	//std::string name = "2CylinderEngine";
	//	std::string fn = names[i] + "/glTF/" + names[i] + ".gltf";
	//	IO::GLTFImporter importer;
	//	auto root = importer.importModel(path + "/" + fn);
	//	auto rootTransform = root->getComponent<Transform>();
	//	root->update(glm::mat4(1.0f));

	//	AABB aabb;		
	//	auto meshEntities = root->getChildrenWithComponent<Renderable>();
	//	for (auto m : meshEntities)
	//	{
	//		auto r = m->getComponent<Renderable>();
	//		auto t = m->getComponent<Transform>();
	//		glm::mat4 M = t->getTransform();
	//		auto vertices = r->getVertices();
	//		for (auto& v : vertices)
	//		{
	//			glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0));
	//			aabb.expand(pos);
	//		}
	//	}

	//	glm::vec3 s = aabb.getSize();
	//	std::cout << s.x << " " << s.y << " " << s.z << std::endl;
	//	float scale = 1.0f / glm::max(glm::max(s.x, s.y), s.z);
	//	std::cout << "scale: " << scale << std::endl;

	//	float x = i % 10 * 2;
	//	float y = i / 10 * 2;
	//	float z = 0;

	//	rootTransform->setPosition(glm::vec3(x, y, z));
	//	rootTransform->setScale(glm::vec3(scale));

	//	rootEntitis.push_back(root);
	//	auto children = importer.getEntities();
	//	entities.insert(entities.end(), children.begin(), children.end());
	//}

	cameraUBO.bindBase(0);
	 
	loadGLTFModels(path);

	//root->getComponent<Transform>()->update(glm::mat4(1.0f));

	//IO::ModelImporter importer;
	//auto rootEntity = importer.importModel(path + "/" + fn);
	//rootEntitis.push_back(rootEntity);
	//entities = importer.getEntities();
	//importer.clear();
	 
	return true;
}

void Renderer::initShader()
{
	std::string shaderPath = "../../src/Shaders";
	auto shaderList = IO::loadShadersFromPath(shaderPath);
	for (auto s : shaderList)
		shaders.insert(std::pair(s->getName(), s));
	defaultShader = shaders["Default"];
	defaultShader->setUniform("material.baseColorFactor", glm::vec4(1.0f));
	defaultShader->setUniform("material.alphaCutOff", 0.0f);
	defaultShader->setUniform("material.baseColorTex", 0);
	defaultShader->setUniform("material.pbrTex", 1);
	defaultShader->setUniform("material.normalTex", 2);
	defaultShader->setUniform("material.occlusionTex", 3);
	defaultShader->setUniform("material.emissiveTex", 4);

	defaultShader->setUniform("irradianceMap", 5);
	defaultShader->setUniform("specularMap", 6);
	defaultShader->setUniform("brdfLUT", 7);

	skyboxShader = shaders["Skybox"];
	skyboxShader->setUniform("envMap", 0);
}

void Renderer::initEnvMaps()
{
	auto pano2cmShader = shaders["PanoToCubeMap"];
	auto irradianceShader = shaders["IBLDiffuseIrradiance"];
	auto specularShader = shaders["IBLSpecular"];
	auto integrateBRDFShader = shaders["IBLIntegrateBRDF"];

	unitCube = Primitives::createCube(glm::vec3(0), 1.0f);

	std::string assetPath = "E:/Seafile/Assets/EnvMaps";
	//auto pano = IO::loadTextureHDR(assetPath + "/Factory_Catwalk/Factory_Catwalk_2k.hdr");
	auto pano = IO::loadTextureHDR(assetPath + "/Newport_Loft/Newport_Loft_Ref.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");

	//auto pano = IO::loadTextureHDR(assetPath + "/blaubeuren_outskirts_16k.hdr");
	//auto pano = IO::loadTextureHDR(assetPath + "/office.hdr");

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
		cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR);

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
	specularShader->use();

	{
		int size = 256;
		specularMap = TextureCubeMap::create(size, size, GL::RGB32F);
		specularMap->generateMipmaps();
		specularMap->setFilter(GL::LINEAR_MIPMAP_LINEAR);

		auto specFBO = Framebuffer::create(size, size);
		unsigned int maxMipLevel = 8;
		for (unsigned int mip = 0; mip < maxMipLevel; mip++)
		{
			unsigned int mipWidth = size * std::pow(0.5, mip);
			unsigned int mipHeight = size * std::pow(0.5, mip);
			float roughness = (float)mip / (float)(maxMipLevel - 1);
			specularShader->setUniform("roughness", roughness);

			specFBO->resize(mipWidth, mipHeight);
			specFBO->addRenderTexture(GL::COLOR0, specularMap, mip);
			specFBO->begin();
			cubeMap->use(0);
			unitCube->draw();
			specFBO->end();
		}
	}

	glEnable(GL_DEPTH_TEST);

	auto screenQuad = Primitives::createQuad(glm::vec3(0.0f), 2.0f);

	{
		int size = 512;
		brdfLUT = Texture2D::create(size, size, GL::RG16F);
		brdfLUT->setWrap(GL::CLAMP_TO_EDGE);

		integrateBRDFShader->use();
		auto brdfFBO = Framebuffer::create(size, size);
		brdfFBO->addRenderTexture(GL::COLOR0, brdfLUT);
		brdfFBO->begin();
		screenQuad->draw();
		brdfFBO->end();
	}

	glViewport(0, 0, 1280, 720);
}

void Renderer::loadGLTFModels(std::string path)
{
	std::ifstream file(path + "/model-index.json");
	std::stringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();

	json::Document doc;
	doc.Parse(content.c_str());

	//IO::ModelImporter importer;
	IO::GLTFImporter importer;
	int i = 0;
	std::vector<std::string> filenames;
	for (auto &el : doc.GetArray())
	{
		std::string name(el.FindMember("name")->value.GetString());
		auto it = el.FindMember("variants");
		if (it != el.MemberEnd())
		{
			auto it2 = it->value.FindMember("glTF");
			if (it2 != it->value.MemberEnd())
			{
				//if (name.compare("Sponza") == 0)
				{
					std::cout << "loading model " << name << "...";
					std::string fn = name + "/glTF/" + name + ".gltf";
					auto rootEntity = importer.importModel(path + "/" + fn);
					if (rootEntity != nullptr)
					{
						auto rootTransform = rootEntity->getComponent<Transform>();

						rootEntity->update(glm::mat4(1.0f));

						AABB aabb;
						auto meshEntities = rootEntity->getChildrenWithComponent<Renderable>();
						for (auto m : meshEntities)
						{
							auto r = m->getComponent<Renderable>();
							auto t = m->getComponent<Transform>();
							glm::mat4 M = t->getTransform();
							auto vertices = r->getVertices();
							for (auto& v : vertices)
							{
								glm::vec3 pos = glm::vec3(M * glm::vec4(v.position, 1.0));
								aabb.expand(pos);
							}
						}

						glm::vec3 s = aabb.getSize();
						float scale = 1.0f / glm::max(glm::max(s.x, s.y), s.z);

						float x = i % 10 * 2;
						float y = i / 10 * 2;
						float z = 0;

						rootTransform->setPosition(glm::vec3(x, y, z));
						rootTransform->setScale(glm::vec3(scale));

						rootEntitis.push_back(rootEntity);
						auto children = importer.getEntities();
						entities.insert(entities.end(), children.begin(), children.end());
					}
					importer.clear();
					std::cout << "done!" << std::endl;
				}
			}
		}

		i++;
		//if (i == 10)
		//	break;
	}
}

void Renderer::loadModel(std::string path)
{
	IO::GLTFImporter importer;
	auto root = importer.importModel(path);
	rootEntitis.push_back(root);
	entities = importer.getEntities();
	importer.clear();
}

void Renderer::updateAnimations(float dt)
{
	if (rootEntitis.empty())
		return;
	//defaultShader->setUniform("hasAnimations", false); // TODO: put shader uniforms in render func
	//std::cout << "update animation" << std::endl;
	// apply transformations from animations
	//auto rootEntity = rootEntitis[modelIndex];
	for (auto rootEntity : rootEntitis)
	{
		// TODO: now there is only one animator per skin 
		// but node based animations can have multiple!!! 
		// fix this
		auto animators = rootEntity->getChildrenWithComponent<Animator>();
		if (animators.empty())
		{
			auto a = rootEntity->getComponent<Animator>();
			//a->switchAnimation(modelIndex);
			if(a)
				a->update(dt);
		}

		for (auto e : animators)
		{
			auto a = e->getComponent<Animator>();
			auto t = e->getComponent<Transform>();

			a->update(dt);

			if (a->hasRiggedAnim())
			{
				auto boneTransforms = a->getBoneTransform();
				auto normalTransforms = a->getNormalTransform();
				//defaultShader->setUniform("bones[0]", boneTransforms);
				//defaultShader->setUniform("normals[0]", normalTransforms);
				//defaultShader->setUniform("hasAnimations", true);
			}
			else if (a->hasMorphAnim())
			{
				//glm::vec2 weights = a->getWeights();
				//defaultShader->setUniform("w0", weights.x);
				//defaultShader->setUniform("w1", weights.y);
			}
			else
				a->transform(t);
		}

		// propagate the transformations trough the scene hirarchy
		//rootEntity->getComponent<Transform>()->update(glm::mat4(1.0f));
		rootEntity->update(glm::mat4(1.0f));
	}
}

void Renderer::updateCamera(Camera& camera)
{
	Camera::UniformData cameraData;
	camera.writeUniformData(cameraData);
	cameraUBO.upload(&cameraData, 1);
}

void Renderer::nextModel()
{
	//modelIndex = (++modelIndex) % (unsigned int)rootEntitis.size();
	modelIndex = (++modelIndex) % 3;
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	defaultShader->use();

	irradianceMap->use(5);
	specularMap->use(6);
	brdfLUT->use(7);

	if (rootEntitis.size() > 0)
	{
		//auto e = rootEntitis[modelIndex];
		for(auto e : rootEntitis)
		{
			auto animators = e->getChildrenWithComponent<Animator>();
			if (animators.empty())
			{
				auto a = e->getComponent<Animator>();
				if (a)
				{
					auto boneTransforms = a->getBoneTransform();
					auto normalTransforms = a->getNormalTransform();
					defaultShader->setUniform("hasAnimations", true);
					defaultShader->setUniform("bones[0]", boneTransforms);
					defaultShader->setUniform("normals[0]", normalTransforms);
				}
				else
				{
					defaultShader->setUniform("hasAnimations", false);
				}
			}
			else
			{
				for (auto animEntity : animators)
				{
					auto a = animEntity->getComponent<Animator>();
					if (a->hasRiggedAnim())
					{
						auto boneTransforms = a->getBoneTransform();
						auto normalTransforms = a->getNormalTransform();
						defaultShader->setUniform("hasAnimations", true);
						defaultShader->setUniform("bones[0]", boneTransforms);
						defaultShader->setUniform("normals[0]", normalTransforms);
					}
					else if (a->hasMorphAnim())
					{
						glm::vec2 weights = a->getWeights();
						defaultShader->setUniform("w0", weights.x);
						defaultShader->setUniform("w1", weights.y);
					}
					else
						defaultShader->setUniform("hasAnimations", false);
				}
			}


			// TODO: do depth/tansparent sorting
			auto models = e->getChildrenWithComponent<Renderable>();
			std::vector<Entity::Ptr> transparentEntities;
			std::vector<Entity::Ptr> opaqueEntities;
			for (auto m : models)
			{
				auto r = m->getComponent<Renderable>();
				if (r->useBlending())
					transparentEntities.push_back(m);
				else
					opaqueEntities.push_back(m);
			}
			for (auto m : opaqueEntities)
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();

				t->setUniforms(defaultShader);
				r->render(defaultShader);
			}
			for (auto m : transparentEntities)
			{
				auto r = m->getComponent<Renderable>();
				auto t = m->getComponent<Transform>();

				t->setUniforms(defaultShader);
				r->render(defaultShader);
			}
		}
	}

	skyboxShader->use();
	cubeMap->use(0);
	unitCube->draw();
}
