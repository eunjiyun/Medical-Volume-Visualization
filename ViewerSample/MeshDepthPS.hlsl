//// FaceMesh_WithCT_PS.hlsl
//Texture2D ctTexture : register(t0);
//SamplerState samp : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float2 screenUV : TEXCOORD1;
	float viewZ : TEXCOORD2;  //  추가
};

struct PSOutput
{
	float4 color : SV_Target0;    // 기존 컬러 출력
	float depth : SV_Target1;     //  추가: SceneDepth 출력
};

float main(PSInput input) : SV_Target
{
	//PSOutput output;

	////// 기존 컬러 렌더링
	////output.color = ctTexture.Sample(samp, input.uv);

	////  View Space Z 출력
	//output.depth = input.viewZ;
	//output.color = float4(1, 0, 0, 0.0);
	//return output;


	return input.viewZ;
}