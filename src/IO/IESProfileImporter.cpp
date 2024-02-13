#include "IESProfileImporter.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace IO
{
	void IESProfile::load(const std::string& filename)
	{
		std::ifstream iesFile(filename);
		std::string line;

		iesFile >> formatVersion;
		while (std::getline(iesFile, line))
		{
			if (line.empty())
				continue;
			if (line[0] == '[')
			{
				int idx = line.find_first_of(']');
				int len = line.length();
				auto key = line.substr(1, idx - 1);
				auto value = line.substr(idx + 1, len - idx + 1);
				metaData.insert(std::make_pair(key, value));
			}
			else if (line.size() >= 4)
			{
				if (line.substr(0, 4).compare("TILT") == 0)
					break;
			}
		}

		iesFile >> numLamps;
		iesFile >> lumenPerLamp;
		iesFile >> multiplier;
		iesFile >> numVerticalAngles;
		iesFile >> numHorizontalAngles;

		int photoType;
		int unitType;
		iesFile >> photoType >> unitType;
		photometricType = static_cast<PhotometricType>(photoType);
		unitsType = static_cast<UnitsType>(unitType);

		iesFile >> width >> length >> height;
		iesFile >> ballastFactor >> inputWatts >> inputWatts;

		uint32 numCandelaValues = numVerticalAngles * numHorizontalAngles;
		for (int i = 0; i < numVerticalAngles; i++)
		{
			float value;
			iesFile >> value;
			verticalAngles.push_back(value);
		}
		for (int i = 0; i < numHorizontalAngles; i++)
		{
			float value;
			iesFile >> value;
			horizontalAngles.push_back(value);
		}
		for (int i = 0; i < numCandelaValues; i++)
		{
			float value;
			iesFile >> value;
			candelaValues.push_back(value);
		}
	}

	void IESProfile::print()
	{
		std::cout << "Format Version: " << formatVersion << std::endl;
		for (auto [k, v] : metaData)
			std::cout << "[" << k << "]" << v << std::endl;
		std::cout << "Number of Lamps: " << numLamps << std::endl;
		std::cout << "Lumen per Lamp: " << lumenPerLamp << std::endl;
		std::cout << "Multiplier: " << multiplier << std::endl;
		std::cout << "Vertical Angles: " << numVerticalAngles << std::endl;
		std::cout << "Horizontal Angles: " << numHorizontalAngles << std::endl;
		std::cout << "Photometric Type: " << (int)photometricType << std::endl;
		std::cout << "Units Type: " << (int)unitsType << std::endl;
		std::cout << "Luminous Opening: " << width << " " << length << " " << height << std::endl;
		std::cout << "Ballast Factor: " << ballastFactor << std::endl;
		std::cout << "Input Watts: " << inputWatts << std::endl;

		std::cout << "V. Angles: ";
		for (auto v : verticalAngles)
			std::cout << v << " ";
		std::cout << std::endl;

		std::cout << "H. Angles: ";
		for (auto v : horizontalAngles)
			std::cout << v << " ";
		std::cout << std::endl;

		std::cout << "Candela: ";
		for (auto v : candelaValues)
			std::cout << v << " ";
		std::cout << std::endl;
	}

	void IESProfile::generateSamplePoints()
	{
		float maxIntensity = *std::max_element(std::begin(candelaValues), std::end(candelaValues));
		for (auto v : candelaValues)
			candelaNormalized.push_back(v / maxIntensity);

		float lastAngle = horizontalAngles[horizontalAngles.size() - 1];
		switch (int(lastAngle))
		{
			case 90:
			{
				std::vector<float> angles;
				std::vector<float> candelas;
				for (int q = 0; q < 4; q++)
				{
					for (int i = 0; i < horizontalAngles.size(); i++)
					{
						angles.push_back(horizontalAngles[i] + q * 90.0f);
						for (int j = 0; j < verticalAngles.size(); j++)
						{
							if (q % 2 == 0)
								candelas.push_back(candelaNormalized[i * numVerticalAngles + j]);
							else 
								candelas.push_back(candelaNormalized[(numHorizontalAngles - i - 1) * numVerticalAngles + j]);
						}
					}
				}

				horizontalAngles = angles;
				candelaNormalized = candelas;
				break;
			}
			case 180:
			{
				std::vector<float> angles = horizontalAngles;
				std::vector<float> candelas = candelaNormalized;
				for (int i = 0; i < horizontalAngles.size(); i++)
				{
					angles.push_back(horizontalAngles[i] + 180.0f);
					for (int j = 0; j < verticalAngles.size(); j++)
						candelas.push_back(candelaNormalized[(numHorizontalAngles - i - 1) * numVerticalAngles + j]);
				}
					
				horizontalAngles = angles;
				candelaNormalized = candelas;
				break;
			}
		}
	}

	Mesh::Ptr IESProfile::createPhotometricNetSymmetric()
	{
		std::vector<glm::vec3> prevPositions;
		TriangleSurface surf;
		int numSegments = 16;
		for (int i = 0; i < verticalAngles.size(); i++)
		{
			float vAngle = glm::radians(verticalAngles[i]);
			float radius = candelaNormalized[i];

			std::vector<glm::vec3> positions;
			for (int j = 0; j < numSegments; j++)
			{
				float step = static_cast<float>(j) / static_cast<float>(numSegments);
				float hAngle = glm::radians(step * 360.0f);
				float x = radius * glm::sin(vAngle) * std::cos(hAngle);
				float y = radius * glm::sin(vAngle) * glm::sin(hAngle);
				float z = -radius * glm::cos(vAngle);
				positions.push_back(glm::vec3(x, y, z));
			}

			for (int j = 0; j < numSegments; j++)
			{
				Vertex v0, v1;
				v0.position = positions[j];
				if (j + 1 < numSegments)
					v1.position = positions[j + 1];
				else
					v1.position = positions[0];

				surf.addVertex(v0);
				surf.addVertex(v1);
			}

			if (!prevPositions.empty())
			{
				for (int j = 0; j < numSegments; j++)
				{
					Vertex v0, v1;
					v0.position = prevPositions[j];
					v1.position = positions[j];
					surf.addVertex(v0);
					surf.addVertex(v1);
				}
			}

			prevPositions = positions;
		}

		SubMesh m;
		m.primitive = Primitive::create("photo_net", surf, GL_LINES);
		m.material = getDefaultMaterial();
		m.material->addProperty("material.unlit", true);
		m.material->addProperty("material.baseColorFactor", glm::vec4(0, 1, 0, 1));

		auto mesh = Mesh::create("photo_net");
		mesh->addSubMesh(m);

		return mesh;
	}

	Mesh::Ptr IESProfile::createPhotometricNet()
	{
		TriangleSurface surf;
		std::vector<glm::vec3> prevPositions;
		for (int i = 0; i < horizontalAngles.size(); i++)
		{
			std::vector<glm::vec3> positions;
			float hAngle = glm::radians(horizontalAngles[i]);
			for (int j = 0; j < verticalAngles.size(); j++)
			{
				float vAngle = glm::radians(verticalAngles[j]);
				float radius = candelaNormalized[i * numVerticalAngles + j];
				float x = radius * glm::sin(vAngle) * std::sin(hAngle);
				float y = radius * glm::sin(vAngle) * glm::cos(hAngle);
				float z = -radius * glm::cos(vAngle);
				positions.push_back(glm::vec3(x, y, z));
			}

			for (int j = 0; j < positions.size(); j++)
			{
				Vertex v0, v1;
				v0.position = positions[j];
				if (j + 1 < positions.size())
					v1.position = positions[j + 1];
	
				surf.addVertex(v0);
				surf.addVertex(v1);
			}

			if (!prevPositions.empty())
			{
				for (int j = 0; j < positions.size(); j++)
				{
					Vertex v0, v1;
					v0.position = prevPositions[j];
					v1.position = positions[j];
					surf.addVertex(v0);
					surf.addVertex(v1);
				}
			}

			prevPositions = positions;
		}

		//TriangleSurface surf;
		//for (auto p : positions) 
		//{
		//	Vertex v;
		//	v.position = p;
		//	surf.addVertex(v);
		//}

		SubMesh m;
		m.primitive = Primitive::create("photo_net", surf, GL_LINES);
		m.material = getDefaultMaterial();
		m.material->addProperty("material.unlit", true);
		m.material->addProperty("material.baseColorFactor", glm::vec4(0, 1, 0, 1));

		auto mesh = Mesh::create("photo_net");
		mesh->addSubMesh(m);

		return mesh;
	}

	float* IESProfile::generateLUT(uint32 size)
	{
		bool symmetric = (horizontalAngles.size() == 1);
		float* buffer = new float[size * size];
		for(int row = 0; row < size; row++)
		{
			float stepY = static_cast<float>(row) / static_cast<float>(size);
			float hAngle = stepY * 360.0f;

			int hIndex = -1;
			for (int i = 0; i < horizontalAngles.size() - 1; i++)
			{
				if (hAngle < horizontalAngles[i + 1])
				{
					hIndex = i;
					break;
				}
			}
			hAngle = glm::radians(hAngle);

			for (int col = 0; col < size; col++)
			{
				int bufferIdx = row * size + col;
				float stepX = static_cast<float>(col) / static_cast<float>(size);
				float cosPhi = stepX * 2.0f - 1.0f;
				float vAngle = acos(cosPhi);

				int vIndex = -1;
				float vAngleDegree = glm::degrees(vAngle);
				for (int i = 0; i < verticalAngles.size() - 1; i++)
				{
					if (vAngleDegree < verticalAngles[i + 1])
					{
						vIndex = i;
						break;
					}
				}

				if (vIndex < 0)
				{
					buffer[bufferIdx] = 0.0f;
					continue;
				}

				if (symmetric)
				{
					float vAngle0 = glm::radians(verticalAngles[vIndex]);
					float vAngle1 = glm::radians(verticalAngles[vIndex + 1]);
					float c1 = candelaNormalized[vIndex];
					float c2 = candelaNormalized[vIndex + 1];

					float vWeight = (vAngle - vAngle0) / (vAngle1 - vAngle0);
					float c = glm::mix(c1, c2, vWeight);

					buffer[bufferIdx] = c;
				}
				else
				{
					float vAngle0 = glm::radians(verticalAngles[vIndex]);
					float vAngle1 = glm::radians(verticalAngles[vIndex + 1]);
					float hAngle0 = glm::radians(horizontalAngles[hIndex]);
					float hAngle1 = glm::radians(horizontalAngles[hIndex + 1]);
					float c1 = candelaNormalized[hIndex * numVerticalAngles + vIndex];
					float c2 = candelaNormalized[hIndex * numVerticalAngles + vIndex + 1];
					float c3 = candelaNormalized[(hIndex + 1) * numVerticalAngles + vIndex];
					float c4 = candelaNormalized[(hIndex + 1) * numVerticalAngles + vIndex + 1];

					float vWeight = (vAngle - vAngle0) / (vAngle1 - vAngle0);
					float hWeight = (hAngle - hAngle0) / (hAngle1 - hAngle0);
					float cv1 = glm::mix(c1, c2, vWeight);
					float cv2 = glm::mix(c3, c4, vWeight);
					float c = glm::mix(cv1, cv2, hWeight);

					buffer[bufferIdx] = c;
				}
			}
		}

		return buffer;
	}
}
