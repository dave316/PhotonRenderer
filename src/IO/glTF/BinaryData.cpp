#include "BinaryData.h"

#include <base64/base64.h>
#include <fstream>
#include <iostream>

namespace IO
{
	namespace glTF
	{
		std::vector<unsigned char> readBinaryFile(const std::string& filename, unsigned int byteLength)
		{
			std::vector<unsigned char> buffer;
			std::ifstream file(filename, std::ios::binary);
			if (file.is_open())
			{
				buffer.resize(byteLength);
				file.read((char*)buffer.data(), byteLength);
			}
			else
			{
				std::cout << "error opening file " << filename << std::endl;
			}
			return buffer;
		}

		void BinaryData::loadBufferFromBinary(Buffer& buffer)
		{
			buffers.push_back(buffer);
		}

		void BinaryData::loadBufferFromJson(const json::Document& doc, const std::string& path)
		{
			// TODO check if members exist....
			auto buffersNode = doc.FindMember("buffers");
			for (auto& bufferNode : buffersNode->value.GetArray())
			{
				Buffer buffer;
				if (bufferNode.HasMember("uri"))
				{
					unsigned int byteLength = bufferNode.FindMember("byteLength")->value.GetInt();
					std::string uri(bufferNode.FindMember("uri")->value.GetString());

					if (uri.find(':') != std::string::npos) // data uri
					{
						int sepIndex = uri.find_last_of(',');
						int dataStart = sepIndex + 1;
						int dataLen = uri.length() - dataStart;
						std::string dataURI = uri.substr(0, sepIndex); // TODO: check if media type is correct etc...
						//std::cout << dataURI << std::endl;

						std::string dataBase64 = uri.substr(dataStart, dataLen);
						std::string data = base64_decode(dataBase64);
						buffer.data.insert(buffer.data.end(), data.begin(), data.end());
					}
					else // file uri
					{
						buffer.data = readBinaryFile(path + "/" + uri, byteLength);
					}
					buffers.push_back(buffer);
				}				
			}
		}

		void BinaryData::loadBufferViews(const json::Document& doc)
		{
			auto bufferViewsNode = doc.FindMember("bufferViews");
			for (auto& bufferViewNode : bufferViewsNode->value.GetArray())
			{
				BufferView bufferView;
				bufferView.buffer = bufferViewNode.FindMember("buffer")->value.GetInt();
				if (bufferViewNode.HasMember("byteOffset"))
					bufferView.byteOffset = bufferViewNode.FindMember("byteOffset")->value.GetInt();
				if (bufferViewNode.HasMember("byteStride"))
					bufferView.byteStride = bufferViewNode.FindMember("byteStride")->value.GetInt();
				bufferView.byteLength = bufferViewNode.FindMember("byteLength")->value.GetInt();
				//bufferView.target = bufferViewNode.FindMember("target")->value.GetInt(); 
				bufferViews.push_back(bufferView);
			}
		}

		void BinaryData::loadAccessors(const json::Document& doc)
		{
			auto accessorsNode = doc.FindMember("accessors");
			for (auto& accessorNode : accessorsNode->value.GetArray())
			{
				Accessor accessor;
				if (accessorNode.HasMember("bufferView"))
					accessor.bufferView = accessorNode["bufferView"].GetInt();
				if (accessorNode.HasMember("byteOffset"))
					accessor.byteOffset = accessorNode["byteOffset"].GetInt();
				accessor.componentType = accessorNode.FindMember("componentType")->value.GetInt();
				if (accessorNode.HasMember("normalized"))
					accessor.normalized = accessorNode["normalized"].GetBool();
				accessor.count = accessorNode.FindMember("count")->value.GetInt();
				accessor.type = accessorNode.FindMember("type")->value.GetString();
				if (accessorNode.HasMember("min"))
					for (auto& minNode : accessorNode["min"].GetArray())
						accessor.minValues.push_back(minNode.GetFloat());
				if (accessorNode.HasMember("max"))
					for (auto& maxNode : accessorNode["max"].GetArray())
						accessor.maxValues.push_back(maxNode.GetFloat());

				if (accessorNode.HasMember("sparse"))
				{
					auto& sparseNode = accessorNode["sparse"];
					accessor.sparseCount = sparseNode["count"].GetInt();
										
					auto& indicesNode = sparseNode["indices"];
					accessor.indices.bufferView = indicesNode["bufferView"].GetInt();
					if(indicesNode.HasMember("byteOffset"))
						accessor.indices.byteOffset = indicesNode["byteOffset"].GetInt();
					accessor.indices.componentType = indicesNode["componentType"].GetInt();

					auto& valuesNode = sparseNode["values"];
					accessor.values.bufferView = valuesNode["bufferView"].GetInt();
					if (valuesNode.HasMember("byteOffset"))
						accessor.values.byteOffset = valuesNode["byteOffset"].GetInt();
				}

				accessors.push_back(accessor);
			}
		}
	}
}
