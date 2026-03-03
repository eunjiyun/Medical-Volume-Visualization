// SimpleQuad_PS.hlsl
Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
	//return float4(1,0,0,1);
	return tex.Sample(samp, input.uv);
	return float4(input.uv.x,input.uv.x,input.uv.x,1.0f);
}