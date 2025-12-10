//struct VS_OUTPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//};
//
//VS_OUTPUT main(uint vertexID : SV_VertexID)
//{
//	VS_OUTPUT output;
//
//	// 풀스크린 삼각형 생성 (버텍스 버퍼 없이)
//	float2 texcoord = float2((vertexID << 1) & 2, vertexID & 2);
//	output.Tex = texcoord;
//	output.Pos = float4(texcoord * float2(2, -2) + float2(-1, 1), 0, 1);
//
//	return output;
//}



// FullscreenVS.hlsl
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
	VS_OUTPUT output;

	// 풀스크린 삼각형 생성
	float2 texcoord = float2((vertexID << 1) & 2, vertexID & 2);
	output.Tex = texcoord;
	output.Pos = float4(texcoord * float2(2, -2) + float2(-1, 1), 0, 1);

	return output;
}