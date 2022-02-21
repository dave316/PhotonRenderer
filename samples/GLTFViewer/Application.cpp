#include "Application.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuiFileDialog.h>

#include <algorithm>
#include <sstream>

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

	const char* glsl_version = "#version 460";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// TODO: check if font file available first.....
	io.Fonts->AddFontFromFileTTF("../../../../assets/Fonts/arial.ttf", 28); 
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	samplePath = "../../../../assets/glTF-Sample-Models/2.0";
	initGLTFSamples(samplePath);

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

void Application::initCamera()
{
	auto bbox = scene->getBoundingBox();
	glm::vec3 minPoint = bbox.getMinPoint();
	glm::vec3 maxPoint = bbox.getMaxPoint();
	glm::vec3 diag = maxPoint - minPoint;
	//float maxAxisLength = glm::max(diag.x, diag.y);
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
	//std::cout << "zNear: " << zNear << " zFar: " << zFar << std::endl;
	camera.setPlanes(zNear, zFar);
	camera.setSpeed(longestDistance);
	camera.setVelocity(longestDistance * 0.1f);
}

void Application::initGLTFSamples(const std::string& samplesPath)
{
	std::ifstream file(samplesPath + "/model-index.json");
	if (!file.is_open())
	{
		std::cout << "could not open model-index.json, please set path to GLTF samples folder!" << std::endl;
		return;
	}

	std::set<std::string> supportedVariants;
	supportedVariants.insert("glTF");
	supportedVariants.insert("glTF-Binary");
	supportedVariants.insert("glTF-Draco");
	supportedVariants.insert("glTF-Embedded");
	supportedVariants.insert("glTF-JPG-PNG");
	supportedVariants.insert("glTF-KTX-BasisU");

	std::stringstream ss;
	ss << file.rdbuf();
	std::string jsonStr = ss.str();
	json::Document jsonDoc;
	jsonDoc.Parse(jsonStr.c_str());
	for (auto& sampleNode : jsonDoc.GetArray())
	{
		GLTFSampleInfo info;
		if (sampleNode.HasMember("name"))
			info.name = sampleNode["name"].GetString();
		if (sampleNode.HasMember("screenshot"))
			info.screenshot = sampleNode["screenshot"].GetString();
		if (sampleNode.HasMember("variants"))
		{
			auto& variantsNode = sampleNode["variants"];
			for (auto it = variantsNode.MemberBegin(); it != variantsNode.MemberEnd(); ++it)
			{
				std::string name = it->name.GetString();
				if (supportedVariants.find(name) != supportedVariants.end())
					info.variants.push_back(std::make_pair(it->name.GetString(), it->value.GetString()));
				else
					std::cout << info.name << ", variant " << name << " not supported!" << std::endl;
			}
		}
		samplesInfo.push_back(info);
	}
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
	input.setMouseMoveCallback(std::bind(&Camera::updateRotation, &camera, _1, _2));
	input.setMouseWheelCallback(std::bind(&Camera::updateSpeed, &camera, _1, _2));

	//input.addKeyCallback(GLFW_KEY_M, GLFW_PRESS, std::bind(&Renderer::nextMaterial, &renderer));
	input.addKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, [&] {animate = !animate; });

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
	//renderer.loadModel("model", path);
}



