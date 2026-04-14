#include "SimpleTypes.h"

namespace Unity
{
	namespace YAML
	{
		bool read(ryml::ConstNodeRef n, glm::vec2& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::vec3& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			n["z"] >> value.z;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::quat& value)
		{
			n["x"] >> value.x;
			n["y"] >> value.y;
			n["z"] >> value.z;
			n["w"] >> value.w;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::vec4& value)
		{
			n["r"] >> value.r;
			n["g"] >> value.g;
			n["b"] >> value.b;
			n["a"] >> value.a;
			return true;
		}

		bool read(ryml::ConstNodeRef n, glm::mat4& value)
		{
			n["e00"] >> value[0][0];
			n["e01"] >> value[0][1];
			n["e02"] >> value[0][2];
			n["e03"] >> value[0][3];
			n["e10"] >> value[1][0];
			n["e11"] >> value[1][1];
			n["e12"] >> value[1][2];
			n["e13"] >> value[1][3];
			n["e20"] >> value[2][0];
			n["e21"] >> value[2][1];
			n["e22"] >> value[2][2];
			n["e23"] >> value[2][3];
			n["e30"] >> value[3][0];
			n["e31"] >> value[3][1];
			n["e32"] >> value[3][2];
			n["e33"] >> value[3][3];
			return true;
		}

		bool read(ryml::ConstNodeRef n, FileID* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, InstanceID* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Component* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, ObjectRef* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, TextureProperty* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, FloatProperty* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, ColorProperty* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, OcclusionBakeSettings* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Override* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Modification* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Shadows* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}