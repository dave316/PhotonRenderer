#version 440 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColor[];
in float vLifeTime[];
in float vSize[];
in float vType[];

smooth out vec2 uv;
flat out vec4 gColor;

uniform mat4 VP;
uniform vec3 quadRight;
uniform vec3 quadUp;

void main()
{
	if(vType[0] != 0.0)
	{
		vec3 oldPosition = gl_in[0].gl_Position.xyz;
		float size = vSize[0];
		gColor = vec4(vColor[0], vLifeTime[0]);

		vec3 scaledRight = quadRight * size;
		vec3 scaledUp = quadUp * size;
		vec3 position;

		// left bottom
		position = oldPosition - scaledRight - scaledUp;
		uv = vec2(0.0, 0.0);
		gl_Position = VP * vec4(position, 1.0);
		EmitVertex();

		// right bottom
		position = oldPosition + scaledRight - scaledUp;
		uv = vec2(1.0, 0.0);
		gl_Position = VP * vec4(position, 1.0);
		EmitVertex();

		// left top
		position = oldPosition - scaledRight + scaledUp;
		uv = vec2(0.0, 1.0);
		gl_Position = VP * vec4(position, 1.0);
		EmitVertex();
		
		// right top
		position = oldPosition + scaledRight + scaledUp;
		uv = vec2(1.0, 1.0);
		gl_Position = VP * vec4(position, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}