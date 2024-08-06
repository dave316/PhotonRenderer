#include "Application.h"
#include <Graphics/MeshPrimitives.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <Core/ParticleSystem.h>
#include <IO/AssimpImporter.h>
#include <IO/GLTFImporter.h>
#include <IO/Image/ImageDecoder.h>
#include <IO/Image/Cubemap.h>
#include <IO/SceneExporter.h>
//#include <IO/Unity/UnityImporter.h>
#include <IO/IESProfileImporter.h>
#include <IO/ShaderLoader.h>
#include <Utils/Color.h>
#include <Utils/IBL.h>
#include <Utils/Types.h>
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

glm::vec3 sphericalToCartesian(float phi, float theta)
{
	float x = glm::cos(phi) * glm::cos(theta);
	float y = glm::sin(phi);
	float z = glm::cos(phi) * glm::sin(theta);
	return glm::vec3(x, y, z);
}

glm::vec3 computeIrradiance(std::vector<glm::vec3> sh, glm::vec3 dir)
{
	glm::vec3 irradiance = 
		sh[0] * 0.282095f

		// Band 1
		+ sh[1] * (0.488603f * dir.y)
		+ sh[2] * (0.488603f * dir.z)
		+ sh[3] * (0.488603f * dir.x)

		// Band  2
		+ sh[4] * (1.092548f * dir.x * dir.y)
		+ sh[5] * (1.092548f * dir.y * dir.z)
		+ sh[6] * (0.315392f * (3.0f * dir.z * dir.z - 1.0f))
		+ sh[7] * (1.092548f * dir.x * dir.z)
		+ sh[8] * (0.546274f * (dir.x * dir.x - dir.y * dir.y));
	return irradiance;
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

	scene = Scene::create("Scene");
	//initButterflyScene();
	std::string name = "DamagedHelmet";
	std::string fullPath = gltfPath + "/" + name + "/glTF/" + name + ".gltf";
	
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(fullPath);
	scene->addRootEntity(name, rootEntity);
	//auto root = importer.importModel(assetPath + "/metahuman/f_med_nrw_body.gltf");
	//scene->addRootEntity("metahuman", root);

	//IO::AssimpImporter importer;
	//auto root = importer.importModel(assetPath + "/hulk/hulk_tex_simplified.obj");
	//scene->addRootEntity("hulk", root);

	//for (auto r : root->getComponentsInChildren<Renderable>())
	//{
	//	auto mesh = r->getMesh();
	//	for (auto& sub : mesh->getSubMeshes())
	//	{
	//		sub.material->addProperty("material.metallicFactor", 0.0f);
	//		sub.material->addProperty("material.roughnessFactor", 0.4f);
	//	}
	//}

	//std::vector<glm::vec3> shValues;
	//shValues.push_back(glm::vec3(1));
	//{
	//	std::ifstream file(assetPath + "/sh_coeffs_49.txt");
	//	if (!file.is_open())
	//		std::cout << "error opening file" << std::endl;
	//	std::string line;
	//	while (std::getline(file, line))
	//	{
	//		std::stringstream ss(line);
	//		float value;
	//		ss >> value;
	//		shValues.push_back(glm::vec3(value));
	//	}
	//}

	//glm::vec3 maxDir;
	//float maxSum = 0;
	//for (float u = -1.0; u <= 1.0; u += 0.1)
	//{
	//	for (float v = -1.0; v <= 1.0; v += 0.1)
	//	{
	//		float theta = u * glm::pi<float>() + glm::half_pi<float>();
	//		float phi = -v * glm::half_pi<float>();
	//		glm::vec3 dir = sphericalToCartesian(phi, theta);
	//		dir = glm::normalize(dir);
	//		glm::vec3 irr = computeIrradiance(shValues, dir);
	//		float sum = irr.x + irr.y + irr.z;
	//		if (sum > maxSum)
	//		{
	//			maxSum = sum;
	//			maxDir = dir;
	//		}
	//	}
	//}

	////std::swap(maxDir.x, maxDir.z);
	////std::swap(maxDir.x, maxDir.z);
	//maxDir.x = -maxDir.x;
	//maxDir.y = -maxDir.y;

	//std::cout << "max sum: " << maxSum << std::endl;
	//std::cout << "max dir: " << maxDir.x << " " << maxDir.y << " " << maxDir.z << std::endl;

	//auto light = Light::create(LightType::POINT);

	//light->setColorTemp(5500);
	//light->setLuminousPower(100000);
	//light->setRange(1000); 

	//auto lightEntity = Entity::create("light", nullptr); 
	//auto t = lightEntity->getComponent<Transform>();
	//lightEntity->addComponent(light);
	//t->setLocalPosition(maxDir * 50.0f);
	////t->setLocalPosition(glm::vec3(-4.5f, 45.06f, -2.315f));

	//float radius = 0.1f;

	//SubMesh s;
	//s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
	//s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	//s.material = getDefaultMaterial();
	//s.material->addProperty("material.baseColorFactor", glm::vec4(light->getColor(), 1.0));
	//s.material->addProperty("material.unlit", true);

	//auto mesh = Mesh::create("Sphere");
	//mesh->addSubMesh(s);

	//auto r = lightEntity->addComponent<Renderable>();
	//r->setMesh(mesh);

	//scene->addRootEntity("sun", lightEntity);

	//std::ifstream file(assetPath + "/centers.txt");
	//std::string line;
	//int idx = 0;
	//while (std::getline(file, line))
	//{
	//	std::stringstream ss(line);
	//	glm::vec2 center;
	//	ss >> center.x >> center.y;

	//	if(center.x == 0.0f && center.y == 0.0f)
	//		continue;

	//	float u = 2.0f * static_cast<float>(center.x) / 5640.0f - 1.0f;
	//	float v = 2.0f * static_cast<float>(center.y) / 2820.0f - 1.0f;

	//	float theta = u * glm::pi<float>() + glm::half_pi<float>();
	//	float phi = -v * glm::half_pi<float>();
	//	glm::vec3 dir = sphericalToCartesian(phi, theta);
	//	dir = glm::normalize(dir);
	//
	//	float radius = 1.0f;

	//	SubMesh s;
	//	s.primitive = MeshPrimitives::createUVSphere(dir * 25.f, radius, 32, 32);
	//	s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	//	s.material = getDefaultMaterial();
	//	//s.material->addProperty("material.baseColorFactor", glm::vec4(colors[i], 1.0));
	//	s.material->addProperty("material.unlit", true);

	//	auto mesh = Mesh::create("Sphere");
	//	mesh->addSubMesh(s);

	//	std::string name = "debug_sphere_" + std::to_string(idx);
	//	auto e = Entity::create(name, nullptr);
	//	auto r = e->addComponent<Renderable>();
	//	r->setMesh(mesh);

	//	//scene->addRootEntity(name, e);

	//	idx++;
	//}

	//std::vector<glm::vec3> colors = {
	//	glm::vec3(0,0,1),
	//	glm::vec3(1,0,0),
	//	glm::vec3(1,1,0),
	//	glm::vec3(0,1,1),
	//	glm::vec3(1,0,1),
	//	glm::vec3(0,1,0),
	//};

	//glm::vec2 blue(0, 1410);
	//glm::vec2 red(1410, 1410);
	//glm::vec2 yellow(2820, 1410);
	//glm::vec2 cyan(4230, 1410);
	//glm::vec2 magenta(2820, 0);
	//glm::vec2 green(2820, 2820);
	//std::vector<glm::vec2> pixelCoords;
	//pixelCoords.push_back(blue);
	//pixelCoords.push_back(red);
	//pixelCoords.push_back(yellow);
	//pixelCoords.push_back(cyan);
	//pixelCoords.push_back(magenta);
	//pixelCoords.push_back(green); 	

	//for (int i = 0; i < 6; i++)
	//{
	//	float u = 2.0f * static_cast<float>(pixelCoords[i].x) / 5640.0f - 1.0f;
	//	float v = 2.0f * static_cast<float>(pixelCoords[i].y) / 2820.0f - 1.0f;

	//	float theta = u * glm::pi<float>() + glm::half_pi<float>();
	//	float phi = -v * glm::half_pi<float>();
	//	glm::vec3 dir = sphericalToCartesian(phi, theta);
	//	dir = glm::normalize(dir);
	//	
	//	float radius = 2.0f;

	//	SubMesh s;
	//	s.primitive = MeshPrimitives::createUVSphere(dir * 10.f, radius, 32, 32);
	//	s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	//	s.material = getDefaultMaterial();
	//	s.material->addProperty("material.baseColorFactor", glm::vec4(colors[i], 1.0));
	//	s.material->addProperty("material.unlit", true);

	//	auto mesh = Mesh::create("Sphere");
	//	mesh->addSubMesh(s);

	//	std::string name = "debug_sphere_" + std::to_string(i);
	//	auto e = Entity::create(name, nullptr);
	//	auto r = e->addComponent<Renderable>();
	//	r->setMesh(mesh);

	//	scene->addRootEntity(name, e);
	//}


	//auto root = importer.importModel(assetPath + "/metahuman/f_med_nrw_body.FBX");
	//scene->addRootEntity("metahuman", root);

	//auto colorImg = IO::decodeFromFile(assetPath + "/metahuman/BodyBaseColor.PNG");
	//auto colorTex = colorImg->upload(false);

	//auto normalImg = IO::decodeFromFile(assetPath + "/metahuman/female_body_normal_map.PNG");
	//auto normalTex = normalImg->upload(false);

	//auto roughImg = IO::decodeFromFile(assetPath + "/metahuman/female_body_roughness_map.PNG");
	//auto roughTex = roughImg->upload(false);

	//for (auto e : root->getChildrenWithComponent<Renderable>())
	//{
	//	std::cout << e->getName() << std::endl;
	//	auto name = e->getName();
	//	if (name[name.length() - 1] != '0')
	//		e->setActive(false);

	//	auto r = e->getComponent<Renderable>();
	//	auto mesh = r->getMesh();
	//	for (auto& m : mesh->getSubMeshes())
	//	{
	//		std::cout << "set mat of submesh " << m.primitive->getName() << std::endl;

	//		//m.material->setShader("Default");

	//		auto mat = getDefaultMaterial();
	//		mat->addProperty("material.metallicFactor", 0.0f);

	//		std::string texUniform = "baseColorTex";
	//		mat->addTexture(texUniform + ".tSampler", colorTex);
	//		mat->addProperty(texUniform + ".use", true);
	//		mat->addProperty(texUniform + ".uvIndex", 0);
	//		mat->addProperty(texUniform + ".uvTransform", glm::mat3(1.0f));

	//		//texUniform = "normalTex";
	//		//mat->addTexture(texUniform + ".tSampler", normalTex);
	//		//mat->addProperty(texUniform + ".use", true);
	//		//mat->addProperty(texUniform + ".uvIndex", 0);
	//		//mat->addProperty(texUniform + ".uvTransform", glm::mat3(1.0f));

	//		m.material = mat;

	//		//texUniform = "metalRoughTex";
	//		//m.material->addTexture(texUniform + ".tSampler", roughTex);
	//		//m.material->addProperty(texUniform + ".use", true);
	//		//m.material->addProperty(texUniform + ".uvIndex", 0);
	//		//m.material->addProperty(texUniform + ".uvTransform", glm::mat3(1.0f));
	//	}
	//}

	//auto updateShader = renderer.getShader("ParticleUpdate");
	//auto updateRender = renderer.getShader("ParticleRender");
	//auto ps = ParticleSystem::create(updateShader, updateRender);
	//Generator gen;
	//gen.position = glm::vec3(0.0f, 10.0f, 0.0f);
	//gen.minVelocity = glm::vec3(-1.0f, 0.0f, -1.0f);
	//gen.maxVelocity = glm::vec3(1.0f, 1.0f, 1.0f);
	//gen.gravity = glm::vec3(0.0f, -0.5f, 0.0f);
	////gen.color			= glm::vec3(0.0f, 0.5f, 1.0f);
	//gen.color = glm::vec3(1.0f);
	//gen.lifeMin = 5.0f;
	//gen.lifeMax = 10.0f;
	//gen.size = 0.05f;
	//gen.generationTime = 0.02f;
	//gen.numToGenerate = 5;
	//ps->setGenerator(gen);

	//auto psEnt = Entity::create("ParticleSystem", nullptr);
	//psEnt->addComponent(ps);
	//scene->addRootEntity("ParticleSystem", psEnt);
	
	////auto light = Light::create(LightType::SPOT);
	//auto light = Light::create(LightType::POINT);
	////auto light = Light::create(LightType::DIRECTIONAL);

	//light->setColorTemp(10000);
	//light->setLuminousPower(2500);
	////light->setLuminousIntensity(5);
	//light->setRange(20); 
	////light->setConeAngles(0.1f, 0.5f);

	//auto lightEntity = Entity::create("light", nullptr); 
	//auto t = lightEntity->getComponent<Transform>();
	//lightEntity->addComponent(light);
	////t->setLocalPosition(glm::vec3(9, 5.5, -0.25));
	////t->setLocalPosition(glm::vec3(6.5f, 2.0f, 0.0f));
	//t->setLocalPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	////t->setLocalPosition(glm::vec3(-4.0f, 2.0f, 0.0f));
	////t->setLocalRotation(glm::angleAxis(glm::radians(90 0f), glm::vec3(-1, 0, 0)));
	////t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)) * glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)));
	////t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0)));

	//float radius = 0.1f;

	//SubMesh s;
	//s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
	//s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	//s.material = getDefaultMaterial();
	//s.material->addProperty("material.baseColorFactor", glm::vec4(light->getColor(), 1.0));
	//s.material->addProperty("material.unlit", true);

	//auto mesh = Mesh::create("Sphere");
	//mesh->addSubMesh(s);

	//auto r = lightEntity->addComponent<Renderable>();
	//r->setMesh(mesh);

	//scene->addRootEntity("sun", lightEntity);

	//glm::vec3 positions[8] = {
	//	glm::vec3(-9,1.5,-3.5),
	//	glm::vec3(-9,1.5,3.5),
	//	glm::vec3(9,1.5,-3.5),
	//	glm::vec3(9,1.5,3.5),
	//	glm::vec3(-9,4,-3.5),
	//	glm::vec3(-9,4,3.5),
	//	glm::vec3(9,4,-3.5),
	//	glm::vec3(9,4,3.5),
	//};


	//for (int i = 0; i < 4; i++)
	//{
	//	auto light = Light::create(LightType::POINT);
	//	//auto light = Light::create(LightType::DIRECTIONAL);

	//	light->setColorTemp(3000);
	//	light->setLuminousPower(500);
	//	//light->setLuminousIntensity(10);
	//	light->setRange(10);
	//	//light->setConeAngles(0.1f, 0.5f);

	//	auto lightEntity = Entity::create("light", nullptr);
	//	auto t = lightEntity->getComponent<Transform>();
	//	lightEntity->addComponent(light);
	//	t->setLocalPosition(positions[i]);

	//	float radius = 0.1f;

	//	SubMesh s;
	//	s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
	//	s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
	//	s.material = getDefaultMaterial();
	//	s.material->addProperty("material.baseColorFactor", glm::vec4(light->getColor(), 1.0));
	//	s.material->addProperty("material.unlit", true);

	//	auto mesh = Mesh::create("Sphere");
	//	mesh->addSubMesh(s);

	//	auto r = lightEntity->addComponent<Renderable>();
	//	r->setMesh(mesh);

	//	scene->addRootEntity("light" + std::to_string(i), lightEntity);
	//}

	//{
	//	SubMesh s;
	//	s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), 0.25f, 64, 64);
	//	auto surf = s.primitive->getSurface();
	//	Box bbox;
	//	for (auto& v : surf.vertices)
	//		bbox.expand(v.position);

	//	renderer.setVolumeBox(bbox.getMinPoint(), bbox.getMaxPoint());

	//	s.material = getDefaultMaterial();
	//	s.material->addProperty("material.metallicFactor", 0.0f);
	//	s.material->addProperty("material.roughnessFactor", 1.0f);

	//	auto mesh = Mesh::create("Box");
	//	mesh->addSubMesh(s);

	//	auto e = Entity::create("Box", nullptr);
	//	auto t = e->getComponent<Transform>();
	//	auto r = e->addComponent<Renderable>();
	//	r->setMesh(mesh);

	//	t->setLocalPosition(glm::vec3(-5.5f, 2.0f, 0.0f));
	//	t->setLocalScale(glm::vec3(1.0f));

	//	scene->addRootEntity("box", e);
	//}

	renderer.setTonemappingOp(0);
	renderer.setIBL(true);
	renderer.setBloom(false);

	//std::string treePath = "C:/workspace/code/DeepLens/teaser/CristTreePeter3";
	//IO::AssimpImporter importer;
	//auto model = importer.importModel(treePath + "/CristTreePeter3.fbx");

	//for (auto rendNode : model->getChildrenWithComponent<Renderable>())
	//{
	//	std::cout << rendNode->getName() << std::endl;
	//	auto subMeshes = rendNode->getComponent<Renderable>()->getMesh()->getSubMeshes();
	//	for (auto& m : subMeshes)
	//		std::cout << "submesh: " << m.primitive->getName() << std::endl;
	//}

	//model->getComponent<Transform>()->setLocalScale(glm::vec3(0.01));
	//scene->addRootEntity("tree", model);

	//initIESTestScene();
	//initLightTestScene();

	//Light::lightForward = glm::vec3(0, 0, 1);

	//std::string unityAssetPath = "C:/workspace/code/Archviz/Assets";
	//std::string unityPrefabPath = unityAssetPath + "/ArchVizPRO Interior Vol.6/3D PREFAB";
	//std::string unitySceneFile = "ArchVizPRO Interior Vol.6/3D Scene/AVP6_Desktop.unity";

	////std::string unityAssetPath = "C:/workspace/code/VikingVillage/Assets";
	////std::string unityPrefabPath = unityAssetPath + "/Viking Village/Prefabs";
	////std::string unitySceneFile = "Viking Village/Scenes/The_Viking_Village.unity";

	////std::string unityAssetPath = "C:/workspace/code/Christmas/Assets";
	////std::string unityPrefabPath = unityAssetPath + "/Christmas pack/Prefabs";
	//
	//IO::Unity::Importer sceneImporter(8192);
	//scene = sceneImporter.loadScene(unityAssetPath, unitySceneFile);
	////sceneImporter.loadMetadata(unityAssetPath);
	////auto prefab = sceneImporter.loadPrefab(unityPrefabPath + "/TreeToy_Ball.prefab");
	////scene->addRootEntity("TreeBall", prefab);
	////scene->updateAnimations(0.0f);
	//sceneImporter.clearCache();

	//for (auto [name, entity] : scene->getRootEntities())
	//{
	//	auto models = entity->getChildrenWithComponent<Renderable>();
	//	for (auto m : models)
	//	{
	//		auto r = m->getComponent<Renderable>();
	//		if (name.compare("3D FX") == 0 && m->getName().compare("Sphere001") == 0)
	//		{
	//			r->setType(RenderType::OPAQUE);
	//			r->setPriority(1);
	//		}
	//		if (name.compare("3D HOUSE") == 0)
	//		{
	//			auto modelName = m->getName();
	//			if (modelName.length() == 16)
	//			{
	//				auto prefix = m->getName().substr(0, 14);
	//				if (prefix.compare("Glass_Exterior") == 0)
	//				{
	//					r->setType(RenderType::OPAQUE);
	//					r->setPriority(2);
	//				}
	//			}
	//			else if (modelName.length() == 23)
	//			{
	//				auto prefix = modelName.substr(0, 21);
	//				if (prefix.compare("Window_Glass_Interior") == 0)
	//				{
	//					r->setType(RenderType::OPAQUE);
	//					r->setPriority(2);
	//				}
	//			}
	//		}
	//	}
	//}

	std::string envFn = assetPath + "/Footprint_Court/Footprint_Court_2k.hdr";
	//std::string envFn = assetPath + "/led0.hdr";
	//std::string envFn = assetPath + "/pano.jpg";
	//std::string envFn = "C:/workspace/code/VikingVillage/Assets/Viking Village/Textures/Skies/Daytime/SunsetSkyboxHDR.hdr";
	auto panoImg = IO::decodeHDRFromFile(envFn, true);
	//auto panoImg = IO::decodeFromFile(envFn, true);

	Skybox skybox;
	skybox.texture = panoImg->upload(false);
	skybox.rotation = 0.0f;
	skybox.exposure = 1.0f;
	scene->setSkybox(skybox);

	renderer.updateCamera(camera);
	renderer.prepare(scene);

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

void Application::initButterflyScene()
{
	std::string assetPath = "C:/workspace/code/PhotonRenderer/assets/nature/models/animated-flying-fluttering-butterfly-loop";
	std::string name = "Butterfly";
	
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(assetPath + "/source/Fluttering butterfly.glb");
	scene->addRootEntity(name, rootEntity);

	auto img = IO::decodeFromFile(assetPath + "/gltf_embedded_0_blue.png", true);
	auto tex = img->upload(true);

	auto imgEmissive = IO::decodeFromFile(assetPath + "/luminosity.png", true);
	auto texEmissive = imgEmissive->upload(true);
		
	auto r = rootEntity->getChildrenWithComponent<Renderable>()[0]->getComponent<Renderable>();
	auto& sub = r->getMesh()->getSubMeshes()[0];
	//auto oldTex = sub.material->getTexture("material.baseColorTex");
	sub.material->replaceTexture("diffuseTex", tex);

	std::string texInfoStr = "emissiveTex";
	sub.material->addTexture(texInfoStr, texEmissive);
	sub.material->addProperty(texInfoStr + ".use", true);
	sub.material->addProperty(texInfoStr + ".uvIndex", 0);
	sub.material->addProperty(texInfoStr + ".uvTransform", glm::mat3(1.0));

	sub.material->addProperty("material.emissiveFactor", glm::vec3(1));
	sub.material->addProperty("material.emissiveStrength", 5.0f);

	scene->playAnimations();

	//oldTex = img->upload(true);
}

void Application::initLightTestScene()
{
	std::string assetPath = "../../../../assets";
	IO::AssimpImporter importer;
	auto model = importer.importModel(assetPath + "/plane.obj");
	model->getComponent<Transform>()->setLocalScale(glm::vec3(100));
	scene->addRootEntity("plane", model);

	std::vector<int> temps = { 1500, 3000, 6500, 9000, 12000 };
	float x = -20.0f;
	int index = 0;
	for (int T : temps)
	{
		{
			Light::Ptr light = nullptr; 
			if (index == 0)
				light = Light::create(LightType::SPOT);
			else if (index == 1)
			{
				light = Light::create(LightType::AREA);
				light->setShape(LightShape::SPHERE);
				light->setRadius(0.5f);
			}
			else if (index == 2)
			{
				light = Light::create(LightType::AREA);
				light->setShape(LightShape::DISC);
				light->setRadius(1.0f);
			}
			else if (index == 3)
			{
				light = Light::create(LightType::AREA);
				light->setShape(LightShape::TUBE);
				light->setWidth(3.0f);
				light->setRadius(0.5f);
			}
			else if (index == 4)
			{
				light = Light::create(LightType::AREA);
				light->setShape(LightShape::RECTANGLE);
				light->setWidth(3.0f);
				light->setHeight(2.0f);
			}
				
			light->setColorTemp(T);
			light->setLuminousPower(1200);
			light->setRange(100);

			auto lightEntity = Entity::create("light", nullptr);
			auto t = lightEntity->getComponent<Transform>();
			lightEntity->addComponent(light);
			t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)));
			t->setLocalPosition(glm::vec3(x, 5, -1));

			float radius = 0.1f;

			SubMesh s;
			s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
			s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
			s.material = getDefaultMaterial();
			s.material->addProperty("material.baseColorFactor", glm::vec4(light->getColor(), 1.0));
			s.material->addProperty("material.unlit", true);

			auto mesh = Mesh::create("Sphere");
			mesh->addSubMesh(s);

			auto r = lightEntity->addComponent<Renderable>();
			r->setMesh(mesh);

			scene->addRootEntity("light_" + std::to_string(index) + "_" + std::to_string(T) + "K", lightEntity);
		}

		{
			SubMesh s;
			s.primitive = MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1));
			s.material = getDefaultMaterial();
			s.material->addProperty("material.metallicFactor", 0.0f);
			s.material->addProperty("material.roughnessFactor", 1.0f);

			auto mesh = Mesh::create("Box");
			mesh->addSubMesh(s);

			auto e = Entity::create("Box", nullptr);
			auto t = e->getComponent<Transform>();
			auto r = e->addComponent<Renderable>();
			r->setMesh(mesh);

			t->setLocalPosition(glm::vec3(x, 0.5f, -5.0f));
			t->setLocalScale(glm::vec3(1.0f));

			scene->addRootEntity("box_" + std::to_string(index), e);
		}

		index++;
		x += 10.0f;
	}
	{
		SubMesh s;
		s.primitive = MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1));
		s.material = getDefaultMaterial();
		s.material->addProperty("material.metallicFactor", 0.0f);
		s.material->addProperty("material.roughnessFactor", 1.0f);

		auto mesh = Mesh::create("Box");
		mesh->addSubMesh(s);

		auto e = Entity::create("Box", nullptr);
		auto t = e->getComponent<Transform>();
		auto r = e->addComponent<Renderable>();
		r->setMesh(mesh);

		t->setLocalPosition(glm::vec3(0.0f, 0.0f, 50.0f));
		t->setLocalScale(glm::vec3(100.0f));

		scene->addRootEntity("box", e);
	}
}

