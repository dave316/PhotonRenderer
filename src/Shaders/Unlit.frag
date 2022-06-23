#version 450 core

#include "HDR.glsl"

layout(location = 1) in vec4 vertexColor;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;

uniform sampler2D tex;
uniform vec3 solidColor = vec3(0.5);
uniform bool useTex = false;
uniform bool useVertexColor = false;
uniform bool useGammaEncoding = false;

layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 color = vec3(0);
	if(useTex)
		color = texture(tex, texCoord0).rgb;
	else if(useVertexColor)
		color = vertexColor.rgb;
	else
		color = solidColor;

	if(useGammaEncoding)
		color = linear2sRGB(color);

	fragColor = vec4(color, 1.0);
}