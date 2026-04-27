#include "Application.h"
//#include "UnityTestImporter.h"

#include <IO/GLTFImporter.h>
#include <Core/FogVolume.h>
#include <IO/ImageLoader.h>
#include <Utils/IBL.h>
#include <glm/glm.hpp>
#include <chrono>

using namespace std::chrono;
using namespace std::placeholders;

Application::Application(const char* title, unsigned int width, unsigned int height) :
	api(pr::GraphicsAPI::OpenGL),
	eventHandler(Win32EventHandler::instance()),
	context(pr::GraphicsContext::getInstance())
{
	window = Win32Window::create(title, width, height);
	camera.setAspect((float)width / (float)height);
}

Application::~Application()
{
	
}

bool Application::init()
{	
	context.init(api, window);

	swapchain = context.createSwapchain(window);

	renderer = pr::Renderer::create();
	renderer->init(window, swapchain);

	initScene();
	//initUnitySceneOLD();
	//initUnitySceneNEW();

	renderer->prepare(camera, scene);
	renderer->buildCmdBuffer(scene, swapchain);
	renderer->buildShadowCmdBuffer(scene);
	renderer->buildScatterCmdBuffer(scene);

	setupInput();

	return true;
}

void Application::initScene()
{
	//std::string projPath = "C:/workspace/code/TestProject";
	//assetManager.init(projPath);

	////std::string srcPath = "C:/workspace/code/PhotonRenderer/assets/glTF-Sample-Assets/Models/DamagedHelmet/glTF";
	////auto filenames = pr::getAllFileNames(srcPath, "");
	////auto fileRoot = assetManager.getNodeFromPath("Models\\DamagedHelmet");
	////for (auto fn : filenames)
	////{
	////	std::string fullpath = srcPath + "/" + fn;
	////	assetManager.addFile(fileRoot, fullpath);
	////	std::cout << fullpath << std::endl;
	////}

	////std::string srcPath = "C:/workspace/code/PhotonRenderer/assets/glTF-Sample-Assets/Models/DamagedHelmet/glTF";
	////auto fileRoot = assetManager.getNodeFromPath("Models");
	////assetManager.addDirectory(fileRoot, srcPath);
	//
	////assetManager.loadAssetsAsync();
	//assetManager.loadAssetsFromDisk();
	//assetManager.copyAssetsToGPU();

	//auto node = assetManager.getNodeFromPath("glTF\\DamagedHelmet.gltf");
	//auto root = assetManager.getEntity(node->entityIndex);
	//
	//scene = pr::Scene::create("scene");
	//scene->addRoot(root);
	////scene->update(0.0f);

	std::string assetPath = "../../../../assets";
	std::string modelName = "EmissiveStrengthTest";
	IO::glTF::Importer importer;
	std::vector<pr::Scene::Ptr> scenes;
	importer.importModel(assetPath + "/glTF-Sample-Assets/Models/" + modelName + "/glTF/" + modelName + ".gltf", scenes);
	//scene->addRoot(root);
	scene = scenes[0];

	for (auto root : scene->getRootNodes())
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
			}
		}
	}

	//auto light = pr::Light::create(pr::LightType::POINT, glm::vec3(1), 200.0f, 100.0f);
	//light->setColorTemp(10000);
	//light->setLuminousPower(1500);
	////light->setLuminousPower(25);
	//mainLight = pr::Entity::create("sun", nullptr);
	//auto t = mainLight->getComponent<pr::Transform>();
	//mainLight->addComponent(light);
	////t->setLocalRotation(glm::angleAxis(glm::radians(85.0f), glm::vec3(-1, 0, 0)) * glm::angleAxis(glm::radians(35.0f), glm::vec3(0, 1, 0)));
	////t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)));
	//t->setLocalPosition(glm::vec3(0, 5, 0));
	//scene->addRoot(mainLight);

	//glm::vec3 positions[8] = {
	//glm::vec3(-9,1.5,-3.5),
	//glm::vec3(-9,1.5,3.5),
	//glm::vec3(9,1.5,-3.5),
	//glm::vec3(9,1.5,3.5),
	//glm::vec3(-9,4,-3.5),
	//glm::vec3(-9,4,3.5),
	//glm::vec3(9,4,-3.5),
	//glm::vec3(9,4,3.5),
	//};

	//for (int i = 0; i < 4; i++)
	//{
	//	auto light = pr::Light::create(pr::LightType::POINT, glm::vec3(1), 200.0f, 100.0f);

	//	light->setColorTemp(3000);
	//	light->setLuminousPower(500);
	//	//light->setLuminousIntensity(10);
	//	light->setRange(10);
	//	//light->setConeAngles(0.1f, 0.5f);

	//	auto lightEntity = pr::Entity::create("light" + std::to_string(i), nullptr);
	//	auto t = lightEntity->getComponent<pr::Transform>();
	//	lightEntity->addComponent(light);
	//	t->setLocalPosition(positions[i]);

	//	scene->addRoot(lightEntity);
	//}

	//{
	//	{
	//		auto fogEntity = pr::Entity::create("Fog", nullptr);
	//		auto t = fogEntity->getComponent<pr::Transform>();
	//		t->setLocalPosition(glm::vec3(-30, 3, 0));
	//		t->setLocalScale(glm::vec3(2));
	//		auto volume = pr::FogVolume::create();
	//		//volume->setEmissive(glm::vec3(0, 0, 1));
	//		volume->setScattering(glm::vec3(1, 0, 0));
	//		//volume->setPhase(0.75f);
	//		volume->setDensityPerlinNoise(3.0f);
	//		fogEntity->addComponent(volume);
	//		scene->addRoot(fogEntity);
	//	}

	//	{
	//		auto fogEntity = pr::Entity::create("Fog", nullptr);
	//		auto t = fogEntity->getComponent<pr::Transform>();
	//		t->setLocalPosition(glm::vec3(0, 3, 0));
	//		t->setLocalScale(glm::vec3(2));
	//		auto volume = pr::FogVolume::create();
	//		//volume->setEmissive(glm::vec3(0,1,0));
	//		volume->setScattering(glm::vec3(0, 1, 0));
	//		//volume->setPhase(0.25f);
	//		volume->setDensitySphere(5.0f);
	//		fogEntity->addComponent(volume);
	//		scene->addRoot(fogEntity);
	//	}

	//	{
	//		auto fogEntity = pr::Entity::create("Fog", nullptr);
	//		auto t = fogEntity->getComponent<pr::Transform>();
	//		t->setLocalPosition(glm::vec3(3, 3, 0));
	//		t->setLocalScale(glm::vec3(2));
	//		auto volume = pr::FogVolume::create();
	//		//volume->setEmissive(glm::vec3(0, 0, 1));
	//		volume->setScattering(glm::vec3(0, 0, 1));
	//		//volume->setPhase(0.75f);
	//		volume->setAbsorbtion(5.0f);
	//		//volume->setDensityPerlinNoise(1.0f);
	//		fogEntity->addComponent(volume);
	//		scene->addRoot(fogEntity);
	//	}
	//}

	std::string envFn = assetPath + "/glTF-Sample-Environments/doge2.hdr";
	auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
	uint32 width = panoImg->getWidth();
	uint32 height = panoImg->getHeight();
	uint8* data = panoImg->getRawPtr();
	uint32 dataSize = width * height * sizeof(float) * 4;
	auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
	panoTex->upload(data, dataSize);
	panoTex->createData();
	panoTex->uploadData();
	auto skybox = IBL::convertEqui2CM(panoTex, 1024, 0.0f);
	scene->setSkybox(skybox);
}

