//cbuffer ConstantBuffer : register(b0)
//{
//	matrix WVP;
//};
//
//struct VS_INPUT
//{
//	float3 Pos : POSITION;
//	float3 Normal : NORMAL;
//	float2 Tex : TEXCOORD0;
//};
//
//struct VS_OUTPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
//VS_OUTPUT main(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	output.Pos = mul(float4(input.Pos, 1.0f), WVP);
//	output.Tex = input.Tex;
//	output.Normal = input.Normal;
//	return output;
//}


//cbuffer MeshConstantBuffer : register(b0)
//{
//	matrix WVP;
//	matrix World;  // ✅ 추가
//}
//
//struct VS_INPUT
//{
//	float3 Pos : POSITION;
//	float3 Normal : NORMAL;
//	float2 Tex : TEXCOORD0;
//};
//
//struct VS_OUTPUT
//{
//	float4 Pos : SV_POSITION;
//	float3 WorldPos : TEXCOORD1;  // ✅ 추가
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
//VS_OUTPUT main(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	output.Pos = mul(float4(input.Pos, 1.0), WVP);
//	output.WorldPos = mul(float4(input.Pos, 1.0), World).xyz;  // ✅ 추가
//	output.Tex = input.Tex;
//	output.Normal = input.Normal;
//	return output;
//}


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
	float3 WorldPos : TEXCOORD1;  // ✅ 추가
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT input)
{
	//VS_OUTPUT output;
	//output.Pos = mul(float4(input.Pos, 1.0), WVP);
	//output.WorldPos = mul(float4(input.Pos, 1.0), World).xyz;  // ✅ World 좌표
	//output.Tex = input.Tex;
	//output.Normal = input.Normal;
	//return output;



	//VS_OUTPUT output;
	//output.Pos = mul(float4(input.Pos, 1.0), WVP);

	//// ✅ 원본 input.Pos를 그대로 전달
	//output.WorldPos = input.Pos;  // 변환 없이!

	//output.Tex = input.Tex;
	//output.Normal = input.Normal;
	//return output;


	VS_OUTPUT output;
	output.Pos = mul(float4(input.Pos, 1.0), WVP);

	// ✅ World 행렬로 변환
	output.WorldPos = mul(float4(input.Pos, 1.0), World).xyz;

	output.Tex = input.Tex;
	output.Normal = input.Normal;
	return output;
}