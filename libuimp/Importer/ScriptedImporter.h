#ifndef INCLUDED_SCRIPTEDIMPORTER
#define INCLUDED_SCRIPTEDIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		// TODO: add script and name table
		class ScriptedImporter : public Importer
		{
		public:
			ScriptedImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "ScriptedImporter";
		};
	}
}

#endif // INCLUDED_SCRIPTEDIMPORTER