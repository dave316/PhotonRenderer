#ifndef INCLUDED_MATERIAL
#define INCLUDED_MATERIAL

#pragma once

#include "Texture.h"
#include <Core/Component.h>

namespace pr
{
	struct TextureInfo
	{
		int samplerIndex = -1;
		uint32 uvIndex = 0;
		float defaultValue = 0.0f;
		float rotation = 0.0f;
		glm::vec2 offset = glm::vec2(0);
		glm::vec2 scale = glm::vec2(1);
		bool isMainTexture = false;
	};

	struct BinaryBuffer
	{
		std::vector<uint8> data;
		void write(float value)
		{
			uint32 b = *(uint32*)&value;
			data.push_back(b & 0xFF);
			data.push_back((b >> 8) & 0xFF);
			data.push_back((b >> 16) & 0xFF);
			data.push_back((b >> 24) & 0xFF);
		}

		void write(int value)
		{
			data.push_back(value & 0xFF);
			data.push_back((value >> 8) & 0xFF);
			data.push_back((value >> 16) & 0xFF);
			data.push_back((value >> 24) & 0xFF);
		}

		void write(uint32 value)
		{
			data.push_back(value & 0xFF);
			data.push_back((value >> 8) & 0xFF);
			data.push_back((value >> 16) & 0xFF);
			data.push_back((value >> 24) & 0xFF);
		}

		void write(glm::vec2 value)
		{
			write(value.x);
			write(value.y);
		}

		void write(glm::vec4 value)
		{
			write(value.x);
			write(value.y);
			write(value.z);
			write(value.w);
		}

		void write(glm::mat4 value)
		{
			// TODO: check if column or row major!
			for (int row = 0; row < 4; row++)
				for (int col = 0; col < 4; col++)
					write(value[row][col]);
		}

		void write(TextureInfo& info)
		{
			write(info.samplerIndex);
			write(info.uvIndex);
			write(info.defaultValue);
			write(0); // padding
			//write(info.uvTransform);

			glm::vec2 offset = info.offset;
			float rotation = info.rotation;
			glm::vec2 scale = info.scale;

			glm::mat3 T(1.0f);
			T[2][0] = offset.x;
			T[2][1] = offset.y;

			glm::mat3 S(1.0f);
			S[0][0] = scale.x;
			S[1][1] = scale.y;

			glm::mat3 R(1.0f);
			R[0][0] = glm::cos(rotation);
			R[1][0] = glm::sin(rotation);
			R[0][1] = -glm::sin(rotation);
			R[1][1] = glm::cos(rotation);

			glm::mat4 uvTransform = glm::mat3(T * R * S);
			if (GraphicsContext::getInstance().getCurrentAPI() == GraphicsAPI::Direct3D11)
				uvTransform = glm::transpose(uvTransform);
			write(uvTransform);
		}
	};

	class Property
	{
	public:
		Property(std::string name) : name(name)	{}
		virtual void write(BinaryBuffer& buffer) = 0;
		typedef std::shared_ptr<Property> Ptr;
		std::string getName() { return name; }
	protected:
		std::string name;
	};

	template<typename Type>
	class ValueProperty : public Property
	{
	public:
		ValueProperty(std::string name, Type value) :
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

		void write(BinaryBuffer& buffer)
		{
			buffer.write(value);
		}

		typedef std::shared_ptr<ValueProperty<Type>> Ptr;
		static Ptr create(const std::string& name, Type value)
		{
			return std::make_shared<ValueProperty<Type>>(name, value);
		}

	private:
		Type value;
	};

	class Material : public Component
	{
	public:
		Material(std::string name, std::string shaderName) : 
			name(name),
			shaderName(shaderName)
		{
			matID = matCount;
			matCount++;
		}
		~Material() {}

		void addProperty(Property::Ptr prop)
		{
			propertyMap.insert(std::make_pair(prop->getName(), prop));
			properties.push_back(prop);
		}

		template<typename Type>
		void addProperty(std::string name, Type value)
		{
			auto prop = ValueProperty<Type>::create(name, value);
			propertyMap.insert(std::make_pair(name, prop));
			properties.push_back(prop);
		}

