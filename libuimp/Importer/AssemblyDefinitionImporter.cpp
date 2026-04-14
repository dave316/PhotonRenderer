#include "AssemblyDefinitionImporter.h"

namespace Unity
{
	namespace YAML
	{
		AssemblyDefinitionImporter::AssemblyDefinitionImporter()
		{}
		
		void AssemblyDefinitionImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void AssemblyDefinitionImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
