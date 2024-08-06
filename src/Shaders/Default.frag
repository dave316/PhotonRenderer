#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in flat int instanceID;
layout(location = 6) in vec3 tangent;
layout(location = 7) in mat3 wTBN;

struct FSInput
{
	vec3 wPosition;
	vec4 vertexColor;
	vec3 wNormal;
	vec2 texCoord0;
	vec2 texCoord1;
	mat3 wTBN;
};

struct SurfacePoint
{
	vec4 baseColor;
	vec3 shadingNormal;
	float roughness;
	float metallic;
};

void evalMaterial(in FSInput i, inout SurfacePoint o)
{
	
}

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 grabColor;
layout(location = 2) out vec4 brightColor;

struct ModelData
{
	mat4 M;
	mat4 N;
	int animationMode; // 0 - no animation, 1 - vertex skinning, 2 - morph targets
};

layout(std140, binding = 2) uniform ModelUBO
{
	ModelData model;
};


#include "Utils.glsl"
#include "Camera.glsl"
#include "IBL.glsl"
#include "Material.glsl"
#include "Light.glsl"
#include "BRDF.glsl"
#include "HDR.glsl"

#ifdef IRIDESCENCE
#include "iridescence.glsl"
#endif

uniform bool useIBL = false;
uniform bool computeFlatNormals = false;
uniform float maxLuminance = 0.0;
uniform	vec3 scattering = vec3(1.0);
uniform float absorption = 0.1;
uniform float phase = 0;
uniform vec3 minBox;
uniform vec3 maxBox;
uniform sampler3D accumFogTex;
uniform sampler3D inScatteringTex;
uniform sampler3D fogMaterialTex;
uniform float scale;
uniform float bias;

uniform float deltaTime = 0.0;

uniform bool castShadows = true;

#ifdef DEBUG_OUTPUT
uniform int debugChannel = 0;
#endif

#ifdef GLASS_MATERIAL
vec4 refraction(vec3 wPosition, vec3 wNormal)
{
	vec4 screenPos = camera.VP * vec4(wPosition, 1.0);
	float scale = 1.0; // ??
	float halfPosW = screenPos.w * 0.5;
	float projectionParamsX = 1.0;
	screenPos.y = (screenPos.y - halfPosW) * projectionParamsX * scale + halfPosW;
	
	vec2 projScreenPos = screenPos.xy / screenPos.w;
	projScreenPos = projScreenPos * 0.5 + 0.5;
	vec3 wViewDir = normalize(camera.position - wPosition);
	vec3 refractionOffset = ((((material.indexOfRefraction - 1.0) * vec3(camera.V * vec4(wNormal, 0.0))) * (1.0 / (screenPos.z + 1.0))) * (1.0 - dot(wNormal, wViewDir)));
	vec2 cameraRefraction = vec2(refractionOffset.x, refractionOffset.y);
	vec4 redAlpha = texture(screenTex, projScreenPos + cameraRefraction);
	float green = texture(screenTex, projScreenPos + (cameraRefraction * (1.0 - material.chromaticAberration))).g;
	float blue = texture(screenTex, projScreenPos + (cameraRefraction * (1.0 + material.chromaticAberration))).b;
	return vec4(redAlpha.r, green, blue, redAlpha.a);
}
#endif

float g_HenyeyGreenstein(float cosPhi, float phase)
{
	float g = phase;
	float g2 =  g * g;
	return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - (2.0 * g) * cosPhi, 1.5));
}

vec3 computeVolumetricLight()
{
	if(numLights == 0)
		return vec3(0);

	Light light = lights[0]; // TODO: check all active lights...

	int numberSteps = 128;

	vec3 endPosition = wPosition;
	vec3 startPosition = camera.position;
	vec3 rayVector = endPosition - startPosition;
	float rayLength = length(rayVector);
	//if(rayLength > 100)
	//	return vec3(0);
	vec3 rayDirection = rayVector / rayLength;
	float stepLength = rayLength / float(numberSteps);
	vec3 stepVec = rayDirection * stepLength;
	vec3 currentPos = startPosition;
	vec3 fog = vec3(0);

	// TODO: this is fine for a directional light, but not for other light types
	float lightDotView = dot(rayDirection, -light.direction);
	vec3 lightIntensity = light.color * light.intensity;
	vec3 scatteredLight = g_HenyeyGreenstein(lightDotView, 0) * lightIntensity; 

	for(int i = 0; i < numberSteps; i++)
	{
		vec4 vPosition = camera.V * vec4(currentPos, 1.0);
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
			layer = cascadeCount;

		vec4 lPosition = lightSpaceMatrices[layer] * vec4(currentPos, 1.0);
		vec3 projCoords = lPosition.xyz / lPosition.w;
		projCoords = projCoords * 0.5 + 0.5;

		float shadowValue = texture(shadowCascades, vec3(projCoords.xy, layer)).r;
		if(projCoords.z < shadowValue) // not in shadowCascades
			fog += scatteredLight;

		currentPos += stepVec;
	}

	fog /= float(numberSteps);
	vec3 attCoeff = -log(vec3(0.9)) / 1000.0;
	vec3 scatterCoeff = vec3(1);
	vec3 transmittance = fog * exp(attCoeff * scatterCoeff * rayLength);
	return transmittance * 0.25;
}

