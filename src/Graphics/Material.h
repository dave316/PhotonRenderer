#ifndef INCLUDED_MATERIAL
#define INCLUDED_MATERIAL

#pragma once

#include "Texture.h"
#include "Shader.h"

#include <vector>

#include <glm/glm.hpp>

class IProperty
{
protected:
	std::string name;
	bool useTextureMap;
public:
	IProperty(const std::string& name, bool useTextureMap) :
		name(name),
		useTextureMap(useTextureMap)
	{

	}
	virtual void setUniform(Shader::Ptr shader) = 0;
	bool isTexture() { return useTextureMap; }
	typedef std::shared_ptr<IProperty> Ptr;
};

template<typename T>
class Property : public IProperty
{
private:
	T value;
public:
	Property(const std::string& name, T value, bool useTextureMap) :
		IProperty(name, useTextureMap),
		value(value)
	{

	}

	void setUniform(Shader::Ptr shader)
	{
		shader->setUniform(name, value);
	}

	T getValue()
	{
		return value;
	}

	typedef std::shared_ptr<Property<T>> Ptr;
	static Ptr create(const std::string& name, T value, bool useTextureMap)
	{
		return std::make_shared<Property<T>>(name, value, useTextureMap);
	}
};

class Material
{
	std::string shaderName;
	std::vector<Texture2D::Ptr> textures;
	std::map<std::string, Texture2D::Ptr> texNames;
	std::map<std::string, IProperty::Ptr> properties;
	bool blending = false;
	bool doubleSided = false;
	bool transmissive = false;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
public:
	Material();
	
	template<typename T>
	void addProperty(const std::string& name, T value)
	{
		properties[name] = Property<T>::create(name, value, false);
	}

	template<typename T>
	T getPropertyValue(const std::string& name)
	{
		auto prop = std::dynamic_pointer_cast<Property<T>>(properties[name]);
		return prop->getValue();
	}

	void addTexture(const std::string& texName, Texture2D::Ptr texture)
	{
		int index = (int)textures.size();
		textures.push_back(texture);
		texNames.insert(std::make_pair(texName, texture));

		std::string uniform = texName + ".tSampler";
		properties[uniform] = Property<int>::create(uniform, index, true);
	}
	void setDoubleSided(bool doubleSided)
	{
		this->doubleSided = doubleSided;
	}
	void setBlending(bool blending)
	{
		this->blending = blending;
	}
	void setTransmissive(bool transmissive)
	{
		this->transmissive = transmissive;
	}
	bool isDoubleSided()
	{
		return doubleSided;
	}
	bool isTransmissive()
	{
		return transmissive;
	}
	bool useBlending()
	{
		return blending;
	}

	int getUsedTexUnits()
	{
		return textures.size();
	}

	void setUniforms(Shader::Ptr shader);

	void setShader(std::string name)
	{
		this->shaderName = name;
	}

	std::string getShader()
	{
		return shaderName;
	}

	Texture2D::Ptr getTexture(std::string name)
	{
		return texNames[name];
	}

	typedef std::shared_ptr<Material> Ptr;
	static Ptr create()
	{
		return std::make_shared<Material>();
	}
};

Material::Ptr getDefaultMaterial();

#endif // INCLUDED_MATERIAL