#version 460 core

//#define MORPH_TARGETS

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord;
layout(location = 4) in vec4 vTangent;
//#ifdef MORPH_TARGETS
//layout(location = 6) in vec3 vTargetPosition0;
//layout(location = 7) in vec3 vTargetNormal0;
//layout(location = 8) in vec3 vTargetTangent0;
//layout(location = 9) in vec3 vTargetPosition1;
//layout(location = 10) in vec3 vTargetNormal1;
//layout(location = 11) in vec3 vTargetTangent1;
//#else
layout(location = 5) in vec4 boneIDs;
layout(location = 6) in vec4 boneWeights;
//#endif

out vec3 wPosition;
out vec3 wNormal;
out mat3 wTBN;
out vec4 vertexColor;
out vec2 texCoord;

#include "Camera.glsl"

//uniform mat4 VP;
uniform mat4 M;
uniform mat3 N;
uniform mat4 bones[64];
uniform mat3 normals[64];
uniform bool hasAnimations;

void main()
{
	vec3 vbitangent = cross(vNormal, vTangent.xyz) * vTangent.w;

	if(hasAnimations)
	{
		mat4 B = mat4(0.0);
		//mat3 C = mat3(0.0);
		for(int i = 0; i < 4; i++)
		{
			int index = int(boneIDs[i]);
			B += bones[index] * boneWeights[i];
			//C += normals[index] * boneWeights[i];
		}
		mat3 C = transpose(inverse(mat3(B)));

		wPosition = vec3(M * B * vec4(vPosition, 1.0));
		wNormal = normalize(N * C * vNormal);
		vec3 t = normalize(N * C * vTangent.xyz);
		vec3 b = normalize(N * C * vbitangent); 
		wTBN = mat3(t,b,wNormal);
	}
	else
	{
		wPosition = vec3(M * vec4(vPosition, 1.0));
		wNormal = normalize(N * vNormal);
		vec3 t = normalize(N * vTangent.xyz);
		vec3 b = normalize(N * vbitangent); 
		wTBN = mat3(t,b,wNormal);
	}

	vertexColor = vColor;
	texCoord = vTexCoord;
	gl_Position = camera.VP * vec4(wPosition, 1.0);
}