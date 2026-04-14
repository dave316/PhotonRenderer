#ifndef INCLUDED_AUDIOIMPORTER
#define INCLUDED_AUDIOIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		struct AudioSettings
		{
		public:
			AudioSettings();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, AudioSettings& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<uint32> loadType = { "loadType", 0 };
			YamlField<uint32> sampleRateSetting = { "sampleRateSetting", 0 };
			YamlField<uint32> sampleRateOverride = { "sampleRateOverride", 0 };
			YamlField<uint32> compressionFormat = { "compressionFormat", 0 };
			YamlField<uint32> quality = { "quality", 0 };
			YamlField<uint32> conversionMode = { "conversionMode", 0 };
		};

		class AudioImporter : public Importer
		{
		public:
			AudioImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "AudioImporter";
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<AudioSettings> defaultSettings = { "defaultSettings", {} };
			// TODO: add platformSettingOverrides (need to figure out the type)
			YamlField<bool> forceToMono = { "forceToMono", false };
			YamlField<bool> normalize = { "normalize", false };
			YamlField<bool> preloadAudioData = { "preloadAudioData", false };
			YamlField<bool> loadInBackground = { "loadInBackground", false };
			YamlField<bool> ambisonic = { "ambisonic", false };
			YamlField<bool> _3D = { "3D", false };
		};

		bool read(ryml::ConstNodeRef n, AudioSettings* value);
	}
}

#endif // INCLUDED_AUDIOIMPORTER