cbuffer MeshConstantBuffer : register(b0)
{
	matrix WVP;
	matrix World;  // ✅ 추가
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 LocalPos  : TEXCOORD1;  // ✅ 추가
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Pos = mul(float4(input.Pos, 1.0), WVP);
	output.LocalPos = input.Pos;
	output.Tex = input.Tex;
	output.Normal = input.Normal;
	return output;
}