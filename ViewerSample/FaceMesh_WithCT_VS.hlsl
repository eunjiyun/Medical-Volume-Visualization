// FaceMesh_WithCT_VS.hlsl

cbuffer MeshCB : register(b0)
{
	matrix WorldViewProj;
	float4 CTBlendParams;  // x = strength, y = unused...
};

struct VSInput
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float2 screenUV : TEXCOORD1;  // 스크린 UV
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.pos = mul(float4(input.position, 1), WorldViewProj);
	output.uv = input.uv;

	// 스크린 좌표 계산
	output.screenUV = output.pos.xy / output.pos.w * 0.5 + 0.5;
	output.screenUV.y = 1.0 - output.screenUV.y;  // D3D flip

	return output;
}