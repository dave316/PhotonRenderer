#include "LightData.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

namespace IO
{
	namespace Unity
	{

		int countTabs(std::string line)
		{
			int numTabs = 0;
			for (int i = 0; i < line.length(); i++)
			{
				if (line[i] == '\t')
					numTabs++;
				else
					break;
			}
			return numTabs;
		}

		void LightData::loadAsset(const std::string& filename)
		{
			std::ifstream file(filename);
			std::stringstream ss;
			if (file.is_open())
				ss << file.rdbuf();
			else
				std::cout << "could not open file " << std::endl;

			std::vector<LightMapData> lightMapData;
			std::vector<LightMapDataID> lightMapDataIDs;
			std::map<int, std::string> fileID2GUID;
			std::vector<int> lMaps;
			std::vector<int> dMaps;
			std::vector<int> reflProbes;
			std::vector<int64_t> reflectionObjects;

			std::string line;
			int count = 0;
			while (std::getline(ss, line))
			{
				int numTabs = countTabs(line);
				if (numTabs == 0)
				{
					if (!line.empty())
					{
						if (line.substr(0, 4).compare("path") == 0)
						{
							std::stringstream lineSS(line);
							std::string path, guid, empty;
							lineSS >> path >> empty;
							lineSS >> empty >> guid;

							int i0 = path.find('(') + 1;
							int i1 = path.find(')');
							int len = i1 - i0;
							int fileID = std::stoll(path.substr(i0, len));

							fileID2GUID.insert(std::make_pair(fileID, guid));
						}
					}
				}
				else if (numTabs == 1)
				{
					std::string key;
					std::stringstream lineSS(line);
					lineSS >> key;
					//std::cout << key << std::endl;

					if (key.compare("m_Data") == 0)
					{
						int count = 0;
						while (std::getline(ss, line))
						{
							std::string key;
							std::stringstream lineSS(line);
							lineSS >> key;
							int numTabs = countTabs(line);
							if (numTabs == 2)
							{
								//std::cout << key << std::endl;
								if (key.compare("m_Tetrahedralization") == 0)
								{
									std::getline(ss, line);

									std::string key, value, type;
									std::getline(ss, line);
									std::stringstream lineSS(line);
									lineSS >> key >> value >> type;
									int size = std::stoi(value);
									//std::cout << "number of entries: " << size << std::endl;
									tetrahedras.resize(size);

									int index = 0;
									while (index < size)
									{
										std::getline(ss, line);

										Tetrahedron t;

										int indices[4];
										int neighbors[4];
										for (int i = 0; i < 4; i++)
										{
											std::getline(ss, line);
											std::stringstream lineSS(line);
											std::string key, value, type;
											lineSS >> key >> value >> type;
											t.indices[i] = std::stoi(value);
										}
										for (int i = 0; i < 4; i++)
										{
											std::getline(ss, line);
											std::stringstream lineSS(line);
											std::string key, value, type;
											lineSS >> key >> value >> type;
											t.neighbors[i] = std::stoi(value);
										}

										std::getline(ss, line);

										float mat[12];
										for (int i = 0; i < 12; i++)
										{
											std::getline(ss, line);
											std::stringstream lineSS(line);
											std::string key, value, type;
											lineSS >> key >> value >> type;
											mat[i] = std::stof(value);
										}
										t.matrix = glm::make_mat3x4(mat);

										std::getline(ss, line);
										tetrahedras[index] = t;
										index++;
									}
								}
								else if (key.compare("m_ProbeSets") == 0)
								{
									std::getline(ss, line);
									std::string key, value, type;
									std::stringstream lineSS(line);
									lineSS >> key >> value >> type;
									int size = std::stoi(value);
									//std::cout << "number of entries: " << size << std::endl;
									probeSets.resize(size);

									int index = 0;
									while (index < size)
									{
										ProbeSet p;

										for (int i = 0; i < 18; i++)
											std::getline(ss, line);
										std::getline(ss, line);
										std::string key, value, type;
										std::stringstream lineSS1(line);
										lineSS1 >> key >> value >> type;
										p.offset = std::stoi(value);

										std::getline(ss, line);
										std::stringstream lineSS2(line);
										lineSS2 >> key >> value >> type;
										p.size = std::stoi(value);
										probeSets[index] = p;

										//std::cout << "probe group " << index << " offset: " << p.offset << " size: " << p.size << std::endl;
										index++;
									}
								}
								else if (key.compare("m_Positions") == 0)
								{
									std::getline(ss, line);
									std::string key, value, type;
									std::stringstream lineSS(line);
									lineSS >> key >> value >> type;
									int size = std::stoi(value);
									//std::cout << "number of entries: " << size << std::endl;
									probePositions.resize(size);

									int index = 0;
									while (index < size)
									{
										std::getline(ss, line);
										std::string key, values[3], type;
										std::stringstream lineSS(line);
										lineSS >> key >> values[0] >> values[1] >> values[2] >> type;
										glm::vec3 pos;
										pos.x = std::stof(values[0].substr(1, values[0].length() - 1));
										pos.y = std::stof(values[1]);
										pos.z = std::stof(values[2].substr(0, values[2].length() - 1));
										probePositions[index] = pos;

										index++;
									}
								}
							}

							if (numTabs == 0)
								count++;
							if (count == 5)
								break;
						}
					}
					else if (key.compare("m_BakedCoefficients") == 0)
					{
						std::getline(ss, line);
						std::stringstream lineSS(line);
						std::string key, value, type;
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						//std::cout << "number of entries: " << size << std::endl;
						probeCoeffs.resize(size);

						int index = 0;
						while (index < size)
						{
							SH9 sh;
							std::getline(ss, line);
							for (int i = 0; i < 27; i++)
							{
								std::getline(ss, line);
								int numSpace = 0;
								for (auto c : line)
									if (c == ' ')
										numSpace++;

								std::string key, value, type;
								std::stringstream lineSS(line);
								if (numSpace == 3)
									lineSS >> key >> value >> value >> type;
								else if (numSpace == 2)
									lineSS >> key >> value >> type;
								else
									std::cout << "invalid amount of spaces!" << std::endl;

								sh.coefficients[i] = std::stof(value);
							}
							probeCoeffs[index] = sh;
							index++;
						}
					}
					else if (key.compare("m_Lightmaps") == 0)
					{
						std::string key, value, type;
						std::getline(ss, line);
						std::stringstream lineSS(line);
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						int numTabs = countTabs(line);

						lightMapData.resize(size);
						int index = 0;
						std::getline(ss, line);
						while (index < size)
						{
							std::getline(ss, line);
							if (line.empty())
								break;

							std::string key;
							std::stringstream lineSS(line);
							lineSS >> key;

							if (key.compare("m_Lightmap") == 0)
							{
								std::getline(ss, line);

								std::stringstream lineSS(line);
								std::string key, value;
								lineSS >> key >> value;

								if (key.compare("m_FileID") == 0)
									lMaps.push_back(std::stoi(value));

								std::getline(ss, line);
							}

							if (key.compare("m_DirLightmap") == 0)
							{
								std::getline(ss, line);

								std::stringstream lineSS(line);
								std::string key, value;
								lineSS >> key >> value;

								if (key.compare("m_FileID") == 0)
									dMaps.push_back(std::stoi(value));

								std::getline(ss, line);
							}

							if (countTabs(line) == numTabs)
								index++;
						}
					}
					else if (key.compare("m_LightmappedRendererData") == 0)
					{
						std::string key, value, type;
						std::getline(ss, line);
						std::stringstream lineSS(line);
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						int numTabs = countTabs(line);

						lightMapData.resize(size);
						int index = 0;
						std::getline(ss, line);
						while (index < size)
						{
							std::getline(ss, line);
							if (line.empty())
								break;
							std::stringstream lineSS(line);
							lineSS >> key;
							if (key.compare("lightmapIndex") == 0)
							{
								std::string value;
								lineSS >> value;
								lightMapData[index].index = std::stoi(value);
							}
							else if (key.compare("lightmapST") == 0)
							{
								glm::vec4 lightMapST;
								std::string values[4];
								for (int i = 0; i < 4; i++)
									lineSS >> values[i];
								lightMapData[index].scale.x = std::stof(values[0].substr(1, values[0].length() - 1));
								lightMapData[index].scale.y = std::stof(values[1]);
								lightMapData[index].offset.x = std::stof(values[2]);
								lightMapData[index].offset.y = std::stof(values[3].substr(0, values[3].length() - 1));
							}

							if (countTabs(line) == numTabs)
								index++;
						}
					}
					else if (key.compare("m_LightmappedRendererDataIDs") == 0)
					{
						std::string key, value, type;
						std::getline(ss, line);
						std::stringstream lineSS(line);
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						int numTabs = countTabs(line);

						lightMapDataIDs.resize(size);
						int index = 0;
						std::getline(ss, line);
						while (index < size)
						{
							std::getline(ss, line);
							if (line.empty())
								break;

							std::stringstream lineSS(line);
							lineSS >> key;
							if (key.compare("targetObject") == 0)
							{
								std::string value;
								lineSS >> value;
								lightMapDataIDs[index].targetObject = std::stoll(value);
							}
							else if (key.compare("targetPrefab") == 0)
							{
								std::string value;
								lineSS >> value;
								lightMapDataIDs[index].targetPrefab = std::stoll(value);
							}

							if (countTabs(line) == numTabs)
								index++;
						}
					}
					else if (key.compare("m_BakedReflectionProbeCubemaps") == 0)
					{
						std::string key, value, type;
						std::getline(ss, line);
						std::stringstream lineSS(line);
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						int numTabs = countTabs(line);

						int index = 0;
						std::getline(ss, line);
						while (index < size)
						{
							std::getline(ss, line);
							if (line.empty())
								break;

							std::stringstream lineSS(line);
							std::string value;
							lineSS >> key >> value;
							if (key.compare("m_FileID") == 0)
								reflProbes.push_back(std::stoi(value));

							if (countTabs(line) == numTabs)
								index++;
						}
					}

					else if (key.compare("m_BakedReflectionProbes") == 0)
					{
						std::string key, value, type;
						std::getline(ss, line);
						std::stringstream lineSS(line);
						lineSS >> key >> value >> type;
						int size = std::stoi(value);
						int numTabs = countTabs(line);

						int index = 0;
						std::getline(ss, line);
						while (index < size)
						{
							std::getline(ss, line);
							if (line.empty())
								break;

							std::stringstream lineSS(line);
							std::string value;
							lineSS >> key >> value;
							if (key.compare("targetObject") == 0)
								reflectionObjects.push_back(std::stoll(value));

							if (countTabs(line) == numTabs)
								index++;
						}
					}
				}
			}

			//for (int i = 0; i < lightMaps.size(); i++)
			//	std::cout << "Light map " << i << ", fileID:  " << lightMaps[i] << std::endl;
			//for (int i = 0; i < dirMaps.size(); i++)
			//	std::cout << "Dir map " << i << ", fileID:  " << dirMaps[i] << std::endl;
			//for (int i = 0; i < reflectionProbes.size(); i++)
			//	std::cout << "Reflection probe " << i << ", fileID:  " << reflectionProbes[i] << std::endl;
			//for (int i = 0; i < reflectionObjects.size(); i++)
			//	std::cout << "Reflection probe " << i << ", objectID:  " << reflectionObjects[i] << std::endl;

			for (auto id : lMaps)
				if (fileID2GUID.find(id) != fileID2GUID.end())
					lightMaps.push_back(fileID2GUID[id]);

			for (auto id : dMaps)
				if (fileID2GUID.find(id) != fileID2GUID.end())
					directionMaps.push_back(fileID2GUID[id]);

			for (int i = 0; i < reflectionObjects.size(); i++)
			{
				int fileID = reflProbes[i];
				int64_t objectID = reflectionObjects[i];
				if (fileID2GUID.find(fileID) != fileID2GUID.end())
				{
					std::string guid = fileID2GUID[fileID];
					reflectionProbes.insert(std::make_pair(objectID, guid));
				}
			}

			for (int i = 0; i < lightMapData.size(); i++)
			{
				auto idx = lightMapDataIDs[i];
				auto ld = lightMapData[i];

				if (idx.targetPrefab == 0)
				{
					objectLightMapData.insert(std::make_pair(idx.targetObject, ld));
				}
				else
				{
					auto& plmd = prefabLightMapData;
					ld.meshRendererID = idx.targetObject;
					if (plmd.find(idx.targetPrefab) == plmd.end())
						plmd.insert(std::make_pair(idx.targetPrefab, std::map<int64_t, LightMapData>()));
					plmd[idx.targetPrefab].insert(std::make_pair(ld.meshRendererID, ld));
				}
			}
		}
	}
}
