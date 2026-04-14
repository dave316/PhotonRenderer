#include "PackageManifestImporter.h"

namespace Unity
{
	namespace YAML
	{
		PackageManifestImporter::PackageManifestImporter()
		{}
		
		void PackageManifestImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void PackageManifestImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
