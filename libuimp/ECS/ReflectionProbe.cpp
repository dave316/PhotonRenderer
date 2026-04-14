#include "ReflectionProbe.h"

namespace Unity
{
	ReflectionProbe::ReflectionProbe()
	{

	}

	ReflectionProbe::~ReflectionProbe()
	{

	}

	void ReflectionProbe::setSourceFile(const std::string& filepath)
	{
		this->sourceFile = filepath;
	}

	void ReflectionProbe::setBoxSize(glm::vec3 boxSize)
	{
		this->boxSize = boxSize;
	}

	void ReflectionProbe::setBoxOffset(glm::vec3 boxOffset)
	{
		this->boxOffset = boxOffset;
	}

	std::string ReflectionProbe::getSourceFile()
	{
		return sourceFile;
	}

	glm::vec3 ReflectionProbe::getBoxSize()
	{
		return boxSize;
	}

	glm::vec3 ReflectionProbe::getBoxOffset()
	{
		return boxOffset;
	}
}