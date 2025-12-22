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
	float WorldY : TEXCOORD2;  // ✅ 추가!
};

float4 main(PS_INPUT input) : SV_TARGET
{

float4 color = meshTexture.Sample(samplerState, input.Tex);
color.a = 0.55;
return color;
}

