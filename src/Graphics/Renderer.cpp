#include "Renderer.h"

#include <GL/GLBuffer.h>

//#include <IO/ModelLoader.h>
#include <IO/GLTFImporter.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace json = rapidjson;

std::string loadTxtFile(const std::string& fileName)
{
	std::ifstream file(fileName);
	std::stringstream ss;

	if (file.is_open())
	{
		ss << file.rdbuf();
	}
	else
	{
		std::cout << "could not open file " << fileName << std::endl;
	}

	return ss.str();
}

Renderer::Renderer(unsigned int width, unsigned int height)
{
}

bool Renderer::init()
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, 1920, 1080);

	std::string shaderPath = "../src/Shaders";
	auto vsCode = loadTxtFile(shaderPath + "/Unlit.vs.glsl");
	auto fsCode = loadTxtFile(shaderPath + "/Unlit.fs.glsl");

	if (!vs.compile(vsCode.c_str()))
	{
		std::cout << "error compiling vertex shader" << std::endl;
		std::cout << vs.getErrorLog() << std::endl;
	}

	if (!fs.compile(fsCode.c_str()))
	{
		std::cout << "error compiling fragment shader" << std::endl;
		std::cout << fs.getErrorLog() << std::endl;
	}
	
	program.attachShader(vs);
	program.attachShader(fs);
	if (!program.link())
	{
		std::cout << "error linking shader program" << std::endl;
		std::cout << program.getErrorLog() << std::endl;
	}
	program.loadUniforms();

	program.setUniform("material.diffuseColor", glm::vec3(0.5f));
	program.setUniform("material.diffuseTexture", 0);

	//std::string path = "D:/Assets/Models/sponza/sponza.obj";
	//std::string basePath = "D:/glTF-Sample-Models/2.0";
	//std::string path = basePath + "/DamagedHelmet/glTF/DamagedHelmet.gltf";
	//std::string path = basePath + "/FlightHelmet/glTF/FlightHelmet.gltf";
	//std::string path = basePath + "/2CylinderEngine/glTF/2CylinderEngine.gltf";
	//renderable = IO::load3DModel(path);
	//renderable->print();

	//loadGLTFModels("D:/glTF-Sample-Models/2.0");

	std::string path = "../assets/glTF-Sample-Models/2.0";
	std::string name = "Buggy";
	std::cout << "loading model " << name << std::endl;
	std::string fn = name + "/glTF/" + name + ".gltf";

	IO::GLTFImporter importer;
	auto root = importer.importModel(path + "/" + fn);
	rootEntitis.push_back(root);
	entities = importer.getEntities();

	//IO::ModelImporter importer;
	//auto rootEntity = importer.importModel(path + "/" + fn);
	//rootEntitis.push_back(rootEntity);
	//entities = importer.getEntities();
	//importer.clear();

	//std::cout << "loaded " << entities.size() << " entities" << std::endl;

	//auto models = rootEntity->getChildrenWithComponent<Renderable>();
	//std::cout << models.size() << " entities with renderable component" << std::endl;

	//for (auto m : models)
	//{
	//	std::cout << "entity: " << m->getName() << std::endl;
	//}

	//rootEntity = IO::load3DModel(path + "/" + fn, entities);

	//auto entity = Entity::create(name);
	//entity->addComponent(renderable);

	return true;
}

void Renderer::loadGLTFModels(std::string path)
{
	std::ifstream file(path + "/model-index.json");
	std::stringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();

	json::Document doc;
	doc.Parse(content.c_str());

	IO::ModelImporter importer;
	
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
						rootEntitis.push_back(rootEntity);
						auto children = importer.getEntities();
						entities.insert(entities.end(), children.begin(), children.end());
					}
					importer.clear();
					std::cout << "done!" << std::endl;
				}
			}
		}
	}
}

void Renderer::updateAnimations(float dt)
{
	auto e = rootEntitis[modelIndex];
	auto renderables = e->getComponentsInChildren<Renderable>();
	auto rootRenderable = e->getComponent<Renderable>();
	if (rootRenderable)
		renderables.push_back(rootRenderable);
	for (auto r : renderables)
		r->update(dt);
}

void Renderer::updateCamera(Camera& camera)
{
	program.setUniform("VP", camera.getViewProjectionMatrix());
}

void Renderer::nextModel()
{
	modelIndex = (++modelIndex) % (unsigned int)rootEntitis.size();
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	program.use();
	
	auto e = rootEntitis[modelIndex];
	{
		//auto rootRenderable = e->getComponent<Renderable>();
		//if (rootRenderable)
		//{
		//	glm::mat4 A(1.0f);
		//	if (rootRenderable->hasAnimations())
		//		A = rootRenderable->getTransform();
		//	else if (rootRenderable->hasMorphAnim())
		//	{
		//		glm::vec2 weights = rootRenderable->getWeights();
		//		program.setUniform("w0", weights.x);
		//		program.setUniform("w1", weights.y);

		//		//std::cout << "w: " << weights.x << " " << weights.y << std::endl;
		//	}
		//	else
		//	{
		//		program.setUniform("w0", 0.0f);
		//		program.setUniform("w1", 0.0f);
		//	}

		//	glm::mat4 M = *(e->getComponent<Transform>());
		//	program.setUniform("M", M * A);
		//	rootRenderable->render(program);
		//}
			
		auto models = e->getChildrenWithComponent<Renderable>();
		for (auto m : models)
		{
			glm::mat4 A(1.0f);
			auto r = m->getComponent<Renderable>();

			if (r)
			{
				if (r->hasAnimations())
					A = r->getTransform();
				else if (r->hasMorphAnim())
				{
					glm::vec2 weights = r->getWeights();
					program.setUniform("w0", weights.x);
					program.setUniform("w1", weights.y);
				}
				else
				{
					program.setUniform("w0", 0.0f);
					program.setUniform("w1", 0.0f);
				}

				glm::mat4 M = *(m->getComponent<Transform>());
				//glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f));


				//for (int row = 0; row < 4; row++)
				//{
				//	for (int col = 0; col < 4; col++)
				//	{
				//		std::cout << M[row][col] << " ";
				//	}
				//	std::cout << std::endl;
				//}

				program.setUniform("M", M);
				r->render(program);
			}
		}
	}
}