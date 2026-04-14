
float PI = 3.14159265358979323846;

// fresnel
float F_Schlick(float F0, float F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick(vec3 F0, vec3 F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick_Rough(vec3 F0, float HoV, float roughness)
{
	return max(F0, F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HoV, 5.0));
}

// micro-facet distributions
float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float D_GGX_Anisotropic(float NdotH, float TdotH, float BdotH, float at, float ab)
{
	float a2 = at * ab;
	vec3 f = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
	float w = a2 / dot(f, f);
	return a2 * w * w / PI;
}

float D_Charlie(float sheenRoughness, float NoH)
{
	float alpha = max(sheenRoughness * sheenRoughness, 0.000001);
	float invR = 1.0 / alpha;
	float cos2h = NoH * NoH;
	float sin2h = 1.0 - cos2h;
	return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

// visibility
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if (GGX > 0.0)
	{
		return 0.5 / GGX;
	}
	return 0.0;
}

float V_GGX_Anisotropic(float NdotL, float NdotV, float BdotV, float TdotV, float TdotL, float BdotL, float at, float ab)
{
	float GGXV = NdotL * length(vec3(at * TdotV, ab * BdotV, NdotV));
	float GGXL = NdotV * length(vec3(at * TdotL, ab * BdotL, NdotL));
	float V = 0.5 / (GGXV + GGXL);
	return clamp(V, 0.0, 1.0);
}

float l(float x, float alpha)
{
	float oneMinusAlphaSq = (1.0 - alpha) * (1.0 - alpha);
	float a = mix(21.5473, 25.3245, oneMinusAlphaSq);
	float b = mix(3.82987, 3.32435, oneMinusAlphaSq);
	float c = mix(0.19823, 0.16801, oneMinusAlphaSq);
	float d = mix(-1.97760, -1.27393, oneMinusAlphaSq);
	float e = mix(-4.32054, -4.85967, oneMinusAlphaSq);
	return a / (1.0 + b * pow(x, c)) + d * x + e;
}

float lambdaSheen(float cosTheta, float alpha)
{
	if(abs(cosTheta) < 0.5)
		return exp(l(cosTheta, alpha));
	else
		return exp(2.0 * l(0.5, alpha) - l(1.0 - cosTheta, alpha));
}

float V_Sheen(float NoL, float NoV, float sheenRoughness)
{
	float alpha = sheenRoughness * sheenRoughness;
	float sheenVis = 1.0 / ((1.0 + lambdaSheen(NoV, alpha) + lambdaSheen(NoL, alpha)) * (4.0 * NoV * NoL));
	return clamp(sheenVis, 0.0, 1.0);
}

// diffuse BRDF
vec3 diffuseLambert(vec3 color)
{
	return color / PI;
}

// specular BRDFs
float specularGGX(float NoL, float NoV, float NoH, float alpha)
{
	float V = V_GGX(NoL, NoV, alpha);
	float D = D_GGX(NoH, alpha);
	return V * D;
}

vec3 specularTransmission(vec3 n, vec3 v, vec3 l, float alphaRoughness, vec3 F0, vec3 F90, vec3 baseColor, float ior)
{
	float transmissionRoughness = applyIorToRoughness(alphaRoughness, ior);
	vec3 l_mirror = normalize(reflect(l, n));
	vec3 h = normalize(l_mirror + v);
	float D = D_GGX(clamp(dot(n, h), 0.0, 1.0), transmissionRoughness);
	vec3 F = F_Schlick(F0, F90, clamp(dot(v, h), 0.0, 1.0));
	float V = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRoughness);
	return (1 - F) * baseColor * D * V;
}