void Application::gui()
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

	if (ImGui::Begin("GLTF Samples"))
	{
		if (!samplesInfo.empty())
		{
			static int sampleIndex = 0;
			static int variantIndex = 0;
			static int prevSampleIndex = 0;
			static int prevVariantIndex = 0;
			ImGui::Text("Sample");
			ImGui::SameLine(100);

			ImGuiComboFlags flags = ImGuiComboFlags_HeightLarge;

			GLTFSampleInfo& currentInfo = samplesInfo[sampleIndex];
			if (ImGui::BeginCombo("##sample", currentInfo.name.c_str(), flags))
			{
				for (int i = 0; i < samplesInfo.size(); i++)
				{
					const bool isSelected = (sampleIndex == i);
					if (ImGui::Selectable(samplesInfo[i].name.c_str(), isSelected))
						sampleIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				if (sampleIndex != prevSampleIndex)
				{
					GLTFSampleInfo info = samplesInfo[sampleIndex];
					scene->clear();
					cameraIndex = 0;
					materialIndex = 0;

					variantIndex = 0;
					prevVariantIndex = 0;

					std::string variant = info.variants[variantIndex].first;
					std::string fn = info.variants[variantIndex].second;
					std::string fullPath = samplePath + "/" + info.name + "/" + variant + "/" + fn;
					scene->loadModel(info.name, fullPath);
					renderer.initLights(scene);

					initCamera();

					prevSampleIndex = sampleIndex;
					std::cout << "loading " << fn << std::endl;
					//std::cout << "selected " << samplesInfo[sampleIndex].name << std::endl;
				}

				ImGui::EndCombo();
			}

			ImGui::Text("Variant");
			ImGui::SameLine(100);
			auto& variants = currentInfo.variants;
			if (ImGui::BeginCombo("##variant", variants[variantIndex].first.c_str()))
			{
				for (int i = 0; i < variants.size(); i++)
				{
					const bool isSelected = (variantIndex == i);
					if (ImGui::Selectable(variants[i].first.c_str(), isSelected))
						variantIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				if (variantIndex != prevVariantIndex)
				{
					GLTFSampleInfo info = samplesInfo[sampleIndex];
					scene->clear();
					cameraIndex = 0;
					materialIndex = 0;

					std::string variant = info.variants[variantIndex].first;
					std::string fn = info.variants[variantIndex].second;
					std::string fullPath = samplePath + "/" + info.name + "/" + variant + "/" + fn;
					scene->loadModel(info.name, fullPath);
					renderer.initLights(scene);

					initCamera();

					prevVariantIndex = variantIndex;
					std::cout << "loading " << fn << std::endl;
					//std::cout << "selected " << variants[variantIndex].first << std::endl;
				}

				ImGui::EndCombo();
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Control"))
	{
		auto cameras = scene->getCameraNames();
		if (!cameras.empty())
		{
			ImGui::Text("Camera");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##camera", cameras[cameraIndex].c_str()))
			{
				for (int i = 0; i < cameras.size(); i++)
				{
					const bool isSelected = (cameraIndex == i);
					if (ImGui::Selectable(cameras[i].c_str(), isSelected))
						cameraIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				if (cameraIndex > 0)
				{
					auto cam = scene->getCamera(cameraIndex - 1);
					renderer.updateCamera(cam.P, cam.V, cam.pos);
				}

				ImGui::EndCombo();
			}
		}

		auto materials = scene->getVariantNames();
		if (!materials.empty())
		{
			ImGui::Text("Material");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##material", materials[materialIndex].c_str()))
			{
				for (int i = 0; i < materials.size(); i++)
				{
					const bool isSelected = (materialIndex == i);
					if (ImGui::Selectable(materials[i].c_str(), isSelected))
						materialIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				scene->switchVariant(materialIndex);

				ImGui::EndCombo();
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Information"))
	{
		if (ImGui::TreeNode("Scene"))
		{
			if (ImGui::TreeNode("Node1"))
			{
				ImGui::TreePop();

			}
			if (ImGui::TreeNode("Node2"))
			{
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			scene->clear();
			cameraIndex = 0;
			materialIndex = 0;

			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			std::string name = fileName.substr(0, fileName.find_last_of("."));
			scene->loadModel(name, filePathName);
		}
		ImGuiFileDialog::Instance()->Close();
	}

	//bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
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

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		gui();

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

		renderer.render(scene);
		//renderer.renderText();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();

		dt = glfwGetTime() - startTime;
		animTime += dt;
		updateTime += dt;

		if (updateTime >= 1.0)
		{
			std::string title = "GLTF Viewer, FPS: " + std::to_string(frames);
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
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}