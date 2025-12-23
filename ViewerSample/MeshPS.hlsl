Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)
{
	float4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	float3 padding;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : TEXCOORD0;
	float2 Tex : TEXCOORD1;
	float WorldY : TEXCOORD2;  // ✅ 추가!
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = meshTexture.Sample(samplerState, input.Tex);

	// ===== 컬러: 약간만 유지 =====
	float3 orig = color.rgb;
	float gray = dot(orig, float3(0.299, 0.587, 0.114));
	color.rgb = lerp(orig, gray.xxx, 0.3);  // 채도 더 낮춤
	color.rgb *= 0.95;  // 밝기 살짝 낮춤

	// ===== 알파: 매우 투명하게 ===== ★핵심★
	float alpha = 0.25;  // 기본 투명도 (0.5 → 0.25)

	// 텍스처 밝기에 따라 약간 조절 (선택사항)
	float brightness = (color.r + color.g + color.b) / 3.0;
	alpha += brightness * 0.1;  // 밝은 부분만 살짝 더 보이게

	alpha = clamp(alpha, 0.6, 0.85);  // 범위: 매우 투명

	color.a = alpha;

	return color;
}


//
//float4 main(PS_INPUT input) : SV_TARGET
//{
//
//float4 color = meshTexture.Sample(samplerState, input.Tex);
//
////
////// 👉 컬러 약화 (핵심)
////color.rgb = lerp(color.rgb, dot(color.rgb, float3(0.299, 0.587, 0.114)), 0.5);
////// 또는
////color.rgb *= 0.65;
////
////float alphaBase = 0.25;   // 기본 매우 낮음
////float alphaBoost = 0.2;   // 보정치
////
////// 얼굴 밝을수록 조금만 보이게
////float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));
////float alpha = alphaBase + alphaBoost * luminance;
//
//color.a = saturate(0.5);
//return color;
//}
//
//
//
