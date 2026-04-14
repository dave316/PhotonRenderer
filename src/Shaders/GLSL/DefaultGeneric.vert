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

#include "Camera.glsl"
#include "Model.glsl"
#include "Animation.glsl"

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