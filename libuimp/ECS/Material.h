#ifndef INCLUDED_UNITYMATERIAL
#define INCLUDED_UNITYMATERIAL

#pragma once

#include <Types/DataTypes.h>
#include "Component.h"
#include <map>
#include <string>
#include <set>
#include <glm/glm.hpp>

namespace Unity
{
	class Property 
	{
	public:
		Property(const std::string& name) : name(name)
		{

		}
		virtual ~Property() = 0
		{}
		typedef std::shared_ptr<Property> Ptr;
		std::string getName() {
			return name;
		}
	private:
		std::string name;
	};

	template<typename Type>
	class ValueProperty : public Property
	{
	public:
		ValueProperty(const std::string& name, Type value) :
			Property(name),
			value(value)
		{

		}

		void set(Type value)
		{
			this->value = value;
		}

		Type get()
		{
			return value;
		}

		typedef std::shared_ptr<ValueProperty<Type>> Ptr;
		static Ptr create(const std::string& name, Type value)
		{
			return std::make_shared<ValueProperty<Type>>(name, value);
		}

	private:
		Type value;
	};

	class TextureProperty : public Property
	{
	public:
		TextureProperty(const std::string& name, const std::string& texGUID, glm::vec2 offset, glm::vec2 scale) :
			Property(name),
			textureGUID(texGUID),
			offset(offset),
			scale(scale)
		{

		}

		std::string getGUID()
		{
			return textureGUID;
		}

		glm::vec2 getScale()
		{
			return scale;
		}

		glm::vec2 getOffset()
		{
			return offset;
		}

		typedef std::shared_ptr<TextureProperty> Ptr;
		static Ptr create(const std::string& name, const std::string& texGUID, glm::vec2 offset, glm::vec2 scale)
		{
			return std::make_shared<TextureProperty>(name, texGUID, offset, scale);
		}

	//private:
		std::string textureGUID;
		glm::vec2 scale;
		glm::vec2 offset;
	};

	class Material : public Component
	{
	public:
		Material(const std::string& name, const std::string& shaderFile, bool internalShader);
		~Material();

		template<typename Type>
		void addProperty(std::string name, Type value)
		{
			auto prop = ValueProperty<Type>::create(name, value);
			properties.insert(std::make_pair(name, prop));
		}

		Property::Ptr getProperty(std::string name)
		{
			if (properties.find(name) != properties.end())
			{
				return properties[name];
			}
			//else
			//	std::cout << "error: could not find property " << name << " in material " << this->name << std::endl;
			return nullptr;
		}

		bool hasProperty(std::string name)
		{
			return properties.find(name) != properties.end();
		}

		template<typename Type>
		Type getValue(std::string name)
		{
			auto prop = getProperty(name);
			if (std::dynamic_pointer_cast<ValueProperty<Type>>(prop))
			{
				auto valueProp = std::dynamic_pointer_cast<ValueProperty<Type>>(prop);
				return valueProp->get();
			}
		}

		void addTexture(std::string name, TextureProperty::Ptr texProp)
		{
			textures.insert(std::make_pair(name, texProp));
		}

		void setTransform(glm::vec2 offset, glm::vec2 scale)
		{
			for (auto&& [_, texProp] : textures)
			{
				if (texProp)
				{
					texProp->offset = offset;
					texProp->scale = scale;
				}
			}
		}

		void addKeyword(std::string keyword)
		{
			keywords.insert(keyword);
		}

		bool hasKeyword(std::string keyword)
		{
			return keywords.find(keyword) != keywords.end();
		}

		TextureProperty::Ptr getTexture(std::string name)
		{
			if (textures.find(name) != textures.end())
				return textures[name];
			else
				return nullptr;
		}

		std::string getName()
		{
			return name;
		}

		std::string getShaderFile()
		{
			return shaderFile;
		}

		typedef std::shared_ptr<Material> Ptr;
		static Ptr create(const std::string& name, const std::string& shaderFile, bool internalShader)
		{
			return std::make_shared<Material>(name, shaderFile, internalShader);
		}
	private:
		std::string name;
		std::string shaderFile; // path to shader file inside Asset folder
		std::set<std::string> keywords;
		bool internalShader = false; // true if the shader used is a unity internal one
		std::map<std::string, Property::Ptr> properties;
		std::map<std::string, TextureProperty::Ptr> textures;
	};
}

#endif // INCLUDED_UNITYMATERIAL