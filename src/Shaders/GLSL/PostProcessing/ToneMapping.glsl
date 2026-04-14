struct ReflectionProbe
{
	vec4 position;
	vec4 boxMin;
	vec4 boxMax;
	int index;
};

#define MAX_REFLECTION_PROBES 15

#ifdef USE_OPENGL
layout(std140, binding = 4) uniform ReflectionProbeUBO
#else
layout(std140, set = 5, binding = 0) uniform ReflectionProbeUBO
#endif
{
	ReflectionProbe probes[MAX_REFLECTION_PROBES];
};

#ifdef USE_OPENGL
layout(binding = 12) uniform samplerCube irradianceMap;
layout(binding = 13) uniform samplerCubeArray specularMapGGX;
layout(binding = 14) uniform samplerCube specularMapSheen;
layout(binding = 15) uniform sampler2D brdfLUT;
#else
layout(set = 5, binding = 1) uniform samplerCube irradianceMap;
layout(set = 5, binding = 2) uniform samplerCubeArray specularMapGGX;
layout(set = 5, binding = 3) uniform samplerCube specularMapSheen;
layout(set = 5, binding = 4) uniform sampler2D brdfLUT;
#endif

