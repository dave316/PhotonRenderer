#include "Application.h"
#include <Graphics/MeshPrimitives.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <IO/AssimpImporter.h>
#include <IO/Image/ImageDecoder.h>
#include <IO/Image/Cubemap.h>
#include <IO/SceneExporter.h>
#include <Utils/Types.h>
#include <random>

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
	std::string srcPath = assetPath + "/glTF-Sample-Models/sourceModels";
	std::string gltfPath = assetPath + "/glTF-Sample-Models/2.0";
	std::vector<std::string> names;

	//names.push_back("Lights");
	//names.push_back("Sponza");
	//names.push_back("DamagedHelmet");
	//names.push_back("FlightHelmet");
	//names.push_back("SheenChair");
	//names.push_back("GlamVelvetSofa");
	//names.push_back("LightsPunctualLamp");
	//names.push_back("IridescentDishWithOlives");
	//names.push_back("IridescenceSuzanne");
	//names.push_back("StainedGlassLamp");
	//names.push_back("ToyCar");	
	
	scene = Scene::create("scene");

	//scene->addLight("light");

	//std::string name = "Waterfall";
	//scene->loadModelGLTF(name, assetPath + "/AnimationPointer/" + name + "/glTF/" + name + ".gltf");
	//scene->loadModelGLTF(name, assetPath + "/AnimationPointer/OtherTests-glTF-Binary/SimpleRecording.glb");

	//IO::AssimpImporter importer;
	//auto model = importer.importModel(assetPath + "/plane.obj");
	//model->getComponent<Transform>()->setLocalScale(glm::vec3(2));
	//scene->addEntity("plane", model);

	////IO::GLTFImporter importer;
	//IO::AssimpImporter importer;
	////auto fullPath = gltfPath + "/BrainStem/glTF/BrainStem.gltf";
	////auto fullPath = srcPath + "/CesiumMan/CesiumMan.dae";
	////auto fullPath = srcPath + "/BarramundiFish/BarramundiFish.fbx";
	//auto fullPath = assetPath + "/TigerNew/tiger.dae";
	//auto model = importer.importModel(fullPath);
	//scene->addEntity("Tiger", model);

	//importer.clear();

	//std::ifstream file(assetPath + "/probe.txt");
	//std::string line;
	//IO::SphericalLightingProbe shProbe(9);
	//int i = 0;
	//while (std::getline(file, line))
	//{
	//	std::stringstream ss(line);
	//	std::string strR, strG, strB;
	//	float r = std::stof(line.substr(1, 18));
	//	float g = std::stof(line.substr(21, 18));
	//	float b = std::stof(line.substr(41, 18));
	//	if(i < shProbe.coeffs.size())
	//		shProbe.coeffs[i] = glm::vec3(r, g, b);
	//	i++;
	//}

	//std::string fn = assetPath + "/Footprint_Court/Footprint_Court_2k.hdr";
	//auto panoImgHDR = IO::decodeHDRFromFile(fn);

	////{
	////	auto panoImgHDR = IO::loadImageFromFile(fn);
	////	uint32_t w = panoImgHDR->getWidth();
	////	uint32_t h = panoImgHDR->getHeight();
	////	uint32_t c = panoImgHDR->getChannels();
	////	uint32_t size = w * h * c;
	////	float* data = panoImgHDR->getRawPtr();
	////	glm::vec3 avgColor(0);
	////	for (int i = 0; i < size; i += 3)
	////	{
	////		avgColor.r += data[i];
	////		avgColor.g += data[i + 1];
	////		avgColor.b += data[i + 2];
	////	}
	////	avgColor /= (float)w * h;
	////	std::cout << "avg: " << avgColor.r << " " << avgColor.g << " " << avgColor.b << std::endl;
	////}

	////std::ifstream is(fn, std::ios::binary);
	////LinearImage linputImage = ImageDecoder::decode(is, fn);
	////uint32_t w = linputImage.getWidth();
	////uint32_t h = linputImage.getHeight();
	////uint32_t c = linputImage.getChannels();
	////auto panoImgHDR = ImageRGB32F::create(w, h);
	////panoImgHDR->setFromMemory(linputImage.getPixelRef(), w * h * c);
	////{
	////	float* data = linputImage.getPixelRef();
	////	uint32_t w = linputImage.getWidth();
	////	uint32_t h = linputImage.getHeight();
	////	uint32_t c = linputImage.getChannels();
	////	uint32_t size = w * h * c;
	////	glm::vec3 avgColor(0);
	////	for (int i = 0; i < size; i += 3)
	////	{
	////		avgColor.r += data[i];
	////		avgColor.g += data[i + 1];
	////		avgColor.b += data[i + 2];
	////	}
	////	avgColor /= (float)w * h;
	////	std::cout << "avg: " << avgColor.r << " " << avgColor.g << " " << avgColor.b << std::endl;
	////}

	//auto image = IO::ImageF32::create(1024, 1024, 3);
	//auto pixel = image->getPixel(34, 45);
	
	//auto cm = IO::CubemapF32::create(256, 3);
	//////auto cm = CubemapRGB32F::create(256);
	//cm->setFromEquirectangularMap(panoImgHDR);

	////for (int f = 0; f < 6; f++)
	////{
	////	auto face = cm->getFaceImage(CubemapRGB32F::Face(f));
	////	uint32_t faceW = face->getWidth();
	////	uint32_t faceH = face->getHeight();
	////	//std::cout << faceW << "x" << faceH << std::endl;
	////	uint32_t faceC = face->getChannels();
	////	uint32_t faceSize = faceW * faceH * faceC;
	////	float* faceData = face->getRawPtr();
	////	glm::vec3 avgColor(0);
	////	for (int y = 0; y < faceH; y++)
	////	{
	////		for (int x = 0; x < faceW; x++)
	////		{
	////			avgColor += face->getPixel(x, y);
	////			//std::cout << "face " << f << " pixel " << y * w + x << " n: " << c.x << " " << c.y << " " << c.z << std::endl;
	////		}
	////	}
	////	avgColor /= (float)faceW * faceH;
	////	std::cout << "face " << f << " avg: " << avgColor.r << " " << avgColor.g << " " << avgColor.b << std::endl;
	////}

	//auto cm2 = CubemapRGB32F::create(256);
	//cm2->mirror(cm);

	//for (int f = 0; f < 6; f++)
	//{
	//	auto face = cm2->getFaceImage(CubemapRGB32F::Face(f));
	//	uint32_t faceW = face->getWidth();
	//	uint32_t faceH = face->getHeight();
	//	//std::cout << faceW << "x" << faceH << std::endl;
	//	uint32_t faceC = face->getChannels();
	//	uint32_t faceSize = faceW * faceH * faceC;
	//	float* faceData = face->getRawPtr();
	//	glm::vec3 avgColor(0);
	//	for (int y = 0; y < faceH; y++)
	//	{
	//		for (int x = 0; x < faceW; x++)
	//		{
	//			avgColor += face->getPixel(x, y);
	//			//std::cout << "face " << f << " pixel " << y * w + x << " n: " << c.x << " " << c.y << " " << c.z << std::endl;
	//		}
	//	}
	//	avgColor /= (float)faceW * faceH;
	//	std::cout << "face " << f << " avg: " << avgColor.r << " " << avgColor.g << " " << avgColor.b << std::endl;
	//}

	//auto shProbe = cm2->computeSH9();
	//
	////for (int i = 0; i < 6; i++)
	////{
	////	auto facePosX = cm->getFaceImage(CubemapRGB32F::Face(i));
	////	IO::saveImageToFile("cm_face_" + std::to_string(i) + ".hdr", facePosX);
	////}
	//
	////IO::HDRSphere hdrSphere(assetPath + "/Footprint_Court/Footprint_Court_2k.hdr");
	////hdrSphere.convertCylindricalToSpherical(1024);
	////auto samples = IO::generateRandomSamples(128, 5454);
	////auto shProbe = hdrSphere.createEnvironmentProbe(samples);

	////for (auto coeffs : shProbe.coeffs)
	////	std::cout << coeffs.x << " " << coeffs.y << " " << coeffs.z << std::endl;
	//renderer.updateSHEnv(shProbe);

	//initScene();
	//initShadowScene();

	//scene->loadModel(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");
	//for (int j = 0; j < 5; j++)
	{
		for (int i = 0; i < names.size(); i++)
		{
			std::string name = names[i];
			std::string eName = names[i];// +"_" + std::to_string(j);
			scene->loadModelGLTF(eName, gltfPath + "/" + name + "/glTF/" + name + ".gltf");

			//auto e = scene->getRootNode(eName);
			//if (e == nullptr)
			//	continue;
			//auto renderables = e->getChildrenWithComponent<Renderable>();

			//AABB worldBox;
			//for (auto e : renderables)
			//{
			//	auto t = e->getComponent<Transform>();
			//	auto r = e->getComponent<Renderable>();
			//	glm::mat4 M = t->getTransform();
			//	AABB bbox = r->getBoundingBox();
			//	glm::vec3 minPoint = M * glm::vec4(bbox.getMinPoint(), 1.0f);
			//	glm::vec4 maxPoint = M * glm::vec4(bbox.getMaxPoint(), 1.0f);
			//	worldBox.expand(minPoint);
			//	worldBox.expand(maxPoint);
			//}
			//glm::vec3 center = worldBox.getCenter();
			//glm::vec3 size = worldBox.getSize();

			//auto t = e->getComponent<Transform>();
			//t->setLocalScale(glm::vec3(1.0f / size.x));
			//t->setLocalPosition(glm::vec3((i - 2) * 1.5f, (-center.y / size.y), 0));
		}
	}

	std::map<std::string, int> shaderCount;
	auto roots = scene->getRootEntities();
	for (auto [name, entity] : roots)
	{
		auto renderables = entity->getComponentsInChildren<Renderable>();
		for (auto r : renderables)
		{
			auto primitives = r->getMesh()->getPrimitives();
			for (auto p : primitives)
			{
				std::string shaderName = p->getMaterial()->getShader();
				if (shaderCount.find(shaderName) != shaderCount.end())
					shaderCount[shaderName]++;
				else
					shaderCount.insert(std::make_pair(shaderName, 1));
			}				
		}
	}

	//for (auto [name, count] : shaderCount)
	//	std::cout << "shader: " << name << ", materials: " << count << std::endl;
	
	//scene->addLight("light");

	scene->updateAnimations(0.0f);
		
	std::string envFn = assetPath + "/Footprint_Court/Footprint_Court_2k.hdr";
	renderer.initEnv(envFn, scene);
	renderer.initLights(scene);
	renderer.setLights(scene->numLights());
	scene->initShadowMaps();
	renderer.updateShadows(scene);

	//auto lightProbe = renderer.renderToCubemap(scene);
	//renderer.initLightProbe(lightProbe, scene);

	//std::string name = "DamagedHelmet";
	//scene->loadModelGLTF(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");
	//auto e = scene->getRootNode(name);
	//e->getComponent<Transform>()->setLocalPosition(glm::vec3(0, 2, 0));
	//e->getComponent<Transform>()->setLocalScale(glm::vec3(0.5));

	//auto mat = getDefaultMaterial();
	//mat->addProperty("material.computeFlatNormals", false);
	//mat->addProperty("material.baseColorFactor", glm::vec4(1.0f));
	//mat->addProperty("material.metallicFactor", 1.0f);
	//mat->addProperty("material.roughnessFactor", 0.3f);
	//mat->setShader("Default");

	//auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), 0.1f, 32, 32);
	//prim->setBoundingBox(glm::vec3(-0.1), glm::vec3(0.1));
	//prim->setMaterial(mat);

	//auto mesh = Mesh::create("Sphere");
	//mesh->addPrimitive(prim);

	//auto e = Entity::create("Sphere", nullptr);
	//auto r = e->addComponent<Renderable>();
	//r->setMesh(mesh);
	//e->getComponent<Transform>()->setLocalPosition(glm::vec3(0,5,0));
	//scene->addEntity("Sphere", e);
	// 
	//scene->updateAnimations(0.0f);

	initCamera(); // TODO: dont set camera if scene is empty
	setupInput();

	scene->playAnimations();
	
	return true;
}

