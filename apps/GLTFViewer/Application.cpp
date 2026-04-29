#include "Application.h"

#include <IO/FileIO.h>
#include <IO/GLTFImporter.h>
#include <Core/FogVolume.h>

#include <glm/glm.hpp>
#include <fstream>
#include <chrono>
#include <IO/ImageLoader.h>
#include <Utils/IBL.h>
#include <sstream>

using namespace std::chrono;
using namespace std::placeholders;

Application::Application(const char* title, unsigned int width, unsigned int height) :
	api(pr::GraphicsAPI::OpenGL),
	eventHandler(Win32EventHandler::instance()),
	context(pr::GraphicsContext::getInstance())
{
	window = Win32Window::create(title, width, height);
	userCamera.setAspect((float)width / (float)height);
}

Application::~Application()
{

}

bool Application::init()
{
	context.init(api, window);

	swapchain = context.createSwapchain(window);

	renderer = pr::Renderer::create();
	renderer->init(window);

	gui = pr::GUI::create(window);
	gui->prepare(renderer->getDescriptorPool(), 2);
	gui->preparePipeline(swapchain);
	gui->addTexture(1, renderer->getFinalTex());

	std::string assetPath = "../../../../assets";
	samplesPath = assetPath + "/glTF-Sample-Assets/Models";
	std::string envPath = assetPath + "/glTF-Sample-Environments";
	loadGLTFSamplesInfo(samplesPath);
	loadGLTFSampleEnvironments(envPath);
	//initScene();

	//std::string filename = "../../../../assets/pisa_cube.ktx";
	////std::string filename = "../../../../assets/uffizi_cube.ktx";
	//ktxTexture* ktxTexture;
	//ktxResult result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	//if (result != KTX_SUCCESS)
	//	std::cout << "error loading ktx texture!" << std::endl;

	//uint32 levels = ktxTexture->numLevels;
	//uint32 faces = ktxTexture->numFaces;
	//uint8* ktxTexData = ktxTexture_GetData(ktxTexture);
	//uint32 ktxTexSize = ktxTexture_GetDataSize(ktxTexture);

	//skybox = pr::TextureCubeMap::create(ktxTexture->baseWidth, GPU::Format::RGBA16F, levels, GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled);
	//skybox->setAddressMode(GPU::AddressMode::ClampToEdge);
	//for (uint32 face = 0; face < 6; face++)
	//{
	//	for (uint32 level = 0; level < levels; level++)
	//	{
	//		ktx_size_t offset;
	//		ktxResult result = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
	//		ktx_size_t size = ktxTexture_GetImageSize(ktxTexture, level);
	//		skybox->upload(ktxTexData + offset, size, face, level);
	//	}
	//}

	//std::string envFn = assetPath + "/glTF-Sample-Environments/doge2.hdr";
	//auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
	//uint32 width = panoImg->getWidth();
	//uint32 height = panoImg->getHeight();
	//uint8* data = panoImg->getRawPtr();
	//uint32 dataSize = width * height * sizeof(float) * 4;
	//auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
	//panoTex->upload(data, dataSize);
	//skybox = IBL::convertEqui2CM(panoTex, 1024);

	auto scene = pr::Scene::create("Scene");
	scene->setSkybox(environments[environmentIndex].envMap);
	scenes.push_back(scene);

	CameraInfo camInfo;
	camInfo.name = "UserCamera";
	cameras.push_back(camInfo);
	materials.push_back("None");
	animations.push_back("None");

	renderer->prepare(userCamera, scenes[sceneIndex]);
	renderer->buildCmdBuffer(scenes[sceneIndex]);
	renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
	
	setupInput();

	debugChannels.push_back("None");
	debugChannels.push_back("Texture Coords 0");
	debugChannels.push_back("Texture Coords 1");
	debugChannels.push_back("Geometry Normal");
	debugChannels.push_back("Shading Normal");
	debugChannels.push_back("Ambient Occlusion");
	debugChannels.push_back("Emission");
	debugChannels.push_back("Base Color");
	debugChannels.push_back("Roughness");
	debugChannels.push_back("Metallic");

	debugChannels.push_back("Sheen Color");
	debugChannels.push_back("Sheen Roughness");

	debugChannels.push_back("Clearcoat Factor");
	debugChannels.push_back("Clearcoat Roughness");
	debugChannels.push_back("Clearcoat Normal");

	debugChannels.push_back("Transmission Factor");
	debugChannels.push_back("Thickness");
	debugChannels.push_back("Attenuation Color");

	debugChannels.push_back("Specular Factor");
	debugChannels.push_back("Specular Color");

	debugChannels.push_back("Iridescence Factor");
	debugChannels.push_back("Iridescence Thickness");

	toneMappingOperators.push_back("Linear (no tonemapping)");
	toneMappingOperators.push_back("Reinhard");
	toneMappingOperators.push_back("Exponential");
	toneMappingOperators.push_back("Uncharted 2 (Filmic)");
	toneMappingOperators.push_back("Uncharted 2 (Fast)");
	toneMappingOperators.push_back("Hejl2015 (Filmic)");
	toneMappingOperators.push_back("Narkowicz (ACES)");
	toneMappingOperators.push_back("Hill (ACES)");

	return true;
}

