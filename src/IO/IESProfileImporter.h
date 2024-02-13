#ifndef INCLUDED_IESPROFILEIMPORTER
#define INCLUDED_IESPROFILEIMPORTER

#pragma once

#include <Graphics/Mesh.h>
#include <Utils/Types.h>

#include <map>
#include <string>
#include <vector>

namespace IO
{
	enum class PhotometricType : int
	{
		TypeC = 1,
		TypeB = 2,
		TypeA = 3		
	};
	enum class UnitsType : int
	{
		FEET = 1,
		METERS = 2
	};
	struct IESProfile
	{
		std::string formatVersion;
		std::map<std::string, std::string> metaData;

		uint32 numLamps;
		float lumenPerLamp;
		float multiplier;
		uint32 numVerticalAngles;
		uint32 numHorizontalAngles;
		PhotometricType photometricType;
		UnitsType unitsType;
		float width;
		float length;
		float height;
		float ballastFactor;
		float inputWatts;

		std::vector<float> verticalAngles;
		std::vector<float> horizontalAngles;
		std::vector<float> candelaValues;
		std::vector<float> candelaNormalized;

		void load(const std::string& filename);
		void generateSamplePoints();
		void print();
		Mesh::Ptr createPhotometricNetSymmetric();
		Mesh::Ptr createPhotometricNet();
		float* generateLUT(uint32 size);
	};
}

#endif // INCLUDED_IESPROFILEIMPORTER