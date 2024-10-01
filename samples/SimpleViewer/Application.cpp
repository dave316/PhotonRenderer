#include "Application.h"

#include <IO/AssimpImporter.h>
#include <IO/GLTFImporter.h>
#include <IO/Image/ImageDecoder.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>

using namespace std::chrono;
using namespace std::placeholders;

Application::Application(const char* title, unsigned int width, unsigned int height) :
	window(title, width, height),
	renderer(width, height)
{
	camera.setAspect((float)width / (float)height);
}

bool Application::init()
{
	if (!window.isInitialized())
		return false;
	window.attachInput(input);

	if (!renderer.init())
		return false;

	std::string assetPath = "../../../../assets";
	std::string modelPath = assetPath + "/Models/DamagedHelmet/DamagedHelmet.gltf";
	std::string envPath = assetPath + "/IBL/Footprint_Court/Footprint_Court_2k.hdr";

	scene = Scene::create("Scene");
		
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(modelPath);
	scene->addRootEntity(rootEntity);

	auto panoImg = IO::decodeHDRFromFile(envPath, true);

	Skybox skybox;
	skybox.texture = panoImg->upload(false);
	skybox.rotation = 0.0f;
	skybox.exposure = 1.0f;
	scene->setSkybox(skybox);

	renderer.updateCamera(camera);
	renderer.prepare(scene);
	renderer.setTonemappingOp(0);
	renderer.setIBL(true);
	renderer.setBloom(false);

	//initCamera();
	setupInput();
	//scene->playAnimations();
	
	return true;
}

void Application::initCamera()
{
	auto bbox = scene->getBoundingBox();
	glm::vec3 minPoint = bbox.getMinPoint();
	glm::vec3 maxPoint = bbox.getMaxPoint();
	glm::vec3 diag = maxPoint - minPoint;

	float aspect = camera.getAspect();
	float fovy = camera.getFov();
	float fovx = fovy * aspect;
	float xZoom = diag.x * 0.5 / glm::tan(fovx / 2.0f);
	float yZoom = diag.y * 0.5 / glm::tan(fovy / 2.0f);
	float dist = glm::max(xZoom, yZoom);
	glm::vec3 center = bbox.getCenter();
	center.z += dist * 2.0f;
	camera.setPosition(center);

	float longestDistance = 10.0f * glm::distance(minPoint, maxPoint);
	float zNear = dist - (longestDistance * 0.6f);
	float zFar = dist + (longestDistance * 0.6f);
	zNear = glm::max(zNear, zFar / 10000.0f);

	//std::cout << "zNear: " << zNear << " zFar: " << zFar << std::endl;

	camera.setPlanes(zNear, zFar);
	camera.setSpeed(longestDistance / 10.0f);
	camera.setVelocity(longestDistance / 100.0f);
}

void Application::setupInput()
{
	input.addKeyCallback(GLFW_KEY_ESCAPE, GLFW_PRESS, std::bind(&GLWindow::close, &window));
	input.addKeyCallback(GLFW_KEY_W, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::RIGHT));
	input.addKeyCallback(GLFW_KEY_W, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::RIGHT));
	input.setMouseMoveCallback(std::bind(&FPSCamera::updateRotation, &camera, _1, _2));
	input.setMouseWheelCallback(std::bind(&FPSCamera::updateSpeed, &camera, _1, _2));
	input.addKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, [&] {animate = !animate; });
	input.setDropCallback(std::bind(&Application::handleDrop, this, _1, _2));
}

bool Application::loadGLTFModel(const std::string& name, const std::string& fullpath)
{
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(fullpath);
	if (rootEntity)
	{
		scene->addRootEntity(name, rootEntity);
		return true;
	}
	return false;
}

#ifdef WITH_ASSIMP
bool Application::loadModelASSIMP(std::string name, std::string path)
{
	IO::AssimpImporter importer;
	auto model = importer.importModel(path);
	if (model == nullptr)
		return false;
	scene->addRootEntity(name, model);
	return true;
}
#endif

void Application::handleDrop(int count, const char** paths)
{
	if (count != 1)
	{
		std::cout << "only one file allowed!" << std::endl;
		return;
	}

	scene->clear();

	for (int i = 0; i < count; i++)
	{
		std::string fullPath(paths[i]);
		std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
		int fnIndex = fullPath.find_last_of('/') + 1;
		std::string filename = fullPath.substr(fnIndex, fullPath.length() - fnIndex);
		int lastDot = filename.find_last_of('.');
		std::string name = filename.substr(0, lastDot);

		int extIndex = lastDot + 1;
		std::string ext = filename.substr(extIndex, filename.length() - extIndex);

		if (ext.compare("gltf") == 0 || ext.compare("glb") == 0)
			loadGLTFModel(name, fullPath);
		else
#ifdef WITH_ASSIMP
			loadModelASSIMP(name, fullPath);
#else
			std::cout << "not supported file extension: " << ext << std::endl;
#endif
	}

	renderer.prepare(scene);
	scene->playAnimations();
	initCamera();
}

void Application::loop()
{
	double dt = 0.0;
	double animTime = 0.0f;
	double tickTime = 1.0f / 60.0f;
	double updateTime = 0.0f;
	int frames = 0;

	while (!window.shouldClose())
	{
		double startTime = glfwGetTime();

		glfwPollEvents();

		if (animTime > tickTime)
		{
			camera.move(animTime);
			camera.rotate(animTime);
			renderer.updateCamera(camera, dt);

			if (animate)
			{
				scene->updateAnimations(animTime);
				scene->updateAnimationState(animTime);
				auto ppp = scene->getCurrentProfile(camera.getPosition());
				renderer.updatePostProcess(ppp);
				renderer.updateTime(startTime);
				renderer.initLights(scene, camera);
				renderer.updateShadows(scene);
			}
			animTime = 0.0f;
		}

		renderer.renderToScreen(scene);
		window.swapBuffers();

		dt = glfwGetTime() - startTime;
		animTime += dt;
		updateTime += dt;

		if (updateTime >= 1.0)
		{
			std::string title = "PhotonRenderer, FPS: " + std::to_string(frames);
			window.setWindowTitle(title);
			updateTime = 0.0;
			frames = 0;
		}
		else
		{
			frames++;
		}
	}
}

void Application::shutdown()
{
}