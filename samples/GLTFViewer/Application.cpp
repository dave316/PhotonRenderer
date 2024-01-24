#include "Application.h"

#include <IO/Image/ImageDecoder.h>
#include <IO/GLTFImporter.h>
#include <Graphics/MeshPrimitives.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <rapidjson/document.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace json = rapidjson;
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

	std::string assetPath = "../../../../assets";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// TODO: check if font file available first.....
	std::string fontPath = assetPath + "/Fonts/arial.ttf";
	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 28);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	samplePath = assetPath + "/glTF-Sample-Models/2.0";
	//samplePath = "C:/workspace/code/PhotonRendererNew/assets/glTF-Sample-Assets/Models";
	initGLTFSamples(samplePath);

	if (!renderer.init())
		return false;

	scene = Scene::create("Scene");

	std::string envFn = assetPath + "/Footprint_Court/Footprint_Court_2k.hdr";
	//std::string envFn = "C:/workspace/code/VikingVillage/Assets/Viking Village/Textures/Skies/Daytime/SunsetSkyboxHDR.hdr";
	auto panoImg = IO::decodeHDRFromFile(envFn, true);

	Skybox skybox;
	skybox.texture = panoImg->upload(false);
	skybox.exposure = 1.0;
	scene->setSkybox(skybox);

	//fogMaterial = Texture3D::create(256, 256, 256, GL::R32F);
	//float* buffer = new float[256 * 256 * 256];
	//for (int i = 0; i < 256; i++)
	//{
	//	for (int j = 0; j < 256; j++)
	//	{
	//		for (int k = 0; k < 256; k++)
	//		{
	//			int c = 1;
	//			float x = static_cast<float>(i) / 64.0f;
	//			float y = static_cast<float>(j) / 64.0f;
	//			float z = static_cast<float>(k) / 64.0f;

	//			int o;
	//			float frequency = 1.0f;
	//			float amplitude = 1.0f;
	//			float sum = 0.0f;

	//			for (o = 0; o < 8; o++) {
	//				float r = stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 4, 4, 4, (unsigned char)o) * amplitude;
	//				sum += (float)fabs(r);
	//				frequency *= 2.0f;
	//				amplitude *= 0.5f;
	//			}

	//			//glm::vec3 pos(x, y, z);
	//			//glm::vec3 rPos = pos - glm::vec3(0.5);
	//			//float value = 0.0;
	//			//if (glm::length(rPos) < 0.25)
	//			//	value = glm::length(rPos) * 1.0;

	//			//float noiseR = stb_perlin_noise3_seed(x, y, z, 8, 8, 8, 75839);
	//			//float noiseR = stb_perlin_fbm_noise3(x, y, z, 2.0f, 0.5f, 8);
	//			//float noiseG = stb_perlin_noise3_seed(x, y, z, 1, 1, 1, 5678);
	//			//float noiseB = stb_perlin_noise3_seed(x, y, z, 1, 1, 1, 9012);
	//			buffer[k * 256 * 256 * c + j * 256 * c + i * c + 0] = sum;
	//			//buffer[k * 256 * 256 * 3 + j * 256 * 3 + i * 3 + 1] = 0.5 + noiseG;
	//			//buffer[k * 256 * 256 * 3 + j * 256 * 3 + i * 3 + 2] = 0.5 + noiseB;
	//			//std::cout << noise << std::endl;
	//		}
	//	}
	//}
	//fogMaterial->upload(buffer);
	//fogMaterial->use(23);
	//delete[] buffer;

	renderer.updateCamera(camera);
	renderer.prepare(scene);

	//fogMaterial = Texture3D::create(256, 256, 256, GL::RGBA32F);
	//fogMaterial->setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
	//fogMaterial->bind();
	//glBindImageTexture(0, fogMaterial->getID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	//auto vScatterShader = renderer.getShader("VolumeScatter");
	//vScatterShader->use();
	//glDispatchCompute(256, 256, 256);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//fogMaterial->use(24);

	setupInput();

	debugChannels.push_back("None");
	debugChannels.push_back("Texture Coords 0");
	debugChannels.push_back("Texture Coords 1");
	debugChannels.push_back("Geometry Normal");
	debugChannels.push_back("Shading Normal");
	debugChannels.push_back("Ambient Occlusion");
	debugChannels.push_back("Emission");

	debugChannels.push_back("Metallic Roughness");
	debugChannels.push_back("Base Color");
	debugChannels.push_back("Roughness");
	debugChannels.push_back("Metallic");

	debugChannels.push_back("Sheen");
	debugChannels.push_back("Sheen Color");
	debugChannels.push_back("Sheen Roughness");

	debugChannels.push_back("Clearcoat");
	debugChannels.push_back("Clearcoat Factor");
	debugChannels.push_back("Clearcoat Roughness");
	debugChannels.push_back("Clearcoat Normal");

	debugChannels.push_back("Transmission");
	debugChannels.push_back("Transmission Factor");
	debugChannels.push_back("Thickness");
	debugChannels.push_back("Attenuation Color");

	debugChannels.push_back("Specular");
	debugChannels.push_back("Specular Factor");
	debugChannels.push_back("Specular Color");

	debugChannels.push_back("Iridescence");
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
	supportedVariants.insert("glTF-Quantized");
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

