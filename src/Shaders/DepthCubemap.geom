#version 460 core

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 VP[NUM_FACES];
uniform int lightIndex;

layout(location = 0) in vec2 texCoord0[3];
layout(location = 1) in vec2 texCoord1[3];

layout(location = 0) out vec3 wPosition;
layout(location = 1) out vec2 fTexCoord0;
layout(location = 2) out vec2 fTexCoord1;

void main()
{
	for(int face = 0; face < NUM_FACES; face++)
	{
		gl_Layer = lightIndex * 6 + face;
		for(int i = 0; i < 3; i++)
		{
			vec4 pos = gl_in[i].gl_Position;
			gl_Position = VP[face] * pos;
			wPosition = pos.xyz;
			fTexCoord0 = texCoord0[i];
			fTexCoord1 = texCoord1[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