void Application::initCamera()
{
	auto bbox = scenes[sceneIndex]->getBoundingBox();
	glm::vec3 center = bbox.getCenter();
	glm::vec3 minPoint = bbox.getMinPoint();
	glm::vec3 maxPoint = bbox.getMaxPoint();
	glm::vec3 diag = maxPoint - minPoint;

	float aspect = userCamera.getAspect();
	float fovy = userCamera.getFov();
	float fovx = fovy * aspect;
	float xZoom = diag.x * 0.5f / glm::tan(fovx / 2.0f);
	float yZoom = diag.y * 0.5f / glm::tan(fovy / 2.0f);
	float dist = glm::max(xZoom, yZoom);

	userCamera.setCenter(center);
	userCamera.setDistance(dist * 2.0f);

	float longestDistance = 10.0f * glm::distance(minPoint, maxPoint);
	float zNear = dist - (longestDistance * 0.6f);
	float zFar = dist + (longestDistance * 0.6f);
	zNear = glm::max(zNear, zFar / 10000.0f);

	std::cout << "zNear: " << zNear << " zFar: " << zFar << std::endl;

	userCamera.setPlanes(zNear, zFar);
	userCamera.setSpeed(longestDistance / 100.0f);
}

void Application::loadGLTFSamplesInfo(const std::string& samplePath)
{
	std::string modelIndexFile = "model-index.json";
	std::ifstream file(samplesPath + "/" + modelIndexFile);
	if (!file.is_open())
	{
		std::cout << "error: could not open " 
			<< modelIndexFile 
			<< "! Please set path to the GLTF sample folder." 
			<< std::endl;
		return;
	}

	std::set<std::string> supportedVariants;
	supportedVariants.insert("glTF");
	supportedVariants.insert("glTF-Binary");
	supportedVariants.insert("glTF-Embedded");
	supportedVariants.insert("glTF-Draco");
	supportedVariants.insert("glTF-Quantized");

	std::stringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();

	json::Document doc;
	doc.Parse(content.c_str());

	for (auto& sampleNode : doc.GetArray())
	{
		GLTFSampleInfo info;
		if (sampleNode.HasMember("label"))
			info.label = sampleNode["label"].GetString();
		if (sampleNode.HasMember("name"))
			info.name = sampleNode["name"].GetString();
		if (sampleNode.HasMember("screenshot"))
			info.screenshot = sampleNode["screenshot"].GetString();

		if (sampleNode.HasMember("variants"))
		{
			auto& variantsNode = sampleNode["variants"];
			for (auto it = variantsNode.MemberBegin(); it != variantsNode.MemberEnd(); ++it)
			{
				GLTFVariant variant;
				variant.name = it->name.GetString();
				variant.filename = it->value.GetString();
				info.variants.push_back(variant);
				//if (supportedVariants.find(variant.name) != supportedVariants.end())
				//	info.variants.push_back(variant);
				//else
				//	std::cout << info.name << " variant " << variant.name << " not supported!" << std::endl;
			}
		} 

		samplesInfo.push_back(info);
	}
}

