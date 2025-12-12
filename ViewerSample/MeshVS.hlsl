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

	float3 WorldPos : TEXCOORD2;  // ✨ 추가
	float3 Normal : TEXCOORD3;    // ✨ 추가
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT o;

	o.Pos = mul(float4(input.Pos, 1.0), WVP);
	o.ViewPos = mul(float4(input.Pos, 1.0), WorldView).xyz;  // ✅ View 좌표
	o.Tex = input.Tex;

	// ========== ✨ 여기 2줄 추가 ==========
	o.WorldPos = mul(float4(input.Pos, 1.0), World).xyz;        // World 좌표
	o.Normal = mul(input.Normal, (float3x3)World);              // World Normal

	return o;
}