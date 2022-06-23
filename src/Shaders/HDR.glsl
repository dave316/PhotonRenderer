const float gamma = 2.2;
const float gammaInv = 1.0 / 2.0;

vec3 linear2sRGB(vec3 rgb)
{
	return pow(rgb, vec3(gammaInv));
}

vec3 sRGB2Linear(vec3 srgb)
{
	return pow(srgb, vec3(gamma));
}

float EV = 0.0;
float exposure = 1.0f;
//intensity = vec3(1.0) - exp(-intensity * exposure);
//intensity = intensity * pow(2.0, EV);
//intensity = intensity / (1.0 + intensity);			// reinhard

//if (pq)
//{
//	float rangeExponentSDR = 46.42;
//	vec3 colorScaled = intensity / 10000.0;
//	vec3 apertureAdjustedColor = colorScaled * apertureFactor;
//	vec3 ootf = BT_2100_OOTF(apertureAdjustedColor, rangeExponentSDR, 2.4);
//	vec3 oetf = BT_2100_OETF(ootf);
//	intensity = max(oetf, 0.0);
//}