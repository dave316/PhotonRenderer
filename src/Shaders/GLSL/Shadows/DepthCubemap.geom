#version 460 core

#define NUM_FACES 6

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

#ifdef USE_OPENGL
layout(std140, binding = 3) uniform ViewsUBO
#else
layout(std140, set = 4, binding = 0) uniform ViewsUBO
#endif
{
	mat4 VP[NUM_FACES];
	int lightIndex;
} views;

layout(location = 0) in vec2 texCoord0[3];
layout(location = 1) in vec2 texCoord1[3];

layout(location = 0) out vec3 wPosition;
layout(location = 1) out vec2 fTexCoord0;
layout(location = 2) out vec2 fTexCoord1;

void main()
{
	for(int face = 0; face < NUM_FACES; face++)
	{
//#ifdef USE_OPENGL // TODO: opengl always attaches all layers of a layered texture to the fbo, so use texture views!
//		gl_Layer = views.lightIndex * 6 + face;
//#else
		gl_Layer = face;
//#endif
		for(int i = 0; i < 3; i++)
		{
			vec4 pos = gl_in[i].gl_Position;
			gl_Position = views.VP[face] * pos;
			wPosition = pos.xyz;
			fTexCoord0 = texCoord0[i];
			fTexCoord1 = texCoord1[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