//vec3 computeVolumeTransmittance(vec3 surfaceRadiance)
//{
//	int numberSteps = 64;
//	if(numLights == 0)
//		return vec3(0);
//
//	Light light = lights[0]; // TODO: check all active lights...
//
//	vec3 viewVec = camera.position - wPosition;
//	vec3 viewDir = normalize(viewVec);
//	float s = length(viewVec);
//	float stepLength = s / float(numberSteps);
//
//	//float g = g_HenyeyGreenstein(dot(viewDir, -light.direction), phase);
//	vec3 transmittance = surfaceRadiance * exp(-s * absorption);
//	vec3 samplePos = wPosition;
//	for(float l = s - stepLength; l >= 0; l -= stepLength)
//	{
//		samplePos += viewDir * stepLength;
//		float d = length(light.position - samplePos);
////		float d = 1.0;
////		vec4 vPosition = camera.V * vec4(samplePos, 1.0);
////		float depth = abs(vPosition.z);
////		int layer = -1;
////		for (int c = 0; c < cascadeCount; c++)
////		{
////			if(depth < cascadePlaneDistance[c])
////			{
////				layer = c;
////				break;
////			}
////		}
////		if(layer == -1)
////			layer = cascadeCount;
////
////		vec4 lPosition = lightSpaceMatrices[layer] * vec4(samplePos, 1.0);
////		vec3 projCoords = lPosition.xyz / lPosition.w;
////		projCoords = projCoords * 0.5 + 0.5;
////		float shadowDepth = texture(shadowCascades, vec3(projCoords.xy, layer)).r;
////		float visibility = float(projCoords.z < shadowDepth);
//
//		vec3 f = samplePos - light.position;
//		float len = length(f);
//		float depth = (len / light.range) - 0.0001; // TODO: add to light properties
//		float visibility = 1.0;// texture(shadowMaps[0], vec4(samplePos - light.position, depth));
//
//		vec3 lightIntensity = light.color * light.intensity;
//		
//		// TODO: add distance to light for local lights
//		float g = g_HenyeyGreenstein(dot(viewDir, normalize(light.position - samplePos)), phase);
//		vec3 Lin = exp(-d * absorption) * visibility * lightIntensity * getAttenuation(light, light.position - samplePos);
//		vec3 Li = Lin * absorption * scattering * g;
//		transmittance += Li * exp(-l * absorption) * stepLength;
//	}
//	return transmittance;
//}

vec3 computeVolumeTransmittance(vec3 surfaceRadiance)
{
	int numberSteps = 64;
	if(numLights == 0)
		return vec3(0);

	vec3 viewVec = camera.position - wPosition;
	vec3 viewDir = normalize(viewVec);
	float s = length(viewVec);
	float stepLength = s / float(numberSteps);

	//float g = g_HenyeyGreenstein(dot(viewDir, -light.direction), phase);
	vec3 transmittance = surfaceRadiance * exp(-s * absorption);
	vec3 samplePos = wPosition;
	for(float l = s - stepLength; l >= 0; l -= stepLength)
	{
		samplePos += viewDir * stepLength;

		float scatterFog = 20.0 * texture(fogMaterialTex, samplePos * 0.1 + vec3(0,0,deltaTime * 0.1)).r;
		float extinction = absorption + scatterFog;

		vec3 Li = vec3(0);
		for(int i = 0; i < numLights; i++)
		{
			Light light = lights[i];

			vec3 viewVec = camera.position - samplePos;
			vec3 lightVec = light.position - samplePos;
			float lightDist = length(lightVec);
			vec3 l = lightVec / lightDist;
			vec3 v = normalize(viewVec);

			//float zValue = float(globalIndex.z) / float(gl_NumWorkGroups.z);
			vec3 f = -lightVec;
			float bias = 0.0001;
			float depth = (lightDist / light.range) - bias; // TODO: add to light properties
			float shadow = 0.0;
			float radius = 0.005;
			for (int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					for (int z = -1; z <= 1; z++)
					{
						vec3 offset = vec3(x, y, z);
						vec3 uvw = f + offset * radius;
						shadow += texture(shadowMaps, vec4(uvw, i), depth);
					}
				}
			}
			float visibility = shadow / 27.0;
			//float visibility = texture(shadowMaps, vec4(f, i), depth);

			vec3 lightIntensity = light.color * light.intensity;
			float g = g_HenyeyGreenstein(dot(v, l), phase);
			//vec3 Lin = exp(-lightDist * absorption) * visibility * lightIntensity * getAttenuation(light, lightVec);
			vec3 Lin = visibility * lightIntensity * getAttenuation(light, lightVec);
			Li += Lin * scatterFog * g;
		}

		transmittance += Li * exp(-l * extinction) * stepLength;
	}

	return transmittance;
}

float linearDepth(float depth)
{
	float ndc = 2.0 * depth - 1.0;
	float zNear = 0.1;
	float zFar = 25;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - ndc * (zFar - zNear));
    return linear;
}

