#include "PrefabImporter.h"

namespace Unity
{
	namespace YAML
	{
		PrefabImporter::PrefabImporter()
		{}
		
		void PrefabImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void PrefabImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
