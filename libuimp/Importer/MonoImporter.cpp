#include "MonoImporter.h"

namespace Unity
{
	namespace YAML
	{
		MonoImporter::MonoImporter()
		{
		}

		void MonoImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			n[serializedVersion.key] >> serializedVersion.value;
			// TODO: each entry is a name + object reference
			//n[defaultReferences.key] >> defaultReferences.value;
			n[executionOrder.key] >> executionOrder.value;
			n[icon.key] >> icon.value;
		}

		void MonoImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			serializedVersion.print(subPrefix);
			std::cout << subPrefix << defaultReferences.key << ":" << std::endl;
			for (auto& ref : defaultReferences.value)
				std::cout << subPrefix << "- " << ref << std::endl;
			executionOrder.print(subPrefix);
			icon.print(subPrefix);
			Importer::print(subPrefix);
		}
	}
}
