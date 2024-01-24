
#define MAX_DYNAMIC_LIGHTS 5

struct Light
{
	vec3 position; // used for every light type except directional lights
	vec3 direction; // light direction for dir/spot lights, plane normal for disk/rect lights
	vec3 up;
	vec3 left;
	vec3 color;
	float intensity;
	float range;
	float angleScale; // for spot lights
	float angleOffset; // for spot lights
	float radius; // radius used for sphere, disk and tube lights
	float width; // width used for tube and rect lights
	float height; // only used for rect lights
	int type; // 0 - directional, 1 - point, 2 - spot, 3 - area
	int shape; // 0 - sphere, 1 - disk, 2 - tube, 3 - rect
	int iesProfile;
	bool on;
	mat4 worldToLight;
};

layout(std140, binding = 1) uniform LightUBO
{
	Light lights[MAX_DYNAMIC_LIGHTS];
};

// TODO: put in texture array to save units
uniform int numLights;
uniform sampler2DArray iesProfile;
uniform samplerCubeArrayShadow shadowMaps;

#define MAX_CASCADES 8

// CSM
uniform mat4 lightSpaceMatrices[MAX_CASCADES];
uniform float cascadePlaneDistance[MAX_CASCADES];
uniform int cascadeCount;
uniform sampler2DArray shadowCascades; // TODO: replace with shadow sampler

// Lightmaps
uniform sampler2DArray lightMaps;
uniform sampler2DArray directionMaps;

float PIPI = 3.14159265358979323846;

float illuminanceDiskSphere(float cosTheta, float sinSigmaSqr)
{
	float cosThetaSqr = cosTheta * cosTheta;
	float sinTheta = sqrt(1.0 - cosThetaSqr);
	float illuminance = 0.0;
	if(cosThetaSqr > sinSigmaSqr)
		illuminance = PIPI * sinSigmaSqr * clamp(cosTheta, 0.0, 1.0);
	else
	{
		float x = sqrt(1.0 / sinSigmaSqr - 1.0);
		float y = -x * (cosTheta / sinTheta);
		float sinThetaSqrtY = sinTheta * sqrt(1.0 - y * y);
		illuminance = (cosTheta * acos(y) - x * sinThetaSqrtY) * sinSigmaSqr + atan(sinThetaSqrtY / x);
		illuminance = max(illuminance, 0.0);
	}
	return illuminance;
}

float rectangleSolidAngle(vec3 p, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	vec3 v0 = p0 - p;
	vec3 v1 = p1 - p;
	vec3 v2 = p2 - p;
	vec3 v3 = p3 - p;

	vec3 n0 = normalize(cross(v0, v1));
	vec3 n1 = normalize(cross(v1, v2));
	vec3 n2 = normalize(cross(v2, v3));
	vec3 n3 = normalize(cross(v3, v0));

	float g0 = acos(dot(-n0, n1));
	float g1 = acos(dot(-n1, n2));
	float g2 = acos(dot(-n2, n3));
	float g3 = acos(dot(-n3, n0));

	return g0 + g1 + g2 + g3 - 2.0 * PIPI;
}

vec3 closestPointOnLine(vec3 a, vec3 b, vec3 c)
{
	vec3 ab = b - a;
	float t = dot(c - a, ab) / dot(ab, ab);
	return a + t * ab;
}

vec3 closestPointOnSegment(vec3 a, vec3 b, vec3 c)
{
	vec3 ab = b - a;
	float t = dot(c - a, ab) / dot(ab, ab);
	return a + clamp(t, 0.0, 1.0) * ab;
}

