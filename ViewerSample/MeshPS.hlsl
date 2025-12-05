Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//return meshTexture.Sample(samplerState, input.Tex);
	//// return float4(1.0f, 0.0f, 0.0f, 1.0f);  // ✅ 빨간색으로 강제 출력


	float4 color = meshTexture.Sample(samplerState, input.Tex);
color.a = 0.5;   // 투명도 절반
return color;

}


//// MeshPS.hlsl
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	// ✅ 노멀을 색으로 변환 (RGB 범위로)
//	float3 color = input.Normal * 0.5 + 0.5;
//	return float4(color, 1.0f);
//}
