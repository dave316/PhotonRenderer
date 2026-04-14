struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

float main(PSInput input) : SV_Depth
{
//#ifdef METAL_ROUGH_MATERIAL
//	vec4 baseColor = getBaseColor(fTexCoord0, fTexCoord1);
//#else
//	vec4 baseColor = getDiffuseColor(fTexCoord0, fTexCoord1);
//#endif
//	float transparency = baseColor.a;
//	if(material.alphaMode == 1)
//		if(transparency < material.alphaCutOff)
//			discard;
    return input.position.z / input.position.w;
}