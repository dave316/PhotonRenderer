
struct TextureInfo
{
	int samplerIndex;
	uint uvIndex;
	uint padding0;
	uint padding1;
	mat4 uvTransform;
};

// material uniform + samplers (descriptor set 2)
#ifdef USE_OPENGL
layout(std140, binding = 3) uniform MaterialUBO
#else
layout(std140, set = 4, binding = 0) uniform MaterialUBO
#endif
{
	vec4 baseColor;
	vec4 emissive;
	float roughness;
	float metallic;
	float occlusion;
	float normalScale;
	int alphaMode;
	float alphaCutOff;
	bool computeFlatNormals;
	float ior;
#ifdef SHEEN
	vec4 sheen;
#endif
#ifdef CLEARCOAT
	float clearcoat;
	float clearcoatRoughness;
	float padding0;
	float padding1;
#endif
#ifdef TRANSMISSION
	float transmission;
	float thickness;
	float dispersion;
	vec4 attenuation;
#endif
#ifdef SPECULAR
	vec4 specular;
#endif
#ifdef IRIDESCENCE
	vec4 iridescence;
#endif
#ifdef ANISOTROPY
	float anisotropyStrength;
	float anisotropyRotation;
	float padding2;
	float padding3;
#endif
#ifdef TRANSLUCENCY
	vec4 translucency;
	vec4 scattering; 
	vec4 attenuation;
	float thickness;
	float dispersion;
#endif
	
	TextureInfo baseColorTex;
	TextureInfo normalTex;
	TextureInfo metalRoughTex;
	TextureInfo emissiveTex;
	TextureInfo occlusionTex;
#ifdef SHEEN
	TextureInfo sheenColorTex;
	TextureInfo sheenRoughnessTex;
#endif
#ifdef CLEARCOAT
	TextureInfo clearcoatTex;
	TextureInfo clearcoatRoughnessTex;
	TextureInfo clearcoatNormalTex;
#endif
#ifdef TRANSMISSION
	TextureInfo transmissionTex;
	TextureInfo thicknessTex;
#endif
#ifdef SPECULAR
	TextureInfo specularTex;
	TextureInfo specularColorTex;
#endif
#ifdef IRIDESCENCE
	TextureInfo iridescenceTex;
	TextureInfo iridescenceThicknessTex;
#endif
#ifdef ANISOTROPY
	TextureInfo anisotropyTex;
#endif
#ifdef TRANSLUCENCY
	TextureInfo translucencyTex;
	TextureInfo translucencyColorTex;
	TextureInfo thicknessTex;
#endif
} material;

#define MAX_MATERIAL_TEXTURES 10

#ifdef USE_OPENGL
layout(binding = 1) uniform sampler2D materialTextures[MAX_MATERIAL_TEXTURES];
#else
layout(set = 4, binding = 1) uniform sampler2D materialTextures[];
#endif

vec4 getTexel(in TextureInfo info, in vec2 texCoords[2])
{
	vec3 uv = vec3(texCoords[info.uvIndex], 1.0);
	uv = mat3(info.uvTransform) * uv;
	return texture(materialTextures[info.samplerIndex], uv.xy);
}

struct Surface
{
	vec3 baseColor;
	float alpha;
	int alphaMode;
	vec3 normal;
	vec3 F0;
	vec3 F90;
	float roughness;
	float metallic;
	float ao;
	vec3 emission;
	float alphaRoughness;
	float specularWeight;
	float ior;
#ifdef SHEEN
	vec4 sheen;
#endif
#ifdef CLEARCOAT
	float clearcoat;
	float clearcoatRoughness;
	vec3 clearcoatNormal;
	vec3 clearcoatF0;
	vec3 clearcoatF90;
#endif
#ifdef TRANSMISSION
	float transmission;
	float thickness;
	float dispersion;
	vec4 attenuation;
#endif
#ifdef SPECULAR
	float specular;
	vec3 specularColor;
#endif
#ifdef IRIDESCENCE
	float iridescence;
	float iridescenceThickness;
	float iridescenceIOR;
#endif
#ifdef ANISOTROPY
	float anisotropyStrength;
	vec3 anisotropicTangent;
	vec3 anisotropicBitangent;
#endif
#ifdef TRANSLUCENCY
	float translucency;
	vec3 translucencyColor;
	vec3 multiScatter;
	float anisotropy;
	float thickness;
	float dispersion;
	vec4 attenuation;
#endif
};

