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

struct PBRMetalRoughMaterial
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

#ifdef POM
	float scale;
	float refPlane;
	float curvFix;
	float curvatureU;
	float curvatureV;
#endif

#ifdef SHEEN // sheen
	vec3 sheenColorFactor;
	float sheenRoughnessFactor;
#endif

#ifdef CLEARCOAT // clearcoat
	float clearcoatFactor;
	float clearcoatRoughnessFactor;
#endif

#ifdef TRANSMISSION // transmission + volume
	float transmissionFactor;
	float thicknessFactor;
	float attenuationDistance;
	vec3 attenuationColor;
#endif

#ifdef TRANSLUCENCY
	float translucencyFactor;
	vec3 translucencyColorFactor;
#endif

#ifdef SPECULAR // specular
	float specularFactor;
	vec3 specularColorFactor;
#endif

#ifdef IRIDESCENCE // iridescence
	float iridescenceFactor;
	float iridescenceIor;
	float iridescenceThicknessMin;
	float iridescenceThicknessMax;
#endif

#ifdef ANISOTROPY // anisotropy
	float anisotropyFactor;
	vec3 anisotropyDirection;
#endif

	// misc
	bool unlit;
	float normalScale;
	float ior;
};
uniform PBRMetalRoughMaterial material;

//layout(std140, binding = 2) uniform MaterialUBO
//{
//	PBRMetalRoughMaterial material;
//};

// PBR base material textures
uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo metalRoughTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;

#ifdef POM
uniform TextureInfo heightTex;
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
#endif

vec4 getBaseColor(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
#ifdef POM
	if (baseColorTex.use)
	{
		vec2 dx = dFdx(uv1);
		vec2 dy = dFdy(uv1);
		//vec2 uv = vec2(baseColorTex.uvTransform * vec3(uv0, 1.0));
		baseColor *= textureGrad(baseColorTex.tSampler, uv0, dx, dy);
	}
#else
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
#endif
	return baseColor;
}

vec3 getNormal(vec2 uv0, vec2 uv1)
{
#ifdef POM
	vec2 dx = dFdx(uv1);
	vec2 dy = dFdy(uv1);
	//vec2 uv = vec2(normalTex.uvTransform * vec3(uv0, 1.0));
	vec3 normal = textureGrad(normalTex.tSampler, uv0, dx, dy).rgb * 2.0 - 1.0;
	//vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);
#else
	vec3 n = getTexel(normalTex, uv0, uv1).rgb;
	vec3 normal = n * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);
#endif
	return normalize(normal);
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
#ifdef POM
	if (metalRoughTex.use)
	{
		vec2 dx = dFdx(uv1);
		vec2 dy = dFdy(uv1);
		//vec2 uv = vec2(metalRoughTex.uvTransform * vec3(uv0, 1.0));
		orm *= vec3(1.0, textureGrad(metalRoughTex.tSampler, uv0, dx, dy).gb);
	}
	if (occlusionTex.use)
	{
		vec2 dx = dFdx(uv1);
		vec2 dy = dFdy(uv1);
		//vec2 uv = vec2(occlusionTex.uvTransform * vec3(uv0, 1.0));
		orm.r = 1.0 + orm.r * (textureGrad(occlusionTex.tSampler, uv0, dx, dy).r - 1.0);
	}
#else
	if (metalRoughTex.use)
		orm *= vec3(1.0, getTexel(metalRoughTex, uv0, uv1).gb);
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);
#endif
	return vec3(orm.r, orm.g, orm.b);
}

// PBR extensions material textures
#ifdef SHEEN
uniform TextureInfo sheenColortex;
uniform TextureInfo sheenRoughtex;

vec3 getSheenColor(vec2 uv0, vec2 uv1)
{
	vec3 sheenColor = material.sheenColorFactor;
	if (sheenColortex.use)
		sheenColor *= getTexel(sheenColortex, uv0, uv1).rgb;
	return sheenColor;
}

