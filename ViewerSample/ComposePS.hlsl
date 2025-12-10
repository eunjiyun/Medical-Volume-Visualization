//// ComposePS.hlsl
//Texture2D accumTexture : register(t0);
//Texture2D revealTexture : register(t1);
//SamplerState samplerState : register(s0);
//
//float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_Target
//{
//	float4 accum = accumTexture.Sample(samplerState, tex);
//	float reveal = revealTexture.Sample(samplerState, tex).r;
//
//	// ✅ 최종 합성
//	return float4(accum.rgb / max(accum.a, 0.00001), 1.0 - reveal);
//}


//
//Texture2D layerTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//struct VS_OUTPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//};
//
//float4 main(VS_OUTPUT input) : SV_Target
//{
//	float4 color = layerTexture.Sample(samplerState, input.Tex);
//	return color;
//}


Texture2D layerTexture : register(t0);
SamplerState samplerState : register(s0);

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	float4 color = layerTexture.Sample(samplerState, input.Tex);

	// ✅ 투명하면 discard
	if (color.a < 0.01)
	{
		discard;
	}

	return color;
}