vec3 getIntensity(Light light, vec3 p, vec3 n, vec3 lightDir)
{
	if (light.shape == 1) // Sphere
	{
		vec3 l = normalize(lightDir);
		float cosTheta = clamp(dot(n, l), -0.999, 0.999);
		float sqrDist = dot(lightDir, lightDir);
		float sqrLightRadius = light.radius * light.radius;
		float sinSigmaSqr = min(sqrLightRadius / sqrDist, 0.999);
		float illuminance = illuminanceDiskSphere(cosTheta, sinSigmaSqr);
		return light.color * light.intensity * illuminance;
	}
	else if (light.shape == 2) // Disk
	{
		vec3 l = normalize(lightDir);
		float cosTheta = dot(n, l);
		float sqrDist = dot(lightDir, lightDir);
		float sqrLightRadius = light.radius * light.radius;
		float sinSigmaSqr = sqrLightRadius / (sqrLightRadius + max(sqrLightRadius, sqrDist));
		float illuminance = illuminanceDiskSphere(cosTheta, sinSigmaSqr) * clamp(dot(light.direction, -l), 0.0, 1.0);
		return light.color * light.intensity * illuminance;
	}
	else if (light.shape == 3) // Tube
	{
		float illuminance = 0.0;
		vec3 l = normalize(lightDir);
		float halfW = light.width * 0.5;
		vec3 P0 = light.position + light.left * halfW;
		vec3 P1 = light.position - light.left * halfW;

		vec3 forward = normalize(closestPointOnLine(P0, P1, p) - p);
		vec3 left = light.left;
		vec3 up = cross(left, forward);
		vec3 p0 = light.position - left * halfW + light.radius * up;
		vec3 p1 = light.position - left * halfW - light.radius * up;
		vec3 p2 = light.position + left * halfW - light.radius * up;
		vec3 p3 = light.position + left * halfW + light.radius * up;

		float solidAngle = rectangleSolidAngle(p, p0, p1, p2, p3);
		illuminance = solidAngle * 0.2 * (
			clamp(dot(normalize(p0 - p), n), 0.0, 1.0) +
			clamp(dot(normalize(p1 - p), n), 0.0, 1.0) +
			clamp(dot(normalize(p2 - p), n), 0.0, 1.0) +
			clamp(dot(normalize(p3 - p), n), 0.0, 1.0) +
			clamp(dot(l, n), 0.0, 1.0));

		vec3 spherePosition = closestPointOnSegment(P0, P1, p);
		vec3 sphereUnormL = spherePosition - p ;
		vec3 sphereL = normalize(sphereUnormL);
		float sqrSphereDistance = dot(sphereUnormL, sphereUnormL);

		float illuminanceSphere = PIPI * clamp(dot(sphereL, n), 0.0, 1.0) * ((light.radius * light.radius) / sqrSphereDistance);
		illuminance += illuminanceSphere;
		return light.color * light.intensity * illuminance;
	}
	else if (light.shape == 4) // Rectangle
	{
		float illuminance = 0.0;
		vec3 l = normalize(lightDir);
		if(dot(-l, light.direction) > 0.0)
		{
			float halfW = light.width * 0.5;
			float halfH = light.height * 0.5;
			vec3 p0 = light.position + light.left * -halfW + light.up * halfH;
			vec3 p1 = light.position + light.left * -halfW + light.up * -halfH;
			vec3 p2 = light.position + light.left * halfW + light.up * -halfH;
			vec3 p3 = light.position + light.left * halfW + light.up * halfH;
			float solidAngle = rectangleSolidAngle(p, p0, p1, p2, p3); // This can be precomputed
			illuminance = solidAngle * 0.2 * (
				clamp(dot(normalize(p0 - p), n), 0.0, 1.0) +
				clamp(dot(normalize(p1 - p), n), 0.0, 1.0) +
				clamp(dot(normalize(p2 - p), n), 0.0, 1.0) +
				clamp(dot(normalize(p3 - p), n), 0.0, 1.0) +
				clamp(dot(l, n), 0.0, 1.0)
			);
		}
		return light.color * light.intensity * illuminance;
	}
	else // punctual light
		return light.color * light.intensity;
}