vec3 computeVolumeTransmittancePrecomputedInscattering(vec3 surfaceRadiance)
{
	int numberSteps = 32;
//	if(numLights == 0)
//		return vec3(0);

	Light light = lights[0]; // TODO: check all active lights...

	vec3 viewVec = camera.position - wPosition;
	vec3 viewDir = normalize(viewVec);
	float s = length(viewVec);
	float stepLength = s / float(numberSteps);

	//float g = g_HenyeyGreenstein(dot(viewDir, -light.direction), phase);
	vec3 transmittance = surfaceRadiance * exp(-s * absorption);
	vec3 samplePos = wPosition;

//	vec4 viewPos = camera.V * vec4(samplePos, 1.0);
//	vec4 clipPos = camera.P * viewPos; // clip space 
//	vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
//	vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
//
//	float zNear = 0.1;
//	float zFar = 25.0;
//	float scale = 1.0 / log2(zFar / zNear);
//	float bias = -(log2(zNear) * scale);
//	normalizedCoords.z = clamp(log2(-viewPos.z)*scale+bias, 0.0, 1.0);
//
//	int index = int(normalizedCoords.z * 128.0) % 8;
//	switch(index)
//	{
//		case 0: transmittance = vec3(0,0,0); break;
//		case 1: transmittance = vec3(1,0,0); break;
//		case 2: transmittance = vec3(0,1,0); break;
//		case 3: transmittance = vec3(0,0,1); break;
//		case 4: transmittance = vec3(1,1,0); break;
//		case 5: transmittance = vec3(0,1,1); break;
//		case 6: transmittance = vec3(1,0,1); break;
//		case 7: transmittance = vec3(1,1,1); break;
//	}

	for(float l = s - stepLength; l >= 0; l -= stepLength)
	{
		//vec2 xi = Hammersley(index, 256);
		//index++;
		//float d = stepLength * xi.y;
		samplePos += viewDir * stepLength; // TODO: convert world position to clipspace volume
		vec4 viewPos = camera.V * vec4(samplePos, 1.0);
		vec4 clipPos = camera.P * vec4(viewPos.xyz, 1.0); // clip space 
		vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
		vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
		float zNear = camera.zNear;
		float zFar = camera.zFar;
//		float scale = 1.0 / log2(zFar / zNear);
//		float bias = -(log2(zNear) * scale);
		//normalizedCoords.z = clamp(log2(linearDepth(normalizedCoords.z))*scale+bias, 0.0, 1.0);
		normalizedCoords.z = clamp(log2(-viewPos.z)*scale+bias, 0.0, 1.0);
		vec3 Li = texture(inScatteringTex, normalizedCoords).rgb;
		transmittance += Li * exp(-l * absorption) * stepLength;
	}
	return transmittance;
}

vec3 applyFogScattering(vec3 fragColor)
{
//	int numberSteps = 32;
//	Light light = lights[0]; // TODO: check all active lights...
//
//	vec3 viewVec = camera.position - wPosition;
//	vec3 viewDir = normalize(viewVec);
//	float s = length(viewVec);
//	float stepLength = s / float(numberSteps);
//
//	vec3 transmittance = fragColor * exp(-s * absorption);
//	vec3 samplePos = wPosition;
//	for(float l = s - stepLength; l >= 0; l -= stepLength)
//	{
//		samplePos += viewDir * stepLength; // TODO: convert world position to clipspace volume
//		vec4 viewPos = camera.V * vec4(samplePos, 1.0);
//		vec4 clipPos = camera.P * vec4(viewPos.xyz, 1.0); // clip space 
//		vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
//		vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
//		float zNear = camera.zNear;
//		float zFar = camera.zFar;
//		normalizedCoords.z = clamp(log2(-viewPos.z)*scale+bias, 0.0, 1.0);
//		vec3 Li = texture(inScatteringTex, normalizedCoords).rgb;
//		transmittance += Li * exp(-l * absorption) * stepLength;
//	}
//	return transmittance;
	vec4 viewPos = camera.V * vec4(wPosition, 1.0);
	vec4 clipPos = camera.P * vec4(viewPos.xyz, 1.0);
	vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
	vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
	normalizedCoords.z = clamp(log2(-viewPos.z)*scale+bias, 0.0, 1.0);

	vec4 scatteringTransmittance = texture(accumFogTex, normalizedCoords);
	vec3 inScattering = scatteringTransmittance.rgb;
	float transmittance = scatteringTransmittance.a;
	vec3 finalColor = fragColor * transmittance + inScattering;
	return finalColor;
}