//void Application::initUnitySceneOLD()
//{
//	pr::Light::lightForward = glm::vec3(0, 0, 1);
//
//	//std::string unityAssetPath = "C:/workspace/code/Archviz/Assets";
//	//std::string unityPrefabPath = unityAssetPath + "/ArchVizPRO Interior Vol.6/3D PREFAB";
//	//std::string unitySceneFile = "ArchVizPRO Interior Vol.6/3D Scene/AVP6_Desktop.unity";
//
//	std::string unityAssetPath = "C:/workspace/code/VikingVillage/Assets";
//	std::string unityPrefabPath = unityAssetPath + "/Viking Village/Prefabs";
//	std::string unitySceneFile = "Viking Village/Scenes/The_Viking_Village.unity";
//
//	IO::Unity::Importer sceneImporter(8192);
//	scene = sceneImporter.loadScene(unityAssetPath, unitySceneFile);
//	scene->checkWindingOrder();
//	sceneImporter.clear();
//
//	//IO::Unity::Importer sceneImporter(8192);
//	//sceneImporter.loadMetadata(unityAssetPath);
//	////scene = sceneImporter.loadScene(unityAssetPath, unitySceneFile);
//	//auto root = sceneImporter.loadPrefab(unityPrefabPath + "/Buildings/pf_build_crane_01.prefab");
//	//for (auto r : root->getComponentsInChildren<pr::Renderable>())
//	//	r->setDiffuseMode(0);
//	//scene = pr::Scene::create("scene");
//	//scene->addRoot(root);
//	//sceneImporter.clear();
//
//	//for (auto entity : scene->getRootNodes())
//	//{
//	//	auto name = entity->getName();
//	//	auto models = entity->getChildrenWithComponent<pr::Renderable>();
//	//	for (auto m : models)
//	//	{
//	//		auto r = m->getComponent<pr::Renderable>();
//	//		if (name.compare("3D FX") == 0 && m->getName().compare("Sphere001") == 0)
//	//		{
//	//			r->setType(pr::RenderType::Opaque);
//	//			r->setPriority(1);
//	//		}
//	//		if (name.compare("3D HOUSE") == 0)
//	//		{
//	//			auto modelName = m->getName();
//	//			if (modelName.length() == 16)
//	//			{
//	//				auto prefix = m->getName().substr(0, 14);
//	//				if (prefix.compare("Glass_Exterior") == 0)
//	//				{
//	//					r->setType(pr::RenderType::Opaque);
//	//					r->setPriority(2);
//	//				}
//	//			}
//	//			else if (modelName.length() == 23)
//	//			{
//	//				auto prefix = modelName.substr(0, 21);
//	//				if (prefix.compare("Window_Glass_Interior") == 0)
//	//				{
//	//					r->setType(pr::RenderType::Opaque);
//	//					r->setPriority(2);
//	//				}
//	//			}
//	//		}
//	//	}
//	//}
//
//	std::string assetPath = "../../../../assets";
//	std::string envFn = assetPath + "/glTF-Sample-Environments/doge2.hdr";
//	auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
//	uint32 width = panoImg->getWidth();
//	uint32 height = panoImg->getHeight();
//	uint8* data = panoImg->getRawPtr();
//	uint32 dataSize = width * height * sizeof(float) * 4;
//	auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
//	panoTex->upload(data, dataSize);
//	panoTex->createData();
//	panoTex->uploadData();
//	auto skybox = IBL::convertEqui2CM(panoTex, 1024, 0.0f);
//	scene->setSkybox(skybox);
//}
//
//void Application::initUnitySceneNEW()
//{
//	pr::Light::lightForward = glm::vec3(0, 0, 1);
//
//	//std::string unityAssetPath = "c:/workspace/code/Archviz/Assets";
//	//std::string unityPrefabPath = unityAssetPath + "/ArchVizPRO Interior Vol.6/3D PREFAB";
//	//std::string unitySceneFile = "ArchVizPRO Interior Vol.6/3D Scene/AVP6_Desktop.unity";
//
//	std::string unityAssetPath = "C:/workspace/code/VikingVillage/Assets";
//	std::string unityPrefabPath = unityAssetPath + "/Viking Village/Prefabs";
//	std::string unitySceneFile = "Viking Village/Scenes/The_Viking_Village.unity";
//
//	UnityTestImporter importer;
//	scene = importer.importScene(unityAssetPath, unitySceneFile);
//	scene->checkWindingOrder();
//
//	//UnityTestImporter importer;
//	//auto root = importer.importPrefab(unityAssetPath, "/Viking Village/Prefabs/Buildings/pf_build_blacksmith_01.prefab");
//	//for (auto r : root->getComponentsInChildren<pr::Renderable>())
//	//	r->setDiffuseMode(0);
//	//scene = pr::Scene::create("scene");
//	//scene->addRoot(root);
//
//	//for (auto entity : scene->getRootNodes())
//	//{
//	//	auto name = entity->getName();
//	//	auto models = entity->getChildrenWithComponent<pr::Renderable>();
//	//	for (auto m : models)
//	//	{
//	//		auto r = m->getComponent<pr::Renderable>();
//	//		if (name.compare("3D FX") == 0 && m->getName().compare("Sphere001") == 0)
//	//		{
//	//			r->setType(pr::RenderType::Opaque);
//	//			r->setPriority(1);
//	//		}
//	//		if (name.compare("3D HOUSE") == 0)
//	//		{
//	//			auto modelName = m->getName();
//	//			if (modelName.length() == 16)
//	//			{
//	//				auto prefix = m->getName().substr(0, 14);
//	//				if (prefix.compare("Glass_Exterior") == 0)
//	//				{
//	//					r->setType(pr::RenderType::Opaque);
//	//					r->setPriority(2);
//	//				}
//	//			}
//	//			else if (modelName.length() == 23)
//	//			{
//	//				auto prefix = modelName.substr(0, 21);
//	//				if (prefix.compare("Window_Glass_Interior") == 0)
//	//				{
//	//					r->setType(pr::RenderType::Opaque);
//	//					r->setPriority(2);
//	//				}
//	//			}
//	//		}
//	//	}
//	//}
//
//	//std::string assetPath = "../../../../assets";
//	//std::string envFn = assetPath + "/glTF-Sample-Environments/doge2.hdr";
//	//auto panoImg = IO::ImageLoader::loadHDRFromFile(envFn);
//	//uint32 width = panoImg->getWidth();
//	//uint32 height = panoImg->getHeight();
//	//uint8* data = panoImg->getRawPtr();
//	//uint32 dataSize = width * height * sizeof(float) * 4;
//	//auto panoTex = pr::Texture2D::create(width, height, GPU::Format::RGBA32F);
//	//panoTex->upload(data, dataSize);
//	//panoTex->createData();
//	//panoTex->uploadData();
//	//auto skybox = IBL::convertEqui2CM(panoTex, 1024, 0.0f);
//	//scene->setSkybox(skybox);
//}

