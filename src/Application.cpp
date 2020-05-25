#include "Application.h"

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

	setupInput();

	return true;
}

void Application::setupInput()
{
	input.addKeyCallback(GLFW_KEY_ESCAPE, GLFW_PRESS, std::bind(&GLWindow::close, &window));
	input.addKeyCallback(GLFW_KEY_W, GLFW_PRESS, std::bind(&Camera::setDirection, &camera, Camera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_PRESS, std::bind(&Camera::setDirection, &camera, Camera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_PRESS, std::bind(&Camera::setDirection, &camera, Camera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_PRESS, std::bind(&Camera::setDirection, &camera, Camera::Direction::RIGHT));
	input.addKeyCallback(GLFW_KEY_W, GLFW_RELEASE, std::bind(&Camera::releaseDirection, &camera, Camera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_RELEASE, std::bind(&Camera::releaseDirection, &camera, Camera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_RELEASE, std::bind(&Camera::releaseDirection, &camera, Camera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_RELEASE, std::bind(&Camera::releaseDirection, &camera, Camera::Direction::RIGHT));
	input.setMouseCallback(std::bind(&Camera::updateRotation, &camera, _1, _2));

	input.addKeyCallback(GLFW_KEY_R, GLFW_PRESS, std::bind(&Renderer::nextModel, &renderer));

	input.setDropCallback(std::bind(&Application::handleDrop, this, _1, _2));
}

void Application::handleDrop(int count, const char** paths)
{
	if (count != 1)
	{
		std::cout << "only one file allowed!" << std::endl;
		return;
	}		

	std::string path(paths[0]);
	std::replace(path.begin(), path.end(), '\\', '/');

	std::cout << "loading model " << path << std::endl;
	renderer.loadModel(path);
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
			renderer.updateCamera(camera);

			renderer.updateAnimations(startTime);
			animTime = 0.0f;
		}

		//camera.move(dt);
		//camera.rotate(dt);

		//renderer.updateCamera(camera);
		//renderer.updateAnimations(startTime);
		renderer.render();

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