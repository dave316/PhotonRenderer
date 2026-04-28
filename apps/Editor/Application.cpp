#include "Application.h"


#include <Core/FogVolume.h>

#include <glm/glm.hpp>
#include <fstream>
#include <chrono>

#include <Utils/IBL.h>
#include <imgui_stdlib.h>
#include <algorithm>
#include <tchar.h>

#include <IO/SceneLoader.h>

using namespace std::chrono;
using namespace std::placeholders;


glm::vec3 linearToSRGB(glm::vec3 linearRGB, float gamma)
{
	return glm::pow(linearRGB, glm::vec3(1.0f / gamma));
}

glm::vec3 sRGBToLinear(glm::vec3 sRGB, float gamma)
{
	return glm::pow(sRGB, glm::vec3(gamma));
}

glm::vec4 linearToSRGBAlpha(glm::vec4 linearRGBA, float gamma)
{
	glm::vec3 rgb = glm::vec3(linearRGBA);
	float alpha = linearRGBA.a;
	glm::vec3 sRGB = linearToSRGB(rgb, gamma);
	return glm::vec4(sRGB, alpha);
}

glm::vec4 sRGBAlphaToLinear(glm::vec4 sRGBAlpha, float gamma)
{
	glm::vec3 sRGB = glm::vec3(sRGBAlpha);
	float alpha = sRGBAlpha.a;
	glm::vec3 linearRGB = sRGBToLinear(sRGB, gamma);
	return glm::vec4(linearRGB, alpha);
}

glm::vec3 convertTemp2RGB(int tempInKelvin)
{
	double T = tempInKelvin;
	double T2 = T * T;
	double u = (0.860117757 + 1.54118254e-4 * T + 1.28641212e-7 * T2) / (1.0 + 8.42420235e-4 * T + 7.08145163e-7 * T2);
	double v = (0.317398726 + 4.22806245e-5 * T + 4.20481691e-8 * T2) / (1 - 2.89741816e-5 * T + 1.61456053e-7 * T2);
	double x = (3.0 * u) / (2.0 * u - 8.0 * v + 4.0);
	double y = (2.0 * v) / (2.0 * u - 8.0 * v + 4.0);
	double Y = 1.0;
	double X = x * Y / y;
	double Z = (1 - x - y) * Y / y;
	glm::vec3 XYZ = glm::vec3(X, Y, Z);
	glm::mat3 XYZ2RGB = glm::mat3(
		3.2404542, -1.5371385, -0.4985314,
		-0.9692660, 1.8760108, 0.0415560,
		0.0556434, -0.2040259, 1.057225
	);
	glm::vec3 rgbLinear = glm::transpose(XYZ2RGB) * XYZ;
	rgbLinear /= glm::max(glm::max(rgbLinear.x, rgbLinear.y), rgbLinear.z);
	return glm::clamp(rgbLinear, 0.0f, 1.0f);
}

Application::Application(const char* title, unsigned int width, unsigned int height) :
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
	api = pr::GraphicsAPI::OpenGL;
	context.init(api, window);

	std::string assetPath = "C:/workspace/code/TestProject";
	assetManager.init(assetPath);
	assetManager.loadAssetsFromDisk();
	assetManager.copyAssetsToGPU();
	selectedFileNode = assetManager.getRoot();

	swapchain = context.createSwapchain(window);

	renderer = pr::Renderer::create();
	renderer->init(window);

	gui = pr::GUI::create(window);
	gui->prepare(renderer->getDescriptorPool(), 2);
	gui->preparePipeline(swapchain);
	gui->addTexture(1, renderer->getFinalTex());

	assetPath = "../../../../assets";
	scenes.push_back(pr::Scene::create("Scene"));

	std::string envFn = assetPath + "/glTF-Sample-Environments/doge2.hdr";
	auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
	uint32 width = panoImg->getWidth();
	uint32 height = panoImg->getHeight();
	uint8* data = panoImg->getRawPtr();
	uint32 dataSize = width * height * sizeof(float) * 4;
	auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
	panoTex->createData();
	panoTex->upload(data, dataSize);
	panoTex->uploadData();
	auto skybox = IBL::convertEqui2CM(panoTex, 1024, 0.0f);

	scenes[0]->setSkybox(skybox);

	renderer->prepare(userCamera, scenes[sceneIndex]);
	renderer->buildCmdBuffer(scenes[sceneIndex]);
	renderer->buildScatterCmdBuffer(scenes[sceneIndex]);

	components.push_back("Animator");
	components.push_back("Camera");
	components.push_back("FogVolume");
	components.push_back("Light");
	components.push_back("LightProbe");
	components.push_back("Renderable");

	setupInput();

	return true;
}

