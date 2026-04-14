#include "DataTypes.h"

namespace Unity
{

	std::ostream& operator<< (std::ostream& stream, const glm::vec2& value)
	{
		stream << "{" << "x: " << value.x << ", y: " << value.y << "}";
		return stream;
	}

	std::ostream& operator<< (std::ostream& stream, const glm::vec3& value)
	{
		stream << "{" << "x: " << value.x << ", y: " << value.y << ", z: " << value.z << "}";
		return stream;
	}

	std::ostream& operator<< (std::ostream& stream, const glm::vec4& value)
	{
		stream << "{" << "r: " << value.r << ", g: " << value.g << ", b: " << value.b << ", a: " << value.a << "}";
		return stream;
	}

	std::ostream& operator<< (std::ostream& stream, const glm::quat& value)
	{
		stream << "{" << "x: " << value.x << ", y: " << value.y << ", z: " << value.z << ", w: " << value.w << "}";
		return stream;
	}

	std::ostream& operator<< (std::ostream& stream, const glm::mat4& value)
	{
		return stream;
	}
}
