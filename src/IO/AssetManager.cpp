#include "AssetManager.h"
#include "GLTFImporter.h"
#include "ImageLoader.h"

namespace IO
{
	void AssetManager::init(std::string assetFolder)
	{
		entries.clear();
		entries.push_back(fs::directory_entry(assetFolder));
		for (auto& file : fs::recursive_directory_iterator(assetFolder))
			entries.push_back(file);

		nodes.resize(entries.size());

		root = buildFileTreeRecursive(0, 0);
	}

	void AssetManager::destroy()
	{
		for (auto root : entities)
			root->clearParent();
		entities.clear();
		primitives.clear();
		materials.clear();
		textures.clear();
	}	

	void AssetManager::addFile(FileNode::Ptr dstNode, std::string srcFilenpath)
	{	
		auto dstPath = fs::path(dstNode->fullpath);
		auto srcPath = fs::path(srcFilenpath);

		if (fs::is_regular_file(srcPath))
			fs::copy(srcPath, dstPath);
		else
		{
			std::cout << "error: " << srcFilenpath << " is not a regular file!" << std::endl;
			return;
		}

		int newIndex = entries.size();
		auto entry = fs::directory_entry(dstNode->fullpath + '\\' + srcPath.filename().string());
		entries.push_back(entry);
		
		auto node = FileNode::create();
		node->filename = entry.path().filename().string();
		node->fullpath = entry.path().string();
		node->extension = entry.path().extension().string();
		node->isDirectory = entry.is_directory();
		node->nodeIndex = newIndex;
		node->depth = dstNode->depth + 1;
		nodes.push_back(node);
		dstNode->children.push_back(node);
	}

	void AssetManager::addDirectory(FileNode::Ptr dstNode, std::string srcDirPath)
	{		
		auto srcPath = fs::path(srcDirPath);
		if (fs::is_directory(srcDirPath))
		{
			auto dirName = srcPath.filename().string();
			auto dstDirPath = dstNode->fullpath + "\\" + dirName;
			auto dstPath = fs::path(dstDirPath);

			// TODO: check if directory exists
			fs::create_directory(dstPath);
			fs::copy(srcPath, dstPath, fs::copy_options::recursive);

			// add new file entries
			uint32 rootIndex = entries.size();
			entries.push_back(fs::directory_entry(dstDirPath));
			for (auto& file : fs::recursive_directory_iterator(dstDirPath))
				entries.push_back(file);

			nodes.resize(entries.size());

			dstNode->children.push_back(buildFileTreeRecursive(rootIndex, dstNode->depth + 1));
		}
		else
		{
			std::cout << "error: " << srcDirPath << " is not a directory!" << std::endl;
			return;
		}
	}

	void AssetManager::makeDirectory(FileNode::Ptr dstNode, std::string dirName)
	{

	}

	void AssetManager::renameDirectory(FileNode::Ptr node, std::string newName)
	{

	}

	void AssetManager::loadAssetsFromDisk()
	{
		assetsLoaded = false;
		loadAssetsRecursive(root);
		assetsLoaded = true;

		if (assetLoadThread.joinable())
			assetLoadThread.detach();
	}

