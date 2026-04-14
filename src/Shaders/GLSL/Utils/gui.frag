#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

#ifdef USE_OPENGL
layout(binding = 0) uniform sampler2D tex;
#else
layout(set = 0, binding = 0) uniform sampler2D tex;
#endif

void main() 
{
	vec4 linearRGBA = vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
	outColor = linearRGBA * texture(tex, inUV);
}