void Application::loadGLTFSampleEnvironments(const std::string& samplePath)
{
	auto hdrFiles = IO::getAllFileNames(samplePath, ".hdr");
	for (auto hdrFile : hdrFiles)
	{
		std::cout << "loading env " << hdrFile << std::endl;

		int sepIndex = static_cast<int>(hdrFile.find_last_of('.'));
		std::string name = hdrFile.substr(0, sepIndex);
		std::string envFn = samplePath + "/" + hdrFile;
		auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
		uint32 width = panoImg->getWidth();
		uint32 height = panoImg->getHeight();
		uint8* data = panoImg->getRawPtr();
		uint32 dataSize = width * height * sizeof(float) * 4;
		auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
		panoTex->upload(data, dataSize);
		panoTex->createData();
		panoTex->uploadData();
		GLTFEnvironment env;
		env.name = name;
		env.envMap = IBL::convertEqui2CM(panoTex, 1024, 0.0f);
		environments.push_back(env);
	}
}

bool Application::loadGLTFModel(const std::string& name, const std::string& fullpath)
{
	IO::glTF::Importer importer;
	std::vector<pr::Scene::Ptr> importedScenes;
	int defaultScene = importer.importModel(fullpath, importedScenes);
	if (defaultScene >= 0)
	{
		animations.clear();
		cameras.clear();
		materials.clear();
		scenes.clear();

		scenes = importedScenes;
		sceneIndex = defaultScene;
		for (auto root : scenes[sceneIndex]->getRootNodes())
		{
			for (auto r : root->getComponentsInChildren<pr::Renderable>())
			{
				auto mesh = r->getMesh();
				for (auto& sm : mesh->getSubMeshes())
				{
					sm.primitive->createData();
					sm.primitive->uploadData();
					for (auto tex : sm.material->getTextures())
					{
						tex->createData();
						tex->uploadData();
					}
					for (auto mat : sm.variants)
					{
						for (auto tex : mat->getTextures())
						{
							tex->createData();
							tex->uploadData();
						}
					}
				}
			}
		}

		renderer->initScene(userCamera, scenes[sceneIndex]);
		for (auto scene : scenes)
		{
			scene->setSkybox(environments[environmentIndex].envMap);
			scene->update(0.0f);
		}

		for (auto root : scenes[sceneIndex]->getRootNodes())
		{
			CameraInfo camInfo;
			camInfo.name = "UserCamera";
			cameras.push_back(camInfo);

			for (auto entity : root->getChildrenWithComponent<pr::Camera>())
			{
				auto camera = entity->getComponent<pr::Camera>();
				camera->setAspectRatio(userCamera.getAspect());

				auto transform = entity->getComponent<pr::Transform>();
				glm::mat4 T = transform->getTransform();
				
				CameraInfo camInfo;
				camInfo.name = camera->getName();
				camInfo.P = camera->getProjection();
				camInfo.V = glm::inverse(T);
				camInfo.pos = T * glm::vec4(0, 0, 0, 1);
				cameras.push_back(camInfo);
			}

			auto r = root->getComponentsInChildren<pr::Renderable>();
			if (!r.empty())
				materials = r[0]->getMesh()->getVariants();
			if (materials.empty())
				materials.push_back("None");
		}

		auto animator = scenes[sceneIndex]->getRootNodes()[0]->getComponent<pr::Animator>();
		if (animator)
			animations = animator->getAnimationNames();
		else
			animations.push_back("None");
				
		renderer->buildCmdBuffer(scenes[sceneIndex]);
		renderer->buildScatterCmdBuffer(scenes[sceneIndex]);

		initCamera();
		return true;
	}

	return false;
}