	void AssetManager::copyAssetsToGPU()
	{
		for (auto [_, prim] : primitives)
		{
			prim->createData();
			prim->uploadData();
		}

		// TODO: texture can be referenced by multiple materials...
		// ... so they should be only initialized once otherwise their reference becomes invalid!
		for (auto [_, mat] : materials)
		{
			for (auto tex : mat->getTextures())
			{
				if (!tex->isLoadedOnGPU())
				{
					tex->createData();
					tex->uploadData();
					tex->generateMipmaps();
					tex->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);
					tex->setAddressMode(GPU::AddressMode::Repeat);
				}
				else
				{
					std::cout << "texture already loaded on GPU!" << std::endl;
				}
			}
		}
	}

	void AssetManager::loadAssetsAsync()
	{
		assetLoadThread = std::thread(&AssetManager::loadAssetsFromDisk, this);
	}

	void AssetManager::printTree(FileNode::Ptr node)
	{

	}

	pr::Entity::Ptr AssetManager::getEntity(int index)
	{
		if (index < entities.size())
			return entities[index];
		return nullptr;
	}

	pr::Texture2D::Ptr AssetManager::getTexture(int index)
	{
		if (index < textures.size())
			return textures[index];
		return nullptr;
	}

	pr::Primitive::Ptr AssetManager::getPrimitive(int index)
	{
		if (primitives.find(index) != primitives.end())
			return primitives[index];
		return nullptr;
	}

	pr::Material::Ptr AssetManager::getMaterial(int index)
	{
		if (materials.find(index) != materials.end())
			return materials[index];
		return nullptr;
	}

	FileNode::Ptr AssetManager::getRoot()
	{
		return nodes[0];
	}

	FileNode::Ptr AssetManager::getNode(uint32 index)
	{
		return nodes[index];
	}

	FileNode::Ptr AssetManager::getNodeFromPath(std::string assetPath)
	{
		std::string fullPath = root->fullpath + "\\" + assetPath;

		for (auto& node : nodes)
		{
			if (fullPath.compare(node->fullpath) == 0)
				return node;
		}

		std::cout << "error: cannot find asset with path " << fullPath << std::endl;
		return nodes[0];
	}

	void AssetManager::loadAssetsRecursive(FileNode::Ptr node)
	{
		auto filepath = fs::path(node->fullpath);
		auto ext = filepath.extension().string();

		if (ext.compare(".gltf") == 0 || ext.compare(".glb") == 0)
		{
			std::cout << "loading model " << filepath.filename().string() << std::endl;
			node->entityIndex = static_cast<int>(entities.size());
			IO::glTF::Importer importer;
			auto root = importer.importModel(node->fullpath);
			root->setURI(node->relativePath);

			for (auto r : root->getComponentsInChildren<pr::Renderable>())
			{
				auto mesh = r->getMesh();
				for (auto& subMesh : mesh->getSubMeshes())
				{
					auto prim = subMesh.primitive;
					auto mat = subMesh.material;
					primitives.insert(std::make_pair(prim->getID(), prim));
					materials.insert(std::make_pair(mat->getID(), mat));

					for (auto mat : subMesh.variants)
					{
						if (materials.find(mat->getID()) != materials.end())
						{
							// TODO: material has already been added
						}
						else
						{
							materials.insert(make_pair(mat->getID(), mat));
						}
					}
				}
			}

			entities.push_back(root);
		}
		//else if (ext.compare(".png") == 0 || ext.compare(".jpg") == 0)
		//{
		//	auto img = IO::ImageLoader::loadFromFile(node->fullpath);

		//	uint32 width = img->getWidth();
		//	uint32 height = img->getHeight();
		//	uint8* data = img->getRawPtr();
		//	uint32 dataSize = width * height * 4;

		//	GPU::ImageUsage flags = GPU::ImageUsage::TransferSrc | GPU::ImageUsage::TransferDst | GPU::ImageUsage::Sampled;
		//	GPU::Format format = GPU::Format::RGBA8;
		//	//if (name.compare("baseColorTex") == 0 ||
		//	//	name.compare("emissiveTex") == 0)
		//	//	format = GPU::Format::SRGB8;

		//	auto texture = pr::Texture2D::create(width, height, format);
		//	texture->createData();
		//	texture->upload(data, dataSize);
		//	texture->uploadData();
		//	//texture->setFilter(GPU::Filter::Linear, GPU::Filter::Linear);
		//	//texture->setAddressMode(GPU::AddressMode::Repeat);

		//	node->texIndex = static_cast<int>(textures.size());
		//	textures.push_back(texture);
		//}

		for (auto childIdx : node->children)
			loadAssetsRecursive(childIdx);
	}

	FileNode::Ptr AssetManager::buildFileTreeRecursive(uint32 index, uint32 depth)
	{
		auto entry = entries[index];

		auto node = FileNode::create();
		node->filename = entry.path().filename().string();
		node->fullpath = entry.path().string();
		node->extension = entry.path().extension().string();
		node->isDirectory = entry.is_directory();
		node->nodeIndex = index;
		node->depth = depth;
		
		std::string rootPath = entries[0].path().string();
		int rootPathLen = rootPath.length();
		int relPathLen = node->fullpath.length() - rootPathLen;
		std::string relPath = node->fullpath.substr(rootPathLen, relPathLen);
		relPath.erase(relPath.begin());
		//std::replace(relPath.begin(), relPath.end(), '\\', '/');
		std::cout << "rel path: " << relPath << std::endl;
		node->relativePath = relPath;

		nodes[index] = node;

		for (uint32 i = 0; i < depth; i++)
			std::cout << "-";
		std::cout << node->fullpath << std::endl;

		depth++;

		for (int i = 0; i < entries.size(); i++)
		{
			auto fullpath = entries[i].path().string();
			int n = static_cast<int>(std::count(fullpath.begin(), fullpath.end(), '\\'));

			if (fullpath.compare(0, node->fullpath.size() + 1, node->fullpath + '\\') == 0)
				if (n == depth)
					node->children.push_back(buildFileTreeRecursive(i, depth));
		}
		
		return node;
	}
}