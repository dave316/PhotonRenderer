#version 460 core

#define MAX_JOINTS 128
#define MAX_MORPH_TARGETS 8
#define MORPH_TARGET_POSITION_OFFSET 0
#define MORPH_TARGET_NORMAL_OFFSET 1
#define MORPH_TARGET_TANGENT_OFFSET 2

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 4) in vec2 vTexCoord1;
layout(location = 5) in vec4 vTangent;
layout(location = 6) in vec4 boneIDs;
layout(location = 7) in vec4 boneWeights;

layout(location = 0) out vec4 vertexColor;
layout(location = 1) out vec3 wNormal;
layout(location = 2) out vec2 texCoord0;
layout(location = 3) out vec2 texCoord1;
layout(location = 4) out vec4 lightPosition;
layout(location = 5) out mat3 wTBN;

#include "Camera.glsl"

uniform mat4 VP;
uniform mat4 M;
uniform mat3 N;
uniform mat4 bones[MAX_JOINTS];
uniform mat3 normals[MAX_JOINTS];
uniform bool hasAnimations;
uniform float morphWeights[MAX_MORPH_TARGETS];
uniform int numMorphTargets = 0;
uniform sampler2DArray morphTargets;


vec3 getOffset(int vertexID, int targetIndex, int texSize)
{
	int x = vertexID % texSize;
	int y = vertexID / texSize;
	return texelFetch(morphTargets, ivec3(x, y, targetIndex), 0).rgb;
}

vec3 getTargetAttribute(int vertexID, int attrOffset)
{
	// TODO: update number of attributes with uniform
	vec3 offset = vec3(0);
	int texSize = textureSize(morphTargets, 0).x;
	for(int i = 0; i < numMorphTargets; i++)
		offset += morphWeights[i] * getOffset(vertexID, i * 3 + attrOffset, texSize);
	return offset;
}

void main()
{
	vec3 mPosition = vPosition;
	vec3 mNormal = vNormal;
	vec3 mTangent = vTangent.xyz;
	vec3 mBitangent = cross(mNormal, mTangent) * sign(vTangent.w);
	if(numMorphTargets > 0)
	{
		mPosition += getTargetAttribute(gl_VertexID, MORPH_TARGET_POSITION_OFFSET);
		mNormal += getTargetAttribute(gl_VertexID, MORPH_TARGET_NORMAL_OFFSET);
		mTangent += getTargetAttribute(gl_VertexID, MORPH_TARGET_TANGENT_OFFSET);
	}

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

	vec3 wPosition = vec3(M * vec4(mPosition, 1.0));
	wNormal = normalize(N * mNormal);
	vec3 wTangent = normalize(N * mTangent);
	vec3 wBitangent = normalize(N * mBitangent);
	wTBN = mat3(wTangent, wBitangent, wNormal);	

	vertexColor = vColor;
	texCoord0 = vTexCoord0;
	texCoord1 = vTexCoord1;
	lightPosition = VP * vec4(wPosition, 1.0);
	gl_Position = vec4(wPosition, 1.0);
}