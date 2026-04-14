#ifndef INCLUDED_DEFAULTIMPORTER
#define INCLUDED_DEFAULTIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class DefaultImporter : public Importer
		{
		public:
			DefaultImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "DefaultImporter";
		};
	}
}

#endif // INCLUDED_DEFAULTIMPORTER