Surface evalMaterial()
{
	Surface surface;

	vec4 baseColor = material.baseColor * vertexColor;
	vec2 texCoords[2];
	texCoords[0] = texCoord0;
	texCoords[1] = texCoord1;

	// base PBR material properties
	if(material.baseColorTex.samplerIndex >= 0)
		baseColor *= getTexel(material.baseColorTex, texCoords);
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;
	surface.alpha = material.alphaMode == 2 ? alpha : 1.0;
 
	surface.normal = normalize(wNormal);
	if (material.computeFlatNormals)
#ifdef USE_OPENGL
		surface.normal = normalize(cross(dFdx(wPosition), dFdy(wPosition)));
#else
		surface.normal = -normalize(cross(dFdx(wPosition), dFdy(wPosition)));
#endif
	if (material.normalTex.samplerIndex >= 0)
	{
		vec3 tNormal = getTexel(material.normalTex, texCoords).xyz * 2.0 - 1.0;
		tNormal *= vec3(material.normalScale, material.normalScale, 1.0);
		surface.normal = normalize(wTBN * normalize(tNormal));
	}
	surface.emission = material.emissive.rgb * material.emissive.a;
	if (material.emissiveTex.samplerIndex >= 0)
		surface.emission *= getTexel(material.emissiveTex, texCoords).rgb;
	vec3 orm = vec3(material.occlusion, material.roughness, material.metallic);
	if (material.metalRoughTex.samplerIndex >= 0)
		orm *= vec3(1.0, getTexel(material.metalRoughTex, texCoords).gb);
	if (material.occlusionTex.samplerIndex >= 0)
		orm.r = 1.0 + orm.r * (getTexel(material.occlusionTex, texCoords).r - 1.0);

#ifdef SHEEN
	// sheen
	vec3 sheenColor = material.sheen.rgb;
	if (material.sheenColorTex.samplerIndex >= 0)
		sheenColor *= getTexel(material.sheenColorTex, texCoords).rgb;
	float sheenRoughness = material.sheen.a;
	if (material.sheenRoughnessTex.samplerIndex >= 0)
		sheenRoughness *= getTexel(material.sheenRoughnessTex, texCoords).a;
	surface.sheen = vec4(sheenColor, sheenRoughness);
#endif

#ifdef CLEARCOAT
	surface.clearcoat = material.clearcoat;
	if (material.clearcoatTex.samplerIndex >= 0)
		surface.clearcoat *= getTexel(material.clearcoatTex, texCoords).r;
	surface.clearcoatRoughness = material.clearcoatRoughness;
	if (material.clearcoatRoughnessTex.samplerIndex >= 0)
		surface.clearcoatRoughness *= getTexel(material.clearcoatRoughnessTex, texCoords).g;
	surface.clearcoatF0 = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
	surface.clearcoatF90 = vec3(1.0);
	surface.clearcoatNormal = normalize(wNormal);
	if (material.clearcoatNormalTex.samplerIndex >= 0)
	{
		vec3 tNormal = getTexel(material.clearcoatNormalTex, texCoords).xyz * 2.0 - 1.0;
		//tNormal *= vec3(material.normalScale, material.normalScale, 1.0); //TODO: clearcoat normal scale
		surface.clearcoatNormal = normalize(wTBN * normalize(tNormal));
	}
	//float clearcoatNoV = clamp(dot(clearcoatNormal, v), 0.0, 1.0);
	//vec3 clearcoatFresnel = F_Schlick(F0, F90, clearcoatNoV);
	if (!gl_FrontFacing)
		surface.clearcoatNormal = -surface.clearcoatNormal;
#endif

#ifdef TRANSMISSION
	// transmission
	surface.transmission = material.transmission;
	if (material.transmissionTex.samplerIndex >= 0)
		surface.transmission *= getTexel(material.transmissionTex, texCoords).r;
	surface.thickness = material.thickness;
	if (material.thicknessTex.samplerIndex >= 0)
		surface.thickness *= getTexel(material.thicknessTex, texCoords).g;
	surface.attenuation = material.attenuation;
	surface.dispersion = material.dispersion;
#endif

	// standard surface parameters
	surface.ao = orm.r;
	surface.roughness = clamp(orm.g, 0.05, 1.0);
	surface.alphaRoughness = surface.roughness * surface.roughness;
	surface.metallic = orm.b;
	surface.baseColor = baseColor.rgb;
	surface.F0 = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
	surface.F90 = vec3(1.0);
	surface.specularWeight = 1.0;
	surface.ior = material.ior;
	surface.alphaMode = material.alphaMode;

#ifdef SPECULAR
	surface.specular = material.specular.a;
	if (material.specularTex.samplerIndex >= 0)
		surface.specular *= getTexel(material.specularTex, texCoords).a;
	surface.specularColor = material.specular.rgb;
	if (material.specularColorTex.samplerIndex >= 0)
		surface.specularColor *= getTexel(material.specularColorTex, texCoords).rgb;

	surface.F0 = min(surface.F0 * surface.specularColor, vec3(1.0));
	surface.specularWeight = surface.specular;
#endif

#ifdef IRIDESCENCE
	surface.iridescence = material.iridescence.r;
	if (material.iridescenceTex.samplerIndex >= 0)
		surface.iridescence  *= getTexel(material.iridescenceTex, texCoords).r;
	surface.iridescenceThickness = material.iridescence.a;
	if (material.iridescenceThicknessTex.samplerIndex >= 0)
	{
		float thicknessWeight = getTexel(material.iridescenceThicknessTex, texCoords).g;
		surface.iridescenceThickness = mix(material.iridescence.b, material.iridescence.a, thicknessWeight);
	}
	surface.iridescenceIOR = material.iridescence.g;
#endif

#ifdef ANISOTROPY
	float anisotropyStrength = material.anisotropyStrength;
	vec2 anisotropyDirection = vec2(1.0, 0.0);
	if(material.anisotropyTex.samplerIndex >= 0)
	{
		vec3 anisotropySample = getTexel(material.anisotropyTex, texCoords).xyz;
		anisotropyDirection = anisotropySample.xy * 2.0 - vec2(1.0);
		anisotropyStrength *= anisotropySample.z;
	}

	mat2 aniRotationMat = mat2(
		cos(material.anisotropyRotation), sin(material.anisotropyRotation),
		-sin(material.anisotropyRotation), cos(material.anisotropyRotation)
	);
	anisotropyDirection = aniRotationMat * anisotropyDirection;

	surface.anisotropicTangent = wTBN * normalize(vec3(anisotropyDirection, 0.0));
	surface.anisotropicBitangent = normalize(cross(surface.normal, surface.anisotropicTangent));
	surface.anisotropyStrength = clamp(anisotropyStrength, 0.0, 1.0);
#endif

#ifdef TRANSLUCENCY
	surface.translucency = material.translucency.a;
	if (material.translucencyTex.samplerIndex >= 0)
		surface.translucency *= getTexel(material.translucencyTex, texCoords).a;
	surface.translucencyColor = material.translucency.rgb;
	if (material.translucencyColorTex.samplerIndex >= 0)
		surface.translucencyColor *= getTexel(material.translucencyColorTex, texCoords).rgb;
	surface.multiScatter = material.scattering.rgb;
	surface.anisotropy = material.scattering.a;
	surface.thickness = material.thickness;
	if (material.thicknessTex.samplerIndex >= 0)
		surface.thickness *= getTexel(material.thicknessTex, texCoords).g;
	surface.attenuation = material.attenuation;
	surface.dispersion = material.dispersion;
#endif
	
	return surface;
}
