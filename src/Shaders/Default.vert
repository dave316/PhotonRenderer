#version 460 core

#define MORPH_TARGETS_2
//#define MORPH_TARGETS_8

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 4) in vec2 vTexCoord1;
layout(location = 5) in vec4 vTangent;
layout(location = 6) in vec4 boneIDs;
layout(location = 7) in vec4 boneWeights;

#ifdef MORPH_TARGETS_2
layout(location = 8) in vec3 vTargetPosition0;
layout(location = 9) in vec3 vTargetNormal0;
layout(location = 10) in vec3 vTargetTangent0;
layout(location = 11) in vec3 vTargetPosition1;
layout(location = 12) in vec3 vTargetNormal1;
layout(location = 13) in vec3 vTargetTangent1;
#endif

#ifdef MORPH_TARGETS_8
layout(location = 8) in vec3 vTargetPosition0;
layout(location = 9) in vec3 vTargetPosition1;
layout(location = 10) in vec3 vTargetPosition2;
layout(location = 11) in vec3 vTargetPosition3;
layout(location = 12) in vec3 vTargetPosition4;
layout(location = 13) in vec3 vTargetPosition5;
layout(location = 14) in vec3 vTargetPosition6;
layout(location = 15) in vec3 vTargetPosition7;
#endif

layout(location = 0) out vec3 wPosition;
layout(location = 1) out vec4 vertexColor;
layout(location = 2) out vec3 wNormal;
layout(location = 3) out vec2 texCoord0;
layout(location = 4) out vec2 texCoord1;
layout(location = 5) out mat3 wTBN;

#include "Camera.glsl"

uniform mat4 M;
uniform mat3 N;
uniform mat4 bones[64];
uniform mat3 normals[64];
uniform bool hasAnimations;
uniform float morphWeights[8];
uniform int numMorphTargets = 0;

void main()
{
	vec3 mPosition = vPosition;
	vec3 mNormal = vNormal;
	vec3 mTangent = vTangent.xyz;
	vec3 mBitangent = cross(vNormal, vTangent.xyz) * vTangent.w;
	
#ifdef MORPH_TARGETS_2
	if(numMorphTargets > 0)
	{
		mPosition += vTargetPosition0 * morphWeights[0];
		mNormal += vTargetNormal0 * morphWeights[0];
		mTangent += vTargetTangent0 * morphWeights[0];
	}
	if(numMorphTargets > 1)
	{
		mPosition += vTargetPosition1 * morphWeights[1];
		mNormal += vTargetNormal1 * morphWeights[1];
		mTangent += vTargetTangent1 * morphWeights[1];
	}
#endif

#ifdef MORPH_TARGETS_8
	if(numMorphTargets > 0)
		mPosition += vTargetPosition0 * morphWeights[0];
	if(numMorphTargets > 1)
		mPosition += vTargetPosition1 * morphWeights[1];
	if(numMorphTargets > 2)
		mPosition += vTargetPosition2 * morphWeights[2];
	if(numMorphTargets > 3)
		mPosition += vTargetPosition3 * morphWeights[3];
	if(numMorphTargets > 4)
		mPosition += vTargetPosition4 * morphWeights[4];
	if(numMorphTargets > 5)
		mPosition += vTargetPosition5 * morphWeights[5];
	if(numMorphTargets > 6)
		mPosition += vTargetPosition6 * morphWeights[6];
	if(numMorphTargets > 7)
		mPosition += vTargetPosition7 * morphWeights[7];
#endif

	if(hasAnimations)
	{
		mat4 B = mat4(0.0);
		mat3 C = mat3(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(boneIDs[i]);
			B += bones[index] * boneWeights[i];
			C += normals[index] * boneWeights[i];
		}

		mPosition = vec3(B * vec4(mPosition, 1.0));
		mNormal = normalize(C * mNormal);
		mTangent = normalize(C * mTangent);
		mBitangent = normalize(C * mBitangent); 
	}

	wPosition = vec3(M * vec4(mPosition, 1.0));
	wNormal = normalize(N * mNormal);
	vec3 wTangent = normalize(N * mTangent);
	vec3 wBitangent = normalize(N * mBitangent); 
	wTBN = mat3(wTangent, wBitangent, wNormal);

	vertexColor = vColor;
	texCoord0 = vTexCoord0;
	texCoord1 = vTexCoord1;
	gl_Position = camera.VP * vec4(wPosition, 1.0);
}