void Application::setupInput()
{
	Win32EventHandler& eventHandler = Win32EventHandler::instance();
	eventHandler.registerKeyCB(KeyCode::W, 0, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::FORWARD));
	eventHandler.registerKeyCB(KeyCode::S, 0, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::BACK));
	eventHandler.registerKeyCB(KeyCode::A, 0, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::LEFT));
	eventHandler.registerKeyCB(KeyCode::D, 0, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::RIGHT));
	eventHandler.registerKeyCB(KeyCode::W, 1, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::FORWARD));
	eventHandler.registerKeyCB(KeyCode::S, 1, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::BACK));
	eventHandler.registerKeyCB(KeyCode::A, 1, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::LEFT));
	eventHandler.registerKeyCB(KeyCode::D, 1, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::RIGHT));
	
	static int mouseX = 0;
	static int mouseY = 0;
	eventHandler.registerKeyCB(KeyCode::RightMouseButton, 0, [&] {
		eventHandler.setMouseMoveEnabled(true);
		window->hideCursor();
		window->getCursorPos(mouseX, mouseY);
	});

	eventHandler.registerKeyCB(KeyCode::RightMouseButton, 1, [&] {
		eventHandler.setMouseMoveEnabled(false);
		window->showCursor();
		window->setCursorPos(mouseX, mouseY);
	});

	eventHandler.registerMouseMoveCB([&](int x, int y) {
		float dx = (float)(x - mouseX);
		float dy = (float)(mouseY - y);
		camera.updateRotation(dx, dy);
		window->setCursorPos(mouseX, mouseY);
	});
}

