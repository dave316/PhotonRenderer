#version 450 core

in vec3 wPosition[3];

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 VP[NUM_FACES];

out vec3 uvw;

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