		void addProperty(std::string name, Property::Ptr prop)
		{
			propertyMap.insert(std::make_pair(name, prop));
			properties.push_back(prop);
		}

		template<typename Type>
		void setProperty(std::string name, Type value)
		{
			if (propertyMap.find(name) != propertyMap.end())
			{
				auto prop = std::dynamic_pointer_cast<ValueProperty<Type>>(propertyMap[name]);
				prop->set(value);
			}				
			else
				std::cout << "error: could not find property " << name << " in material " << this->name << std::endl;
		}

		Property::Ptr getProperty(std::string name) 
		{
			if (propertyMap.find(name) != propertyMap.end())
			{
				return propertyMap[name];
			}
			else
				std::cout << "error: could not find property " << name << " in material " << this->name << std::endl;
			return nullptr;
		}

		void setTextureOffset(std::string name, glm::vec2 offset)
		{
			if (textureMap.find(name) != textureMap.end())
			{
				texInfos[textureMap[name]].offset = offset;
			}
			else
			{
				std::cout << "error: material texture " << name << " not found!" << std::endl;
			}
		}

		void setTextureScale(std::string name, glm::vec2 scale)
		{
			if (textureMap.find(name) != textureMap.end())
			{
				texInfos[textureMap[name]].scale = scale;
			}
			else
			{
				std::cout << "error: material texture " << name << " not found!" << std::endl;
			}
		}

		void setTextureRotation(std::string name, float rotation)
		{
			if (textureMap.find(name) != textureMap.end())
			{
				texInfos[textureMap[name]].rotation = rotation;
			}
			else
			{
				std::cout << "error: material texture " << name << " not found!" << std::endl;
			}
		}

		void addTexture(std::string name, pr::Texture2D::Ptr texture, pr::TextureInfo info = pr::TextureInfo())
		{
			textureMap.insert(std::make_pair(name, (int)texInfos.size()));

			if (texture)
			{
				info.samplerIndex = static_cast<int>(textures.size());
				textures.push_back(texture);

				if (info.isMainTexture)
				{
					mainTexture = texture;
					mainTexInfo = info;
				}					
			}
			
			texInfos.push_back(info);
		}

		void setTexture(std::string name, pr::Texture2D::Ptr texture, pr::TextureInfo info = pr::TextureInfo())
		{
			auto& texInfo = texInfos[textureMap[name]];
			if (texInfo.samplerIndex == -1)
			{
				uint32 id = static_cast<int>(textures.size());
				texInfo.samplerIndex = id;
				textures.push_back(texture);
			}
			else
			{
				textures[texInfo.samplerIndex] = texture;
			}
		}

		pr::TextureInfo getTexInfo(std::string name)
		{
			if (textureMap.find(name) != textureMap.end())
			{
				return texInfos[textureMap[name]];
			}
			else
			{
				std::cout << "error: material texture " << name << " not found!" << std::endl;
			}
			return pr::TextureInfo();
		}

		pr::Texture2D::Ptr getTexture(std::string name)
		{
			if (textureMap.find(name) != textureMap.end())
			{
				auto texIndex = texInfos[textureMap[name]].samplerIndex;
				if (texIndex >= 0)
					return textures[texIndex];
				else
					return nullptr;
			}
			else
			{
				std::cout << "error: material texture " << name << " not found!" << std::endl;
			}
			return nullptr;
		}

		void update(GPU::DescriptorPool::Ptr descriptorPool)
		{
			{
				BinaryBuffer buffer;
				for (auto& prop : properties)
					prop->write(buffer);
				for (auto& texInfo : texInfos)
					buffer.write(texInfo);

				auto& ctx = GraphicsContext::getInstance();
				auto size = static_cast<uint32>(buffer.data.size());
				mainMaterialUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, size, 0);
				mainMaterialUBO->uploadMapped(buffer.data.data());

				mainDS = descriptorPool->createDescriptorSet("Material", (uint32)textures.size());
				mainDS->addDescriptor(mainMaterialUBO->getDescriptor());
				for (auto tex : textures)
					mainDS->addDescriptor(tex->getDescriptor());
				mainDS->updateVariable();
			}

