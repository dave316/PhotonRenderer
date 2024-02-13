#version 440 core

layout(points) in;
layout(points, max_vertices = 40) out;

in vec3 vPosition[];
in vec3 vVelocity[];
in vec3 vColor[];
in float vLifeTime[];
in float vSize[];
in float vType[];

out vec3 gPosition;
out vec3 gVelocity;
out vec3 gColor;
out float gLifeTime;
out float gSize;
out float gType;

uniform vec3 genPosition;
uniform vec3 genGravity;
uniform vec3 genVelocityMin;
uniform vec3 genVelocityRange;
uniform vec3 genColor;
uniform float genSize;
uniform float genLifeMin;
uniform float genLifeRange;
uniform float timePassed;

uniform vec3 randomSeed;
vec3 localSeed;

uniform int numToGenerate;

float rand()
{
	uint n = floatBitsToUint(localSeed.y * 214013.0 + localSeed.x * 2531011.0 + localSeed.z * 141251.0);
	n = n * (n * n * 15731u + 789221u);
	n = (n >> 9u) | 0x3F800000u;

	float result = 2.0 - uintBitsToFloat(n);
	localSeed = vec3(localSeed.x + 147158.0 * result, localSeed.y * result + 415161.0 * result, localSeed.z + 324154.0 * result);
	return result;
}

void main()
{
	localSeed = randomSeed;

	gPosition = vPosition[0];
	gVelocity = vVelocity[0];
	gLifeTime = vLifeTime[0];
	if(vType[0] != 0.0)
	{
		gPosition += vec3(sin(gLifeTime), 0.0, cos(gLifeTime)) * 0.01;
		gPosition += gVelocity * timePassed;
		gVelocity += genGravity * timePassed;
	}
	gColor = vColor[0];
	gLifeTime = gLifeTime - timePassed;
	gSize = vSize[0];
	gType = vType[0];

	if(gType == 0.0)
	{
		EmitVertex();
		EndPrimitive();

		for(int i = 0; i < numToGenerate; i++)
		{
			gPosition = genPosition;
			gVelocity = genVelocityMin + vec3(genVelocityRange.x * rand(), genVelocityRange.y * rand(), genVelocityRange.z * rand());
			gColor = genColor;
			gLifeTime = genLifeMin + genLifeRange * rand();
			gSize = genSize;
			gType = 1;

			EmitVertex();
			EndPrimitive();
		}
	}
	else if(gLifeTime > 0.0)
	{
		EmitVertex();
		EndPrimitive();
	}
}