void Application::addSceneNode(pr::Entity::Ptr entity)
{
	std::string name = entity->getName();
	unsigned int currentID = entity->getID();
	int selectedID = (selectedModel != nullptr ? selectedModel->getID() : -1);
	if (selectedModel != nullptr)
	{
		auto parent = selectedModel->getParent();
		while (parent != nullptr)
		{ 
			if (parent->getID() == currentID)
				ImGui::SetNextItemOpen(openSelectedTree, ImGuiCond_Always);
			parent = parent->getParent();
		}
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= (currentID == selectedID ? ImGuiTreeNodeFlags_Selected : 0);
	flags |= (entity->numChildren() == 0 ? ImGuiTreeNodeFlags_Leaf : 0);

	if (!entity->isActive())
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

	std::string nodeName = name + "##" + std::to_string(entity->getID());
	bool open = ImGui::TreeNodeEx(nodeName.c_str(), flags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		if (selectedID == currentID)
		{
			selectedModel = nullptr;
			scenes[sceneIndex]->unselect();
		}
		else
			selectedModel = entity;
	}

	if (selectedModel)
	{
		auto parent = selectedModel->getParent();
		bool partOfSubTree = false;
		while (parent != nullptr)
		{
			if (parent->getID() == currentID)
				partOfSubTree = true;
			parent = parent->getParent();
		}

		if (partOfSubTree && ImGui::IsItemToggledOpen())
		{
			openSelectedTree = !openSelectedTree;
			std::cout << "open subtree: " << openSelectedTree << std::endl;
		}
	}

	if (open)
	{
		for (int i = 0; i < entity->numChildren(); i++)
			addSceneNode(entity->getChild(i));

		ImGui::TreePop();
	}

	if (!entity->isActive())
		ImGui::PopStyleVar();
}

void Application::addFileNode(IO::FileNode::Ptr node)
{
	if (!node->isDirectory)
		return;

	std::string name = node->filename;
	static int selectedNodeIndex = -1;

	bool noDirectory = true;
	for (auto child : node->children)
	{
		if (child->isDirectory)
		{
			noDirectory = false;
			break;
		}
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ((node->children.empty() || noDirectory) ? ImGuiTreeNodeFlags_Leaf : 0);
	flags |= (selectedNodeIndex == node->nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

	std::string nodeName = name;
	bool open = ImGui::TreeNodeEx(nodeName.c_str(), flags);
	
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selectedFileNode = node;
		selectedNodeIndex = node->nodeIndex;
	}

	if (open)
	{
		for (int i = 0; i < node->children.size(); i++)
			addFileNode(node->children[i]);

		ImGui::TreePop();
	}
}

void Application::addAssetNode(IO::FileNode::Ptr node)
{
	std::string name = node->filename;
	static int selectedNode = -1;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= (node->entityIndex < 0) ? ImGuiTreeNodeFlags_Leaf : 0;
	flags |= (selectedNode == node->nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

	std::string nodeName = name;
	bool open = ImGui::TreeNodeEx(nodeName.c_str(), flags);

	if (ImGui::BeginDragDropSource())
	{
		int index = -1;
		std::string type = "";
		if (node->extension.compare(".gltf") == 0 ||
			node->extension.compare(".glb") == 0)
		{
			index = node->entityIndex;
			type = "Model";
		}			
		else if (node->extension.compare(".jpg") == 0 ||
				 node->extension.compare(".png") == 0)
		{
			index = node->texIndex;
			type = "Texture";
		}			

		if (!type.empty())
		{
			ImGui::SetDragDropPayload(type.c_str(), &index, sizeof(int));
			ImGui::Text(node->filename.c_str());
		}
		ImGui::EndDragDropSource();
	}

	if (open)
	{
		if (node->entityIndex >= 0)
		{
			auto root = assetManager.getEntity(node->entityIndex);
			std::map<std::string, pr::Primitive::Ptr> primitives;
			std::map<std::string, pr::Material::Ptr> materials;
			uint32 primIdx = 0;
			uint32 matIdx = 0;
			for (auto r : root->getComponentsInChildren<pr::Renderable>())
			{
				auto mesh = r->getMesh();
				for (auto& subMesh : mesh->getSubMeshes())
				{
					auto prim = subMesh.primitive;
					auto mat = subMesh.material;
					if (prim)
						primitives.insert(std::make_pair(prim->getName(), prim));
					if (mat)
						materials.insert(std::make_pair(mat->getName(), mat));
				}
				
				for (auto [_, prim] : primitives)
				{
					auto name = prim->getName() + "##" + std::to_string(primIdx++);
					if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf))
					{
						if (ImGui::BeginDragDropSource())
						{
							int id = prim->getID();
							ImGui::SetDragDropPayload("Primitive", &id, sizeof(int));
							ImGui::Text(name.c_str());
							ImGui::EndDragDropSource();
						}
						ImGui::TreePop();
					}
				}
				for (auto [_, mat] : materials)
				{
					auto name = mat->getName() + "##" + std::to_string(matIdx++);
					if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf))
					{
						if (ImGui::BeginDragDropSource())
						{
							int id = mat->getID();
							ImGui::SetDragDropPayload("Material", &id, sizeof(int));
							ImGui::Text(name.c_str());
							ImGui::EndDragDropSource();
						}
						ImGui::TreePop();
					}						
				}
			}
		}

		ImGui::TreePop();
	}
}

