#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in flat int instanceID;
layout(location = 6) in mat3 wTBN;

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
#include "Light.glsl"
#include "BRDF.glsl"
#include "HDR.glsl"
#include "Materials/DefaultMaterial.glsl"

uniform bool useIBL = false;
uniform bool computeFlatNormals = false;
uniform float maxLuminance = 0.0;
uniform bool castShadows = true;

void main()
{
	vec2 uv0 = texCoord0;
	vec2 uv1 = texCoord1;

	evalMaterial(uv0, uv1); 

	// alpha = mix(alpha, 1.0, metallic); // Is this still needed?

	// compute shading normal
	vec3 n = normalize(wNormal);
	if (computeFlatNormals)
		n = normalize(cross(dFdx(wPosition), dFdy(wPosition)));
	if(normalTex.use)
		n = normalize(wTBN * surface.normal);
	if(gl_FrontFacing == false)
		n = -n;

	// view vector
	vec3 v = normalize(camera.position - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NoV = clampDot(n, v);

	vec3 f_diffuse = vec3(0);
	vec3 f_specular = vec3(0);
	vec3 f_emissive = surface.emission;
	
	// image based lights
	if(useIBL)
	{
		vec3 F_ambient = F_Schlick_Rough(surface.F0, NoV, surface.roughness);
	
		// TODO: can specular weight be removed?
		f_diffuse = surface.ao * getIBLRadianceLambert(n, surface.F0, F_ambient, surface.color, NoV, surface.roughness, 1.0);
		f_specular = surface.ao * getIBLRadianceGGX(r, F_ambient, NoV, surface.roughness, 1.0);
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
					shadow = getDirectionalShadowCSM(camera.V, wPosition, NoL, camera.zFar);
				else
					shadow = getPointShadow(wPosition, i);
			}

			vec3 intensity = getIntensity(light, wPosition, n, lightDir) * getAttenuation(light, lightDir);
			vec3 luminance = intensity * NoL * shadow;

			vec3 F = F_Schlick(surface.F0, surface.F90, HoV);
			f_diffuse += luminance * (1 - F * 0.0) * lambert(surface.color); 
			f_specular += luminance * F * 1.0 * specularGGX(NoL, NoV, NoH, surface.alphaRoughness);
		}
	}

	// default layer: diffuse BRDF + specular dielectric/metallic BRDF
	vec3 color = f_emissive + f_diffuse + f_specular;

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(color, 1.0);
	else 
		fragColor = vec4(color * surface.alpha, surface.alpha);

	grabColor = fragColor;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0.0), 1.0);
}