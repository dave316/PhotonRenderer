#version 460 core

#define MAX_CASCADES 4

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

#ifdef USE_OPENGL
layout(std140, binding = 3) uniform ViewsUBO
#else
layout(std140, set = 4, binding = 0) uniform ViewsUBO
#endif
{
	mat4 VP[MAX_CASCADES];
} views;

layout(location = 0) in vec2 texCoord0[3];
layout(location = 1) in vec2 texCoord1[3];

layout(location = 0) out vec2 fTexCoord0;
layout(location = 1) out vec2 fTexCoord1;

void main()
{
	for(int i = 0; i < 3; i++)
	{
		gl_Position = views.VP[gl_InvocationID] * gl_in[i].gl_Position;
#ifndef USE_OPENGL
		gl_Position.y = -gl_Position.y;
		gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0f;
#endif
		gl_Layer = gl_InvocationID;
		fTexCoord0 = texCoord0[i];
		fTexCoord1 = texCoord1[i];
		EmitVertex();
	}
	EndPrimitive();
}
