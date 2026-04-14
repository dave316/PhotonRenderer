#version 450 core

layout(location = 0) in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#ifdef USE_OPENGL
layout(binding = 0) uniform sampler2D panorama;
#else
layout(set = 0, binding = 2) uniform sampler2D panorama;
#endif

void main()
{
	vec3 v = normalize(uvw);
	vec2 uv = vec2(atan(-v.x, v.z), asin(v.y));
	uv *= vec2(0.1591, 0.3183);
	uv += 0.5;
	vec3 color = texture(panorama, uv).rgb;
	fragColor = vec4(color, 1.0);
}