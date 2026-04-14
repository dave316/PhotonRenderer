#ifndef INCLUDED_MONOIMPORTER
#define INCLUDED_MONOIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class MonoImporter : public Importer
		{
		public:
			MonoImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "MonoImporter";
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<std::vector<ObjectRef>> defaultReferences = { "defaultReferences", std::vector<ObjectRef>() };
			YamlField<uint32> executionOrder = { "executionOrder", 0 };
			YamlField<InstanceID> icon = { "icon", 0 };
		};
	}
}

#endif // INCLUDED_MONOIMPORTER