vec3 computeLocalVolumeTransmittance(vec3 surfaceRadiance)
{
	int numberSteps = 32;
	if(numLights == 0)
		return vec3(0);

	Light light = lights[0]; // TODO: check all active lights...

	vec3 viewVec = camera.position - wPosition;
	vec3 viewDir = normalize(viewVec);

	vec3 bMin = (minBox - camera.position) / -viewDir;
	vec3 bMax = (maxBox - camera.position) / -viewDir;

	vec3 mMin = min(bMin, bMax);
	vec3 mMax = max(bMin, bMax);
	float tmin = max(max(mMin.x, mMin.y), mMin.z);
	float tmax = min(min(mMax.x, mMax.y), mMax.z);
	if (tmax < 0 || tmin > tmax)
		return surfaceRadiance;

	vec3 start = camera.position + tmin * (-viewDir);
	vec3 end = camera.position + tmax * (-viewDir);

	if(length(camera.position - start) > length(viewVec))
		return surfaceRadiance;

	vec3 rayVec = end - start;
	vec3 rayDir = normalize(rayVec);
	float s = length(rayVec);
	float stepLength = s / float(numberSteps);

	vec3 transmittance = surfaceRadiance * exp(-s * absorption);
	vec3 samplePos = start;
	//for(float l = s - stepLength; l >= 0; l -= stepLength)
	for(float l = 0; l < s; l += stepLength)
	{
		samplePos += rayDir * stepLength;
		vec3 normalPos = (samplePos - minBox) / (maxBox - minBox);
		float scatterFog = texture(inScatteringTex, normalPos + vec3(0,0,deltaTime * 0.15)).r;
		//vec3 scatterFog = texture(fogMaterial, normalPos).rgb;
		//transmittance += vec3(0.1);
		float d = length(light.position - samplePos);
//		float d = 1.0;
//		vec4 vPosition = camera.V * vec4(samplePos, 1.0);
//		float depth = abs(vPosition.z);
//		int layer = -1;
//		for (int c = 0; c < cascadeCount; c++)
//		{
//			if(depth < cascadePlaneDistance[c])
//			{
//				layer = c;
//				break;
//			}
//		}
//		if(layer == -1)
//			layer = cascadeCount;
//
//		vec4 lPosition = lightSpaceMatrices[layer] * vec4(samplePos, 1.0);
//		vec3 projCoords = lPosition.xyz / lPosition.w;
//		projCoords = projCoords * 0.5 + 0.5;
//		float shadowDepth = texture(shadowCascades, vec3(projCoords.xy, layer)).r;
//		float visibility = float(projCoords.z < shadowDepth);

		vec3 f = samplePos - light.position;
		float len = length(f);
		float depth = (len / light.range) - 0.0001; // TODO: add to light properties
		float visibility =  1.0;//texture(shadowMaps[0], vec4(samplePos - light.position, depth));

//		vec3 irradiance = texture(irradianceMaps, vec4(rayDir,0)).rgb;

		vec3 lightIntensity = light.color * light.intensity;

		vec3 lightDir = light.position - samplePos;
		float g = 1.0;//g_HenyeyGreenstein(dot(viewDir, normalize(lightDir)), phase);
		
		// TODO: add distance to light for local lights
		vec3 Lin = exp(-d * absorption) * visibility * lightIntensity * getAttenuation(light, lightDir);
		vec3 Li = Lin * absorption * scatterFog * g;
		transmittance += Li * exp(-l * absorption) * stepLength;
	}

	return transmittance;
//	vec3 normalPos = (start - minBox) / (maxBox - minBox);
//	return texture(fogMaterial, normalPos).rgb;
}


