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

struct PBRSpecGlossMaterial
{
	int alphaMode;
	float alphaCutOff;
};
uniform PBRSpecGlossMaterial material;

uniform TextureInfo albedo1;
uniform TextureInfo albedo2;
uniform TextureInfo normal1;
uniform TextureInfo normal2;
uniform TextureInfo specular1;
uniform TextureInfo specular2;
uniform TextureInfo mask;

vec4 getDiffuseColor(vec2 uv0, vec2 uv1)
{
	vec3 color1 = getTexel(albedo1, uv0, uv1).rgb;
	vec3 color2 = getTexel(albedo2, uv0, uv1).rgb;
	float m = getTexel(mask, uv0, uv1).r;
	return vec4(mix(color1, color2, m), 1.0);
}

vec4 getSpecGloss(vec2 uv0, vec2 uv1)
{
	vec4 spec1 = getTexel(specular1, uv0, uv1);
	vec4 spec2 = getTexel(specular2, uv0, uv1);
	float m = getTexel(mask, uv0, uv1).r;
	return mix(spec1, spec2, m);
}

vec3 getNormal(vec2 uv0, vec2 uv1)
{
	vec3 n1 = getTexel(normal1, uv0, uv1).rgb * 2.0 - 1.0;
	vec3 n2 = getTexel(normal2, uv0, uv1).rgb * 2.0 - 1.0;
	float m = getTexel(mask, uv0, uv1).r;
	return mix(n1, n2, m);
}

vec3 getEmission(vec2 uv0, vec2 uv1)
{
	return vec3(0);
}

vec3 getPBRValues(vec2 uv0, vec2 uv1)
{
	return vec3(1,0,0);
}
