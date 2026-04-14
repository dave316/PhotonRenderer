#include "NativeFormatImporter.h"

namespace Unity
{
	namespace YAML
	{
		NativeFormatImporter::NativeFormatImporter()
		{
		}

		void NativeFormatImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			if (mainObjectFileID.isPresent = n.has_child(mainObjectFileID.key))
				n[mainObjectFileID.key] >> mainObjectFileID.value;
		}

		void NativeFormatImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			if (mainObjectFileID.isPresent)
				mainObjectFileID.print(subPrefix);
			Importer::print(subPrefix);
		}
	}
}
