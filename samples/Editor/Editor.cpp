#include "Editor.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuiFileDialog.h>

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <sstream>

using namespace std::placeholders;

Editor::Editor(const char* title, unsigned int width, unsigned int height) :
	window(title, width, height),
	renderer(width, height)
{
	camera.setAspect((float)width / (float)height);
}

bool Editor::init()
{
	if (!window.isInitialized())
		return false;
	window.attachInput(input);

	const char* glsl_version = "#version 460";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// TODO: check if font file available first.....
	io.Fonts->AddFontFromFileTTF("../../../../assets/Fonts/arial.ttf", 28); 
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	if (!renderer.init())
		return false;

	scene = Scene::create("scene");
	scene->updateAnimations(0.0f);

	renderer.initEnv(scene);
	renderer.initLights(scene);
	renderer.updateShadows(scene);

	setupInput();

	return true;
}

void Editor::initCamera()
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

	float longestDistance = glm::distance(minPoint, maxPoint);
	float zNear = dist - (longestDistance * 5.0f);
	float zFar = dist + (longestDistance * 5.0f);
	zNear = glm::max(zNear, zFar / 10000.0f);
	camera.setPlanes(zNear, zFar);
	camera.setSpeed(longestDistance);
	camera.setVelocity(longestDistance * 0.1f);
}

void Editor::setupInput()
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
	input.setMouseMoveCallback(std::bind(&Camera::updateRotation, &camera, _1, _2));
	input.setMouseWheelCallback(std::bind(&Camera::updateSpeed, &camera, _1, _2));
	input.addKeyCallback(GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, std::bind(&Editor::selectModel, this));

	//input.addKeyCallback(GLFW_KEY_M, GLFW_PRESS, std::bind(&Renderer::nextMaterial, &renderer));
	input.addKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, [&] {animate = !animate; });

	input.addKeyCallback(GLFW_KEY_E, GLFW_PRESS, [&] { op = ImGuizmo::TRANSLATE;  });
	input.addKeyCallback(GLFW_KEY_R, GLFW_PRESS, [&] { op = ImGuizmo::ROTATE;  });
	input.addKeyCallback(GLFW_KEY_T, GLFW_PRESS, [&] { op = ImGuizmo::SCALE;  });

	input.setDropCallback(std::bind(&Editor::handleDrop, this, _1, _2));
}

void Editor::handleDrop(int count, const char** paths)
{
	if (count != 1)
	{
		std::cout << "only one file allowed!" << std::endl;
		return;
	}		

	std::string fullPath(paths[0]);
	std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
	int fnIndex = fullPath.find_last_of('/') + 1;
	std::string filename = fullPath.substr(fnIndex, fullPath.length() - fnIndex);
	int lastDot = filename.find_last_of('.');
	std::string name = filename.substr(0, lastDot);

	scene->loadModel(name, fullPath);
	initCamera();
}

void Editor::selectModel()
{
	int mouseX = ImGui::GetMousePos().x - windowPos.x;
	int mouseY = ImGui::GetMousePos().y - windowPos.y;
	//std::cout << "clicket at pos " << mouseX << " " << mouseY << std::endl;

	if (mouseX > 0 && mouseX < windowSize.x && mouseY > 0 && mouseY < windowSize.y)
	{
		if (ImGuizmo::IsUsing())
			return;

		glm::mat4 P = camera.getProjectionMatrix();
		glm::mat4 V = camera.getViewMatrix();

		glm::vec4 vp(0.0f, 0.0f, windowSize.x, windowSize.y);
		glm::vec3 start = glm::vec3(mouseX, windowSize.y - mouseY, 0.0f);
		glm::vec3 end = glm::vec3(mouseX, windowSize.y - mouseY, 1.0f);
		glm::vec3 startWorld = glm::unProject(start, V, P, vp);
		glm::vec3 endWorld = glm::unProject(end, V, P, vp);
		selectedModel = scene->selectModelRaycast(startWorld, endWorld);
	}
}

void addNode(Entity::Ptr entity)
{
	if (entity->numChildren() == 0)
	{
		ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::Text(entity->getName().c_str());
		ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
	}
	else
	{
		if (ImGui::TreeNode(entity->getName().c_str()))
		{
			for (int i = 0; i < entity->numChildren(); i++)
				addNode(entity->getChild(i));

			ImGui::TreePop();
		}
	}
}