void Application::loadModel(std::string filename)
{
	auto filepath = fs::path(filename);
	auto ext = filepath.extension().string();
	std::cout << "loading model " << filepath.filename().string() << std::endl;
	if (ext.compare(".gltf") == 0 || ext.compare(".glb") == 0)
	{
		IO::glTF::Importer importer;
		auto root = importer.importModel(filename);
		if (root)
			scenes[sceneIndex]->addRoot(root);
	}
	else
	{
		std::cout << ext << " extension not supported!" << std::endl;
	}

	scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
	renderer->buildCmdBuffer(scenes[sceneIndex]);
	renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
}

void Application::setupInput()
{
	Win32EventHandler& eventHandler = Win32EventHandler::instance();
	eventHandler.registerKeyCB(KeyCode::W, 0, std::bind(&FPSCamera::setDirection, &userCamera, FPSCamera::Direction::FORWARD));
	eventHandler.registerKeyCB(KeyCode::S, 0, std::bind(&FPSCamera::setDirection, &userCamera, FPSCamera::Direction::BACK));
	eventHandler.registerKeyCB(KeyCode::A, 0, std::bind(&FPSCamera::setDirection, &userCamera, FPSCamera::Direction::LEFT));
	eventHandler.registerKeyCB(KeyCode::D, 0, std::bind(&FPSCamera::setDirection, &userCamera, FPSCamera::Direction::RIGHT));
	eventHandler.registerKeyCB(KeyCode::W, 1, std::bind(&FPSCamera::releaseDirection, &userCamera, FPSCamera::Direction::FORWARD));
	eventHandler.registerKeyCB(KeyCode::S, 1, std::bind(&FPSCamera::releaseDirection, &userCamera, FPSCamera::Direction::BACK));
	eventHandler.registerKeyCB(KeyCode::A, 1, std::bind(&FPSCamera::releaseDirection, &userCamera, FPSCamera::Direction::LEFT));
	eventHandler.registerKeyCB(KeyCode::D, 1, std::bind(&FPSCamera::releaseDirection, &userCamera, FPSCamera::Direction::RIGHT));

	eventHandler.registerKeyCB(KeyCode::LeftMouseButton, 1, std::bind(&Application::selectModel, this));

	static int mouseX = 0;
	static int mouseY = 0;
	eventHandler.registerKeyCB(KeyCode::RightMouseButton, 0, [&] {
		if (mouseOver3DView)
		{
			eventHandler.setMouseMoveEnabled(true);
			eventHandler.setMouseWheelEnabled(true);
			window->hideCursor();
			window->getCursorPos(mouseX, mouseY);
		}
	});

	eventHandler.registerKeyCB(KeyCode::RightMouseButton, 1, [&] {
		if (mouseOver3DView)
		{
			eventHandler.setMouseMoveEnabled(false);
			eventHandler.setMouseWheelEnabled(false);
			for (int i = 0; i < 4; i++)
				userCamera.releaseDirection(FPSCamera::Direction(i));
			window->showCursor();
			window->setCursorPos(mouseX, mouseY);
		}
	});

	eventHandler.registerMouseMoveCB([&](int x, int y) {
		if (mouseOver3DView)
		{
			float dx = (float)(x - mouseX);
			float dy = (float)(mouseY - y);
			userCamera.updateRotation(dx, dy);
			window->setCursorPos(mouseX, mouseY);
		}
	});

	eventHandler.registerKeyCB(KeyCode::E, 0, [&] { gizmoOp = ImGuizmo::TRANSLATE; });
	eventHandler.registerKeyCB(KeyCode::R, 0, [&] { gizmoOp = ImGuizmo::ROTATE; });
	eventHandler.registerKeyCB(KeyCode::T, 0, [&] { gizmoOp = ImGuizmo::SCALE; });
	eventHandler.registerDropCB([&](std::vector<std::string> filepaths) {
		for (auto fp : filepaths) {
			auto filepath = fs::path(fp);
			if (fs::is_regular_file(filepath))
				assetManager.addFile(selectedFileNode, fp);
			else
				assetManager.addDirectory(selectedFileNode, fp);

			assetManager.loadAssetsFromDisk();
			assetManager.copyAssetsToGPU();
		}

		//for (auto fp : filepaths)
		//{
		//	auto filepath = fs::path(fp);
		//	auto ext = filepath.extension().string();
		//	std::cout << "loading model " << filepath.filename().string() << std::endl;
		//	if (ext.compare(".gltf") == 0 || ext.compare(".glb") == 0)
		//	{
		//		IO::glTF::Importer importer;
		//		auto root = importer.importModel(fp);
		//		if (root)
		//			scenes[sceneIndex]->addRoot(root);
		//	}
		//	else
		//	{
		//		std::cout << ext << " extension not supported!" << std::endl;
		//	}

		//	//renderer->prepare(userCamera, scenes[sceneIndex]);
		//	scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
		//	renderer->buildCmdBuffer(scenes[sceneIndex]);
		//	renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
		//}
	});

	eventHandler.registerResizeCB([&](int width, int height) {
		std::cout << "resize window to " << width << "x" << height << std::endl;
		swapchain->resize(width, height);
	});
}