void Application::initIESTestScene()
{
	renderer.setBloom(false);

	std::string iesPath = "C:/workspace/code/ies/library";
	auto iesFilelist = IO::getAllFileNames(iesPath);
	int numProfiles = iesFilelist.size();

	uint32 size = 256;
	float* buffer = new float[numProfiles * size * size];
	for (int i = 0; i < numProfiles; i++)
	{
		auto iesFn = iesFilelist[i];
		//std::cout << iesFn << std::endl;

		IO::IESProfile iesProfile;
		iesProfile.load(iesPath + "/" + iesFn);
		iesProfile.generateSamplePoints();

		float* iesLUT = iesProfile.generateLUT(size);
		uint32 offset = i * size * size;
		std::memcpy(buffer + offset, iesLUT, size * size * sizeof(float));
		delete[] iesLUT;
	}

	auto iesProfileTex = Texture2DArray::create(size, size, numProfiles, GL::R32F);
	iesProfileTex->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	iesProfileTex->upload(buffer);
	delete[] buffer;
	scene->setIESProfile(iesProfileTex);

	//auto mesh = iesProfile.createPhotometricNet();
	//auto e = Entity::create("blub", nullptr);
	//auto r = e->addComponent<Renderable>();
	//r->setMesh(mesh);
	//scene->addEntity("IES_Light", e);
	//return;

	std::vector<int> temps = { 1500, 3000, 6500, 9000, 12000 };
	float x = -4.0f;
	int index = 0;
	for (int T : temps)
	{
		{
			auto light = Light::create(LightType::POINT);
			light->setColorTemp(T);
			light->setLuminousPower(125.0f);
			light->setRange(5.0f);
			light->setIESProfile(index);

			auto lightEntity = Entity::create("light", nullptr);
			auto t = lightEntity->getComponent<Transform>();
			lightEntity->addComponent(light);
			t->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)));
			t->setLocalPosition(glm::vec3(x, 1, -0.05));

			float radius = 0.01f;

			SubMesh s;
			s.primitive = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
			s.primitive->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
			s.material = getDefaultMaterial();
			s.material->addProperty("material.baseColorFactor", glm::vec4(light->getColor(), 1.0));
			s.material->addProperty("material.unlit", true);

			auto mesh = Mesh::create("Sphere");
			mesh->addSubMesh(s);

			auto r = lightEntity->addComponent<Renderable>();
			r->setMesh(mesh);

			scene->addRootEntity("light_" + std::to_string(T) + "K", lightEntity);
		}

		{
			SubMesh s;
			s.primitive = MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1));
			s.material = getDefaultMaterial();
			s.material->addProperty("material.metallicFactor", 0.0f);
			s.material->addProperty("material.roughnessFactor", 1.0f);

			auto mesh = Mesh::create("Box");
			mesh->addSubMesh(s);

			auto e = Entity::create("Box", nullptr);
			auto t = e->getComponent<Transform>();
			auto r = e->addComponent<Renderable>();
			r->setMesh(mesh);

			t->setLocalPosition(glm::vec3(x, 0.05f, -1.0f));
			t->setLocalScale(glm::vec3(0.1f));

			scene->addRootEntity("box_" + std::to_string(index), e);
		}

		index++;
		x += 2.0f;
	}