void Application::initScene()
{
	//std::string assetPath = "../../../../assets";
	//Image2D<unsigned char> colorImage(assetPath + "/iron-rusted4/iron-rusted4_albedo.png");
	//Image2D<unsigned char> normalImage(assetPath + "/iron-rusted4/iron-rusted4_normal.png");
	//Image2D<unsigned char> roughImage(assetPath + "/iron-rusted4/iron-rusted4_roughness.png");
	//Image2D<unsigned char> metalImage(assetPath + "/iron-rusted4/iron-rusted4_metallic.png");
	//Texture2D::Ptr colorTex = colorImage.upload(true);
	//Texture2D::Ptr normalTex = normalImage.upload(false);

	//Image2D<unsigned char> ormImage(2048, 2048, 3);
	//ormImage.addChannel(1, 0, roughImage.getDataPtr(), roughImage.getChannels());
	//ormImage.addChannel(2, 0, metalImage.getDataPtr(), metalImage.getChannels());
	//Texture2D::Ptr ormTex = ormImage.upload(false);

	for (int i = 0; i <= 10; i++)
	{
		float value = (float)i / 10.0f;
		//float value2 = (float)i / 5.0f - 1.0f;

		float angle = (float)i * 36.0f;
		float x = glm::cos(glm::radians(angle));
		float y = glm::sin(glm::radians(angle));

		glm::vec3 srgb = glm::vec3(102.0f, 51.0f, 0.0f);
		glm::vec3 rgb = glm::pow(srgb / 255.0f, glm::vec3(2.2f));

		std::string name = "sphere_" + std::to_string(i);
		
		auto e = Entity::create(name, nullptr);
		auto r = e->addComponent<Renderable>();

		auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), 1.0f, 32, 32);
		prim->setBoundingBox(glm::vec3(-1), glm::vec3(1));
		
		auto mat = getDefaultMaterial();
		mat->addProperty("material.computeFlatNormals", false);
		mat->addProperty("material.baseColorFactor", glm::vec4(glm::vec3(0.2,0.5,0.3f), 1.0f));
		mat->addProperty("material.metallicFactor", 0.0f);
		mat->addProperty("material.roughnessFactor", value);
		//prim.materials[0]->addProperty("material.sheenColorFactor", glm::vec3(value));
		//prim.materials[0]->addProperty("material.sheenRoughnessFactor", 0.3f);

		//prim.materials[0]->addTexture("baseColorTex.tSampler", colorTex);
		//prim.materials[0]->addProperty("baseColorTex.use", true);
		//prim.materials[0]->addProperty("baseColorTex.uvIndex", 0); 
		//prim.materials[0]->addProperty("baseColorTex.uvTransform", glm::mat3(1.0f));
		 
		//prim.materials[0]->addTexture("normalTex.tSampler", normalTex);
		//prim.materials[0]->addProperty("normalTex.use", true);
		//prim.materials[0]->addProperty("normalTex.uvIndex", 0);
		//prim.materials[0]->addProperty("normalTex.uvTransform", glm::mat3(1.0f));
		//prim.materials[0]->addProperty("material.normalScale", 1.0f);

		//prim.materials[0]->addTexture("metalRoughTex.tSampler", ormTex);
		//prim.materials[0]->addProperty("metalRoughTex.use", true);
		//prim.materials[0]->addProperty("metalRoughTex.uvIndex", 0);
		//prim.materials[0]->addProperty("metalRoughTex.uvTransform", glm::mat3(1.0f));

		//// Translucency
		//mat->addProperty("material.translucencyFactor", 1.0f);
		//mat->addProperty("material.translucencyColorFactor", glm::vec3(1.0));

		// Transmission
		mat->addProperty("material.transmissionFactor", 1.0f);
		mat->addProperty("transmissionTex.use", false);

		// Volume
		mat->addProperty("material.thicknessFactor", 0.0f);
		mat->addProperty("material.attenuationDistance", 0.0f);
		mat->addProperty("material.attenuationColor", glm::vec3(1.0));
		mat->addProperty("thicknessTex.use", false);

		mat->setTransmissive(true);

		//prim.materials[0]->addProperty("material.clearcoatFactor", value);
		//prim.materials[0]->addProperty("material.clearcoatRoughnessFactor", 0.0f);
		//prim.materials[0]->addProperty("material.anisotropyFactor", 0.5f);
		//prim.materials[0]->addProperty("material.anisotropyDirection", glm::vec3(1, 0, 0));
		//prim.materials[0]->addProperty("anisotropyTex.use", false);
		//prim.materials[0]->addProperty("anisotropyDirectionTex.use", false);
		//prim.materials[0]->addProperty("material.iridescenceFactor", 1.0f);
		//prim.materials[0]->addProperty("material.iridescenceIOR", 1.8f);
		//prim.materials[0]->addProperty("material.iridescenceThicknessMax", 400.0f);
		//prim.materials[0]->addProperty("iridescenceTex.use", false);
		//prim.materials[0]->addProperty("iridescenceThicknessTex.use", false);
		//prim.materials[0]->setShader("Default_ANISOTROPY_IRIDESCENCE");
		//prim.materials[0]->setShader("Default_ANISOTROPY");
		mat->setShader("Default_TRANSMISSION");
		//mat->setShader("Default_TRANSLUCENCY");
		//mat->setShader("Default_TRANSMISSION_TRANSLUCENCY");
		//mat->setShader("Default");

		prim->setMaterial(mat);

		auto mesh = Mesh::create("Sphere");
		mesh->addPrimitive(prim);
		//mesh->addMaterial(mat);
		r->setMesh(mesh);
		e->getComponent<Transform>()->setLocalPosition(glm::vec3((i - 5) * 2.5, 0, 0));
		scene->addEntity(name, e);
	}

	//glm::vec3 lightColor = glm::vec3(1.0f);
	//auto lightEntity = Entity::create("light", nullptr);
	//lightEntity->addComponent(Light::create(LightType::DIRECTIONAL, lightColor, 1.0f, 2.0f));
	////lightEntity->getComponent<Transform>()->setScale(glm::vec3(0.05f));
	//scene->addEntity(lightEntity->getName(), lightEntity);
}

