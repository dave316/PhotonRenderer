#ifndef INCLUDED_TEXTSCRIPTIMPORTER
#define INCLUDED_TEXTSCRIPTIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class TextScriptImporter : public Importer
		{
		public:
			TextScriptImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "TextScriptImporter";
		};
	}
}

#endif // INCLUDED_TEXTSCRIPTIMPORTER