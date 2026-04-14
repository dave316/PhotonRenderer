#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout(location = 0) in vec3 wPosition[3];

layout(location = 0) out vec3 uvw;

#define NUM_FACES 6

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform ViewsUBO
#else
layout(std140, set = 0, binding = 0) uniform ViewsUBO
#endif
{
	mat4 VP[NUM_FACES];
	int layerID;
} views;

void main()
{
	for(int face = 0; face < NUM_FACES; face++)
	{
		gl_Layer = face;

		for(int i = 0; i < 3; i++)
		{
			uvw = wPosition[i];
			gl_Position = views.VP[face] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}