glm::vec2 hammersley(unsigned int i, float iN)
{
	constexpr float tof = 0.5f / 0x80000000U;
	uint32_t bits = i;
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return glm::vec2(i * iN, bits * tof);
}

void Application::initShadowScene()
{
#ifdef WITH_ASSIMP
	std::string assetPath = "../../../../assets";
	IO::AssimpImporter importer;
	auto model = importer.importModel(assetPath + "/plane.obj");
	model->getComponent<Transform>()->setLocalScale(glm::vec3(25));
	scene->addEntity("plane", model);
#else
	std::cout << "not supported file extension" << std::endl;
#endif

	//std::string name = "IridescenceSuzanne";
	//std::string gltfPath = assetPath + "/glTF-Sample-Models/2.0";
	//scene->loadModelGLTF(name, gltfPath + "/" + name + "/glTF/" + name + ".gltf");

	std::default_random_engine gen;
	std::uniform_real<float> dist;
	
	int numSamples = 10;
	float iN = 1.0f / (float)numSamples;
	for (int i = 0; i < numSamples; i++)
	{
		glm::vec2 h = hammersley(i, iN);

		float x = h.x * 10.0f - 5.0f;
		float y = 1.0f + dist(gen);
		float z = h.y * 10.0f - 5.0f;
		glm::vec3 pos(x, y, z);
		glm::quat rot(1.0f, dist(gen), dist(gen), dist(gen));
		glm::vec3 scale(0.5 + dist(gen));
		rot = glm::normalize(rot);

		auto mesh = Mesh::create("Box");
		mesh->addPrimitive(MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1)));

		auto r = Renderable::create();
		r->setMesh(mesh);
		auto e = Entity::create("Box", nullptr);
		e->addComponent<Renderable>(r);
		e->getComponent<Transform>()->setLocalPosition(pos);
		e->getComponent<Transform>()->setLocalRotation(rot);
		e->getComponent<Transform>()->setLocalScale(scale);

		scene->addEntity("box_" + std::to_string(i), e);
	}		

	scene->addLight("light");
}