void Application::setupInput()
{
	Win32EventHandler& eventHandler = Win32EventHandler::instance();
	eventHandler.registerMouseWheelCB(std::bind(&OrbitCamera::move, &userCamera, _1));

	static int mouseX = 0;
	static int mouseY = 0;
	eventHandler.registerKeyCB(KeyCode::LeftMouseButton, 0, [&] {
		if (mouseOver3DView)
		{
			eventHandler.setMouseMoveEnabled(true);
			eventHandler.setMouseWheelEnabled(true);
			eventHandler.setKeysEnabled(true);	
			window->hideCursor();
			window->getCursorPos(mouseX, mouseY);
		}
	});

	eventHandler.registerKeyCB(KeyCode::LeftMouseButton, 1, [&] {
		eventHandler.setMouseMoveEnabled(false);
		eventHandler.setMouseWheelEnabled(false);
		eventHandler.setKeysEnabled(false);
		window->showCursor();
	});

	//eventHandler.registerMouseMoveCB(std::bind(&Application::moveMouse, this, _1, _2));
	eventHandler.registerMouseMoveCB([&](int x, int y) {
		float dx = (float)(x - mouseX);
		float dy = (float)(y - mouseY);
		userCamera.updateRotation(dx, dy);
		window->setCursorPos(mouseX, mouseY);
	});
}

