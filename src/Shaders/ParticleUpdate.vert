#version 440 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 velocity;
layout(location = 2) in vec3 color;
layout(location = 3) in float lifeTime;
layout(location = 4) in float size;
layout(location = 5) in float type;

out vec3 vPosition;
out vec3 vVelocity;
out vec3 vColor;
out float vLifeTime;
out float vSize;
out float vType;

void main()
{   
	vPosition = position;
	vVelocity = velocity;
	vColor = color;
	vLifeTime = lifeTime;
	vSize = size;
	vType = type;
}