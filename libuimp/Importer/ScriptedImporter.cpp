#include "ScriptedImporter.h"

namespace Unity
{
	namespace YAML
	{
		ScriptedImporter::ScriptedImporter()
		{}
		
		void ScriptedImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void ScriptedImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
