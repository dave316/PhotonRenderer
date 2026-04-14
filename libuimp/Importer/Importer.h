#ifndef INCLUDED_IMPORTER
#define INCLUDED_IMPORTER

#pragma once

#include <Types/SimpleTypes.h>

namespace Unity
{
	namespace YAML
	{
		struct ExternalObject
		{
		public:
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
			friend std::ostream& operator<< (std::ostream& stream, ExternalObject& value)
			{
				value.print("    ");
				return stream;
			}

		//private:
			YamlField<std::string> type = { "type", "" };
			YamlField<std::string> assembly = { "assembly", "" };
			YamlField<std::string> name = { "name", "" };
			YamlField<ObjectRef> reference = { "second", {} };
		};

		class Importer
		{
		public:
			Importer();
			virtual ~Importer() = 0 {};
			virtual void fromYAML(ryml::ConstNodeRef n) = 0;
			void fromBaseYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

		//protected:
			YamlField<std::vector<ExternalObject>> externalObjects = { "externalObjects", std::vector<ExternalObject>() };
			YamlField<std::string> userData = { "userData", "" };
			YamlField<std::string> assetBundleName = { "assetBundleName", "" };
			YamlField<std::string> assetBundleVariant = { "assetBundleVariant", "" };
		};

		bool read(ryml::ConstNodeRef n, Importer* value);
		bool read(ryml::ConstNodeRef n, ExternalObject* value);
	}
}

#endif // INCLUDED_IMPORTER