void Application::updateGUI()
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImVec2 menuBarSize;
	ImGuiIO& io = ImGui::GetIO();
	//std::cout << io.MousePos.x << " " << io.MousePos.y << std::endl;

	ImGuiWindowFlags flags = 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar;
	if (ImGui::Begin("Main Window", 0, flags))
	{
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2((float)window->getWidth(), (float)window->getHeight()));

		if (ImGui::BeginMenuBar())
		{
			menuBarSize = ImGui::GetWindowSize();
			//std::cout << "Main Menu size: " << barSize.x << " " << barSize.y << std::endl;
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("Open GLTF");
				ImGui::MenuItem("Open HDR");
				// TODO: open file dialog to load GLTF and HDR files
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Show Scene Hirarchy");
				ImGui::MenuItem("Show GLTF Lists");
				ImGui::MenuItem("Settings");
				// TODO: open file dialog to load GLTF and HDR files
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("About");
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		//flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
		ImGuiChildFlags childFlags = 0;
		if (ImGui::BeginChild("GLTF Samples", ImVec2(300,0), childFlags))
		{
			if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
			{
				static int sampleIndex = 0;
				static int variantIndex = 0;
				GLTFSampleInfo currentInfo = samplesInfo[sampleIndex];
				if (ImGui::BeginCombo("Sample", currentInfo.name.c_str(), ImGuiComboFlags_HeightLarge))
				{
					for (uint32 i = 0; i < samplesInfo.size(); i++)
					{
						const bool isSelected = (sampleIndex == i);
						if (ImGui::Selectable(samplesInfo[i].name.c_str(), isSelected))
						{
							sampleIndex = i;

							currentInfo = samplesInfo[sampleIndex];
							variantIndex = 0;
							animationIndex = 0;
							materialIndex = 0;
							cameraIndex = 0;

							for (uint32 i = 0; i < currentInfo.variants.size(); i++)
							{
								if (currentInfo.variants[i].name.compare("glTF") == 0)
								{
									variantIndex = i;
									break;
								}									
							}

							std::string name = currentInfo.name;
							std::string variant = currentInfo.variants[variantIndex].name;
							std::string filename = currentInfo.variants[variantIndex].filename;
							std::string fullpath = samplesPath + "/" + name + "/" + variant + "/" + filename;
							if (loadGLTFModel(name, fullpath))
								std::cout << "loaded model from " << fullpath << std::endl;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				auto& variants = currentInfo.variants;
				if (ImGui::BeginCombo("Variant", variants[variantIndex].name.c_str()))
				{
					for (uint32 i = 0; i < variants.size(); i++)
					{
						const bool isSelected = (variantIndex == i);
						if (ImGui::Selectable(variants[i].name.c_str(), isSelected))
						{
							variantIndex = i;
							std::cout << "variant: " << variants[variantIndex].name << " is selected" << std::endl;
							std::string name = currentInfo.name;
							std::string variant = currentInfo.variants[variantIndex].name;
							std::string filename = currentInfo.variants[variantIndex].filename;
							std::string fullpath = samplesPath + "/" + name + "/" + variant + "/" + filename;
							if (loadGLTFModel(name, fullpath))
								std::cout << "loaded model from " << fullpath << std::endl;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Scene", scenes[sceneIndex]->getName().c_str()))
				{
					for (uint32 i = 0; i < scenes.size(); i++)
					{
						const bool isSelected = (sceneIndex == i);
						if (ImGui::Selectable(scenes[i]->getName().c_str(), isSelected))
						{
							sceneIndex = i;

							renderer->initScene(userCamera, scenes[sceneIndex]);
							renderer->buildCmdBuffer(scenes[sceneIndex]);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Camera", cameras[cameraIndex].name.c_str()))
				{
					for (uint32 i = 0; i < cameras.size(); i++)
					{
						bool isSelected = (cameraIndex == i);
						if (ImGui::Selectable(cameras[i].name.c_str(), isSelected))
						{
							cameraIndex = i;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Material", materials[materialIndex].c_str()))
				{
					for (uint32 i = 0; i < materials.size(); i++)
					{
						bool isSelected = (materialIndex == i);
						if (ImGui::Selectable(materials[i].c_str(), isSelected))
						{
							materialIndex = i;
							scenes[sceneIndex]->switchVariant(materialIndex);
							renderer->buildCmdBuffer(scenes[sceneIndex]);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Animation", animations[animationIndex].c_str()))
				{
					for (uint32 i = 0; i < animations.size(); i++)
					{
						bool isSelected = (animationIndex == i);
						if (ImGui::Selectable(animations[i].c_str(), isSelected))
						{
							animationIndex = i;
							scenes[sceneIndex]->switchAnimation(animationIndex);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}
			}

			if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginCombo("Environment", environments[environmentIndex].name.c_str()))
				{
					for (uint32 i = 0; i < environments.size(); i++)
					{
						bool isSelected = (environmentIndex == i);
						if (ImGui::Selectable(environments[i].name.c_str(), isSelected))
						{
							environmentIndex = i;
							scenes[sceneIndex]->setSkybox(environments[environmentIndex].envMap);
							renderer->prepare(userCamera, scenes[sceneIndex]);
							renderer->buildCmdBuffer(scenes[sceneIndex]);
							renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				static bool showEnv = true;
				if (ImGui::Checkbox("Show environment", &showEnv))
				{

				}
				static bool useIBL = true;
				if (ImGui::Checkbox("IBL", &useIBL))
				{

				}
				static bool useLights = true;
				if (ImGui::Checkbox("Lights", &useLights))
				{

				}
			}

			if (ImGui::CollapsingHeader("Postprocessing", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginCombo("Tonemapper", toneMappingOperators[toneMappingIndex].c_str()))
				{
					for (uint32 i = 0; i < toneMappingOperators.size(); i++)
					{
						bool isSelected = (toneMappingIndex == i);
						if (ImGui::Selectable(toneMappingOperators[i].c_str(), isSelected))
						{
							toneMappingIndex = i;
							post.toneMappingMode = toneMappingIndex;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				static float gamma = 2.2f;
				ImGui::SliderFloat("Exposure", &post.manualExposure, -5.0f, 5.0f);
				ImGui::SliderFloat("Gamma", &gamma, 0.0f, 5.0f);
				static bool useBloom = true;
				if (ImGui::Checkbox("Bloom", (bool*)&post.applyBloom))
				{
					std::cout << "checked bloom " << post.applyBloom << std::endl;
				}
				static float bloomThreshold = 0.0f;
				ImGui::SliderFloat("Intensity", &post.bloomIntensity, 0.0f, 1.0f);
				ImGui::SliderFloat("Threshold", &bloomThreshold, 0.0f, 1.0f);
				ImGui::ColorEdit3("Bloom Tint", (float*)&post.bloomTint, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
			}

			if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginCombo("Channel", debugChannels[debugIndex].c_str()))
				{
					for (uint32 i = 0; i < debugChannels.size(); i++)
					{
						bool isSelected = (debugIndex == i);
						if (ImGui::Selectable(debugChannels[i].c_str(), isSelected))
						{
							debugIndex = i;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();

		static ImGuiWindowFlags gizmoWindowFlags = 0;
		if (ImGui::BeginChild("3D View", ImVec2(0, 0), gizmoWindowFlags))
		{
			//int width = 800;
			//int height = 450;

			//if (width > 0 && height > 0)
			//{
			//	ImVec2 newSize = ImVec2(width + 16, height + 50);
			//	ImGui::SetWindowSize(newSize);
			//}
			//static ImVec2 lastSize = ImVec2(0, 0);
			ImVec2 windowSize = ImGui::GetWindowSize();

			ImVec2 wMin = ImGui::GetWindowContentRegionMin();
			ImVec2 wMax = ImGui::GetWindowContentRegionMax();
			wMin += ImGui::GetWindowPos();
			wMax += ImGui::GetWindowPos();
			ImVec2 contentSize = wMax - wMin;
			userCamera.setAspect(contentSize.x / contentSize.y);

			//std::cout << "content size: " << contentSize.x << " " << contentSize.y << std::endl;

			ImGui::GetWindowDrawList()->AddImage(
				1,
				wMin,
				wMax,
				ImVec2(0, 1),
				ImVec2(1, 0)
			);

			ImGui::EndChild();

			mouseOver3DView = ImGui::IsItemHovered();
		}

		ImGui::End();
	}

	bool show_demo_window = true;
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Render();
}

void Application::loop()
{
	float dt = 0.0f;
	float animTime = 0.0f;
	float tickTime = 1.0f / 60.0f;
	float updateTime = 0.0f;
	float totalTime = 0.0f;
	uint32 frames = 0;
	
	while (eventHandler.handleEvents())
	{
		auto startTime = high_resolution_clock::now();
		if (animTime > tickTime)
		{
			scenes[sceneIndex]->update(animTime);

			if (cameraIndex == 0)
			{
				userCamera.rotate(animTime);
				renderer->updateCamera(scenes[sceneIndex], userCamera, totalTime, debugIndex);
			}
			else
			{
				auto cam = cameras[cameraIndex];
				renderer->updateCamera(scenes[sceneIndex], cam.P, cam.V, cam.pos, totalTime, debugIndex);
			}

			renderer->updateLights(userCamera, scenes[sceneIndex]);
			renderer->updatePost(post);

			animTime = 0.0f;
		}

		// acquire next frame (only need for Vulkan)
		int currentBuffer = swapchain->acquireNextFrame();

		renderer->renderToTexture(scenes[sceneIndex]);
		auto mainCmdBuf = renderer->getCommandBuffer(currentBuffer);
		context.submitCommandBuffer(swapchain, mainCmdBuf);
		context.waitDeviceIdle(); // TODO: check if this is still needed

		// update gui elements, containing render result
		updateGUI();

		// build gui command buffers, TODO: check if buffers changed...
		if (gui->update())
			gui->buildCmd(swapchain);
		else
			gui->buildCmd2(swapchain);

		auto guiCmdBuf = gui->getCommandBuffer(currentBuffer);
		context.submitCommandBuffer(mainCmdBuf, guiCmdBuf);

		swapchain->present(guiCmdBuf);

		auto endTime = high_resolution_clock::now();
		long long diff = duration_cast<microseconds>(endTime - startTime).count();
		dt = (float)diff / 1000000.0f;
		animTime += dt;
		updateTime += dt;
		totalTime += dt;

		if (updateTime >= 1.0)
		{
			std::string title = "PhotonRenderer, API: " + apis[int(api)] + ", FPS: " + std::to_string(frames) + ", AVG.Frametime : " + std::to_string(updateTime / frames);
			window->setTitle(title);
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
	context.waitDeviceIdle();

	environments.clear();
	scenes[sceneIndex].reset();
	gui.reset();
	renderer.reset();
	swapchain.reset();
	context.destroy();
}