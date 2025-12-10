cbuffer MeshConstantBuffer : register(b0)
{
	matrix WVP;
	matrix World;  // ✅ 추가
	matrix WorldView;  // ✅ 추가!
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
	float3 ViewPos : TEXCOORD0;  // ✅ View 좌표
	float2 Tex : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT o;
	o.Pos = mul(float4(input.Pos, 1.0), WVP);
	o.ViewPos = mul(float4(input.Pos, 1.0), WorldView).xyz;  // ✅ View 좌표
	o.Tex = input.Tex;
	return o;
}