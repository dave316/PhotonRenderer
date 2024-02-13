#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in vec4 lightPosition;
layout(location = 6) in mat3 wTBN;

layout(location = 0) out vec4 fragColor;

uniform float _Normal1_Power;
uniform sampler2D _Normal1;
uniform float _Tile1;
uniform float _Normal2_Power;
uniform sampler2D _Normal2;
uniform float _Tile2;
uniform sampler2D _Height1;
uniform sampler2D _Height2;
uniform float _Height2Scale;
uniform float _Height2Offset;
uniform float _Color2Scale;
uniform float _Color2Offset;
uniform vec4 _Color1;
uniform sampler2D _Albedo1;
uniform vec4 _Color2;
uniform sampler2D _Albedo2;
uniform float _Metallic1_Power;
uniform sampler2D _Metallic1;
uniform float _Metallic2_Power;
uniform sampler2D _Metallic2;
uniform float _Smoothness1;
uniform float _Smoothness2;

struct Input
{
	vec2 uv_texcoord;
	vec4 vertexColor;
};

struct SurfaceOutputStandard
{
	vec3 Normal;
	vec3 Albedo;
	float Metallic;
	float Smoothness;
	float Alpha;
};

vec3 UnpackScaleNormal(vec4 n, float s)
{
	return vec3(0);
}

vec4 saturate(vec4 value)
{
	return clamp(value, 0.0, 1.0);
}

void surf( Input i , inout SurfaceOutputStandard o )
{
	vec2 temp_cast_0 = (_Tile1).xx;
	vec2 uv_TexCoord179 = i.uv_texcoord * temp_cast_0;
	vec2 temp_cast_1 = (_Tile2).xx;
	vec2 uv_TexCoord180 = i.uv_texcoord * temp_cast_1;
	vec4 appendResult135 = (vec4(texture( _Height1, uv_TexCoord179 ).r , texture( _Height2, uv_TexCoord180 ).r , 0.0 , 0.0));
	vec4 appendResult111 = (vec4(0.0 , _Height2Scale , 0.0 , 0.0));
	vec4 appendResult114 = (vec4(0.0 , _Height2Offset , 0.0 , 0.0));
	vec4 temp_output_101_0 = (( 1.0 - appendResult135 )*appendResult111 + appendResult114);
	vec4 appendResult120 = (vec4(0.0 , _Color2Scale , 0.0 , 0.0));
	vec4 appendResult121 = (vec4(0.0 , _Color2Offset , 0.0 , 0.0));
	vec4 temp_output_97_0 = (i.vertexColor*appendResult120 + appendResult121);
	vec4 Blender124 = ( 1.0 - saturate( ( temp_output_101_0 + temp_output_97_0 ) ) );
	vec3 lerpResult72 = mix( UnpackScaleNormal( texture( _Normal1, uv_TexCoord179 ) ,_Normal1_Power ) , UnpackScaleNormal( texture( _Normal2, uv_TexCoord180 ) ,_Normal2_Power ) , (Blender124).y);
	o.Normal = lerpResult72;
	vec4 lerpResult29 = mix( ( _Color1 * texture( _Albedo1, uv_TexCoord179 ) ) , ( _Color2 * texture( _Albedo2, uv_TexCoord180 ) ) , (Blender124).y);
	o.Albedo = lerpResult29.rgb;
	vec4 tex2DNode140 = texture( _Metallic1, uv_TexCoord179 );
	vec4 tex2DNode141 = texture( _Metallic2, uv_TexCoord180 );
	float temp_output_156_0 = (Blender124).y;
	float lerpResult143 = mix( ( _Metallic1_Power * tex2DNode140.r ) , ( _Metallic2_Power * tex2DNode141.r ) , temp_output_156_0);
	o.Metallic = lerpResult143;
	float lerpResult144 = mix( ( tex2DNode140.a * _Smoothness1 ) , ( tex2DNode141.a * _Smoothness2 ) , temp_output_156_0);
	o.Smoothness = lerpResult144;
	o.Alpha = 1;
}

void main()
{
	Input i;
	i.uv_texcoord = texCoord0;
	i.vertexColor = vertexColor;

	SurfaceOutputStandard o;
	surf(i, o);

	fragColor = vec4(1);
}