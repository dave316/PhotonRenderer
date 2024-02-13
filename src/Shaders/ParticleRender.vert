#version 440 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 color;
layout(location = 3) in float lifeTime;
layout(location = 4) in float size;
layout(location = 5) in float type;

out vec3 vColor;
out float vLifeTime;
out float vSize;
out float vType;

void main()
{   
	gl_Position = vec4(position, 1.0);
	vColor = color;
	vLifeTime = lifeTime;
	vSize = size;
	vType = type;
}