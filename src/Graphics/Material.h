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

	typedef std::shared_ptr<Property<T>> Ptr;
	static Ptr create(const std::string& name, T value, bool useTextureMap)
	{
		return std::make_shared<Property<T>>(name, value, useTextureMap);
	}
};

class Material
{
	//std::string name;
	std::vector<Texture2D::Ptr> textures;
	std::map<std::string, IProperty::Ptr> properties;
	bool useBlending = false;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
public:
	Material();
	
	template<typename T>
	void addProperty(const std::string& name, T value)
	{
		properties[name] = Property<T>::create(name, value, false);
	}

	void addTexture(const std::string& name, Texture2D::Ptr texture)
	{
		int index = (int)textures.size();
		textures.push_back(texture);
		properties[name] = Property<int>::create(name, index, true);
	}
	void setBlending(bool useBlending)
	{
		this->useBlending = useBlending;
	}
	bool blend()
	{
		return useBlending;
	}

	void setUniforms(Shader::Ptr shader);

	typedef std::shared_ptr<Material> Ptr;
	static Ptr create()
	{
		return std::make_shared<Material>();
	}
};

#endif // INCLUDED_MATERIAL