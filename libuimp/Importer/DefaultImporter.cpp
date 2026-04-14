#include "DefaultImporter.h"

namespace Unity
{
	namespace YAML
	{
		DefaultImporter::DefaultImporter() 
		{}
		
		void DefaultImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void DefaultImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
