//Texture2D sliceTexture : register(t0);
//SamplerState sliceSampler : register(s0);
//
//cbuffer VolumeConstants : register(b0)
//{
//	matrix World;
//	matrix View;
//	matrix Projection;
//	float4 color;
//};
//
//struct VS_OUTPUT {
//	float4 position : SV_POSITION;
//	float2 texcoord : TEXCOORD;
//};
//
//float4 PSVolume(VS_OUTPUT input) : SV_TARGET
//{
//	float4 texColor = sliceTexture.Sample(sliceSampler, input.texcoord);
//	texColor.a *= color.a;   // C++에서 지정한 알파 반영
//	return texColor;
//}


//Texture2D sliceTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//cbuffer VolumeConstants : register(b0)
//{
//	matrix World;
//	matrix View;
//	matrix Projection;
//	float4 color;
//};
//
//struct VS_OUTPUT {
//	float4 position : SV_POSITION;
//	float2 texcoord : TEXCOORD;
//	float3 worldPos : WORLDPOS;
//};
//
//float4 PSVolume(VS_OUTPUT input) : SV_TARGET
//{
//	// 샘플링
//	float gray = sliceTexture.Sample(samplerState, input.texcoord).r;
//
//// 명암비 강화 — 중간값을 크게 밀어줌
//gray = saturate((gray - 0.08) * 3.2);
//
//// 어두운 갈색 → 밝은 오렌지 톤
//float3 darkTone = float3(0.22, 0.16, 0.10);
//float3 lightTone = float3(1.0, 0.82, 0.6);
//float3 finalColor = lerp(darkTone, lightTone, gray);
//
//// 투명도 좀 더 강하게
//float alpha = pow(gray, 2.0) * 0.6;
//
//// 감마 살짝 (더 따뜻하게)
//finalColor = pow(finalColor, 1.0 / 1.9);
//
//return float4(finalColor, alpha);
//}


Texture2D sliceTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer VolumeConstants : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 color;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 worldPos : WORLDPOS;
};

float4 PSVolume(VS_OUTPUT input) : SV_TARGET
{
	float gray = sliceTexture.Sample(samplerState, input.texcoord).r;

// === HU 윈도우/레벨 (조금 더 낮은 중심) ===
float windowCenter = 0.32; // 어두운 중심으로
float windowWidth = 0.5;  // 살짝 좁힘
gray = saturate((gray - (windowCenter - windowWidth * 0.5)) / windowWidth);

// === 명암비 강화 (더 또렷하게) ===
gray = pow(gray, 1.6);

// === 색상 보간 (따뜻한 오렌지 계열) ===
float3 darkTone = float3(0.25, 0.18, 0.12);
float3 midTone = float3(0.8, 0.6, 0.45);
float3 lightTone = float3(1.0, 0.85, 0.65);
float3 finalColor = lerp(darkTone, lightTone, gray);
finalColor = lerp(finalColor, midTone, 0.25); // 약간 중간 톤 섞기

// === 투명도: 약한 밀도(soft tissue)는 완전 투명, 뼈는 뚜렷 ===
float alpha = smoothstep(0.3, 0.7, gray); // 0.3이하 완전 투명, 0.7이상 완전 불투명
alpha = pow(alpha, 2.0) * 0.65;           // 전체 누적 투명도 조절

// === 감마 보정 ===
finalColor = pow(finalColor, 1.0 / 1.8);

return float4(finalColor, alpha);
}
