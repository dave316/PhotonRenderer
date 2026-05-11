#ifndef INCLUDED_UNITYTESTIMPORTER
#define INCLUDED_UNITYTESTIMPORTER

#pragma once

#include <UnityImporter.h>
#include <Core/Renderable.h>
#include <Core/Scene.h>
#include <Core/Light.h>


struct UniformProperty
{
	std::string name;
	std::string type;
	float defaultValue;
};

class UnityTestImporter
{
public:
	UnityTestImporter();
	~UnityTestImporter();

	void addTexture(pr::Material::Ptr material, Unity::TextureProperty::Ptr textureProp, float defaultValue = 0.0f, bool isMainTex = false);
	pr::Material::Ptr loadStandardMaterial(Unity::Material::Ptr unityMaterial);
	pr::Material::Ptr loadSpecGlossMaterial(Unity::Material::Ptr unityMaterial);
	pr::Material::Ptr loadCustomMaterial(Unity::Material::Ptr unityMaterial, std::string shaderPath);
	pr::Material::Ptr loadShadergraphMaterial(Unity::Material::Ptr unityMaterial, std::string shaderPath);
	bool loadUniformsFromJSON(std::string jsonFileContent, std::vector<UniformProperty>& uniforms);
	bool loadUniformsFromJSONSerialized(std::string jsonFileContent, std::vector<UniformProperty>& uniforms);
	pr::Texture2D::Ptr loadTextureGUID(Unity::YAML::Metadata metadata);
	pr::Material::Ptr loadMaterial(Unity::Material::Ptr unityMaterial);
	pr::Entity::Ptr traverse(Unity::GameObject::Ptr gameObject, pr::Entity::Ptr parent);
	pr::Scene::Ptr importScene(const std::string& assetPath, const std::string& sceneFile);
	pr::Entity::Ptr importPrefab(const std::string& assetPath, const std::string& prefabFile);

	void loadLightMaps(pr::Scene::Ptr scene);
	void loadDirectionMaps(pr::Scene::Ptr scene);
	void getLightProbes(pr::Scene::Ptr scene);

private:
	UnityTestImporter(const UnityTestImporter&) = delete;
	UnityTestImporter& operator=(const UnityTestImporter&) = delete;

	Unity::Importer importer;
	std::map<std::string, pr::Texture2D::Ptr> textureCache;
};

#endif // INCLUDED_UNITYTESTIMPORTER