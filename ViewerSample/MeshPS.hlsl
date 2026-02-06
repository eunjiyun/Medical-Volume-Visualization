//Texture2D meshTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//cbuffer ClipSettings : register(b1)
//{
//	float4 clipPlane;  // (nx, ny, nz, d)
//	int enableClip;
//	float3 padding;
//};
//
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float3 ViewPos : TEXCOORD0;
//	float2 Tex : TEXCOORD1;
//	float WorldY : TEXCOORD2;  //  추가!
//};l
//
//float4 main(PS_INPUT input) : SV_TARGET
//{
//
//float4 color = meshTexture.Sample(samplerState, input.Tex);
//color.a = 0.3;
//
//return color;
//}

//struct PS_INPUT
//{
//	float4 pos   : SV_POSITION; // 필수
//	float  viewZ : TEXCOORD0;   // 우리가 쓸 값
//};
//
//struct VSOutput
//{
//	float4 pos : SV_POSITION;
//	float2 uv : TEXCOORD0;
//	float2 screenUV : TEXCOORD1;  // 스크린 UV
//	float viewZ : TEXCOORD2;  //  추가: View Space Z
//};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float2 screenUV : TEXCOORD1;
	float viewZ : TEXCOORD2;  //  추가: View Space Z
};


float main(PS_INPUT input) : SV_Target0
{
	//return input.viewZ;   // R32_FLOAT RTV

	return 123.f;   // R32_FLOAT RTV
}
