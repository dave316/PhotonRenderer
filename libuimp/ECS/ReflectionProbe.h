#ifndef INCLUDED_UNITYREFLECTIONPROBE
#define INCLUDED_UNITYREFLECTIONPROBE

#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <string>

namespace Unity
{
	class ReflectionProbe : public Component
	{
	public:
		ReflectionProbe();
		~ReflectionProbe();

		void setSourceFile(const std::string& filepath);
		void setBoxSize(glm::vec3 boxSize);
		void setBoxOffset(glm::vec3 boxOffset);
		std::string getSourceFile();
		glm::vec3 getBoxSize();
		glm::vec3 getBoxOffset();

		typedef std::shared_ptr<ReflectionProbe> Ptr;
		static Ptr create()
		{
			return std::make_shared<ReflectionProbe>();
		}
	private:
		std::string sourceFile = "";
		glm::vec3 boxSize = glm::vec3(1.0f);
		glm::vec3 boxOffset = glm::vec3(0.0f);
	};
}

#endif // INCLUDED_UNITYREFLECTIONPROBE