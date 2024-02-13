struct TextureInfo
{
	sampler2D tSampler;
	bool use;
	int uvIndex;
	mat3 uvTransform;
};

vec4 getTexel(in TextureInfo info, vec2 uv0, vec2 uv1)
{
	vec3 uv = vec3(info.uvIndex == 0 ? uv0 : uv1, 1.0);
	uv = info.uvTransform * uv;
	return texture2D(info.tSampler, uv.xy);
}

struct RainMaterial
{
	// PBR base material
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionStrength;
	vec3 emissiveFactor;
	float emissiveStrength;
	int alphaMode;
	float alphaCutOff;
	float specularWeight;

	// rain
	float rainDropsPower;
	float rainDropsTile;
	float rainSpeed;
	float rainMetallic;
	float rainSmoothness;
	float time;

	// pom
	float scale;
	float refPlane;
	float curvFix;
	float curvatureU;
	float curvatureV;

	// misc
	bool unlit;
	float normalScale;
	float ior;
};
uniform RainMaterial material;

// PBR base material textures
uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo heightTex;
uniform TextureInfo metalRoughTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;
uniform TextureInfo maskTex;
uniform TextureInfo sampleTex;

// tf can we reduce the parameters please??
vec2 pom(vec2 uv, vec2 dx, vec2 dy, vec3 wNormal, vec3 wView, vec3 tViewDir, int minSamples, int maxSamples, float parallax, float refPlane, vec2 curv)
{
	vec3 result = vec3(0);
	int stepIndex = 0;
	int numSteps = int(mix(float(maxSamples), float(minSamples), float(dot(wNormal, wView))));
	float layerHeight = 1.0 / numSteps;
	vec2 plane = parallax * (tViewDir.xy / tViewDir.z);
	uv += refPlane * plane;
	vec2 deltaTex = -plane * layerHeight;
	vec2 prevTexOffset = vec2(0);
	float prevRayZ = 1.0f;
	float prevHeight = 0.0f;
	vec2 currTexOffset = deltaTex;
	float currRayZ = 1.0f - layerHeight;
	float currHeight = 0.0f;
	float intersection = 0.0f;
	vec2 finalTexOffset = vec2(0);
	while(stepIndex < numSteps + 1)
	{
		// what does the curv parameter do??
		result.z = dot(curv, currTexOffset * currTexOffset);
		currHeight = textureGrad(heightTex.tSampler, uv + currTexOffset, dx, dy).r * (1 - result.z);
		if(currHeight > currRayZ)
		{
			stepIndex = numSteps + 1;
		}
		else
		{
			stepIndex++;
			prevTexOffset = currTexOffset;
			prevRayZ = currRayZ;
			prevHeight = currHeight;
			currTexOffset += deltaTex;
			currRayZ -= layerHeight * (1 - result.z) * (1 + material.curvFix);
		}
	}
	int sectionSteps = 10;
	int sectionIndex = 0;
	float newZ = 0;
	float newHeight = 0;
	while(sectionIndex < sectionSteps)
	{
		intersection = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
		finalTexOffset = prevTexOffset + intersection * deltaTex;
		newZ = prevRayZ - intersection * layerHeight;
		newHeight = textureGrad(heightTex.tSampler, uv + finalTexOffset, dx, dy).r;
		if(newHeight > newZ)
		{
			currTexOffset = finalTexOffset;
			currHeight = newHeight;
			currRayZ = newZ;
			deltaTex = intersection * deltaTex;
			layerHeight = intersection * layerHeight;
		}
		else
		{
			prevTexOffset = finalTexOffset;
			prevHeight = newHeight;
			prevRayZ = newZ;
			deltaTex = (1 - intersection) * deltaTex;
			layerHeight = (1 - intersection) * layerHeight;
		}
		sectionIndex++;
	}
	if(result.z > 1)
		discard;
	return uv + finalTexOffset;
}

vec4 getBaseColor(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
	{
		vec2 dx = dFdx(uv1);
		vec2 dy = dFdy(uv1);
		baseColor *= textureGrad(baseColorTex.tSampler, uv0, dx, dy);
	}		
	return baseColor;
}

vec3 getNormal(vec2 uv0, vec2 uv1, vec4 vertexColor)
{
	vec2 uv = uv0 * material.rainDropsTile;
	vec2 uv_frac = fract(uv);
	int flipBookTiles = 64;
	float flipBookColOffset = 1.0 / 8.0;
	float flipBookRowOffset = 1.0 / 8.0;
	vec2 flipBookTiling = vec2(flipBookColOffset, flipBookRowOffset);
	int speed = int(material.time * material.rainSpeed);
	int flipBookIndex = speed % flipBookTiles;
	//flipBookIndex += (flipBookIndex < 0) ? flipBookTiles : 0;

	int flipBookLinearIndexToX = flipBookIndex % 8;
	int flipBookLinearIndexToY = flipBookIndex / 8;

	float flipBookOffsetX = float(flipBookLinearIndexToX) * flipBookColOffset;
	float flipBookOffsetY = float(flipBookLinearIndexToY) * flipBookRowOffset;

	vec2 flipBookOffset = vec2(flipBookOffsetX, flipBookOffsetY);
	vec2 flipBookUV = uv_frac * flipBookTiling + flipBookOffset;

	float mask = getTexel(maskTex, uv0, uv1).r * vertexColor.r;
	
	vec2 dx = dFdx(uv1);
	vec2 dy = dFdy(uv1);
	vec3 normal = textureGrad(normalTex.tSampler, uv0, dx, dy).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);

	vec3 sampleNormal = getTexel(sampleTex, flipBookUV, uv1).rgb * 2.0 - 1.0;
	sampleNormal *= vec3(material.rainDropsPower, material.rainDropsPower, 1.0);
	normal = normalize(normal);
	sampleNormal = normalize(sampleNormal);

	return normalize(mix(normal, sampleNormal, mask));
}

vec3 decodeNormal(vec2 uv0, vec2 uv1)
{
	vec2 f = getTexel(normalTex, uv0, uv1).rg * 2.0 - 1.0;
	vec3 normal = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.x));
	float t = saturate(-normal.z);
	normal.x += normal.x >= 0.0 ? -t : t;
	normal.y += normal.y >= 0.0 ? -t : t;
	return normalize(normal);
}

vec3 getEmission(vec2 uv0, vec2 uv1)
{
	vec3 emission = material.emissiveFactor;
	if (emissiveTex.use)
		emission *= getTexel(emissiveTex, uv0, uv1).rgb;
	return emission * material.emissiveStrength;
}

vec3 getPBRValues(vec2 uv0, vec2 uv1)
{
	vec3 orm = vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
	if (metalRoughTex.use)
	{
		vec2 dx = dFdx(uv1);
		vec2 dy = dFdy(uv1);
		orm *= vec3(1.0, textureGrad(metalRoughTex.tSampler, uv0, dx, dy).gb);
	}
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);

	vec3 rainORM = vec3(1.0, 1.0 - material.rainSmoothness, material.rainMetallic);
	float mask = getTexel(maskTex, uv0, uv1).r;
	return mix(orm, rainORM, mask);
}