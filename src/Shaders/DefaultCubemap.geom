#version 460 core

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 VP[NUM_FACES];

layout(location = 0) in vec4 vertexColor[3];
layout(location = 1) in vec3 wNormal[3];
layout(location = 2) in vec2 texCoord0[3];
layout(location = 3) in vec2 texCoord1[3];
layout(location = 4) in vec4 lightPosition[3];
layout(location = 5) in mat3 wTBN[3];

layout(location = 0) out vec3 wPosition;
layout(location = 1) out vec4 fVertexColor;
layout(location = 2) out vec3 fNormal;
layout(location = 3) out vec2 fTexCoord0;
layout(location = 4) out vec2 fTexCoord1;
layout(location = 5) out vec4 fLightPosition;
layout(location = 6) out mat3 fTBN;

void main()
{
	for(int face = 0; face < NUM_FACES; face++)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; i++)
		{
			vec4 pos = gl_in[i].gl_Position;
			gl_Position = VP[face] * pos;
			wPosition = pos.xyz;
			fVertexColor = vertexColor[i];
			fNormal = wNormal[i];
			fTexCoord0 = texCoord0[i];
			fTexCoord1 = texCoord1[i];
			fLightPosition = lightPosition[i];
			fTBN = wTBN[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
