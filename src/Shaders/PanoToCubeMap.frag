#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform sampler2D panorama;
uniform float exposure;
uniform vec3 colorTint;

void main()
{
	vec3 v = normalize(uvw);
	vec2 uv = vec2(atan(-v.x, v.z), asin(v.y));
	uv *= vec2(0.1591, 0.3183);
	uv += 0.5;
	vec3 color = texture2D(panorama, uv).rgb;
	fragColor = vec4(color * exposure, 1.0);
}