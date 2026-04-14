#version 460 core

#ifdef USE_OPENGL
#extension GL_KHR_vulkan_glsl : enable
#endif

#define MAX_MORPH_TARGETS 8

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord0;
layout(location = 4) in vec2 vTexCoord1;
layout(location = 5) in vec4 vTangent;
layout(location = 6) in vec4 vJointIndices;
layout(location = 7) in vec4 vJointWeights;

layout(location = 0) out vec3 wPosition;
layout(location = 1) out vec4 vertexColor;
layout(location = 2) out vec3 wNormal;
layout(location = 3) out vec2 texCoord0;
layout(location = 4) out vec2 texCoord1;
layout(location = 5) out mat3 wTBN;


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

vec3 computeRadianceSHPrescaled(vec3 dir)
{
	vec3 irradiance =
		model.sh[0].xyz

		// Band 1
		+ model.sh[1].xyz * (dir.y)
		+ model.sh[2].xyz * (dir.z)
		+ model.sh[3].xyz * (dir.x)

		// Band  2
		+ model.sh[4].xyz * (dir.x * dir.y)
		+ model.sh[5].xyz * (dir.y * dir.z)
		+ model.sh[6].xyz * (3.0 * dir.z * dir.z - 1.0)
		+ model.sh[7].xyz * (dir.x * dir.z)
		+ model.sh[8].xyz * (dir.x * dir.x - dir.y * dir.y);

	return irradiance;
}
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

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec3 mPosition = vPosition;
	vec3 mNormal = vNormal;
	vec3 mTangent = vTangent.xyz;
	vec3 mBitangent = cross(mNormal, mTangent) * sign(vTangent.w);

	if (model.animMode == 1) // vertex skinning
	{
		mat4 B = mat4(0.0);
		mat3 C = mat3(0.0);
		for(int i = 0; i < 4; i++)
		{
			int idx = int(vJointIndices[i]);
			B += animation.joints[idx] * vJointWeights[i];
			C += mat3(animation.normals[idx]) * vJointWeights[i];
		}

		mPosition = vec3(B * vec4(mPosition, 1.0));
		mNormal = normalize(C * mNormal);
		mTangent = normalize(C * mTangent);
		mBitangent = normalize(C * mBitangent);
	}
	if (model.animMode == 2) // morph targets
	{
#ifdef USE_OPENGL
		mPosition += getTargetAttribute(gl_VertexID, 0);
		mNormal += getTargetAttribute(gl_VertexID, 1);
		mTangent += getTargetAttribute(gl_VertexID, 2);
#else
		mPosition += getTargetAttribute(gl_VertexIndex, 0);
		mNormal += getTargetAttribute(gl_VertexIndex, 1);
		mTangent += getTargetAttribute(gl_VertexIndex, 2);
#endif
	}

	wPosition = vec3(model.localToWorld * vec4(mPosition, 1.0));
	
	mat3 N = inverse(transpose(mat3(model.localToWorld)));
	wNormal = normalize(N * mNormal);
	vec3 wTangent = normalize(N * mTangent);
	vec3 wBitangent = normalize(N * mBitangent);
	wTBN = mat3(wTangent, wBitangent, wNormal);
	vertexColor = vColor;
	texCoord0 = vTexCoord0;
	texCoord1 = vTexCoord1;
	gl_Position = camera.viewProjection * vec4(wPosition, 1.0);
#ifndef USE_OPENGL
	gl_Position.y = -gl_Position.y;
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0f;
#endif
}