Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)
{
	float3 planeNormal;   // 평면 법선
	float  planeD;

	int    enableClip;
	float3 padding;       // 16바이트 정렬 유지
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	//float3 LocalPos : TEXCOORD1;  // ✅ 로컬 좌표
	float3 WorldPos  : TEXCOORD1;   // ★ 추가 (World 좌표)
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(planeNormal, input.WorldPos) + planeD;
//
//		if (dist > 0)
//			discard;
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

float4 main(PS_INPUT input) : SV_TARGET
{
	// ✅ WorldPos.y 값을 색으로 표시
	float y = input.WorldPos.y;

	if (y < -0.5)
		return float4(1, 0, 0, 1);  // 빨강
	else if (y < 0)
		return float4(1, 1, 0, 1);  // 노랑
	else if (y < 0.5)
		return float4(0, 1, 0, 1);  // 초록
	else
		return float4(0, 0, 1, 1);  // 파랑
}