float specularGGXAnisotropic(vec3 n, vec3 l, vec3 v, vec3 t, vec3 b, float alpha, float anisotropy)
{
	vec3 h = normalize(l + v);
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoV = clamp(dot(n, v), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float HoV = clamp(dot(h, v), 0.0, 1.0);

	float ToL = dot(t, l);
	float ToV = dot(t, v);
	float ToH = dot(t, h);
	float BoL = dot(b, l);
	float BoV = dot(b, v);
	float BoH = dot(b, h);

	float at = mix(alpha, 1.0, anisotropy * anisotropy);
	float ab = clamp(alpha, 0.001, 1.0);

	float V = V_GGX_Anisotropic(NoL, NoV, BoV, ToV, ToL, BoL, at, ab);
	float D = D_GGX_Anisotropic(NoH, ToH, BoH, at, ab);

	return V * D;
}

vec3 specularSheen(vec3 sheenColor, float sheenRoughness, float NoL, float NoV, float NoH)
{
	float sheenDistribution = D_Charlie(sheenRoughness, NoH);
	float sheenVisibility = V_Sheen(NoL, NoV, sheenRoughness);
	return sheenColor * sheenDistribution * sheenVisibility;
}

vec3 fresnel0ToIor(vec3 F0)
{
	vec3 sqrtF0 = sqrt(F0);
	return (vec3(1.0) + sqrtF0) / (vec3(1.0) - sqrtF0);
}

vec3 iorToFresnel0(vec3 transmittedIor, float incidentIor)
{
	vec3 f = (transmittedIor - vec3(incidentIor)) / (transmittedIor + vec3(incidentIor));
	return f * f;
}

float iorToFresnel0(float transmittedIor, float incidentIor)
{
	float f = (transmittedIor - incidentIor) / (transmittedIor + incidentIor);
	return f * f;
}

vec3 evalSensitivity(float opd, vec3 shift)
{
	float phase = 2.0 * PI * opd * 1e-9;
	vec3 val = vec3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    vec3 pos = vec3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    vec3 var = vec3(4.3278e+09, 9.3046e+09, 6.6121e+09);

	vec3 xyz = val * sqrt(2.0 * PI * var) * cos(pos * phase + shift) * exp(-(phase * phase) * var);
	xyz.x += 9.7470e-14 * sqrt(2.0 * PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
	xyz /= 1.0685e-7;

	return XYZ_TO_SRGB * xyz;
}


// iridescence
vec3 evalIridescence(float topIor, float bottomIor, float cosTheta1, float thickness, vec3 baseF0)
{
	float iridescenceIor = mix(topIor, bottomIor, smoothstep(0.0, 0.03, thickness));
	float sinTheta2Sq = sq(topIor / iridescenceIor) * (1.0 - sq(cosTheta1));
	float cosTheta2Sq = 1.0 - sinTheta2Sq;
	if(cosTheta2Sq < 0.0)
	{
		return vec3(1.0);
	}
	float cosTheta2 = sqrt(cosTheta2Sq);

	float R0 = iorToFresnel0(iridescenceIor, topIor);
	float R12 = F_Schlick(R0, 1.0, cosTheta1);
	float T121 = 1.0 - R12;
	float phi12 = 0.0;
	if(iridescenceIor < topIor)
		phi12 = PI;
	float phi21 = PI - phi12;

	vec3 baseIor = fresnel0ToIor(clamp(baseF0, 0.0, 0.9999));
	vec3 R1 = iorToFresnel0(baseIor, iridescenceIor);
	vec3 R23 = F_Schlick(R1, vec3(1.0), cosTheta2);
	vec3 phi23 = vec3(0.0);
	for (int i = 0; i < 3; i++)
		if(baseIor[i] < iridescenceIor)
			phi23[i] = PI;

	float opd = 2.0 * iridescenceIor * thickness * cosTheta2;
	vec3 phi = vec3(phi21) + phi23;

	vec3 R123 = clamp(R12 * R23, 1e-5, 0.9999);
	vec3 r123 = sqrt(R123);
	vec3 Rs = sq(T121) * R23 / (vec3(1.0) - R123);

	vec3 C0 = R12 + Rs;
	vec3 I = C0;
	vec3 Cm = Rs - T121;
	for (int m = 1; m <= 2; m++)
	{
		vec3 Sm = 2.0 * evalSensitivity(float(m) * opd, float(m) * phi);
		Cm *= r123;
		I += Cm * Sm;
	}

	return max(I, vec3(0.0));
}
