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

struct DefaultMaterial
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

	// misc
	bool unlit;
	float normalScale;
	float ior;
};
uniform DefaultMaterial material;

struct DefaultSurface
{
	vec3 color;
	float alpha;
	vec3 normal;
	vec3 F0;
	vec3 F90;
	float roughness;
	float metallic;
	float alphaRoughness;
	float ao;
	vec3 emission;
};
DefaultSurface surface;

struct DefaultPixel
{
	vec3 f_diffuse;
	vec3 f_specular;
	vec3 f_emissive;
};
DefaultPixel pixel;

// PBR base material textures
uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo metalRoughTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;

void evalMaterial(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
	surface.alpha = baseColor.a;
	if(material.alphaMode == 1 && surface.alpha < material.alphaCutOff)
		discard;

	vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);
	surface.normal = normalize(normal);

	vec3 emission = material.emissiveFactor;
	if (emissiveTex.use)
		emission *= getTexel(emissiveTex, uv0, uv1).rgb;
	surface.emission = emission * material.emissiveStrength;

	vec3 orm = vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
	if (metalRoughTex.use)
		orm *= vec3(1.0, getTexel(metalRoughTex, uv0, uv1).gb);
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);
			float ao = orm.r; 
	surface.ao = orm.r;
	surface.roughness = clamp(orm.g, 0.05, 1.0);
	surface.metallic = orm.b;

	vec3 black = vec3(0);
	vec3 dielectricSpecular = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
	surface.color = mix(baseColor.rgb, black, surface.metallic);
	surface.F0 = mix(dielectricSpecular, baseColor.rgb, surface.metallic);
	surface.F90 = vec3(1.0);
	surface.alphaRoughness = surface.roughness * surface.roughness;
}

// TODO: compute shading for a given surface + lights
void evalShading()
{

}