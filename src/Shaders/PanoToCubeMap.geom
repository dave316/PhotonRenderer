#version 450 core

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in vec3 wPosition[3];
out vec3 uvw;

uniform mat4 VP[NUM_FACES];

void main()
{
	for(int face = 0; face < NUM_FACES; face++)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; i++)
		{
			uvw = wPosition[i];
			gl_Position = VP[face] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}
