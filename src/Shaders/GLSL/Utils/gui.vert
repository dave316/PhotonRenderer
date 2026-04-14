#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform PushConstants
#else
layout (push_constant) uniform PushConstants 
#endif
{
	mat4 orthoProjection;
	//vec2 scale;
	//vec2 translate;
} pushConstants;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

void main() 
{
	outUV = inUV;
	outColor = inColor;
	//vec2 pos = inPos;
	//gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
	gl_Position = pushConstants.orthoProjection * vec4(inPos.xy, 0.0, 1.0);
#ifndef USE_OPENGL
	gl_Position.y = -gl_Position.y;
#endif
}