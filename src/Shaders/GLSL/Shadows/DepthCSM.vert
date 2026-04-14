#version 460 core

#define MAX_JOINTS 160
#define MAX_MORPH_TARGETS 8
#define MORPH_TARGET_POSITION_OFFSET 0
#define MORPH_TARGET_NORMAL_OFFSET 1
#define MORPH_TARGET_TANGENT_OFFSET 2

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 4) in vec2 vTexCoord1;
layout(location = 6) in vec4 vJointIndices;
layout(location = 7) in vec4 vJointWeights;

layout(location = 0) out vec2 texCoord0;
layout(location = 1) out vec2 texCoord1;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform CameraUBO
#else
layout(std140, set = 0, binding = 0) uniform CameraUBO
#endif
{
	mat4 viewProjection;
	mat4 viewProjectionInv;
	mat4 projection;
	mat4 projectionInv;
	mat4 view;
	mat4 viewInv;
	vec4 position;
	vec4 time;
	vec4 projParams;
	float zNear;
	float zFar;
	float scale;
	float bias;
} camera;


#ifdef USE_OPENGL
layout(std140, binding = 1) uniform ModelUBO
#else
layout(std140, set = 1, binding = 0) uniform ModelUBO
#endif
{
	mat4 localToWorld;
	mat4 localToWorldNormal;
	vec4 weights[MAX_MORPH_TARGETS / 4];
	int animMode;
	int numMorphTargets;
	int irradianceMode;
	int lightMapIndex;
	vec4 lightMapST;
	vec4 sh[9];
	int reflectionProbeIndex;
} model;

#ifdef USE_OPENGL
layout(std140, binding = 2) uniform AnimUBO
#else
layout(std140, set = 2, binding = 0) uniform AnimUBO
#endif
{
	mat4 joints[32];
	mat4 normals[32];
} animation;

#ifdef USE_OPENGL
layout(binding = 0) uniform sampler2DArray morphTargets;
#else
layout(set = 3, binding = 0) uniform sampler2DArray morphTargets;
#endif

vec3 getOffset(int vertexID, int targetIndex, int texSize)
{
	int x = vertexID % texSize;
	int y = vertexID / texSize;
	return texelFetch(morphTargets, ivec3(x, y, targetIndex), 0).xyz;
}

vec3 getTargetAttribute(int vertexID, int attrOffset)
{
	vec3 offset = vec3(0);
	int texSize = textureSize(morphTargets, 0).x;
	for(int i = 0; i < model.numMorphTargets; i++)
		offset += model.weights[i / 4][i % 4] * getOffset(vertexID, i * 3 + attrOffset, texSize);
	return offset;
}

void main()
{
	vec3 mPosition = vPosition;
	if (model.animMode == 1)
	{
		mat4 B = mat4(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(vJointIndices[i]);
			B += animation.joints[index] * vJointWeights[i];
		}
		mPosition = vec3(B * vec4(mPosition, 1.0));
	}
	else if(model.animMode == 2)
	{
#ifdef USE_OPENGL
		mPosition += getTargetAttribute(gl_VertexID, 0);
#else
		mPosition += getTargetAttribute(gl_VertexIndex, 0);
#endif
	}

	texCoord0 = vTexCoord0;
	texCoord1 = vTexCoord1;
	gl_Position = model.localToWorld * vec4(mPosition, 1.0);
}