#include "TextScriptImporter.h"

namespace Unity
{
	namespace YAML
	{
		TextScriptImporter::TextScriptImporter()
		{
		}

		void TextScriptImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
		}

		void TextScriptImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			Importer::print(prefix + "  ");
		}
	}
}
