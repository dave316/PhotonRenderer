#include "Editor.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuiFileDialog.h>

#include <glm/gtx/matrix_decompose.hpp>
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
	io.Fonts->AddFontFromFileTTF("../../../../assets/Fonts/arial.ttf", 20); 
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	if (!renderer.init())
		return false;

	scene = Scene::create("scene");

	//std::string assetPath = "../../../../assets";
	//std::string gltfPath = assetPath + "/glTF-Sample-Models_/2.0";
	//std::string name = "MetalRoughSpheres";
	//scene->loadModel(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");
	//scene->updateAnimations(0.0f);

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
	//if (count != 1)
	//{
	//	std::cout << "only one file allowed!" << std::endl;
	//	return;
	//}

	// TODO: check if valid files etc.
	for (int i = 0; i < count; i++)
	{
		std::string fullPath(paths[i]);
		std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
		int fnIndex = fullPath.find_last_of('/') + 1;
		std::string filename = fullPath.substr(fnIndex, fullPath.length() - fnIndex);
		int lastDot = filename.find_last_of('.');
		std::string name = filename.substr(0, lastDot);
		scene->loadModel(name, fullPath);
	}

	renderer.initLights(scene);
	scene->initShadowMaps();
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
		//selectedModel = scene->selectModelRaycast(startWorld, endWorld);
		auto hitModels = scene->selectModelsRaycast(startWorld, endWorld);

		if (hitModels.empty())
		{
			selectedModel = nullptr;
			scene->unselect();
		}			
		else
		{
			if (selectedModel != nullptr)
			{
				unsigned int lastID = selectedModel->getID();
				int index = -1;
				for (int i = 0; i < hitModels.size(); i++) // check if current model has been selected again
				{
					if (lastID == hitModels[i]->getID())
						index = i;
				}
				if (index >= 0) // if model has been selected again cycle through next models
				{
					int nextIndex = index + 1;
					if (nextIndex < hitModels.size())
						selectedModel = hitModels[nextIndex];
					else
						selectedModel = hitModels[0];
				}
				else // if model is not selected again use first model
				{ 
					selectedModel = hitModels[0];
				}
			}
			else
			{
				selectedModel = hitModels[0]; // if nothing was selected, use the first model
			}
		}
	}
}

