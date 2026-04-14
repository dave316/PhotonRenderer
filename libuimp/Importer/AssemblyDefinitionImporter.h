#ifndef INCLUDED_ASSEMBLYDEFINITIONIMPORTER
#define INCLUDED_ASSEMBLYDEFINITIONIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class AssemblyDefinitionImporter : public Importer
		{
		public:
			AssemblyDefinitionImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "AssemblyDefinitionImporter";
		};
	}
}

#endif // INCLUDED_ASSEMBLYDEFINITIONIMPORTER