void main()
{
//	fragColor = vec4(1);
//	return;

	vec2 uv0 = texCoord0;
	vec2 uv1 = texCoord1;
	vec2 lightUV = uv1;

#ifdef METAL_ROUGH_MATERIAL

#ifdef POM
	vec2 uv = vec2(heightTex.uvTransform * vec3(texCoord0, 1.0));
	vec2 dx0 = dFdx(uv);
	vec2 dy0 = dFdy(uv);
//	vec2 dx1 = dFdx(texCoord1);
//	vec2 dy1 = dFdy(texCoord1);
	vec3 wView = normalize(camera.position - wPosition);
	vec3 tView = inverse(wTBN) * wView;
	vec2 curv = vec2(material.curvatureU, material.curvatureV);
	uv0 = pom(uv, dx0, dy0, wNormal, wView, tView, 128, 128, material.scale, material.refPlane, curv);
	//lightUV = pom(texCoord1, dx1, dy1, wNormal, wView, tView, 128, 128, material.scale, material.refPlane, curv);
	lightUV = uv1;
	uv1 = texCoord0;
#endif
	
#ifdef LAYERED_MATERIAL // blending multiple material layers
	
	vec2 weights = computeWeights(uv0, uv1, vertexColor);

	// base color + alpha
	vec4 baseColor = getBaseColor(uv0, uv1, weights);
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	// emission
	vec3 emission = getEmission(uv0, uv1);

	// metal, roughness, occlusion
	vec3 orm = getPBRValues(uv0, uv1, weights);
	float ao = orm.r; 
	float roughness = clamp(orm.g, 0.05, 1.0);
	float metallic = orm.b;

#else // single layer (default)

	// base color + alpha
#ifdef VELVET_MATERIAL
	vec3 wView = normalize(camera.position - wPosition);
	vec3 tView = inverse(wTBN) * wView;
	vec4 baseColor = vertexColor * getBaseColor(uv0, uv1, tView);
#else
	vec4 baseColor = getBaseColor(uv0, uv1);// * vec4(1.0, 0.8, 0.8, 1.0);
#endif
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	// emission
#ifdef RAIN_REFRACTION
	vec4 screenPos = camera.VP * vec4(wPosition, 1.0);
	vec3 emission = getEmission(uv0, uv1, screenPos);
#else
	vec3 emission = getEmission(uv0, uv1);
#endif

	// unlit material, TODO: put in own #ifdef block
	if(material.unlit)
	{
		vec3 unlitColor = emission + baseColor.rgb;
		float unlitAlpha = baseColor.a;

		if(material.alphaMode == 0 || material.alphaMode == 1)
			fragColor = vec4(unlitColor, 1.0);
		else 
			fragColor = vec4(unlitColor, unlitAlpha);
		return;
	}

	// metal, roughness, occlusion
	vec3 orm = getPBRValues(uv0, uv1);
	float ao = orm.r; 
	float roughness = clamp(orm.g, 0.05, 1.0);
	float metallic = orm.b;
#endif

	// main material parameters
	vec3 black = vec3(0);
	vec3 dielectricSpecular = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
	vec3 c_diffuse = mix(baseColor.rgb, black, metallic);
	vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
	vec3 F90 = vec3(1.0);
	float alphaRoughness = roughness * roughness;
	float specularWeight = material.specularWeight;

	alpha = mix(alpha, 1.0, metallic);

#ifdef SPECULAR
	float specularFactor = getSpecular(uv0, uv1);
	vec3 specularColor = getSpecularColor(uv0, uv1);
	vec3 dielectricSpecularF0 = min(F0 * specularColor, vec3(1.0));
	F0 = mix(dielectricSpecularF0, baseColor.rgb, metallic);
	F90 = vec3(mix(specularFactor, 1.0, metallic));
	specularWeight = specularFactor;
	c_diffuse = mix(baseColor.rgb, black, metallic);
#endif
#else
	vec4 diffuseColor = getDiffuseColor(uv0, uv1);
	vec4 baseColor = diffuseColor;
	float alpha = diffuseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	// emission
	vec3 emission = getEmission(uv0, uv1);

	vec3 orm = getPBRValues(uv0, uv1);
	float ao = orm.r;

	vec4 specularColor = getSpecGloss(uv0, uv1);
	vec3 F0 = specularColor.rgb;
	vec3 F90 = vec3(1.0);
	vec3 c_diffuse = diffuseColor.rgb * (1.0 - max3(F0));
	float roughness = 1.0 - specularColor.a;
	float alphaRoughness = roughness * roughness;
	float specularWeight = 1.0f; 
#endif

	// compute shading normal
	vec3 n = normalize(wNormal);
	if (computeFlatNormals)
		n = normalize(cross(dFdx(wPosition), dFdy(wPosition)));

//		vec2 UV = uv0;
//		vec3 uv_dx = dFdx(vec3(UV, 0.0));
//		vec3 uv_dy = dFdy(vec3(UV, 0.0));
//	
//		vec3 t_ = (uv_dy.t * dFdx(wPosition) - uv_dx.t * dFdy(wPosition)) / (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);
//			
//		vec3 t = normalize(t_ - n * dot(n, t_));
//		vec3 b = cross(n, t);



#ifdef LAYERED_MATERIAL
#ifdef SPECGLOSS
	n = normalize(wTBN * getNormal(uv0, uv1));
#else
	n = normalize(wTBN * getNormal(uv0, uv1, weights));
#endif
#else
#if defined (RAIN_MATERIAL) || defined(RAIN_MATERIAL_POM)
	n = normalize(wTBN * getNormal(uv0, uv1, vertexColor));
#else
	if(normalTex.use)
		n = normalize(wTBN * getNormal(uv0, uv1));
	if(gl_FrontFacing == false)
		n = -n;	
#endif
#endif

	// view vector
	vec3 v = normalize(camera.position - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NoV = clampDot(n, v);

#ifdef SHEEN // sheen
	vec3 sheenColor = getSheenColor(uv0, uv1);
	float sheenRoughness = getSheenRoughness(uv0, uv1);
	sheenRoughness = max(sheenRoughness, 0.07);
#endif

#ifdef CLEARCOAT // clearcoat
	vec3 clearCoatNormal = normalize(wNormal);
	if(clearCoatNormalTex.use)
	{
		vec3 tNormal = getTexel(clearCoatNormalTex, uv0, uv1).rgb * 2.0 - 1.0;
		clearCoatNormal = wTBN * normalize(tNormal);
	}
	if(gl_FrontFacing == false)
		clearCoatNormal = -clearCoatNormal;

	float clearCoatFactor = getClearCoat(uv0, uv1);
	float clearCoatRoughness = getClearCoatRoughness(uv0, uv1);
	float clearcoatNoV = clampDot(clearCoatNormal, v);
	vec3 clearCoatFresnel = F_Schlick(F0, F90, clearcoatNoV);
#endif

#ifdef TRANSMISSION // TODO: seperate thin and thick transmission
	float transmissionFactor = getTransmission(uv0, uv1);
	float thickness = getThickness(uv0, uv1);
#endif

#ifdef IRIDESCENCE
	float iridescenceFactor = getIridescence(uv0, uv1);
	float iridescenceThickness = 0.0;
	vec3 iridescenceFresnel = F0;
	vec3 iridescenceF0 = F0;
	if (iridescenceFactor > 0.0)
	{
		float topIOR = 1.0; // TODO: add clearcoat factor
		float iridescenceIOR = material.iridescenceIor;
		iridescenceThickness = getIridescenceThickness(uv0, uv1);
		iridescenceFresnel = evalIridescence(topIOR, iridescenceIOR, NoV, iridescenceThickness, F0);
		iridescenceF0 = Schlick2F0(iridescenceFresnel, F90, NoV);
	}
#ifdef DEBUG_OUTPUT
	if(debugChannel == 7)
		iridescenceFactor = 0.0;
#endif
#endif
float anisotropy = 0.0;
vec3 anisotropyDirection = vec3(0);
#ifdef ANISOTROPY
//	vec3 uv_dx = dFdx(vec3(uv0, 0.0));
//	vec3 uv_dy = dFdy(vec3(uv0, 0.0));
//	vec3 t_ = (uv_dy.t * dFdx(wPosition) - uv_dx.t * dFdy(wPosition)) / (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);
//	vec3 tangent = normalize(t_ - n * dot(n, t_));
//	vec3 bitangent = cross(n,tangent);

	anisotropy = getAnisotropy(uv0, uv1);
	anisotropyDirection = getAnisotropyDirection(uv0, uv1);
	vec3 t = normalize(wTBN * anisotropyDirection);
	vec3 b = normalize(cross(n, t));
#endif

	vec3 f_diffuse = vec3(0);
	vec3 f_specular = vec3(0);
	vec3 f_emissive = emission;
	vec3 f_sheen = vec3(0);
	vec3 f_clearcoat = vec3(0);
	vec3 f_transmission = vec3(0);
	vec3 f_translucency = vec3(0);

	float albedoScaling = 1.0;
	
	// image based lights
	if(useIBL)
	{
		vec3 F0_diff = F0;
		vec3 F_ambient = F_Schlick_Rough(F0, NoV, roughness);
		vec3 F_diff = F_ambient;
		vec3 F_spec = F_ambient;

#ifdef IRIDESCENCE
		vec3 iridescenceF0Max = vec3(max3(iridescenceF0));
		F0_diff = mix(F0, iridescenceF0Max, iridescenceFactor);
		F_diff = F_Schlick_Rough(F0_diff, NoV, roughness);
		F_spec = mix(F_ambient, iridescenceFresnel, iridescenceFactor);
#endif
#ifdef ANISOTROPY
		vec3 anisotropyDir = anisotropy >= 0.0 ? b : t;
		vec3 anisotropyTangent = cross(anisotropyDir, v);
		vec3 anisotropyNormal = cross(anisotropyTangent, anisotropyDir);
		float bendFactor = abs(anisotropy) * clamp(5.0 * roughness, 0.0, 1.0);
		vec3 bendNormal = normalize(mix(n, anisotropyNormal, bendFactor));
		r = normalize(reflect(-v, bendNormal));
#endif

//		if (useLightMap)
//		{
//			vec3 kD = vec3(1.0) - specularWeight * F_diff;
//			lightUV = vec2(lightUV.x, 1.0 - lightUV.y);
//			lightUV = lightUV * lightMapScale + lightMapOffset;
//			lightUV.y = 1.0 - lightUV.y;
//
//			vec2 dx = dFdx(lightUV);
//			vec2 dy = dFdy(lightUV);
//			//vec3 irradiance = textureGrad(lightMaps[lightMapIndex], lightUV, dx, dy).rgb;
//			//vec3 irradiance = texture(lightMaps[lightMapIndex], lightUV).rgb;
//
//			vec3 irradiance = texture(lightMaps, vec3(lightUV, lightMapIndex)).rgb;
//
//			//irradiance = sRGB2Linear(irradiance);
//			vec4 dir = texture(directionMaps, vec3(lightUV, lightMapIndex));
//			vec3 lightDir = dir.xyz * 2.0 - 1.0;
//			float lightIntensity = dir.w;
//
//			vec3 l = normalize(vec3(-lightDir.x, lightDir.y, lightDir.z));
//			float NoL = clampDot(wNormal, l);
//
//			//vec3 irradiance = 6.0 * rgbm.rgb * rgbm.a;
//			f_diffuse = ao * kD * irradiance * c_diffuse * lightIntensity * NoL;
//			//f_diffuse = ao * kD * irradiance * c_diffuse;
//			//f_diffuse = irradiance * lightIntensity * NoL;// * vec3(0.148, 0.198, 0.359);
//		}
//		else if (useSHProbe)
//		{
//			vec3 kD = vec3(1.0) - specularWeight * F_diff;
//			vec3 irradiance = computeIrradianceSHPrescaled(vec3(-n.x,n.y,n.z));
//			f_diffuse = ao * kD * irradiance * c_diffuse;
//			//f_diffuse = irradiance;
//		}
//		else
//			f_diffuse = ao * getIBLRadianceLambert(n, F0_diff, F_diff, c_diffuse, NoV, roughness, specularWeight);
//
//		if(probeIndex == 0)
//			f_specular = ao * getIBLRadianceGGX(r, F_spec, NoV, roughness, specularWeight);
//		else
//			f_specular = ao * getIBLRadianceGGXPrallaxCorrected(wPosition, r, F_spec, NoV, roughness, specularWeight);

		//IBLModelData ibl = iblData[instanceID];
		if (ibl.diffuseMode == 2)
		{
			vec3 kD = vec3(1.0) - specularWeight * F_diff;
			lightUV = vec2(lightUV.x, 1.0 - lightUV.y);
			lightUV = lightUV * ibl.lightMapST.zw + ibl.lightMapST.xy;
			lightUV.y = 1.0 - lightUV.y;

			vec2 dx = dFdx(lightUV);
			vec2 dy = dFdy(lightUV);
			//vec3 irradiance = textureGrad(lightMaps[lightMapIndex], lightUV, dx, dy).rgb;
			//vec3 irradiance = texture(lightMaps[lightMapIndex], lightUV).rgb;

			vec3 irradiance = texture(lightMaps, vec3(lightUV, ibl.lightMapIndex)).rgb;

			//irradiance = sRGB2Linear(irradiance);
			vec4 dir = texture(directionMaps, vec3(lightUV, ibl.lightMapIndex));
			vec3 lightDir = dir.xyz * 2.0 - 1.0;
			float lightIntensity = dir.w;

			vec3 l = normalize(vec3(-lightDir.x, lightDir.y, lightDir.z));
			float NoL = clampDot(wNormal, l);

			f_diffuse = ao * kD * irradiance * c_diffuse * lightIntensity * NoL;
		}
		else if (ibl.diffuseMode == 1)
		{
			vec3 kD = vec3(1.0) - specularWeight * F_diff;
			vec3 irradiance = computeIrradianceSHPrescaled(vec3(-n.x,n.y,n.z), instanceID);
			f_diffuse = ao * kD * irradiance * c_diffuse;
		}
		else
			f_diffuse = ao * getIBLRadianceLambert(n, F0_diff, F_diff, c_diffuse, NoV, roughness, specularWeight);

		if(ibl.specularProbeIndex == 0)
			f_specular = ao * getIBLRadianceGGX(r, F_spec, NoV, roughness, specularWeight);
		else
			f_specular = ao * getIBLRadianceGGXPrallaxCorrected(wPosition, r, F_spec, NoV, roughness, specularWeight, instanceID);
//

#ifdef SHEEN
		f_sheen = ao * getIBLRadianceCharlie(n, v, sheenRoughness, sheenColor);
#endif
#ifdef CLEARCOAT
		vec3 F_ambient_cc = F_Schlick_Rough(F0, clearcoatNoV, clearCoatRoughness);
		vec3 r_cc = normalize(reflect(-v, clearCoatNormal));
		f_clearcoat = ao * getIBLRadianceGGX(r_cc, F_ambient_cc, clearcoatNoV, clearCoatRoughness, 1.0);
#endif
#ifdef TRANSMISSION
		f_transmission = transmissionFactor * getIBLVolumeRefraction(n, v, roughness, baseColor.rgb, F0, F90, wPosition, material.ior, thickness, material.attenuationDistance, material.attenuationColor);
#endif
#ifdef TRANSLUCENCY
		//vec3 irr = computeIrradianceSHPrescaled(-vec3(-n.x,n.y,n.z), instanceID);
		//f_translucency = material.translucencyFactor * irr * material.translucencyColorFactor * c_diffuse;
		f_translucency = material.translucencyFactor * texture(irradianceMaps, vec4(-n, 0)).rgb * material.translucencyColorFactor * c_diffuse;
#endif
	}

	// punctual lights
	for(int i = 0; i < numLights; i++)
	{
		Light light = lights[i];

		if (!light.on)
			continue;

		vec3 lightDir = vec3(0);
		if(light.type == 0)
			lightDir = -light.direction;
		else
			lightDir = light.position - wPosition;

		vec3 l = normalize(lightDir);
		vec3 h = normalize(l + v);
		float NoL = clampDot(n, l);
		float NoH = clampDot(n, h);
		float HoV = clampDot(h, v);

		float shadow = 1.0;
		if(NoL > 0.0 || NoV > 0.0)
		{
			if(castShadows)
			{
				if(light.type == 0)
				{
//					vec4 vPosition = camera.V * vec4(wPosition, 1.0);
//					float depth = abs(vPosition.z);
//					int layer = -1;
//					for (int c = 0; c < cascadeCount; c++)
//					{
//						if(depth < cascadePlaneDistance[c])
//						{
//							layer = c;
//							break;
//						}
//					}
//					if(layer == -1)
//						layer = cascadeCount;
//
//					switch(layer)
//					{
//						case 0: f_diffuse = vec3(0,1,0); break;
//						case 1: f_diffuse = vec3(0,1,1); break;
//						case 2: f_diffuse = vec3(0,0,1); break;
//						case 3: f_diffuse = vec3(1,0,1); break;
//						case 4: f_diffuse = vec3(1,0,0); break;
//					}

					shadow = getDirectionalShadowCSM(camera.V, wPosition, NoL, camera.zFar);
					//shadow = getDirectionalShadow(lightPosition, NoL);
				}
				else
					shadow = getPointShadow(wPosition, i);
			}

			vec3 intensity = getIntensity(light, wPosition, n, lightDir) * getAttenuation(light, lightDir);
			vec3 luminance = intensity * NoL * shadow;

			vec3 F = F_Schlick(F0, F90, HoV);
#ifdef IRIDESCENCE
			vec3 iridescenceFresnelMax = vec3(max3(iridescenceFresnel));
			vec3 F_diff = mix(F, iridescenceFresnelMax, iridescenceFactor);
			vec3 F_spec = mix(F, iridescenceFresnel, iridescenceFactor);
#else
			vec3 F_diff = F;
			vec3 F_spec = F;
#endif			
#ifdef ANISOTROPY
		   	vec3 Fr = F_spec * specularWeight * specularGGXAnisotropic(n, l, v, t, b, alphaRoughness, anisotropy);
#else		
			vec3 Fr = F_spec * specularWeight * specularGGX(NoL, NoV, NoH, alphaRoughness);
#endif
			f_diffuse += luminance * (1 - F_diff * specularWeight) * lambert(c_diffuse); 
			f_specular += luminance * Fr;
#ifdef SHEEN
			f_sheen += luminance * SpecularSheen(sheenColor, sheenRoughness, NoL, NoV, NoH);
			albedoScaling = min(1.0 - max3(sheenColor) * E(NoV, sheenRoughness), 1.0 - max3(sheenColor) * E(NoL, sheenRoughness));
#endif
#ifdef CLEARCOAT
			float ccNoL = clampDot(clearCoatNormal, l);
			float ccNoH = clampDot(clearCoatNormal, h);
			float ccHoV = clampDot(clearCoatNormal, v);
			vec3 ccLuminance = intensity * ccNoL * shadow;
			f_clearcoat += ccLuminance * F * specularGGX(ccNoL, ccHoV, ccNoH, alphaRoughness);
#endif
		}
#ifdef TRANSMISSION // TODO: add translucency here, also check the dot products! 
					//		 since transmission should only be computed when looking through the surface!
		if(dot(n,l) < 0.0 && NoV > 0.0)
		{
			vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, material.ior, model.M);
			lightDir -= transmissionRay;
			l = normalize(lightDir);
			vec3 intensity = getIntensity(light, wPosition, n, lightDir) * getAttenuation(light, lightDir);
			vec3 transmittedLight = intensity * getPunctualRadianceTransmission(n, v, l, alphaRoughness, F0, vec3(1.0), c_diffuse, material.ior);
			f_transmission += transmissionFactor * F_Schlick(F0, F90, HoV) * abs(dot(n,l)) * applyVolumeAttenuation(transmittedLight, length(transmissionRay), material.attenuationDistance, material.attenuationColor);
		}
#endif
#ifdef TRANSLUCENCY
		if(dot(n,l) < 0.0 && NoV > 0.0)
		{
			vec3 intensity = getIntensity(light, wPosition, n, lightDir) * getAttenuation(light, lightDir);
			vec3 diffuseTransmissionColor = c_diffuse * material.translucencyColorFactor;
			f_translucency += intensity * abs(dot(n,l)) * shadow * lambert(diffuseTransmissionColor);
		}
#endif
	}

	vec3 diffuse = f_diffuse;

// layer blending
#ifdef TRANSLUCENCY // diffuse BSDF
	diffuse = mix(diffuse, f_translucency, material.translucencyFactor);
#endif
#ifdef TRANSMISSION // specular BTDF
	diffuse = mix(diffuse, f_transmission, transmissionFactor);
#endif
	// default layer: diffuse BRDF + specular dielectric/metallic BRDF
	vec3 color =  diffuse + f_specular;
#ifdef SHEEN // sheen specular BRDF
	color = color * albedoScaling + f_sheen;
#endif
	color += f_emissive;
#ifdef CLEARCOAT // clearcoat dielectric BRDF
	color = color * (1.0 - clearCoatFactor * clearCoatFresnel) + f_clearcoat * clearCoatFactor;
#endif

//#ifdef METAL_ROUGH_MATERIAL
#ifdef GLASS_MATERIAL
	fragColor.rgb = color + vec3(refraction(wPosition, n)) * (1.0 - alpha);
	fragColor.a = 1.0;
	
#else
	//vec3 volumetricFog = computeVolumetricLight();
	//color = computeVolumeTransmittance(color);
	//color = computeVolumeTransmittancePrecomputedInscattering(color);
	//color = applyFogScattering(color);
	//color = computeLocalVolumeTransmittance(color);

	// TODO: check blend function/equation and if premulti alpha is needed
	if(material.alphaMode == 0 || material.alphaMode == 1)
		//fragColor = vec4(baseColor.rgb, 1.0);
		fragColor = vec4(color, 1.0);
		//fragColor = vec4(getNormal(uv0, uv1), 1.0);	
	else 
	{
		//fragColor = vec4(sRGB2Linear(n*0.5+0.5), 1.0);
		color = f_emissive + f_diffuse * alpha + f_specular;
		fragColor = vec4(color, alpha);
		//fragColor = vec4(1,0,1,1);
	}
	grabColor = fragColor;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0.0), 1.0);
