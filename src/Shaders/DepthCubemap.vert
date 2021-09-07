#version 450 core

#define MORPH_TARGETS

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 6) in vec4 boneIDs;
layout(location = 7) in vec4 boneWeights;
#ifdef MORPH_TARGETS
layout(location = 8) in vec3 vTargetPosition0;
layout(location = 11) in vec3 vTargetPosition1;
#endif

out vec2 texCoord;

uniform mat4 M;
uniform mat4 bones[64];
uniform mat3 normals[64];
uniform bool hasAnimations;
uniform float morphWeights[2];
uniform int numMorphTargets = 0;

void main()
{
	vec3 mPosition = vPosition;
#ifdef MORPH_TARGETS
	if(numMorphTargets > 0)
		mPosition += vTargetPosition0 * morphWeights[0];
	if(numMorphTargets > 1)
		mPosition += vTargetPosition1 * morphWeights[1];
#endif

	if(hasAnimations)
	{
		mat4 B = mat4(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(boneIDs[i]);
			B += bones[index] * boneWeights[i];
		}

		mPosition = vec3(B * vec4(mPosition, 1.0));
	}

	texCoord = vTexCoord0;
	gl_Position = M * vec4(mPosition, 1.0);
}