void Application::initCamera()
{
	if (scene->getRootEntities().empty())
		return;

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
	center.z += dist * 1.5f;
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

	input.addKeyCallback(GLFW_KEY_M, GLFW_PRESS, std::bind(&Scene::nextMaterial, scene.get()));
	//input.addKeyCallback(GLFW_KEY_C, GLFW_PRESS, std::bind(&Renderer::nextCamera, &renderer));
	input.addKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, [&] {animate = !animate; });
	input.addKeyCallback(GLFW_KEY_R, GLFW_PRESS, std::bind(&Application::saveReflectionProbe, this));

	input.setDropCallback(std::bind(&Application::handleDrop, this, _1, _2));
}

void Application::saveReflectionProbe()
{
	std::cout << "render to cubemap " << std::endl;
	auto cm = renderer.renderToCubemap(scene);

	// TODO: save cubemap!

}

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
			scene->loadModelGLTF(name, fullPath);
		else
#ifdef WITH_ASSIMP
			scene->loadModelASSIMP(name, fullPath);
#else
			std::cout << "not supported file extension: " << ext << std::endl;
#endif
	}

	renderer.initLights(scene);
	scene->initShadowMaps();
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
			// TODO: add switch from main camera to secondary cameras
			camera.move(animTime);
			camera.rotate(animTime);
			renderer.updateCamera(camera);

			if (animate)
			{
				scene->updateAnimations(animTime);
				scene->updateAnimationState(animTime);
				renderer.initLights(scene);
				renderer.updateShadows(scene);
			}	
			animTime = 0.0f;
		}

		renderer.renderToScreen(scene);
		//renderer.renderForward(scene);
		//renderer.renderText();

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