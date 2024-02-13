#version 460 core

#define MAX_CASCADES 8

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 VP[MAX_CASCADES];

layout(location = 0) in vec2 texCoord0[3];
layout(location = 1) in vec2 texCoord1[3];

layout(location = 0) out vec2 fTexCoord0;
layout(location = 1) out vec2 fTexCoord1;

void main()
{
	for(int i = 0; i < 3; i++)
	{
		gl_Position = VP[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		fTexCoord0 = texCoord0[i];
		fTexCoord1 = texCoord1[i];
		EmitVertex();
	}
	EndPrimitive();
}
