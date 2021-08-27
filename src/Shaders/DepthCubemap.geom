#version 450 core

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 VP[NUM_FACES];

in vec2 texCoord[3];
out vec3 wPosition;
out vec2 fTexCoord;

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
			fTexCoord = texCoord[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
