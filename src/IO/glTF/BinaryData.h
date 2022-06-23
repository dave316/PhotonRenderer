#ifndef INCLUDED_BINARYDATA
#define INCLUDED_BINARYDATA

#pragma once

#include <GL/GLBuffer.h>
#include <rapidjson/document.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace json = rapidjson;
namespace IO
{
	namespace glTF
	{
		struct Buffer
		{
			std::vector<unsigned char> data;
		};

		struct BufferView
		{
			int buffer;
			int byteOffset = 0;
			int byteStride = 0;
			int byteLength;
			int target;
		};

		struct SparseIndices
		{
			int bufferView = 0;
			int byteOffset = 0;
			int componentType;
		};

		struct SparseValues
		{
			int bufferView = 0;
			int byteOffset = 0;
		};
		
		struct Accessor
		{
			int bufferView = 0;
			int byteOffset = 0;
			int componentType;
			int count;
			bool normalized = false;
			std::string type;
			std::vector<float> minValues;
			std::vector<float> maxValues;

			int sparseCount = 0;
			SparseIndices indices;
			SparseValues values;
		};

		class BinaryData
		{
		public:
			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Accessor> accessors;

		public:
			void loadBufferFromBinary(Buffer& buffer);
			void loadBufferFromJson(const json::Document& doc, const std::string& path);
			void loadBufferViews(const json::Document& doc);
			void loadAccessors(const json::Document& doc);

			template<typename T>
			void loadData(int accIndex, std::vector<T>& data)
			{
				Accessor& acc = accessors[accIndex];
				BufferView& bv = bufferViews[acc.bufferView];
				Buffer& buffer = buffers[bv.buffer];
				int offset = bv.byteOffset + acc.byteOffset;
				data.resize(acc.count);

				if (bv.byteStride > 0)
				{
					for (int i = 0; i < data.size(); i++)
					{
						memcpy(&data[i], &buffer.data[offset], sizeof(T));
						offset += bv.byteStride;
					}
				}
				else
				{
					memcpy(data.data(), &buffer.data[offset], acc.count * sizeof(T));
				}
			}

			template<typename T>
			void loadSparseData(int accIndex, std::vector<GLuint>& indices, std::vector<T>& values)
			{
				Accessor& acc = accessors[accIndex];
				BufferView& bvIdx = bufferViews[acc.indices.bufferView];
				Buffer& bufferIdx = buffers[bvIdx.buffer];
				int offsetIdx = bvIdx.byteOffset + acc.indices.byteOffset;
				int type = acc.indices.componentType;
				switch (type)
				{
				case GL_UNSIGNED_BYTE:
				{
					std::vector<GLubyte> byteIndices(acc.sparseCount);
					memcpy(byteIndices.data(), &bufferIdx.data[offsetIdx], acc.sparseCount * sizeof(GLubyte));
					for (auto i : byteIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_SHORT:
				{
					std::vector<GLushort> shortIndices(acc.sparseCount);
					memcpy(shortIndices.data(), &bufferIdx.data[offsetIdx], acc.sparseCount * sizeof(GLushort));
					for (auto i : shortIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_INT:
					indices.resize(acc.sparseCount);
					memcpy(indices.data(), &bufferIdx.data[offsetIdx], acc.sparseCount * sizeof(GLuint));
					break;
				default:
					std::cout << "index type not supported!!!" << std::endl;
					break;
				}

				BufferView& bvValues = bufferViews[acc.values.bufferView];
				Buffer& bufferValues = buffers[bvValues.buffer];
				int offsetValues = bvValues.byteOffset + acc.values.byteOffset;
				values.resize(acc.sparseCount);
				memcpy(values.data(), &bufferValues.data[offsetValues], acc.sparseCount * sizeof(T));
			}

			template<int n>
			void loadAttribute(int accIndex, std::vector<glm::vec<n, float, glm::packed_highp>>& buffer)
			{
				typedef glm::vec<n, float, glm::packed_highp> outVec;
				int type = accessors[accIndex].componentType;
				bool normalized = accessors[accIndex].normalized;
				switch (type)
				{
				case GL_BYTE:
				{
					std::vector<glm::vec<n, glm::i8, glm::packed_highp>> positionsInt8;
					loadData(accIndex, positionsInt8);
					for (auto c : positionsInt8)
					{
						if (normalized)
							buffer.push_back(glm::max(outVec(c) / 127.0f, -1.0f));
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_UNSIGNED_BYTE:
				{
					std::vector<glm::vec<n, glm::u8, glm::packed_highp>> positionsUInt8;
					loadData(accIndex, positionsUInt8);
					for (auto c : positionsUInt8)
					{
						if (normalized)
							buffer.push_back(outVec(c) / 255.0f);
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_SHORT:
				{
					std::vector<glm::vec<n, glm::i16, glm::packed_highp>> positionsInt16;
					loadData(accIndex, positionsInt16);
					for (auto c : positionsInt16)
					{
						if (normalized)
							buffer.push_back(glm::max(outVec(c) / 32767.0f, -1.0f));
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_UNSIGNED_SHORT:
				{
					std::vector<glm::vec<n, glm::u16, glm::packed_highp>> positionsUInt16;
					loadData(accIndex, positionsUInt16);
					for (auto c : positionsUInt16)
					{
						if (normalized)
							buffer.push_back(outVec(c) / 65535.0f);
						else
							buffer.push_back(c);
					}
					break;
				}
				case GL_FLOAT:
				{
					loadData(accIndex, buffer);
					break;
				}
				default:
					std::cout << "component type " << type << " not supported!" << std::endl;
					break;
				}
			}

			void loadIndices(int accIndex, std::vector<GLuint>& indices)
			{
				int type = accessors[accIndex].componentType;
				switch (type)
				{
				case GL_UNSIGNED_BYTE:
				{
					std::vector<GLubyte> byteIndices;
					loadData(accIndex, byteIndices);
					for (auto i : byteIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_SHORT:
				{
					std::vector<GLushort> shortIndices;
					loadData(accIndex, shortIndices);
					for (auto i : shortIndices)
						indices.push_back(i);
					break;
				}
				case GL_UNSIGNED_INT:
					loadData(accIndex, indices);
					break;
				default:
					std::cout << "index type not supported!!!" << std::endl;
					break;
				}
			}

			Accessor getAccessor(int index)
			{
				return accessors[index];
			}
		};
	}
}

#endif // INCLUDED_BINARYDATA