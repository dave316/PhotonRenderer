#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		void ExternalObject::fromYAML(ryml::ConstNodeRef n)
		{
			n["first"][type.key] >> type.value;
			n["first"][assembly.key] >> assembly.value;
			n["first"][name.key] >> name.value;
			n[reference.key] >> reference.value;
		}

		void ExternalObject::print(const std::string& prefix)
		{
			std::cout << "first: " << std::endl;
			std::string subPrefix = prefix + "  ";
			type.print(subPrefix);
			assembly.print(subPrefix);
			name.print(subPrefix);
			reference.print(prefix);
		}

		Importer::Importer() 
		{
		}

		void Importer::fromBaseYAML(ryml::ConstNodeRef n)
		{
			if (externalObjects.isPresent = n.has_child(externalObjects.key))
				n[externalObjects.key] >> externalObjects.value;
			if (!n[userData.key].val_is_null())
				n[userData.key] >> userData.value;
			if (!n[assetBundleName.key].val_is_null())
				n[assetBundleName.key] >> assetBundleName.value;
			if (!n[assetBundleVariant.key].val_is_null())
				n[assetBundleVariant.key] >> assetBundleVariant.value;
		}

		void Importer::print(const std::string& prefix)
		{
			std::cout << prefix << externalObjects.key << ":" << std::endl;
			for (auto& extObject : externalObjects.value)
				std::cout << prefix << "- " << extObject;
			userData.print(prefix);
			assetBundleName.print(prefix);
			assetBundleVariant.print(prefix);
		}

		bool read(ryml::ConstNodeRef n, Importer* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, ExternalObject* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}