void Application::selectModel()
{
	int mouseX = (int)(ImGui::GetMousePos().x - windowPos.x);
	int mouseY = (int)(ImGui::GetMousePos().y - windowPos.y);
	
	if (mouseX > 0 && mouseX < windowSize.x && mouseY && mouseY < windowSize.y) // check if mouse is inside 3D view
	{
		if (ImGuizmo::IsUsing()) // gizmo is just being manipulated
			return;
		auto P = userCamera.getProjectionMatrix();
		auto V = userCamera.getViewMatrix();
		auto viewport = glm::vec4(0, 0, windowSize.x, windowSize.y); // TODO: this is wrong? shouldn't this be content size?
		auto start = glm::vec3(mouseX, windowSize.y - mouseY, 0.0f);
		auto end = glm::vec3(mouseX, windowSize.y - mouseY, 1.0f);
		auto startWorld = glm::unProject(start, V, P, viewport);
		auto endWorld = glm::unProject(end, V, P, viewport);
		auto hitModels = scenes[sceneIndex]->selectModelsRaycast(startWorld, endWorld);

		if (hitModels.empty())
		{
			selectedModel = nullptr;
			scenes[sceneIndex]->unselect();
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

		openSelectedTree = true;
	}
}

pr::Material::Ptr getDefaultMaterial()
{
	auto mat = pr::Material::create("Default", "Default");
	mat->addProperty("baseColor", glm::vec4(1));
	mat->addProperty("emissive", glm::vec4(0));
	mat->addProperty("roughness", 1.0f);
	mat->addProperty("metallic", 0.0f);
	mat->addProperty("occlusion", 1.0f);
	mat->addProperty("normalScale", 1.0f);
	mat->addProperty("alphaMode", 0);
	mat->addProperty("alphaCutOff", 0.5f);
	mat->addProperty("computeFlatNormals", false);
	mat->addProperty("ior", 1.5f);
	std::vector<std::string> texNames = {
		"baseColorTex", 
		"normalTex", 
		"metalRoughTex", 
		"emissiveTex", 
		"occlusionTex"
	};
	for (int i = 0; i < texNames.size(); i++)
		mat->addTexture(texNames[i], nullptr);
	return mat;
}

void Application::updateGUI()
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImVec2 menuBarSize;
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::BeginMainMenuBar())
	{
		menuBarSize = ImGui::GetWindowSize();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Scene"))
			{
				char filename[256];
				char extension[256];
				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = window->getWindowHandle();
				ofn.lpstrFilter = _T("JSON files (*.json)\0*.json\0All files (*.*)\0*.*\0\0");
				ofn.lpstrFile = filename;
				ofn.nMaxFile = 256;
			
				ofn.lpstrTitle = _T("Open Scene");
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&ofn))
				{
					std::cout << "selected file " << filename << std::endl;
					scenes[sceneIndex] = IO::SceneLoader::loadScene(assetManager, filename);
					scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
					scenes[sceneIndex]->update(0.0f);

					renderer->updateLights(userCamera, scenes[sceneIndex]);
					renderer->addLights(scenes[sceneIndex]);
					renderer->buildCmdBuffer(scenes[sceneIndex]);
					renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
					renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
					renderer->updateShadows(scenes[sceneIndex]);
				}
				else
				{
					std::cout << "error selecting file" << std::endl;
				}
			}
			if(ImGui::MenuItem("Save Scene"))
			{
				char filename[256];
				char extension[256];
				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = window->getWindowHandle();
				ofn.lpstrFilter = _T("JSON files (*.json)\0*.json\0All files (*.*)\0*.*\0");
				ofn.lpstrFile = filename;
				ofn.nMaxFile = 256;
				//ofn.lpstrCustomFilter = extension;
				//ofn.nMaxCustFilter = 256;
				ofn.lpstrDefExt = "json";
				ofn.lpstrTitle = _T("Save Scene");
				ofn.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

				if (GetSaveFileName(&ofn))
				{
					std::cout << "selected file " << filename << std::endl;

					IO::SceneLoader::saveScene(filename, scenes[sceneIndex]);
				}
				else
				{
					std::cout << "error selecting file" << std::endl;
				}
			}

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

		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Scene"))
	{
		if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto node : scenes[sceneIndex]->getRootNodes())
				addSceneNode(node);
		}
	
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Empty Entity", NULL, false, true))
				scenes[sceneIndex]->addRoot(pr::Entity::create("New Entity", nullptr));

			if (ImGui::MenuItem("Light", NULL, false, true))
			{
				auto light = pr::Light::create(pr::LightType::POINT, glm::vec3(1), 200.0f, 100.0f);
				light->setLuminousPower(1500);
				auto entity = pr::Entity::create("Light", nullptr);
				auto t = entity->getComponent<pr::Transform>();
				entity->addComponent(light);

				scenes[sceneIndex]->addRoot(entity);
				renderer->updateLights(userCamera, scenes[sceneIndex]);
				renderer->addLights(scenes[sceneIndex]);			
				renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
			}
			
			if (ImGui::MenuItem("Fog Volume", NULL, false, true))
			{
				auto fogEntity = pr::Entity::create("Fog", nullptr);
				auto t = fogEntity->getComponent<pr::Transform>();
				auto volume = pr::FogVolume::create();
				volume->setScattering(glm::vec3(1, 0, 0));
				//volume->setDensitySphere(5.0f);
				volume->setDensityPerlinNoise(3.0f);
				fogEntity->addComponent(volume);
				scenes[sceneIndex]->addRoot(fogEntity);
				renderer->updateLights(userCamera, scenes[sceneIndex]);
				renderer->addLights(scenes[sceneIndex]);
				renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
			}

			if (ImGui::BeginMenu("3D"))
			{
				if (ImGui::MenuItem("Box", NULL, false, true))
				{
					pr::SubMesh subMesh;
					subMesh.primitive = createCube(glm::vec3(0), 1.0f);
					subMesh.material = getDefaultMaterial();

					auto mesh = pr::Mesh::create("Box");
					mesh->addSubMesh(subMesh);

					auto box = pr::Entity::create("Box", nullptr);
					box->addComponent(pr::Renderable::create(mesh));

					scenes[sceneIndex]->addRoot(box);
					scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
					renderer->buildCmdBuffer(scenes[sceneIndex]);
					renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
					renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
				}
				if (ImGui::MenuItem("Sphere", NULL, false, true))
				{
					pr::SubMesh subMesh;
					subMesh.primitive = createUVSphere(glm::vec3(0), 0.5f, 64, 64);
					subMesh.material = getDefaultMaterial();

					auto mesh = pr::Mesh::create("Spere");
					mesh->addSubMesh(subMesh);

					auto sphere = pr::Entity::create("Spere", nullptr);
					sphere->addComponent(pr::Renderable::create(mesh));

					scenes[sceneIndex]->addRoot(sphere);
					scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
					renderer->buildCmdBuffer(scenes[sceneIndex]);
					renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
					renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
				}
				if (ImGui::MenuItem("Quad", NULL, false, true))
				{
					pr::SubMesh subMesh;
					subMesh.primitive = createQuad(glm::vec3(0), 1.0f);
					subMesh.material = getDefaultMaterial();

					auto mesh = pr::Mesh::create("Quad");
					mesh->addSubMesh(subMesh);

					auto quad = pr::Entity::create("Quad", nullptr);
					quad->addComponent(pr::Renderable::create(mesh));

					scenes[sceneIndex]->addRoot(quad);
					scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
					renderer->buildCmdBuffer(scenes[sceneIndex]);
					renderer->buildShadowCmdBuffer(scenes[sceneIndex]);
					renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}	
	}
	ImGui::End();

	if (ImGui::Begin("Assets Browser"))
	{
		//if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
		//	std::cout << "Assets Browser!" << std::endl;

		if(ImGui::BeginChild("Assets", ImVec2(400,0), ImGuiChildFlags_ResizeX))
		{
			addFileNode(assetManager.getRoot());
		}
		ImGui::EndChild();
		ImGui::SameLine();

		if (ImGui::BeginChild("Files"))
		{
			if (!selectedFileNode->filename.empty())
			{
				for (auto child: selectedFileNode->children)
					addAssetNode(child);
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	static ImGuiWindowFlags gizmoWindowFlags = 0;
	if (ImGui::Begin("3D View"))
	{
		// Remove focus from any widget if mouse is over 3D view
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
		{
			ImGui::SetWindowFocus(NULL);
		}		

		//if (ImGui::IsWindowHovered())
		//	std::cout << "3D view!" << std::endl;

		ImGui::Dummy(ImGui::GetContentRegionAvail());
		if (ImGui::BeginDragDropTarget())
		{
			// TODO: copy scene hierarchy, otherwise we have ID conflicts in the hierarchy
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model");
			if (payload != nullptr)
			{
				int entityID = *(int*)(payload->Data);
				auto root = assetManager.getEntity(entityID);
				scenes[sceneIndex]->addRoot(root);

				scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
				renderer->buildCmdBuffer(scenes[sceneIndex]);
				renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
			}

			ImGui::EndDragDropTarget();
		}

		windowPos = ImGui::GetWindowPos();
		windowSize = ImGui::GetWindowSize();
		ImVec2 wMin = ImGui::GetWindowContentRegionMin();
		ImVec2 wMax = ImGui::GetWindowContentRegionMax();
		wMin += ImGui::GetWindowPos();
		wMax += ImGui::GetWindowPos();
		ImVec2 contentSize = wMax - wMin;
		//std::cout << "content size: " << contentSize.x << " " << contentSize.y << std::endl;

		// TODO: change renderer viewport and framebuffers!
		if (contentSize.x == 0 || contentSize.y == 0)
			userCamera.setAspect(1.0f);
		else
			userCamera.setAspect(contentSize.x / contentSize.y);

		ImGui::GetWindowDrawList()->AddImage(1, wMin, wMax, ImVec2(0, 1), ImVec2(1, 0));

		ImGuizmo::SetID(0);
		if (selectedModel)
		{
			auto t = selectedModel->getComponent<pr::Transform>();
			auto r = selectedModel->getComponent<pr::Renderable>();
			auto modelMatrix = t->getTransform();
			auto offset = glm::mat4(1.0f);
			if (r != nullptr) // we have node with a mesh so just use the bbox as center
			{					
				auto bbox = r->getBoundingBox();
				auto gizmoPos = bbox.getCenter();
				offset = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix *= offset; // move coordinate frame to center of model
			}
			else // its a root node so use all geometry of the subtree for the bbox
			{
				if (gizmoOp == ImGuizmo::TRANSLATE)
				{
					auto renderEntities = selectedModel->getChildrenWithComponent<pr::Renderable>();
					Box selectedBox;
					for (auto e : renderEntities)
					{
						auto t = e->getComponent<pr::Transform>();
						auto r = e->getComponent<pr::Renderable>();
						auto bbox = r->getBoundingBox();
						auto local2world = t->getTransform();
						auto minPoint = glm::vec3(local2world * glm::vec4(bbox.getMinPoint(), 1.0f));
						auto maxPoint = glm::vec3(local2world * glm::vec4(bbox.getMaxPoint(), 1.0f));
						minPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(minPoint, 1.0f));
						maxPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(maxPoint, 1.0f));
						selectedBox.expand(minPoint);
						selectedBox.expand(maxPoint);
					}
					t->setBounds(selectedBox);
				}

				glm::vec3 gizmoPos = t->getBounds().getCenter();
				offset = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix *= offset;
			}

			// pointers for imguizmo
			const auto* V = glm::value_ptr(userCamera.getViewMatrix());
			const auto* P = glm::value_ptr(userCamera.getProjectionMatrix());
			auto* M = glm::value_ptr(modelMatrix);

			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(wMin.x, wMin.y, contentSize.x, contentSize.y); // set draw able surface to current window
			ImGuizmo::Manipulate(V, P, gizmoOp, ImGuizmo::LOCAL, M); // compute current transform from gizmos

			auto localTransform = modelMatrix * glm::inverse(offset); // remove offset from centering
			auto parentTransform = glm::mat4(1.0f);
			auto parent = selectedModel->getParent();
			if (parent != nullptr)
			{
				parentTransform = parent->getComponent<pr::Transform>()->getTransform();
				localTransform = glm::inverse(parentTransform) * localTransform;
			}
			t->setLocalTransform(localTransform); // set actual local transform
			selectedModel->update(parentTransform);
			scenes[sceneIndex]->select(selectedModel);
			// TODO: update any intermediate stuff (lights/shadows etc.)
		}

		renderer->buildCmdBuffer(scenes[sceneIndex]);
		renderer->buildScatterCmdBuffer(scenes[sceneIndex]);

		mouseOver3DView = ImGui::IsWindowHovered();
		
		eventHandler.setKeysEnabled(mouseOver3DView);
		gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(wMin, wMax) ? ImGuiWindowFlags_NoMove : 0;
	}
	ImGui::End();

	if (ImGui::Begin("Properties"))
	{
		if (selectedModel)
		{
			bool active = selectedModel->isActive();
			std::string checkBoxID = "##" + std::to_string(selectedModel->getID());
			if (ImGui::Checkbox(checkBoxID.c_str(), &active))
				selectedModel->setActive(active);
			ImGui::SameLine();

			auto name = selectedModel->getName();
			if (ImGui::InputText("Name", &name))
				selectedModel->setName(name);

			auto t = selectedModel->getComponent<pr::Transform>();
			if (t)
			{
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto pos = t->getLocalPosition();
					auto rot = t->getLocalRotation();
					auto rotEuler = glm::degrees(glm::eulerAngles(rot));
					auto scale = t->getLocalScale();
					ImGui::InputFloat3("Position", &pos[0]);
					ImGui::InputFloat3("Rotation", &rotEuler[0]);
					ImGui::InputFloat3("Scale", &scale[0]);
					glm::quat q = glm::quat(glm::radians(rotEuler));
					t->setLocalPosition(pos);
					t->setLocalRotation(q);
					t->setLocalScale(scale);
				}
			}

			auto r = selectedModel->getComponent<pr::Renderable>();
			if (r)
			{
				if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto mesh = r->getMesh();
					auto& subMeshes = mesh->getSubMeshes();
					ImGui::InputText("Mesh", &mesh->getName());
					meshNames.clear();
					for (auto& subMesh : subMeshes)
						meshNames.push_back(subMesh.primitive->getName());

					static int primitiveSelected = -1;
					if (ImGui::BeginListBox("Primitives"))
					{
						for (int i = 0; i < meshNames.size(); i++)
						{
							const bool isSelected = (primitiveSelected == i);
							std::string name = meshNames[i] + "##" + std::to_string(i);
							if (ImGui::Selectable(name.c_str(), isSelected))
								primitiveSelected = i;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndListBox();
					}

					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Primitive");
						if (payload != nullptr)
						{
							int primID = *(int*)(payload->Data);
							pr::SubMesh subMesh;
							subMesh.primitive = assetManager.getPrimitive(primID);
							subMesh.material = nullptr;
							mesh->addSubMesh(subMesh);

							scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
							renderer->buildCmdBuffer(scenes[sceneIndex]);
							renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
						}
						ImGui::EndDragDropTarget();
					}

					if (primitiveSelected >= 0 && primitiveSelected < subMeshes.size())
					{
						auto vertexCount = subMeshes[primitiveSelected].primitive->getVertexCount();
						auto triangleCount = subMeshes[primitiveSelected].primitive->getIndexCount() / 3;
						std::string verticesTxt = "Vertices: " + std::to_string(vertexCount);
						std::string trianglesTxt = "Triangles: " + std::to_string(triangleCount);
						ImGui::Text(verticesTxt.c_str());
						ImGui::Text(trianglesTxt.c_str());
						
						if (subMeshes[primitiveSelected].material)
						{
							auto matName = subMeshes[primitiveSelected].material->getName();
							auto maTxt = "Material: " + matName;
							ImGui::Text(maTxt.c_str());
							if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
							{
								auto primitive = subMeshes[primitiveSelected];
								auto material = subMeshes[primitiveSelected].material;
								std::string nameTxt = "Name: " + matName;

								auto properties = material->getProperties();
								for (auto prop : properties)
								{
									if (auto valueProp = std::dynamic_pointer_cast<pr::ValueProperty<float>>(prop))
									{
										float value = valueProp->get();
										ImGui::SliderFloat(prop->getName().c_str(), &value, 0.0f, 1.0f);
										valueProp->set(value);
									}
									if (auto valueProp = std::dynamic_pointer_cast<pr::ValueProperty<glm::vec4>>(prop))
									{
										glm::vec4 value = valueProp->get();
										ImGui::ColorEdit4(valueProp->getName().c_str(), &value[0], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview);
										valueProp->set(value);
									}
								}
								material->update();

								// TODO: this only needs to be done once when the material is clicked
								auto textures = material->getTextures();
								auto textureMap = material->getTextureMap();
								auto textureInfos = material->getTextureInfos();

								for (int i = 0; i < textures.size(); i++)
									gui->removeTexture(i + 2); // TODO: better index handling...
								for (int i = 0; i < textures.size(); i++)
									gui->addTexture(i + 2, textures[i]);

								for (auto&& [name, index] : textureMap)
								{
									auto& texInfos = textureInfos[index];

									ImGui::Text(name.c_str());
									if (texInfos.samplerIndex < textures.size())
										ImGui::ImageWithBg(texInfos.samplerIndex + 2, ImVec2(128, 128), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 1));
									else
									{
										std::string buttonID = "Select##" + std::to_string(index);
										if (ImGui::Button(buttonID.c_str(), ImVec2(128, 128)))
										{
											// TODO: open dialog to select texture from assets
										}
									}

									if (ImGui::BeginDragDropTarget())
									{
										const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture");
										if (payload != nullptr)
										{
											int assetID = *(int*)(payload->Data);

											auto tex = assetManager.getTexture(assetID);
											material->setTexture(name, tex);
											material->update(renderer->getDescriptorPool());
										}

										ImGui::EndDragDropTarget();
									}
								}
							}
						}
						else
						{
							std::string maTxt = "Material: None";
							ImGui::Text(maTxt.c_str());
							if (ImGui::BeginDragDropTarget())
							{
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material");
								if (payload != nullptr)
								{
									int matID = *(int*)(payload->Data);
									subMeshes[primitiveSelected].material = assetManager.getMaterial(matID);

									scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
									renderer->buildCmdBuffer(scenes[sceneIndex]);
									renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
								}

								ImGui::EndDragDropTarget();
							}
						}
					}					
				}
			}

			auto l = selectedModel->getComponent<pr::Light>();
			if (l)
			{
				if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					static int currentType = (int)l->getType();
					static int prevType = currentType;
					std::vector<std::string> types = { "Directional", "Point", "Spot", "Area" };
					std::string typeStr = types[currentType];
					if (ImGui::BeginCombo("Type", typeStr.c_str(), 0))
					{
						for (int i = 0; i < types.size(); i++)
						{
							const bool isSelected = (currentType == i);
							if (ImGui::Selectable(types[i].c_str(), isSelected))
							{
								currentType = i;
								l->setLightType((pr::LightType)currentType);
							}								
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}						

						ImGui::EndCombo();
					}

					static bool useColorTemp = false;
					if (useColorTemp)
					{
						static int temp = 6500;
						ImGui::SliderInt("Color temperature", &temp, 1000, 15000);
						l->setColorTemp(temp);
					}
					else
					{
						auto col = linearToSRGB(l->getColor(), 2.2f);
						ImGui::ColorEdit3("Color", &col[0], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
						l->setColorLinear(sRGBToLinear(col, 2.2f));
					}
					ImGui::Checkbox("Use color temperature", &useColorTemp);

					float intensity = l->getLumen();
					ImGui::SliderFloat("Intensity", &intensity, 0, 10000);
					l->setLuminousPower(intensity);

					float range = l->getRange();
					ImGui::SliderFloat("Range", &range, 0, 100);
					l->setRange(range);
				}
			}

			auto v = selectedModel->getComponent<pr::FogVolume>();
			if (v)
			{
				if (ImGui::CollapsingHeader("FogVolume", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto scatterColor = v->getScattering();
					auto emissiveColor = v->getEmissive();
					auto absorbtion = v->getAbsorbtion();
					auto phase = v->getPhase();
					ImGui::ColorEdit3("Scattering Color", &scatterColor[0], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
					ImGui::ColorEdit3("Emissive Color", &emissiveColor[0], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
					ImGui::SliderFloat("Absorbtion", &absorbtion, 0.0f, 10.0f);
					ImGui::SliderFloat("Phase", &phase, -1.0f, 1.0f);
					v->setScattering(scatterColor);
					v->setEmissive(emissiveColor);
					v->setAbsorbtion(absorbtion);
					v->setPhase(phase);
				}
			}

			static int selectedComponent = -1;
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("Component");
			if (ImGui::BeginPopup("Component"))
			{
				for (int i = 0; i < components.size(); i++)
					if (ImGui::Selectable(components[i].c_str()))
						selectedComponent = i;

				if (selectedComponent >= 0)
				{
					switch (selectedComponent)
					{
						case 0: std::cout << "TODO: add animator" << std::endl; break;
						case 1: std::cout << "TODO: add camera" << std::endl; break;
						case 2: std::cout << "TODO: add fog volume" << std::endl; break;
						case 3: std::cout << "TODO: add light" << std::endl; break;
						case 4: std::cout << "TODO: add light probe" << std::endl; break;
						case 5: 
						{
							auto mesh = pr::Mesh::create("Mesh");
							auto r = pr::Renderable::create(mesh);
							selectedModel->addComponent(r);
							scenes[sceneIndex]->initDescriptors(renderer->getDescriptorPool());
							renderer->buildCmdBuffer(scenes[sceneIndex]);
							renderer->buildScatterCmdBuffer(scenes[sceneIndex]);
							break;
						}
						default: std::cout << "error: unknown component!" << std::endl; break;
					}
					selectedComponent = -1;
				}				
						
				ImGui::EndPopup();
			}
		}		
	}

	ImGui::End();

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
	ImGuiIO& io = ImGui::GetIO();
	while (eventHandler.handleEvents())
	{
		auto startTime = high_resolution_clock::now();
		if (animTime > tickTime)
		{
			scenes[sceneIndex]->update(animTime);

			userCamera.move(animTime);
			userCamera.rotate(animTime);
			renderer->updateCamera(scenes[sceneIndex], userCamera, totalTime);
			renderer->updateLights(userCamera, scenes[sceneIndex]);
			animTime = 0.0f;
		}

		// acquire next frame (only need for Vulkan)
		int currentBuffer = swapchain->acquireNextFrame();

		renderer->renderToTexture(scenes[sceneIndex]);
		auto mainCmdBuf = renderer->getCommandBuffer(currentBuffer);
		context.submitCommandBuffer(swapchain, mainCmdBuf);
		context.waitDeviceIdle(); 

		// update gui elements, containing render result
		updateGUI();

		// build gui command buffers, TODO: check if buffers changed...
		if (gui->update())
			gui->buildCmd(swapchain);
		else
			gui->buildCmd2(swapchain);

		auto guiCmdBuf = gui->getCommandBuffer(currentBuffer);
		context.submitCommandBuffer(mainCmdBuf, guiCmdBuf);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			context.makeCurrent();
		}

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
	
	selectedModel = nullptr;
	scenes[sceneIndex].reset();
	assetManager.destroy();
	gui.reset();
	renderer.reset();
	swapchain.reset();
	context.destroy();
};