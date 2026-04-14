#ifndef INCLUDED_PACKAGEMANIFESTIMPORTER
#define INCLUDED_PACKAGEMANIFESTIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class PackageManifestImporter : public Importer
		{
		public:
			PackageManifestImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "PackageManifestImporter";
		};
	}
}

#endif // INCLUDED_PACKAGEMANIFESTIMPORTER