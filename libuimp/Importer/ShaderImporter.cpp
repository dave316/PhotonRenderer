#include "ShaderImporter.h"

namespace Unity
{
	namespace YAML
	{
		void TextureRef::fromYAML(ryml::ConstNodeRef n)
		{
			const auto& f = n.child(0);
			c4::from_chars(f.key(), &name);
			f >> texture;
		}

		ShaderImporter::ShaderImporter()
		{
		}

		void ShaderImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			// TODO: each entry is name + instance ID
			//n[defaultTextures.key] >> defaultTextures.value;
			n[nonModifiableTextures.key] >> nonModifiableTextures.value;
		}

		void ShaderImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			std::cout << defaultTextures.key << ":" << std::endl;
			for (auto& tex : defaultTextures.value)
				std::cout << prefix << "- " << tex << std::endl;
			std::cout << nonModifiableTextures.key << ":" << std::endl;
			for (auto& tex : nonModifiableTextures.value)
				std::cout << prefix << "- " << tex << std::endl;
			Importer::print(subPrefix);
		}

		bool read(ryml::ConstNodeRef n, TextureRef* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}
