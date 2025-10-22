Texture2D sliceTexture : register(t0);
SamplerState sliceSampler : register(s0);

cbuffer VolumeConstants : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 color;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 PSVolume(VS_OUTPUT input) : SV_TARGET
{
	float4 texColor = sliceTexture.Sample(sliceSampler, input.texcoord);
	texColor.a *= color.a;   // C++에서 지정한 알파 반영
	return texColor;
}