bool Application::loadGLTFModel(const std::string& name, const std::string& fullpath)
{
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(fullpath);
	if (rootEntity)
	{
		modelInfo = importer.getGeneralStats();
		renderInfo = importer.getRenderingStats();

		scene->addRootEntity(name, rootEntity);

		//{
		//	SubMesh s;
		//	s.primitive = MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1));
		//	s.material = getDefaultMaterial();
		//	s.material->addProperty("material.metallicFactor", 0.0f);
		//	s.material->addProperty("material.roughnessFactor", 1.0f);

		//	auto mesh = Mesh::create("Box");
		//	mesh->addSubMesh(s);

		//	auto e = Entity::create("Box", nullptr);
		//	auto t = e->getComponent<Transform>();
		//	auto r = e->addComponent<Renderable>();
		//	r->setMesh(mesh);

		//	t->setLocalPosition(glm::vec3(-3.0f, 3.0f, -0.25f));
		//	t->setLocalScale(glm::vec3(1.0f));

		//	scene->addRootEntity("box", e);
		//}

		{
			SubMesh s;
			s.primitive = MeshPrimitives::createBox(glm::vec3(0, 5, 0), glm::vec3(2));
			auto surf = s.primitive->getSurface();
			Box bbox;
			for (auto& v : surf.vertices)
				bbox.expand(v.position);

			renderer.setVolumeBox(bbox.getMinPoint(), bbox.getMaxPoint());

			s.material = getDefaultMaterial();
			s.material->addProperty("material.metallicFactor", 0.0f);
			s.material->addProperty("material.roughnessFactor", 1.0f);

			auto mesh = Mesh::create("Box");
			mesh->addSubMesh(s);

			auto e = Entity::create("Box", nullptr);
			auto t = e->getComponent<Transform>();
			auto r = e->addComponent<Renderable>();
			r->setMesh(mesh);

			//t->setLocalPosition(glm::vec3(-3.0f, 3.0f, -0.25f));
			//t->setLocalScale(glm::vec3(1.0f));

			//scene->addRootEntity("box", e);
		}

		////auto light = Light::create(LightType::DIRECTIONAL);
		//auto light = Light::create(LightType::POINT);

		//light->setColorTemp(3500);
		//light->setLuminousPower(2500);
		////light->setLuminousIntensity(10);
		//light->setRange(100);

		//auto lightEntity = Entity::create("light", nullptr);
		//auto t = lightEntity->getComponent<Transform>();
		//lightEntity->addComponent(light);
		//t->setLocalPosition(glm::vec3(0,2,0));
		////t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)) * glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)));

		//scene->addRootEntity("sun", lightEntity);

		renderer.updateCamera(camera);
		renderer.prepare(scene);

		if (animate)
			scene->playAnimations();
		else
			scene->stopAnimations();

		initCamera();

		for (auto [_,root] : scene->getRootEntities())
		{
			CameraInfo mainCam;
			mainCam.name = "MainCamera";
			cameras.push_back(mainCam);
			for (auto e : root->getChildrenWithComponent<Camera>())
			{
				auto cam = e->getComponent<Camera>(); 
				auto t = e->getComponent<Transform>();
				glm::mat4 T = t->getTransform();
				
				CameraInfo camInfo;
				camInfo.name = cam->getName();
				camInfo.P = cam->getProjection();
				camInfo.V = glm::inverse(T);
				camInfo.pos = T * glm::vec4(0, 0, 0, 1);
				cameras.push_back(camInfo);
			}

			// TODO: variants are stored in extensions, they should be placed somewhere else then in the mesh
			auto r = root->getComponentsInChildren<Renderable>();
			if(!r.empty())
				variantNames = root->getComponentsInChildren<Renderable>()[0]->getMesh()->getVariants(); // WTF			
		}

		return true;
	}
	return false;
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

void Application::handleDrop(int count, const char** paths)
{
	if (count != 1)
	{
		std::cout << "only one file allowed!" << std::endl;
		return;
	}

	clear();

	if(count >= 1)
	{
		std::string fullPath(paths[0]);
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
			std::cout << "not supported file extension: " << ext << std::endl;
	}
}

void Application::clear()
{
	scene->clear();
	renderer.clear();
	cameras.clear();
	cameraIndex = 0;
	materialIndex = 0;
	animIndex = 0;
}

void Application::gui()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New");
			if (ImGui::MenuItem("Open GLTF"))
				ImGuiFileDialog::Instance()->OpenDialog("ChooseGLTF", "Select GLTF file", ".gltf,.glb", "../../../../assets");
			if (ImGui::MenuItem("Open HDR"))
				ImGuiFileDialog::Instance()->OpenDialog("ChooseHDR", "Select GLTF file", ".hdr", "../../../../assets");
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
					clear();

					variantIndex = 0;
					prevVariantIndex = 0;

					GLTFSampleInfo info = samplesInfo[sampleIndex];
					std::string variant = info.variants[variantIndex].first;
					std::string fn = info.variants[variantIndex].second;
					std::string fullPath = samplePath + "/" + info.name + "/" + variant + "/" + fn;

					if (loadGLTFModel(info.name, fullPath))
					{
						prevSampleIndex = sampleIndex;
						std::cout << "loaded " << fn << std::endl;
					}
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
					clear();

					GLTFSampleInfo info = samplesInfo[sampleIndex];
					std::string variant = info.variants[variantIndex].first;
					std::string fn = info.variants[variantIndex].second;
					std::string fullPath = samplePath + "/" + info.name + "/" + variant + "/" + fn;
					if (loadGLTFModel(info.name, fullPath))
					{
						prevVariantIndex = variantIndex;
						std::cout << "loaded " << fn << std::endl;
					}
				}

				ImGui::EndCombo();
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Control"))
	{
		if (ImGui::Checkbox("IBL", &useIBL))
			renderer.setIBL(useIBL);

		ImGui::SameLine();

		if (ImGui::Checkbox("Lights", &useLights))
			renderer.setLights(useLights ? scene->getNumLights() : 0);

		ImGui::SameLine();

		if (ImGui::Checkbox("Bloom", &useBloom))
			renderer.setBloom(useBloom);

		if (!toneMappingOperators.empty())
		{
			if (ImGui::BeginCombo("ToneMapper", toneMappingOperators[tonemappingIndex].c_str()))
			{
				for (int i = 0; i < toneMappingOperators.size(); i++)
				{
					const bool isSelected = (tonemappingIndex == i);
					if (ImGui::Selectable(toneMappingOperators[i].c_str(), isSelected))
						tonemappingIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				renderer.setTonemappingOp(tonemappingIndex);

				ImGui::EndCombo();
			}
		}

		//ImGui::ColorPicker3("Scattering", (float*)&scattering, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
		//ImGui::ColorPicker3("Absorption", (float*)&absorption, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
		
		ImGui::SliderFloat("Exposure", &ppParams.postExposure, -5.0f, 5.0f);
		ImGui::SliderFloat("Bloom Intensity", &ppParams.bloomIntensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Bloom Threshold", &ppParams.bloomThreshold, 0.0f, 1.0f);
		ImGui::ColorEdit3("Bloom Tint", (float*)&ppParams.bloomTint, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
		ImGui::ColorEdit3("Scatter", (float*)&scatter, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
		ImGui::SliderFloat("Absorption", &absorption, 0.0f, 5.0f);
		ImGui::SliderFloat("Phase", &phase, -1.0f, 1.0f);

		if (!cameras.empty())
		{
			if (ImGui::BeginCombo("Camera", cameras[cameraIndex].name.c_str()))
			{
				for (int i = 0; i < cameras.size(); i++)
				{
					const bool isSelected = (cameraIndex == i);
					if (ImGui::Selectable(cameras[i].name.c_str(), isSelected))
						cameraIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}

		auto animations = scene->getAnimations();
		if (!animations.empty())
		{
			if (ImGui::BeginCombo("Animation", animations[animIndex].c_str()))
			{
				for (int i = 0; i < animations.size(); i++)
				{
					const bool isSelected = (animIndex == i);
					if (ImGui::Selectable(animations[i].c_str(), isSelected))
						animIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				scene->switchAnimations(animIndex);

				ImGui::EndCombo();
			}

			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				animate = !animate;
				if (animate)
					scene->playAnimations();
				else
					scene->stopAnimations();
			}
		}

		if (!variantNames.empty())
		{
			if (ImGui::BeginCombo("Material", variantNames[materialIndex].c_str()))
			{
				for (int i = 0; i < variantNames.size(); i++)
				{
					const bool isSelected = (materialIndex == i);
					if (ImGui::Selectable(variantNames[i].c_str(), isSelected))
						materialIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				scene->switchVariant(materialIndex);

				ImGui::EndCombo();
			}
		}

		if (!debugChannels.empty())
		{
			if (ImGui::BeginCombo("Debug", debugChannels[debugIndex].c_str()))
			{
				for (int i = 0; i < debugChannels.size(); i++)
				{
					const bool isSelected = (debugIndex == i);
					if (ImGui::Selectable(debugChannels[i].c_str(), isSelected))
						debugIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				renderer.setDebugChannel(debugIndex);

				ImGui::EndCombo();
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Information"))
	{
		if(ImGui::CollapsingHeader("Model info", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto [name, count] : modelInfo)
			{
				std::string text = name + ": " + std::to_string(count);
				ImGui::Text(text.c_str());
			}
		}

		if (ImGui::CollapsingHeader("Rendering info", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto [name, count] : renderInfo)
			{
				std::string text = name + ": " + std::to_string(count);
				ImGui::Text(text.c_str());
			}
		}
	}
	ImGui::End();

	if (ImGuiFileDialog::Instance()->Display("ChooseGLTF"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			clear();

			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			std::string name = fileName.substr(0, fileName.find_last_of("."));
			loadGLTFModel(name, filePathName);
		}
		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseHDR"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			renderer.initLightProbes(scene);
		}
		ImGuiFileDialog::Instance()->Close();
	}

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
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

		if (animTime > tickTime)
		{
			if (cameraIndex == 0)
			{
				camera.move(animTime);
				camera.rotate(animTime);
				renderer.updateCamera(camera);
			}
			else
			{
				auto cam = cameras[cameraIndex];
				renderer.updateCamera(cam.P, cam.V, cam.pos);
			}
			renderer.updatePostProcess(ppParams);
			renderer.updateTime(startTime);
			renderer.setVolumeParams(scatter, absorption, phase);

			if (animate)
			{
				scene->updateAnimations(animTime);
				scene->updateAnimationState(animTime);
				renderer.initLights(scene, camera);
				renderer.updateShadows(scene);
			}	
			animTime = 0.0f;
		}

		renderer.renderToScreen(scene);

		gui();

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