void Editor::gui()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New");
			if (ImGui::MenuItem("Open"))
			{
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Select GLTF file", ".gltf,.glb", "../../../../assets");
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Animation"))
		{
			if (ImGui::MenuItem("Play"))
				scene->playAnimations();

			if (ImGui::MenuItem("Stop"))
				scene->stopAnimations();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("About");

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	static ImGuiWindowFlags gizmoWindowFlags = 0;
	if (ImGui::Begin("View", nullptr, gizmoWindowFlags))
	{
		//float windowWidth = (float)ImGui::GetWindowWidth();
		//float windowHeight = (float)ImGui::GetWindowHeight();
		static ImVec2 lastSize = ImVec2(0, 0);
		windowSize = ImGui::GetWindowSize();
		if (windowSize.x != lastSize.x || windowSize.y != lastSize.y)
		{
			std::cout << "window size: " << windowSize.x << " " << windowSize.y << std::endl;
			std::cout << "window pos: " << ImGui::GetCursorScreenPos().x << " " << ImGui::GetCursorScreenPos().y << std::endl;
			camera.setAspect(windowSize.x / windowSize.y);
			renderer.resize(windowSize.x, windowSize.y);
		}			
		lastSize = windowSize;

		auto tex = renderer.renderToTexture(scene);
		ImTextureID id;
		ImGui::GetWindowDrawList()->AddImage(
			(void*)tex->getID(), 
			ImVec2(ImGui::GetCursorScreenPos()), 
			ImVec2(ImGui::GetCursorScreenPos().x + windowSize.x, ImGui::GetCursorScreenPos().y + windowSize.y),
			ImVec2(0, 1), 
			ImVec2(1, 0)
		);

		windowPos = ImGui::GetCursorScreenPos();
		//std::cout << ImGui::GetMousePos().x << " " << ImGui::GetMousePos().y << std::endl;

		static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
		ImGuizmo::SetID(0);
		//static glm::mat4 modelMatrix = glm::mat4(1.0f);

		if (selectedModel != nullptr)
		{
			auto t = selectedModel->getComponent<Transform>();
			glm::mat4 modelMatrix = t->getTransform();
			float* M = glm::value_ptr(modelMatrix);
			const float* V = glm::value_ptr(camera.getViewMatrix());
			const float* P = glm::value_ptr(camera.getProjectionMatrix());
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowSize.x, windowSize.y);

			ImVec2 vMin = ImGui::GetWindowContentRegionMin();
			ImVec2 vMax = ImGui::GetWindowContentRegionMax();

			vMin.x += ImGui::GetWindowPos().x;
			vMin.y += ImGui::GetWindowPos().y;
			vMax.x += ImGui::GetWindowPos().x;
			vMax.y += ImGui::GetWindowPos().y;

			//ImGuiWindow; //*window = ImGui::GetCurrentWindow();
			gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(vMin, vMax) ? ImGuiWindowFlags_NoMove : 0;

			//ImGuizmo::DrawCubes(V, P, M, 1);
			ImGuizmo::Manipulate(V, P, op, ImGuizmo::LOCAL, M);
			t->setTransform(modelMatrix);
			selectedModel->update(glm::mat4(1.0f));
		}
	}
	ImGui::End();

	if (ImGui::Begin("Scene"))
	{
		auto entities = scene->getEntities();
		for(auto [name, entity] : entities)
			addNode(entity);
	}
	ImGui::End();

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			cameraIndex = 0;
			materialIndex = 0;

			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			std::string name = fileName.substr(0, fileName.find_last_of("."));
			scene->loadModel(name, filePathName);
			//renderer.initLights(scene);
			//renderer.updateShadows(scene);
			initCamera();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
}

void Editor::loop()
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

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		if (animTime > tickTime)
		{
			if (cameraIndex == 0)
			{
				camera.move(animTime);
				camera.rotate(animTime);
				renderer.updateCamera(camera);
			}

			if (animate)
			{
				scene->updateAnimations(animTime);
				scene->updateAnimationState(animTime);
				renderer.updateShadows(scene);

				//renderer.updateAnimations(animTime);
				//renderer.updateAnimationState(animTime);
				//renderer.updateShadows();
			}	
			animTime = 0.0f;
		}

		glViewport(0, 0, 1920, 1080);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gui();
		//renderer.renderToTexture(scene);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();

		dt = glfwGetTime() - startTime;
		animTime += dt;
		updateTime += dt;

		if (updateTime >= 1.0)
		{
			std::string title = "Scene Editor, FPS: " + std::to_string(frames);
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

void Editor::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}