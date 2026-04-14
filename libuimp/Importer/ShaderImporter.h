#ifndef INCLUDED_SHADERIMPORTER
#define INCLUDED_SHADERIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		class TextureRef
		{
		public:
			void fromYAML(ryml::ConstNodeRef n);			
			friend std::ostream& operator<< (std::ostream& stream, const TextureRef& value)
			{
				stream << value.name << ": {";
				stream << "fileID: " << value.texture.fileID << ", ";
				stream << "guid: " << value.texture.guid << ", ";
				stream << "type: " << value.texture.type;
				stream << "}";
				return stream;
			}

		private:
			std::string name = "";
			ObjectRef texture;
		};

		class ShaderImporter : public Importer
		{
		public:
			ShaderImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		private:
			const std::string name = "ShaderImporter";
			YamlField<std::vector<TextureRef>> defaultTextures = { "defaultTextures", std::vector<TextureRef>() };
			YamlField<std::vector<TextureRef>> nonModifiableTextures = { "nonModifiableTextures", std::vector<TextureRef>() };
		};

		bool read(ryml::ConstNodeRef n, TextureRef* value);
	}
}

#endif // INCLUDED_SHADERIMPORTER