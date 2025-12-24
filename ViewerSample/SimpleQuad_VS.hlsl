// SimpleQuad_VS.hlsl
struct VSInput
{
	float2 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PSInput main(VSInput input)
{
	PSInput output;
	output.pos = float4(input.pos, 0, 1);
	output.uv = input.uv;
	return output;
}