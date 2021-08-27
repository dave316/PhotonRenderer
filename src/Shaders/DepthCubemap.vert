#version 450 core

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord;
layout(location = 5) in vec4 boneIDs;
layout(location = 6) in vec4 boneWeights;

out vec2 texCoord;

uniform mat4 M;
uniform mat4 bones[64];
uniform mat3 normals[64];
uniform bool hasAnimations;

void main()
{
	vec3 mPosition;
	if(hasAnimations)
	{
		mat4 B = mat4(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(boneIDs[i]);
			B += bones[index] * boneWeights[i];
		}

		mPosition = vec3(B * vec4(vPosition, 1.0));
	}
	else
	{
		mPosition = vPosition;
	}

	texCoord = vTexCoord;
	gl_Position = M * vec4(mPosition, 1.0);
}
