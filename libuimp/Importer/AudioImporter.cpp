#include "AudioImporter.h"

namespace Unity
{
	namespace YAML
	{
		AudioSettings::AudioSettings() {}
		void AudioSettings::fromYAML(ryml::ConstNodeRef n)
		{
			n[loadType.key] >> loadType.value;
			n[sampleRateSetting.key] >> sampleRateSetting.value;
			n[sampleRateOverride.key] >> sampleRateOverride.value;
			n[compressionFormat.key] >> compressionFormat.value;
			n[quality.key] >> quality.value;
			n[conversionMode.key] >> conversionMode.value;
		}

		void AudioSettings::print(const std::string& prefix)
		{
			std::cout << std::endl;
			loadType.print(prefix);
			sampleRateSetting.print(prefix);
			sampleRateOverride.print(prefix);
			compressionFormat.print(prefix);
			quality.print(prefix);
			std::cout << prefix << conversionMode.key << ": " << conversionMode.value;
		}

		AudioImporter::AudioImporter()
		{
		}

		void AudioImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			n[serializedVersion.key] >> serializedVersion.value;
			n[defaultSettings.key] >> defaultSettings.value;
			n[forceToMono.key] >> forceToMono.value;
			n[normalize.key] >> normalize.value;
			n[preloadAudioData.key] >> preloadAudioData.value;
			n[loadInBackground.key] >> loadInBackground.value;
			n[ambisonic.key] >> ambisonic.value;
			n[_3D.key] >> _3D.value;
		}

		void AudioImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			serializedVersion.print(subPrefix);
			defaultSettings.print(subPrefix);
			forceToMono.print(subPrefix);
			normalize.print(subPrefix);
			preloadAudioData.print(subPrefix);
			loadInBackground.print(subPrefix);
			ambisonic.print(subPrefix);
			_3D.print(subPrefix);
			Importer::print(subPrefix);
		}

		bool read(ryml::ConstNodeRef n, AudioSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}