#ifdef WITH_ASSIMP
	std::string assetPath = "../../../../assets";
	IO::AssimpImporter importer;
	auto model = importer.importModel(assetPath + "/plane.obj");
	model->getComponent<Transform>()->setLocalScale(glm::vec3(10));
	scene->addRootEntity("plane", model);
#else
	std::cout << "not supported file extension" << std::endl;
#endif

	{
		SubMesh s;
		s.primitive = MeshPrimitives::createBox(glm::vec3(0), glm::vec3(1));
		s.material = getDefaultMaterial();
		s.material->addProperty("material.metallicFactor", 0.0f);
		s.material->addProperty("material.roughnessFactor", 1.0f);

		auto mesh = Mesh::create("Box");
		mesh->addSubMesh(s);

		auto e = Entity::create("Box", nullptr);
		auto t = e->getComponent<Transform>();
		auto r = e->addComponent<Renderable>();
		r->setMesh(mesh);

		t->setLocalPosition(glm::vec3(0.0f, 0.0f, 5.0f));
		t->setLocalScale(glm::vec3(10.0f));

		scene->addRootEntity("box", e);
	}
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

bool Application::loadModelASSIMP(std::string name, std::string path)
{
	IO::AssimpImporter importer;
	auto model = importer.importModel(path);
	if (model == nullptr)
		return false;
	scene->addRootEntity(name, model);
	return true;
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

				for (auto [_, root] : scene->getRootEntities())
				{
					for (auto ps : root->getComponentsInChildren<ParticleSystem>())
					{
						ps->setVP(camera.getViewProjectionMatrix());
						ps->createBillboard(camera.getForward(), camera.getUp());
						ps->updateTransformFeedback(animTime);
					}
				}

				//GLuint timerQuery;
				//glGenQueries(1, &timerQuery);
				//glBeginQuery(GL_TIME_ELAPSED, timerQuery);
				renderer.initLights(scene, camera);
				renderer.updateShadows(scene);
				//renderer.updateVolumes(scene);
				//glEndQuery(GL_TIME_ELAPSED);

				//int done = 0;
				//while (!done)
				//	glGetQueryObjectiv(timerQuery, GL_QUERY_RESULT_AVAILABLE, &done);

				//GLuint64 elapsedTime;
				//glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsedTime);
				//std::cout << "shadowmaps update time: " << (float)elapsedTime / 1000000.0 << " ms" << std::endl;
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