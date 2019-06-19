#include "GLTFImporter.h"

#include <fstream>
#include <sstream>

namespace IO
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

	Entity::Ptr GLTFImporter::importModel(const std::string& filename)
	{
		std::string path = filename.substr(0, filename.find_last_of('/'));

		std::ifstream file(filename);
		if(!file.is_open())
		{ 
			std::cout << "error opening file " << filename << std::endl;
			return nullptr;
		}

		std::stringstream ss;
		ss << file.rdbuf();
		std::string content = ss.str();

		json::Document doc;
		doc.Parse(content.c_str());

		auto sceneNode = doc.FindMember("scenes");
		for (auto& scene : sceneNode->value.GetArray())
		{
			for (auto& nodes : scene.FindMember("nodes")->value.GetArray())
			{
				std::cout <<  "root node: " << nodes.GetInt() << std::endl;
			}
		}

		loadBuffers(doc, path);
		loadMeshes(doc);

		auto material = Material::create();
		material->setColor(glm::vec3(0.5f));

		glm::mat4 M = glm::mat4(1.0f);
		auto entity = Entity::create("blub", M);
		auto r = Renderable::create(meshes[0], material);
		entity->addComponent(r);
		return entity;
	}

	void GLTFImporter::loadBuffers(const json::Document& doc, const std::string& path)
	{
		auto buffersNode = doc.FindMember("buffers");
		for (auto& bufferNode : buffersNode->value.GetArray())
		{
			std::string uri(bufferNode.FindMember("uri")->value.GetString());
			unsigned int byteLength = bufferNode.FindMember("byteLength")->value.GetInt();
			buffer = readBinaryFile(path + "/" + uri, byteLength);
		}

		auto bufferViewsNode = doc.FindMember("bufferViews");
		for (auto& bufferViewNode : bufferViewsNode->value.GetArray())
		{
			BufferView bv;
			bv.buffer = bufferViewNode.FindMember("buffer")->value.GetInt();
			bv.byteOffset = bufferViewNode.FindMember("byteOffset")->value.GetInt();
			bv.byteLength = bufferViewNode.FindMember("byteLength")->value.GetInt();
			bv.target = bufferViewNode.FindMember("target")->value.GetInt();
			bufferViews.push_back(bv);
		}

		auto accessorsNode = doc.FindMember("accessors");
		for (auto& accessorNode : accessorsNode->value.GetArray())
		{
			Accessor accessor;
			accessor.bufferView = accessorNode.FindMember("bufferView")->value.GetInt();
			accessor.byteOffset = accessorNode.FindMember("byteOffset")->value.GetInt();
			accessor.componentType = accessorNode.FindMember("componentType")->value.GetInt();
			accessor.count = accessorNode.FindMember("count")->value.GetInt();
			accessor.type = accessorNode.FindMember("type")->value.GetString();
			accessors.push_back(accessor);
		}
	}

	void GLTFImporter::loadMeshes(const json::Document& doc)
	{
		TriangleSurface surface;
		auto meshesNode = doc.FindMember("meshes");
		for (auto& meshNode : meshesNode->value.GetArray())
		{
			auto primitvesNode = meshNode.FindMember("primitives");
			for (auto& primitiveNode : primitvesNode->value.GetArray())
			{
				auto attributesNode = primitiveNode.FindMember("attributes");
				if (attributesNode->value.HasMember("POSITION"))
				{
					auto posNode = attributesNode->value.FindMember("POSITION");
					int accIndex = posNode->value.GetInt();
					std::cout << "mesh has positions at index " << accIndex << std::endl;

					Accessor& acc = accessors[accIndex];
					BufferView& bv = bufferViews[acc.bufferView];
					int offset = bv.byteOffset + acc.byteOffset;
					std::vector<glm::vec3> positions(acc.count);
					memcpy(positions.data(), &buffer[offset], acc.count * 12);
					for (auto& p : positions)
					{
						Vertex v;
						v.position = p;
						std::cout << p.x << " " << p.y << " " << p.z << std::endl;
						surface.addVertex(v);
					}
				}

				if (attributesNode->value.HasMember("NORMAL"))
				{
					auto normalNode = attributesNode->value.FindMember("NORMAL");
					int accIndex = normalNode->value.GetInt();
					std::cout << "mesh has normals at index " << accIndex << std::endl;

					Accessor& acc = accessors[accIndex];
					BufferView& bv = bufferViews[acc.bufferView];
					int offset = bv.byteOffset + acc.byteOffset;
					std::vector<glm::vec3> normals(acc.count);
					memcpy(normals.data(), &buffer[offset], acc.count * 12);
					for (int i = 0; i < acc.count; i++)
						surface.vertices[i].normal = normals[i];
				}

				if (primitiveNode.HasMember("indices"))
				{
					auto indicesNode = primitiveNode.FindMember("indices");
					int accIndex = indicesNode->value.GetInt();
					std::cout << "mesh has indices at index " << accIndex << std::endl;

					std::vector<GLuint> indices;
					BufferView& bv_idx = bufferViews[accessors[accIndex].bufferView];
					for (int i = bv_idx.byteOffset; i < bv_idx.byteLength + bv_idx.byteOffset; i += 2)
					{
						GLushort index;
						memcpy(&index, &buffer[i], sizeof(unsigned short));
						indices.push_back(index);
					}

					for (int i = 0; i < indices.size(); i += 3)
					{
						std::cout << "tri " << (i / 3) << "[" << 
							indices[i] << "," << 
							indices[i + 1] << "," << 
							indices[i + 2] << "]" << 
							std::endl;

						Triangle t(indices[i], indices[i + 1], indices[i + 2]);
						surface.addTriangle(t);
					}
				}
			}			
		}

		std::cout << "loaded " << surface.vertices.size() << " verts, " << surface.triangles.size() << " tris " << std::endl;

		//TriangleSurface surface;
		//BufferView& bv_verts = bufferViews[accessors[1].bufferView];
		//std::vector<glm::vec3> positions(accessors[1].count);
		//memcpy(positions.data(), &buffer[bv_verts.byteOffset], bv_verts.byteLength);
		//for (auto& p : positions)
		//{
		//	Vertex v;
		//	v.position = p;
		//	surface.addVertex(v);
		//}

		//std::vector<GLuint> indices;
		//BufferView& bv_idx = bufferViews[accessors[0].bufferView];
		//for (int i = bv_idx.byteOffset; i < bv_idx.byteLength + bv_idx.byteOffset; i += 2)
		//{
		//	GLushort index;
		//	memcpy(&index, &buffer[i], sizeof(short));
		//	indices.push_back(index);
		//}

		//for (int i = 0; i < indices.size(); i += 3)
		//{
		//	Triangle t(indices[i], indices[i + 1], indices[i + 2]);
		//	surface.addTriangle(t);
		//}

		auto mesh = Mesh::create("blub", surface, 0);
		meshes.push_back(mesh);
	}
}