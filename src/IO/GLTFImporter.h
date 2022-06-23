#ifndef INCLUDED_GLTFIMPORTER
#define INCLUDED_GLTFIMPORTER

#pragma once

#include <Core/Entity.h>
#include <Core/Animator.h>
#include <Core/Camera.h>
#include <Core/Renderable.h>
#include <Core/Light.h>

#include <IO/glTF/GLTFAnimation.h>
#include <IO/glTF/GLTFMaterial.h>
#include <IO/glTF/GLTFMesh.h>
#include <IO/glTF/BinaryData.h>
#include <IO/glTF/TextureData.h>

#include <set>
#include <string>

namespace IO
{
	namespace glTF
	{
		struct Node
		{
			int camera = -1;
			int mesh = -1;
			int skin = -1;
			int light = -1;
			std::string name;
			std::vector<int> children;
			glm::vec3 translation = glm::vec3(0.0f);
			glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			std::vector<float> weights;
		};

		class Importer
		{
		private:
			BinaryData binData;
			ImageData imgData;
			std::vector<Node> nodes;
			std::vector<std::string> variants;
			std::set<std::string> supportedExtensions;

			std::vector<Entity::Ptr> entities;
			std::vector<Animation::Ptr> animations;
			std::vector<Mesh::Ptr> meshes;
			std::vector<Material::Ptr> materials;
			std::vector<Camera::Ptr> cameras;
			std::vector<Light::Ptr> lights;
			std::vector<Skin::Ptr> skins;

			std::string loadGLB(const std::string& filename);
			bool checkExtensions(const json::Document& doc);
			void loadExtensionData(const json::Document& doc);
			void loadCameras(const json::Document& doc);

			Entity::Ptr traverse(int nodeIndex, Entity::Ptr parent);
			Entity::Ptr loadScene(std::string name, const json::Document& doc);

			Importer(const Importer&) = delete;
			Importer& operator=(const Importer&) = delete;
		public:
			Importer();
			Entity::Ptr importModel(const std::string& filepath);
			std::map<std::string, int> getGeneralStats();
			std::map<std::string, int> getRenderingStats();
			void clear();
		};
	}
}

#endif // INCLUDED_GLTFIMPORTER