//void Application::updateGUI()
//{
//	ImGui_ImplWin32_NewFrame();
//	ImGui::NewFrame();
//
//	bool show_demo_window = false;
//	if (show_demo_window)
//		ImGui::ShowDemoWindow(&show_demo_window);
//
//	ImGui::Render();
//}

void Application::loop()
{
	float dt = 0.0f;
	float animTime = 0.0f;
	float tickTime = 1.0f / 60.0f;
	float updateTime = 0.0f;
	float totalTime = 0.0f;
	uint32 frames = 0;

	float currentTime = 0.0f;
	float duration = 10.0f;
	glm::vec3 start = glm::vec3(-7.0f, 0, 0);
	glm::vec3 end = glm::vec3(7.0f, 5, 0);
	glm::vec3 currentPosition = glm::vec3(0, 5, 0);

	bool assetsLoaded = false;
	
	while (eventHandler.handleEvents())
	{
		//if (assetManager.assetsReady() && !assetsLoaded)
		//{
		//	assetManager.copyAssetsToGPU();

		//	std::cout << "assets loaded! adding to scene!" << std::endl;
		//	auto node = assetManager.getNodeFromPath("DamagedHelmet\\DamagedHelmet.gltf");
		//	auto root = assetManager.getEntity(node->entityIndex);
		//	scene->addRoot(root);
		//	scene->initDescriptors(renderer->getDescriptorPool());
		//	renderer->buildCmdBuffer(scene, swapchain);
		//	renderer->buildScatterCmdBuffer(scene);

		//	assetsLoaded = true;
		//}

		auto startTime = high_resolution_clock::now();
		if (animTime > tickTime)
		{
			//currentTime += animTime;
			//if (currentTime > duration)
			//currentTime = 0.0f;
			//float factor = currentTime / duration;
			//currentPosition = glm::mix(start, end, factor);
			//auto t = mainLight->getComponent<pr::Transform>();
			//t->setLocalPosition(currentPosition);

			scene->update(animTime);
			camera.move(animTime);
			camera.rotate(animTime);
			renderer->updateLights(camera, scene);
			renderer->updateCamera(scene, camera, totalTime);

			animTime = 0.0f;
		}

		// acquire next frame (only need for Vulkan)
		int currentBuffer = swapchain->acquireNextFrame();
		renderer->renderToTexture(scene);
		if (context.getCurrentAPI() == pr::GraphicsAPI::Direct3D11)
			renderer->buildCmdBuffer(scene, swapchain);
		auto mainCmdBuf = renderer->getCommandBuffer(currentBuffer);
		context.submitCommandBuffer(swapchain, mainCmdBuf);
		swapchain->present(mainCmdBuf);

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

	scene.reset();
	renderer.reset();
	swapchain.reset();
	context.destroy();
}