float getSheenRoughness(vec2 uv0, vec2 uv1)
{
	float sheenRoughness = material.sheenRoughnessFactor;
	if (sheenRoughtex.use)
		sheenRoughness *= getTexel(sheenRoughtex, uv0, uv1).a;
	return sheenRoughness;
}
#endif

#ifdef CLEARCOAT
uniform TextureInfo clearCoatTex;
uniform TextureInfo clearCoatRoughTex;
uniform TextureInfo clearCoatNormalTex;

float getClearCoat(vec2 uv0, vec2 uv1)
{
	float clearCoat = material.clearcoatFactor;
	if (clearCoatTex.use)
		clearCoat *= getTexel(clearCoatTex, uv0, uv1).r;
	return clearCoat;
}

float getClearCoatRoughness(vec2 uv0, vec2 uv1)
{
	float clearCoatRoughness = material.clearcoatRoughnessFactor;
	if (clearCoatRoughTex.use)
		clearCoatRoughness *= getTexel(clearCoatRoughTex, uv0, uv1).g;
	return clearCoatRoughness;
}
#endif

#ifdef TRANSMISSION
uniform TextureInfo transmissionTex;
uniform TextureInfo thicknessTex;

float getTransmission(vec2 uv0, vec2 uv1)
{
	float transmission = material.transmissionFactor;
	if (transmissionTex.use)
		transmission *= getTexel(transmissionTex, uv0, uv1).r;
	return transmission;
}

float getThickness(vec2 uv0, vec2 uv1)
{
	float thickness = material.thicknessFactor;
	if (thicknessTex.use)
		thickness *= getTexel(thicknessTex, uv0, uv1).g;
	return thickness;
}
#endif

#ifdef SPECULAR
uniform TextureInfo specularTex;
uniform TextureInfo specularColorTex;

float getSpecular(vec2 uv0, vec2 uv1)
{
	float specular = material.specularFactor;
	if (specularTex.use)
		specular *= getTexel(specularTex, uv0, uv1).a;
	return specular;
}

vec3 getSpecularColor(vec2 uv0, vec2 uv1)
{
	vec3 specularColor = material.specularColorFactor;
	if (specularColorTex.use)
		specularColor *= getTexel(specularColorTex, uv0, uv1).rgb;
	return specularColor;
}
#endif

#ifdef IRIDESCENCE
uniform TextureInfo iridescenceTex;
uniform TextureInfo iridescenceThicknessTex;

float getIridescence(vec2 uv0, vec2 uv1)
{
	float iridescence = material.iridescenceFactor;
	if (iridescenceTex.use)
		iridescence *= getTexel(iridescenceTex, uv0, uv1).r;
	return iridescence;
}

float getIridescenceThickness(vec2 uv0, vec2 uv1)
{
	float thickness = material.iridescenceThicknessMax;
	if (iridescenceThicknessTex.use)
	{
		float thicknessWeight = getTexel(iridescenceThicknessTex, uv0, uv1).g;
		thickness = mix(material.iridescenceThicknessMin, material.iridescenceThicknessMax, thicknessWeight);
	}
	return thickness;
}
#endif

#ifdef ANISOTROPY
uniform TextureInfo anisotropyTex;
uniform TextureInfo anisotropyDirectionTex;

float getAnisotropy(vec2 uv0, vec2 uv1)
{
	float anisotropy = material.anisotropyFactor; // default is zero, which doesn't make much sense...
	if (anisotropyTex.use)
		anisotropy *= getTexel(anisotropyTex, uv0, uv1).r * 2.0 - 1.0;
	return anisotropy;
}

vec3 getAnisotropyDirection(vec2 uv0, vec2 uv1)
{
	vec3 direction = material.anisotropyDirection;
	if (anisotropyDirectionTex.use)
		direction = getTexel(anisotropyDirectionTex, uv0, uv1).rgb * 2.0 - 1.0;
	return direction;
}
#endif