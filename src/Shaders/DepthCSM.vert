#version 460 core

#define MAX_JOINTS 160
#define MAX_MORPH_TARGETS 8
#define MORPH_TARGET_POSITION_OFFSET 0
#define MORPH_TARGET_NORMAL_OFFSET 1
#define MORPH_TARGET_TANGENT_OFFSET 2

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 4) in vec2 vTexCoord1;
layout(location = 6) in vec4 boneIDs;
layout(location = 7) in vec4 boneWeights;
layout(location = 8) in mat4 modelMatrix;

layout(location = 0) out vec2 texCoord0;
layout(location = 1) out vec2 texCoord1;

struct ModelData
{
	mat4 M;
	mat4 N;
//	mat4 bones[MAX_JOINTS];
//	mat3 normals[MAX_JOINTS];
//	float morphWeights[MAX_MORPH_TARGETS];
	int animationMode; // 0 - no animation, 1 - vertex skinning, 2 - morph targets
	//bool hasAnimations;
};

layout(std140, binding = 2) uniform ModelUBO
{
	ModelData model;
};

//uniform mat4 M;
uniform mat4 bones[MAX_JOINTS];
//uniform bool hasAnimations;
uniform float morphWeights[MAX_MORPH_TARGETS];
uniform int numMorphTargets = 0;
uniform sampler2DArray morphTargets;

uniform bool useInstancing = false;

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
	if(numMorphTargets > 0)
		mPosition += getTargetAttribute(gl_VertexID, MORPH_TARGET_POSITION_OFFSET);

	if(model.animationMode == 1)
	{
		mat4 B = mat4(0.0);
		mat3 C = mat3(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(boneIDs[i]);
			B += bones[index] * boneWeights[i];
		}

		mPosition = vec3(B * vec4(mPosition, 1.0));
	}

	mat4 M = model.M;
	if(useInstancing)
		M = modelMatrix;

	texCoord0 = vTexCoord0;
	texCoord1 = vTexCoord1;
	gl_Position = M * vec4(mPosition, 1.0);
}