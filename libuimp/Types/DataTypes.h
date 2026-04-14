#ifndef INCLUDED_DATATYPES
#define INCLUDED_DATATYPES

#pragma once

#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Unity
{
	typedef unsigned long long uint64;
	typedef unsigned int uint32;
	typedef unsigned short uint16;
	typedef unsigned char uint8;
	typedef long long int64;
	typedef int int32;
	typedef short int16;
	typedef char int8;

	std::ostream& operator<< (std::ostream& stream, const glm::vec2& value);
	std::ostream& operator<< (std::ostream& stream, const glm::vec3& value);
	std::ostream& operator<< (std::ostream& stream, const glm::vec4& value);
	std::ostream& operator<< (std::ostream& stream, const glm::quat& value);
	std::ostream& operator<< (std::ostream& stream, const glm::mat4& value);

	template<typename Type>
	struct YamlField
	{
		const char* key;
		Type value;
		std::string prefix;
		bool isPresent = false;

		void print(const std::string& prefix) {
			std::cout << prefix << key << ": " << value << std::endl;
		}

		operator Type() const
		{
			return value;
		}
	};
}

#endif // INCLUDED_UNITYYAML