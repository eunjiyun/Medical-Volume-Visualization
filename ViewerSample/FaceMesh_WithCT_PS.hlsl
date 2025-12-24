// FaceMesh_WithCT_PS.hlsl

Texture2D faceColorTex : register(t0);  // 얼굴 색상
Texture2D ctTex : register(t1);         // CT 텍스처 (1단계 결과)
SamplerState linearSamp : register(s0);

cbuffer MeshCB : register(b0)
{
	matrix WorldViewProj;   // 64 bytes
	matrix World;           // 64 bytes
	matrix WorldView;       // 64 bytes
	float4 CTBlendParams;   // 16 bytes
};
struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float2 screenUV : TEXCOORD1;
};
//
//float4 main(PSInput input) : SV_Target
//{
//	float4 ctSample = ctTex.Sample(linearSamp, input.screenUV);
//	return float4(ctSample.rgb * 10.0, 1.0);  // 밝기 증폭
//
//
//
//
//	////return float4(1,0,1,1); // 자홍
//
//	//float4 ctSample = ctTex.Sample(linearSamp, input.uv);
//	//return float4(ctSample.rgb * 10.0, 1.0);
//}

//float4 main(PSInput input) : SV_Target
//{
//
////	// ⭐ 디버그 1: screenUV 시각화
////return float4(input.screenUV.x, input.screenUV.y, 0, 1);
//
//
//	// 1. 얼굴 베이스 색상
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//	//return float4(faceColor, 1);
//
//	// 2. CT 텍스처 샘플링
//	float4 ctSample = ctTex.Sample(linearSamp, input.screenUV);
//
//	//return float4(input.screenUV.x, input.screenUV.y, 0, 1);
//	//// CT 밝기 증폭해서 확인
//	//return float4(faceColor, 1.0);
//
//	// 3. 깨끗하게 합성
//	float ctStrength = CTBlendParams.x;  // 0.15 ~ 0.3
//
//	// CT 밝기 계산
//	float ctLuma = dot(ctSample.rgb, float3(0.299, 0.587, 0.114));
//
//	// 곱셈 블렌딩 (어두운 부분만 영향)
//	float3 finalColor = faceColor * (1.0 - ctLuma * ctStrength);
//
//	// 또는 Overlay 블렌딩
//	// float3 finalColor = lerp(faceColor, ctSample.rgb, ctSample.a * ctStrength);
//
//	// 감마 보정
//	finalColor = pow(saturate(finalColor), 1.0 / 2.2);
//
//	return float4(finalColor, 1.0);
//}

//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//
//	float4 ctSample = ctTex.Sample(linearSamp, input.screenUV);
//
//	float ctStrength = 0.25;   // ⭐ 강제값
//
//	float ctLuma = dot(ctSample.rgb, float3(0.299, 0.587, 0.114));
//
//	float3 finalColor = faceColor * (1.0 - ctLuma * ctStrength);
//
//	finalColor = pow(saturate(finalColor), 1.0 / 2.2);
//
//	return float4(finalColor, 1.0);
//}

//
//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//	float3 ctColor = ctTex.Sample(linearSamp, input.screenUV).rgb;
//
//	float ctLuma = dot(ctColor, float3(0.299, 0.587, 0.114));
//
//	// 🔥 Bone mask
//	float boneMask = saturate((ctLuma - 0.25) / 0.35);
//	boneMask = pow(boneMask, 2.2);
//
//	// CT 강조
//	float3 ctBoost = pow(ctColor, 0.6) * 1.3;
//
//	// 얼굴 깎기
//	float ctStrength = 0.6;
//	float3 color = faceColor * (1.0 - boneMask * ctStrength);
//
//	// 뼈 부분만 CT 살짝 섞기
//	color = lerp(color, ctBoost, boneMask * 0.4);
//
//	// 감마
//	color = pow(saturate(color), 1.0 / 2.2);
//
//	return float4(color, 1.0);
//}


