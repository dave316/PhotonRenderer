
#ifdef USE_OPENGL
layout(std140, binding = 1) uniform ModelUBO
#else
layout(std140, set = 1, binding = 0) uniform ModelUBO
#endif
{
	mat4 localToWorld;
	mat4 localToWorldNormal;
	vec4 weights[MAX_MORPH_TARGETS / 4];
	int animMode;
	int numMorphTargets;
	int irradianceMode;
	int lightMapIndex;
	vec4 lightMapST;
	vec4 sh[9];
	int reflectionProbeIndex;
} model;

vec3 computeRadianceSHPrescaled(vec3 dir)
{
	vec3 irradiance =
		model.sh[0].xyz

		// Band 1
		+ model.sh[1].xyz * (dir.y)
		+ model.sh[2].xyz * (dir.z)
		+ model.sh[3].xyz * (dir.x)

		// Band  2
		+ model.sh[4].xyz * (dir.x * dir.y)
		+ model.sh[5].xyz * (dir.y * dir.z)
		+ model.sh[6].xyz * (3.0 * dir.z * dir.z - 1.0)
		+ model.sh[7].xyz * (dir.x * dir.z)
		+ model.sh[8].xyz * (dir.x * dir.x - dir.y * dir.y);

	return irradiance;
}