#ifndef INCLUDED_NATIVEFORMATIMPORTER
#define INCLUDED_NATIVEFORMATIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class NativeFormatImporter : public Importer
		{
		public:
			NativeFormatImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "NativeFormatImporter";
			YamlField<uint32> mainObjectFileID = { "mainObjectFileID", 0 };
		};
	}
}

#endif // INCLUDED_NATIVEFORMATIMPORTER