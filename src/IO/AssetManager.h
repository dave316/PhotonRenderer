#ifndef INCLUDED_ASSETMANAGER
#define INCLUDED_ASSETMANAGER

#pragma once

#include <Core/Entity.h>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

namespace IO
{
	struct FileNode
	{
		typedef std::shared_ptr<FileNode> Ptr;
		std::string filename;
		std::string relativePath;
		std::string fullpath;
		std::string extension;
		int depth = -1;
		int nodeIndex = -1;
		int entityIndex = -1;
		int texIndex = -1;
		bool isDirectory = false;
		std::vector<FileNode::Ptr> children;
		static Ptr create()
		{
			return std::make_shared<FileNode>();
		}
	};

	class AssetManager
	{
	public:	
		void init(std::string assetFolder);
		void destroy();
		void addFile(FileNode::Ptr dstNode, std::string srcFilePath); // adds external file to Assets
		void addDirectory(FileNode::Ptr dstNode, std::string srcDirPath); // adds directory to Assets (recursive)
		void makeDirectory(FileNode::Ptr dstNode, std::string dirName);
		void renameDirectory(FileNode::Ptr node, std::string newName);
		void loadAssetsFromDisk();
		void copyAssetsToGPU();
		void loadAssetsAsync();
		void printTree(FileNode::Ptr node);
		bool assetsReady() { return assetsLoaded; }

		FileNode::Ptr getRoot();
		FileNode::Ptr getNode(uint32 index);
		FileNode::Ptr getNodeFromPath(std::string assetPath);

		pr::Entity::Ptr getEntity(int index);
		pr::Texture2D::Ptr getTexture(int index);
		pr::Primitive::Ptr getPrimitive(int index);
		pr::Material::Ptr getMaterial(int index);
	private:
		FileNode::Ptr root;
		std::vector<FileNode::Ptr> nodes;
		std::vector<fs::directory_entry> entries;

		std::vector<pr::Entity::Ptr> entities;
		std::vector<pr::Texture2D::Ptr> textures;
		std::map<uint32, pr::Primitive::Ptr> primitives;
		std::map<uint32, pr::Material::Ptr> materials;

		std::thread assetLoadThread;
		bool assetsLoaded = false;

		void loadAssetsRecursive(FileNode::Ptr node);
		FileNode::Ptr buildFileTreeRecursive(uint32 index, uint32 depth);
	};
}

#endif // INCLUDED_ASSETMANAGER