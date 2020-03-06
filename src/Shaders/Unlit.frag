#version 450 core

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D tex;
uniform vec3 solidColor = vec3(0.5);
uniform bool useTex = false;
uniform bool useVertexColor = false;

layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 color = vec3(0);
	if(useTex)
		color = texture(tex, texCoord).rgb;
	else if(useVertexColor)
		color = vertexColor.rgb;
	else
		color = solidColor;

	fragColor = vec4(color, 1.0);
}