#endif

#ifdef DEBUG_OUTPUT 
	if(debugChannel > 0)
		fragColor.a = 1.0;

	switch(debugChannel)
	{
	case 0: break; // No debug, keep frag color
	case 1: fragColor.rgb = sRGB2Linear(vec3(uv0, 0.0)); break;
	case 2: fragColor.rgb = sRGB2Linear(vec3(uv1, 0.0)); break;
	case 3: fragColor.rgb = sRGB2Linear(wNormal * 0.5 + 0.5); break;
	case 4: fragColor.rgb = sRGB2Linear(n * 0.5 + 0.5); break;
	case 5: fragColor.rgb = vec3(ao); break;
	case 6: fragColor.rgb = emission; break;
#ifdef METAL_ROUGH_MATERIAL
	case 7: fragColor.rgb = f_diffuse + f_specular; break;
	case 8: fragColor.rgb = baseColor.rgb; break;
	case 9: fragColor.rgb = vec3(roughness); break;
	case 10: fragColor.rgb = vec3(metallic); break;
#ifdef SHEEN
	case 11: fragColor.rgb = f_sheen; break;
	case 12: fragColor.rgb = sheenColor; break;
	case 13: fragColor.rgb = vec3(sheenRoughness); break;
#endif
#ifdef CLEARCOAT
	case 14: fragColor.rgb = f_clearcoat; break;
	case 15: fragColor.rgb = vec3(clearCoatFactor); break;
	case 16: fragColor.rgb = vec3(clearCoatRoughness); break;
	case 17: fragColor.rgb = sRGB2Linear(clearCoatNormal * 0.5 + 0.5); break;
#endif
#ifdef TRANSMISSION
	case 18: fragColor.rgb = f_transmission; break;
	case 19: fragColor.rgb = vec3(transmissionFactor); break;
	case 20: fragColor.rgb = vec3(thickness); break;
	case 21: fragColor.rgb = material.attenuationColor; break;
#endif
#ifdef SPECULAR
	case 22: fragColor.rgb = f_specular; break;
	case 23: fragColor.rgb = vec3(specularFactor); break;
	case 24: fragColor.rgb = specularColor; break;
#endif
#ifdef IRIDESCENCE
	case 25: fragColor.rgb = iridescenceFresnel; break;
	case 26: fragColor.rgb = vec3(iridescenceFactor); break;
	case 27: fragColor.rgb = vec3(iridescenceThickness / 1200.0); break;
#endif
#endif
	default: fragColor.rgb = vec3(0); break;
	}
#endif // DEBUG_OUTPUT
}