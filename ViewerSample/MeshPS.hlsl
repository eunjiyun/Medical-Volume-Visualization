Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)
{
	float4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	float3 padding;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{

	float4 color = meshTexture.Sample(samplerState, input.Tex);
	color.a = 0.1;
	return color;


		// ✅ 무조건 빨간색 출력
	//return float4(1, 0, 0, 0.65);

	//	return float4(1.0, 0.0, 1.0, 0.65);  // 자홍색
}

