#ifndef INCLUDED_UNITYYAML
#define INCLUDED_UNITYYAML

#pragma once

#include <Importer/Importer.h>
#include <Types/ObjectTypes.h>

namespace Unity
{
	namespace YAML
	{
		struct Metadata
		{
			uint32 fileFormat = 0;
			uint32 timeCreated = 0;
			std::string licenseType = "";
			std::string guid = "";
			std::string filepath = "";
			std::string filename = "";
			std::string extension = "";
			bool isFolder = false;
			Importer* importer = nullptr;
		};
		typedef std::map<std::string, Metadata> Metadatabase;

		std::string loadTxtFile(const std::string& fileName);
		void loadMetadata(const std::string& assetPath, std::map<std::string, Metadata>& metaData);
		bool readSceneFile(const std::string& sceneFullpath, std::map<int64, Object*>& objects);
	}
}

#endif // INCLUDED_UNITYYAML