//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//	float3 ctColor = ctTex.Sample(linearSamp, input.screenUV).rgb;
//
//	/* ============================
//	   1️⃣ CT 구조 신호 (연결성 강화)
//	============================ */
//
//	float ctLuma = dot(ctColor, float3(0.299, 0.587, 0.114));
//
//	// ⭐ bone mask를 넓고 부드럽게
//	float boneMask = saturate((ctLuma - 0.18) / 0.55);
//	boneMask = smoothstep(0.0, 1.0, boneMask);
//	boneMask = pow(boneMask, 1.1);   // 끊김 방지
//
//	/* ============================
//	   2️⃣ CT를 "밝기 필드"로 변환
//	============================ */
//
//	// CT를 직접 쓰지 말고 "밝기 압축"
//	float ctField = pow(ctLuma, 0.65);   // ⭐ 핵심
//	ctField *= 1.25;                     // 뼈 밝기 상승
//	ctField = saturate(ctField);
//
//	/* ============================
//	   3️⃣ 피부 얇게 (색 감소 ❌)
//	============================ */
//
//	// 피부를 지우지 말고 "비치게"
//	float skinTrans = lerp(1.0, 0.65, boneMask); // ⭐ 피부 얇게
//	float3 skinLayer = faceColor * skinTrans;
//
//	/* ============================
//	   4️⃣ 플랜메카식 합성 (핵심)
//	   - 뼈 = 얼굴을 깎는 음영
//	============================ */
//
//	float shadowStrength = 0.45; // ⭐ 0.35~0.55
//	float shadow = ctField * boneMask * shadowStrength;
//
//	float3 color = skinLayer * (1.0 - shadow);
//
//	/* ============================
//	   5️⃣ Bone highlight (화이트 복구)
//	============================ */
//
//	float boneHighlight = boneMask * 0.25;
//	color += boneHighlight.xxx;
//
//	/* ============================
//	   6️⃣ 마무리
//	============================ */
//
//	color = pow(saturate(color), 1.0 / 2.2);
//
//	return float4(color, 0.8);
//}


float4 main(PSInput input) : SV_Target
{
	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
	float3 ctColor = ctTex.Sample(linearSamp, input.screenUV).rgb;

	/* ============================
	   1️⃣ CT 구조 신호 (연결성 강화)
	============================ */

	float ctLuma = dot(ctColor, float3(0.299, 0.587, 0.114));

	// ⭐ bone mask를 넓고 부드럽게
	float boneMask = saturate((ctLuma - 0.18) / 0.55);
	boneMask = smoothstep(0.0, 1.0, boneMask);
	boneMask = pow(boneMask, 1.1);   // 끊김 방지

	/* ============================
	   2️⃣ CT를 "밝기 필드"로 변환
	============================ */

	// CT를 직접 쓰지 말고 "밝기 압축"
	float ctField = pow(ctLuma, 0.65);   // ⭐ 핵심
	ctField *= 1.25;                     // 뼈 밝기 상승
	ctField = saturate(ctField);

	/* ============================
	   3️⃣ 피부 얇게 (색 감소 ❌)
	============================ */

	// 피부를 지우지 말고 "비치게"
	float skinTrans = lerp(1.0, 0.35, boneMask); // ⭐ 피부 얇게
	float3 skinLayer = faceColor * skinTrans;

	/* ============================
	   4️⃣ 플랜메카식 합성 (핵심)
	   - 뼈 = 얼굴을 깎는 음영
	============================ */

	// ⭐ CT 강도 증폭 (볼륨 더 보이게)
	float shadowStrength = 0.75; // ⭐ 0.45 → 0.65 (CT 더 강하게)
	float shadow = ctField * boneMask * shadowStrength;

	float3 color = skinLayer * (1.0 - shadow);

	/* ============================
	   5️⃣ Bone highlight (화이트 복구)
	============================ */

	// ⭐ Bone highlight 증폭 (뼈 더 밝게)
	float boneHighlight = boneMask * 0.2;  // ⭐ 0.25 → 0.4
	color += boneHighlight.xxx;

	/* ============================
	   6️⃣ 마무리
	============================ */

	color = pow(saturate(color), 1.0 / 2.2);

	// ⭐ 알파 계산: bone이 많을수록 불투명 (CT 보이게)

	//0.5 메쉬
	float finalAlpha = lerp(CTBlendParams.w, 0.8, boneMask);  // ⭐ 피부(0.3) → 뼈(0.95)

	return float4(color, finalAlpha);
}