			{
				BinaryBuffer buffer;
				buffer.write(mainColor);
				buffer.write(alphaMode);
				buffer.write(alphaCutOff);
				buffer.write(0);
				buffer.write(0);
				buffer.write(mainTexInfo);

				auto& ctx = GraphicsContext::getInstance();
				auto size = static_cast<uint32>(buffer.data.size());
				shadowMaterialUBO = ctx.createBuffer(GPU::BufferUsage::TransferDst | GPU::BufferUsage::UniformBuffer, size, 0);
				shadowMaterialUBO->uploadMapped(buffer.data.data());

				shadowDS = descriptorPool->createDescriptorSet("MaterialShadow", 1);
				shadowDS->addDescriptor(shadowMaterialUBO->getDescriptor());
				if (mainTexture)
					shadowDS->addDescriptor(mainTexture->getDescriptor());
				shadowDS->updateVariable();
			}
		}

		void update()
		{
			if (mainMaterialUBO)
			{
				BinaryBuffer buffer;
				for (auto& prop : properties)
					prop->write(buffer);
				for (auto& texInfo : texInfos)
					buffer.write(texInfo);

				mainMaterialUBO->uploadMapped(buffer.data.data());
			}
		}

		void bindMainMat(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
		{
			cmdBuffer->bindDescriptorSets(pipeline, mainDS, 4);
		}

		void bindShadowMat(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
		{
			cmdBuffer->bindDescriptorSets(pipeline, shadowDS, 5);
		}

		void setTransmissive(bool transmissive)
		{
			this->transmissive = transmissive;
		}

		bool isTransmissive()
		{
			return transmissive;
		}

		void setTransparent(bool transparent)
		{
			this->transparent = transparent;
		}

		bool isTransparent()
		{
			return transparent;
		}

		void setDoubleSided(bool doubleSided)
		{
			this->doubleSided = doubleSided;
		}

		bool isDoubleSided()
		{
			return doubleSided;
		}

		void setShaderName(std::string shaderName)
		{
			this->shaderName = shaderName;
		}

		std::string getName()
		{
			return name;
		}

		std::string getShaderName()
		{
			return shaderName;
		}

		std::vector<Property::Ptr> getProperties()
		{
			return properties;
		}

		std::vector<TextureInfo> getTextureInfos()
		{
			return texInfos;
		}

		std::vector<pr::Texture2D::Ptr> getTextures()
		{
			return textures;
		}

		std::map<std::string, int> getTextureMap()
		{
			return textureMap;
		}

		typedef std::shared_ptr<Material> Ptr;
		static Ptr create(std::string name, std::string shaderName)
		{
			return std::make_shared<Material>(name, shaderName);
		}

		static uint32 matCount;
		uint32 getID()
		{
			return matID;
		}

		void setAlphaMode(uint32 mode, float cutOff)
		{
			alphaMode = mode;
			alphaCutOff = cutOff;
		}

		void setMainColor(glm::vec4 color)
		{
			mainColor = color;
		}
		
	private:
		uint32 matID;
		std::string name;
		std::string shaderName;

		uint32 alphaMode = 0;
		float alphaCutOff = 0.5f;
		glm::vec4 mainColor = glm::vec4(1);
		Texture2D::Ptr mainTexture = nullptr;
		TextureInfo mainTexInfo;
		
		bool doubleSided = false; // render back & front faces
		bool transparent = false; // alpha blending
		bool transmissive = false; // transmission (refraction)
		GPU::DescriptorSet::Ptr mainDS = nullptr;
		GPU::DescriptorSet::Ptr shadowDS = nullptr;
		GPU::Buffer::Ptr mainMaterialUBO; // main material containing all properties and textures
		GPU::Buffer::Ptr shadowMaterialUBO; // shadow material only containing main color/texture and alpha mode
		std::map<std::string, Property::Ptr> propertyMap;
		std::vector<Property::Ptr> properties;
		std::vector<TextureInfo> texInfos;
		std::vector<pr::Texture2D::Ptr> textures;
		std::map<std::string, int> textureMap;

		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;
	};
}

#endif // INCLUDED_MATERIAL