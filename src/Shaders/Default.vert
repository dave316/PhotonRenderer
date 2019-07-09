#version 450 core

//#define MORPH_TARGETS

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord;
layout(location = 4) in vec4 vTangent;
//layout(location = 5) in vec3 vBitangent;
//#ifdef MORPH_TARGETS
//layout(location = 6) in vec3 vTargetPosition0;
//layout(location = 7) in vec3 vTargetNormal0;
//layout(location = 8) in vec3 vTargetTangent0;
//layout(location = 9) in vec3 vTargetPosition1;
//layout(location = 10) in vec3 vTargetNormal1;
//layout(location = 11) in vec3 vTargetTangent1;
//#else
//layout(location = 6) in vec4 boneIDs;
//layout(location = 7) in vec4 boneWeights;
//#endif

out vec3 wPosition;
out vec3 wNormal;
out vec3 color;
out vec2 texCoord;
out mat3 wTBN;

uniform mat4 VP;
uniform mat4 M;
uniform mat3 N;

void main()
{
	vec3 vbitangent = cross(vNormal, vTangent.xyz) * vTangent.w;

	wNormal = normalize(N * vNormal);
	vec3 t = normalize(N * vTangent.xyz);
	vec3 b = normalize(N * vbitangent); 
	wTBN = mat3(t,b,wNormal);

	wPosition = vec3(M * vec4(vPosition, 1.0));
	color = vColor;
	texCoord = vTexCoord;
	gl_Position = VP * vec4(wPosition, 1.0);
}