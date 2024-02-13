#ifndef INCLUDED_LIGHTDATA
#define INCLUDED_LIGHTDATA

#pragma once

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace IO
{
	namespace Unity
	{
		struct LightMapData
		{
			int index;
			glm::vec2 scale;
			glm::vec2 offset;
			int64_t meshRendererID = 0;
		};

		struct LightMapDataID
		{
			int64_t targetObject = 0;
			int64_t targetPrefab = 0;
		};

		struct Tetrahedron
		{
			int indices[4];
			int neighbors[4];
			glm::mat3x4 matrix;
		};

		struct ProbeSet
		{
			int offset;
			int size;
		};

		struct SH9
		{
			float coefficients[27];
		};

		class LightData
		{
		public:
			std::map<std::string, int> probeMapping;
			std::vector<Tetrahedron> tetrahedras;
			std::vector<ProbeSet> probeSets;
			std::vector<glm::vec3> probePositions;
			std::vector<SH9> probeCoeffs;

			std::vector<std::string> lightMaps;
			std::vector<std::string> directionMaps;
			std::map<int64_t, std::string> reflectionProbes;
			std::map<int64_t, LightMapData> objectLightMapData;
			std::map<int64_t, std::map<int64_t, LightMapData>> prefabLightMapData;
		public:
			void loadAsset(const std::string& filename);
		};
	}
}

#endif // INCLUDED_LIGHTDATA
