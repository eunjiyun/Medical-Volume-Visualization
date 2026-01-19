// FaceMesh_WithCT_VS.hlsl

cbuffer MeshCB : register(b0)
{
	matrix WorldViewProj;
	matrix View;
	matrix World;  // ✅ 추가: View 행렬
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
	float viewZ : TEXCOORD2;  // ✅ 추가: View Space Z
};

VSOutput main(VSInput input)
{
	VSOutput output;

	//벡터를 왼쪽에 곱함
	//순서는 S>R>T
	output.pos = mul(float4(input.position, 1), WorldViewProj);
	output.uv = input.uv;

	// 스크린 좌표 계산
	output.screenUV = output.pos.xy / output.pos.w * 0.5 + 0.5;
	output.screenUV.y = 1.0 - output.screenUV.y;  // D3D flip

 // View Space Z
	float4 posWorld = mul(float4(input.position, 1.0f), World);
	float4 posView = mul(posWorld, View);
	output.viewZ = posView.z;


	return output;
}