void Editor::addTreeNode(Entity::Ptr entity)
{
	std::string name = entity->getName();
	unsigned int currentID = entity->getID();
	int selectedID = (selectedModel != nullptr ? selectedModel->getID() : -1);

	if (selectedModel != nullptr)
	{
		auto parent = selectedModel->getParent();
		while(parent != nullptr)
		{
			if(parent->getID() == currentID)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			parent = parent->getParent();
		}
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= (currentID == selectedID ? ImGuiTreeNodeFlags_Selected : 0);
	flags |= (entity->numChildren() == 0 ? ImGuiTreeNodeFlags_Leaf : 0);

	bool open = ImGui::TreeNodeEx(name.c_str(), flags);

	ImGui::PushID(name.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		ImGui::Text(name.c_str());
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		if (selectedID == currentID)
		{
			selectedModel = nullptr;
			scene->unselect();
		}			
		else
			selectedModel = entity;
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");
		if (payload != NULL)
		{
			unsigned int id = *(unsigned int*)(payload->Data);
			auto sourceEntity = scene->getNode(id);
			if (sourceEntity->findByID(entity->getID()) != nullptr)
			{
				std::cout << "target node is part of source tree!" << std::endl;
			}
			else
			{
				std::string sourceName = sourceEntity->getName();
				auto parent = sourceEntity->getParent();
				if (parent == nullptr)
					scene->removeRootEntity(sourceName);
				else
					parent->removeChild(sourceName);

				auto sourceTransform = sourceEntity->getComponent<Transform>();
				glm::mat4 sourceT = sourceTransform->getTransform();
				glm::mat4 targetT = entity->getComponent<Transform>()->getTransform();
				glm::mat4 M = glm::inverse(targetT) * sourceT;
				sourceTransform->setTransform(M);
				sourceEntity->setParent(entity);
				entity->addChild(sourceEntity);
			}
		}
			
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginDragDropSource())
	{
		unsigned int id = entity->getID();
		ImGui::SetDragDropPayload("Entity", &id, sizeof(unsigned int));
		ImGui::Text(name.c_str());
		ImGui::EndDragDropSource();
	}

	if (open)
	{
		for (int i = 0; i < entity->numChildren(); i++)
			addTreeNode(entity->getChild(i));
		ImGui::TreePop();
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

		if (ImGui::BeginMenu("Entity"))
		{
			if (ImGui::MenuItem("Empty"))
				scene->addEntity("Empty", Entity::create("Empty", nullptr));
			if (ImGui::MenuItem("Model"))
			{
				// TODO: add primitive geometry
			}
			if (ImGui::MenuItem("Light"))
			{
				scene->addLight("light_01");
				renderer.initLights(scene);
				scene->initShadowMaps();
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
			//std::cout << "window size: " << windowSize.x << " " << windowSize.y << std::endl;
			//std::cout << "window pos: " << ImGui::GetCursorScreenPos().x << " " << ImGui::GetCursorScreenPos().y << std::endl;
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

		ImGuizmo::SetID(0);

		if (selectedModel != nullptr)
		{
			auto t = selectedModel->getComponent<Transform>();
			glm::mat4 modelMatrix = t->getTransform();
			const float* V = glm::value_ptr(camera.getViewMatrix());
			const float* P = glm::value_ptr(camera.getProjectionMatrix());
			glm::mat4 T = glm::mat4(1.0f);
			auto r = selectedModel->getComponent<Renderable>();
			if (r != nullptr)
			{
				AABB bbox = r->getBoundingBox();
				glm::vec3 gizmoPos = bbox.getCenter();
				T = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix = modelMatrix * T;
			}
			else
			{
				if (op == ImGuizmo::TRANSLATE)
				{
					auto renderEntities = selectedModel->getChildrenWithComponent<Renderable>();
					AABB selectedBox;
					for (auto e : renderEntities)
					{
						auto t = e->getComponent<Transform>();
						auto r = e->getComponent<Renderable>();
						AABB bbox = r->getBoundingBox();
						glm::mat4 local2world = t->getTransform();
						glm::vec3 minPoint = glm::vec3(local2world * glm::vec4(bbox.getMinPoint(), 1.0f));
						glm::vec3 maxPoint = glm::vec3(local2world * glm::vec4(bbox.getMaxPoint(), 1.0f));
						minPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(minPoint, 1.0f));
						maxPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(maxPoint, 1.0f));
						selectedBox.expand(minPoint);
						selectedBox.expand(maxPoint);
					}
					t->setBounds(selectedBox);
				}

				glm::vec3 gizmoPos = t->getBounds().getCenter();
				T = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix = modelMatrix * T;
			}

			float* M = glm::value_ptr(modelMatrix);

			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
			ImGuizmo::Manipulate(V, P, op, ImGuizmo::LOCAL, M);

			auto parent = selectedModel->getParent();
			glm::mat4 localTransform = modelMatrix * glm::inverse(T);
			glm::mat4 parentTransform = glm::mat4(1.0f);
			if (parent != nullptr)
			{
				parentTransform = parent->getComponent<Transform>()->getTransform();
				localTransform = glm::inverse(parentTransform) * localTransform;
			}
			t->setTransform(localTransform);

			selectedModel->update(parentTransform);
			renderer.initLights(scene);
			renderer.updateShadows(scene);
			//scene->updateBoxes();
			scene->selectBox(selectedModel);
		}
		
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(vMin, vMax) ? ImGuiWindowFlags_NoMove : 0;
	}
	ImGui::End();

	if (ImGui::Begin("Scene"))
	{
		auto entities = scene->getEntities();
		for(auto [name, entity] : entities)
			addTreeNode(entity);
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
			renderer.initLights(scene);
			scene->initShadowMaps();
			//renderer.updateShadows(scene);
			initCamera();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
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