float getAttenuation(Light light, vec3 lightDir)
{
	float rangeAttenuation = 1.0;
	float spotAttenuation = 1.0;

	if (light.type != 0)
	{
		float dist = length(lightDir);
		if (light.range < 0.0)
			rangeAttenuation = 1.0 / pow(dist, 2.0);
		else
		{
			float invSquareRange = 1.0 / (light.range * light.range); // can be precomputed on CPU
			float squaredDistance = dot(lightDir, lightDir);
			float factor = squaredDistance * invSquareRange;
			float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
			float attenuation = 1.0 / max(squaredDistance, 0.0001);
			rangeAttenuation = attenuation * smoothFactor * smoothFactor;
			//rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
		}			
	}

	if (light.type == 2)
	{
		float cd = dot(normalize(light.direction), normalize(-lightDir));
		float attenuation = clamp(cd * light.angleScale + light.angleOffset, 0.0, 1.0);
		spotAttenuation = attenuation * attenuation;
		//float cosAngle = dot(normalize(light.direction), normalize(-lightDir));
		//spotAttenuation = 0.0;
		//if (cosAngle > cosOuter)
		//{
		//	spotAttenuation = 1.0;
		//	if (cosAngle < cosInner)
		//		spotAttenuation = smoothstep(cosOuter, cosInner, cosAngle);
		//}
	}

	if (light.iesProfile >= 0)
	{
		vec3 iesSampleDir = mat3(light.worldToLight) * normalize(-lightDir);
		float phi = iesSampleDir.z * 0.5 + 0.5;
		float theta = atan(iesSampleDir.x, iesSampleDir.y) / (2.0 * PIPI) + 0.5;
		vec3 texCoord = vec3(phi, theta, light.iesProfile);
		rangeAttenuation *= texture(iesProfile, texCoord).r;
	}

	return rangeAttenuation * spotAttenuation;
}

float getPointShadow(vec3 fragPos, int index)
{
	Light light = lights[index];
	vec3 f = fragPos - light.position;
	float len = length(f);
	float shadow = 0.0;
	float radius = 0.01;
	float depth = (len / light.range) - 0.005; // TODO: add to light properties

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				vec3 offset = vec3(x, y, z);
				vec3 uvw = f + offset * radius;
				//shadow += texture(shadowMaps[index], vec4(uvw, depth));
				shadow += texture(shadowMaps, vec4(uvw, index), depth);
			}
		}
	}
	return shadow / 27.0;
}

float getDirectionalShadow(vec4 fragPos, float NoL)
{
	vec3 projCoords = fragPos.xyz / fragPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float shadow = 0.0;
	float bias = max(0.001 * (1.0 - NoL), 0.0001);
	vec2 texelSize = 1.0 / vec2(textureSize(shadowCascades, 0));
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			// TODO: add index to layer
			float depth = texture(shadowCascades, vec3(projCoords.xy + vec2(x, y) * texelSize, 0)).r;
			shadow += (projCoords.z - bias) > depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;

	return 1.0 - shadow;
}

float getDirectionalShadowCSM(mat4 V, vec3 wPosition, float NoL, float zFar)
{
	vec4 vPosition = V * vec4(wPosition, 1.0);
	float depth = abs(vPosition.z);
	int layer = -1;
	for (int c = 0; c < cascadeCount; c++)
	{
		if(depth < cascadePlaneDistance[c])
		{
			layer = c;
			break;
		}
	}
	if(layer == -1)
		//return 1.0;
		layer = cascadeCount;

	vec4 lPosition = lightSpaceMatrices[layer] * vec4(wPosition, 1.0);
	vec3 projCoords = lPosition.xyz / lPosition.w;
	projCoords = projCoords * 0.5 + 0.5;

	float currentDepth = projCoords.z;
	if(currentDepth > 1.0)
		return 0.0;

	float bias = max(0.01 * (1.0 - NoL), 0.001);
	float biasMod = 0.5;
	if(layer == cascadeCount)
		bias *= 1.0 / (zFar * biasMod);
	else
		bias *= 1.0 / cascadePlaneDistance[layer] * biasMod;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowCascades, 0));
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture(shadowCascades, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
			shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return 1.0 - shadow;
}