#ifndef INCLUDED_PREFABIMPORTER
#define INCLUDED_PREFABIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class PrefabImporter : public Importer
		{
		public:
			PrefabImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "PrefabImporter";
		};
	}
}

#endif // INCLUDED_PREFABIMPORTER