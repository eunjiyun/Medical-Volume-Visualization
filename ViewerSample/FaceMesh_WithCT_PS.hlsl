// FaceMesh_WithCT_PS.hlsl

Texture2D faceColorTex : register(t0);  // 얼굴 색상
Texture2D ctTex : register(t1);         // CT 텍스처 (1단계 결과)
SamplerState linearSamp : register(s0);
SamplerState pointClamp : register(s1); // 포인트+클램프 추천 (디버그용)

Texture2D<float> SceneDepth : register(t2);

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
float4 main(PSInput input) : SV_Target
{
	/* ============================
	   Input
	============================ */

	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
	float4 ct = ctTex.Sample(linearSamp, input.screenUV); // rgb + a


	//float3 debug = abs(normalize(mul(float4(0, 0, 1, 0), World).xyz));
	//return float4(debug, 1);



	float meshDepth = SceneDepth.Sample(pointClamp, input.uv).r;


	////float d = SceneDepth.SampleLevel(faceColorSamp, uv, 0);
	//return float4(meshDepth, meshDepth, meshDepth, 1);



	float hasFace = step(meshDepth, 0.999); // 메쉬가 있는 픽셀만
	//return float4(hasFace, hasFace, hasFace, 1);

	float ctLuma = dot(ct.rgb, float3(0.299, 0.587, 0.114));
	float ctAlpha = saturate(ct.a);   // 깊이 proxy (0~1)

	// 경계 불안정 픽셀 제거
	ctAlpha = smoothstep(0.05, 0.2, ctAlpha);

	/* ============================
	   1️⃣ Bone mask (넓고 부드럽게)
	   - 절대 하드하지 않게
	============================ */

	float boneMask =
		ctLuma * 0.65 +     // 구조 정보
		ctAlpha * 0.35;      // 깊이 보조

	boneMask *= ctAlpha;   // 핵심 한 줄

	boneMask = saturate(boneMask);



	//boneMask = smoothstep(0.35, 0.85, boneMask);
	//boneMask = pow(boneMask, 1.15);   // 연결성 강화

	//boneMask = smoothstep(0.15, 0.8, boneMask);
	//boneMask = pow(boneMask, 0.85);   //  약한 영역 확장

	boneMask = smoothstep(0.12, 0.75, boneMask);
	boneMask = pow(boneMask, 0.7);   //  중심 강화



	boneMask *= hasFace;


	/* ============================
	   2️⃣ CT occlusion field
	   - "어두워질 영역"만 만든다
	============================ */

	// 밝기는 강하게, 영향은 국소적으로
	float ctField = pow(ctLuma, 0.7);

	// 깊을수록 조금 더 영향
	ctField *= lerp(0.7, 1.1, ctAlpha);
	ctField = saturate(ctField);

	/* ============================
	   3️⃣ 국소 음영만 추출 (핵심)
	============================ */

	float ctPresence = smoothstep(0.25, 0.45, ctField);
	// 0.25 이하는 CT 없음 취급, 0.45 이상만 확실

	float localShadow = ctField * boneMask * ctPresence;

	//// CT가 강한 곳만 음영으로
	//float localShadow = ctField * boneMask;

	// 전체 얼굴 덮지 않게 컷
	localShadow = saturate(localShadow - 0.15);

	// 매우 약하게!
	//localShadow *= 0.35;   // ⭐⭐⭐ 핵심 파라미터
	localShadow *= 0.55;   // 0.35 → 0.55

	/* ============================
	   4️⃣ 피부 레이어 (절대 죽이지 말 것)
	============================ */

	// 피부를 지우지 말고 "얇게"
	float skinAtten = lerp(1.0, 0.55, boneMask);
	float3 skinLayer = faceColor * skinAtten;

	/* ============================
	   5️⃣ Planmeca-style carve
	   - CT = 색 
	   - CT = 음영 
	============================ */

	//float3 color = skinLayer * (1.0 - localShadow);

	// 어두워지는 양 (아주 약하게)
	//float shadow = localShadow * 0.25;
	float shadow = localShadow * 0.35; // 0.25 → 0.35

	// 밝아지는 양 (핵심)
	//float lift = boneMask * 0.15;
	float lift = boneMask * (0.15 * ctAlpha);

	float3 color = skinLayer * (1.0 - shadow);

	//float3 color = float3(1, 1, 1);// = skinLayer * (1.0 - shadow);


	color += lift.xxx;

	/* ============================
	   6️⃣ 미세 구조 대비 보정
	============================ */

	// 뼈를 하얗게 
	// 대신 대비만 살짝
	//color = lerp(color, color * 1.05, boneMask * 0.25);

	//float3 boneTint = float3(1.0, 0.95, 0.85); // 누런 흰색
	float3 boneTint = float3(1.0, 0.96, 0.88); // 조금 더 밝게
	float boneTintStrength = boneMask * 0.18; // 0.12 → 0.18
	//color = lerp(color, boneTint, boneMask * 0.12);

	color = lerp(color, boneTint, boneTintStrength);
	color = lerp(color, color * 1.08, boneMask * 0.25);

	/* ============================
	   7️⃣ Gamma & Output
	============================ */

	color = pow(saturate(color), 1.0 / 2.2);

	//// 알파는 "합성용" 고정
	//float finalAlpha = 1.0;

	float finalAlpha = lerp(CTBlendParams.w*1.5f, 0.6, boneMask);  // ⭐ 피부(0.3) → 뼈(0.95)

	finalAlpha *= hasFace;

	return float4(color, finalAlpha);
}


//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//	float4 ct = ctTex.Sample(linearSamp, input.screenUV);
//
//	float density = ct.r;
//	float edge = ct.g;
//	float coverage = ct.a;
//
//	// density 자체를 회색으로
//	//return float4(density.xxx, 1);
//
//	float densityLF = smoothstep(0.15, 0.6, density);
//	densityLF = pow(densityLF, 0.6);
//
//	//float applyMask = smoothstep(0.1, 0.35, coverage);
//
//	float applyMask = smoothstep(0.25, 0.6, density);
//
//	//return float4(faceColor * applyMask, 1.0);
//
//	// carve (음영)
//	float carve = densityLF * applyMask;
//	float shadow = carve * 0.35;
//	float3 color = faceColor * (1.0 - shadow);
//
//	// edge (윤곽만)
//	float edgeLF = pow(edge, 0.7);
//	float edgeMask = smoothstep(0.25, 0.6, densityLF);
//	float edgeStrength = edgeLF * edgeMask * applyMask * 0.25;
//	color *= (1.0 + edgeStrength);
//
//	// bone lift (아주 약하게)
//	color += densityLF * applyMask * 0.08;
//
//	color = pow(saturate(color), 1.0 / 2.2);
//
//	//return float4(color, 1.0);
//
//
//		float finalAlpha = lerp(CTBlendParams.w, 0.8, edgeMask);  // ⭐ 피부(0.3) → 뼈(0.95)
//
//	return float4(color, finalAlpha);
//}




//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//float4 ct = ctTex.Sample(linearSamp, input.screenUV); // rgb + a
//
//float ctLuma = dot(ct.rgb, float3(0.299, 0.587, 0.114));
//float ctAlpha = saturate(ct.a * 1.4);   // ⭐ 깊이 proxy
//
///* ----------------------------
//   Bone mask (핵심)
//---------------------------- */
//
//// 밝기 + 깊이 혼합
//float boneMask = saturate(
//	ctLuma * 0.75 +     // 밝기 비중 ↑
//	ctAlpha * 0.45      // 깊이 비중 ↓ (핵심)
//);
//
////return float4(boneMask.xxx, 1);
//
//boneMask = smoothstep(0.3, 0.85, boneMask);
//boneMask = pow(boneMask, 1.25);
//
///* ----------------------------
//   CT shadow field
//---------------------------- */
//
//// CT를 색이 아닌 "음영 필드"로
//float ctField = pow(ctLuma, 0.55);
//
//
//ctField *= lerp(0.6, 1.2, ctAlpha);
//ctField = saturate(ctField);
////return float4(ctField.xxx, 1);
//
///* ----------------------------
//   Skin attenuation
//---------------------------- */
//
////float skinTrans = lerp(1.0, 0.32, boneMask);
//float skinTrans = lerp(1.0, 0.45, boneMask);
//float3 skinLayer = faceColor * skinTrans;
//
///* ----------------------------
//   Planmeca-style carve
//---------------------------- */
//
////float shadowStrength = 0.85;   // ⭐ 중요
//float shadowStrength = 0.4f; // 0.85 → 0.55
//float shadow = ctField * boneMask * shadowStrength;
//
//float3 color = skinLayer * (1.0 - shadow);
//
///* ----------------------------
//   Bone highlight (절제)
//---------------------------- */
//
//float boneHL = boneMask * 0.18;
//color += boneHL.xxx;
//
///* ----------------------------
//   Output
//---------------------------- */
//
//color = pow(saturate(color), 1.0 / 2.2);
//
//// 알파는 "합성용"이지 가시성 아님
//float finalAlpha = lerp(0.25, 0.85, boneMask);
//
//return float4(color, finalAlpha);
//
//}

//float4 main(PSInput input) : SV_Target
//{
//	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
//	float3 ctColor = ctTex.Sample(linearSamp, input.screenUV).rgb;
//	//float3 ctColor = float3(0, 0, 0);
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
//	float skinTrans = lerp(1.0, 0.35, boneMask); // ⭐ 피부 얇게
//	float3 skinLayer = faceColor * skinTrans;
//
//	/* ============================
//	   4️⃣ 플랜메카식 합성 (핵심)
//	   - 뼈 = 얼굴을 깎는 음영
//	============================ */
//
//	// ⭐ CT 강도 증폭 (볼륨 더 보이게)
//	float shadowStrength = 0.75; // ⭐ 0.45 → 0.65 (CT 더 강하게)
//	float shadow = ctField * boneMask * shadowStrength;
//
//	float3 color = skinLayer * (1.0 - shadow);
//
//	/* ============================
//	   5️⃣ Bone highlight (화이트 복구)
//	============================ */
//
//	// ⭐ Bone highlight 증폭 (뼈 더 밝게)
//	float boneHighlight = boneMask * 0.2;  // ⭐ 0.25 → 0.4
//	color += boneHighlight.xxx;
//
//	/* ============================
//	   6️⃣ 마무리
//	============================ */
//
//	color = pow(saturate(color), 1.0 / 2.2);
//
//	// ⭐ 알파 계산: bone이 많을수록 불투명 (CT 보이게)
//
//	//0.5 메쉬
//	float finalAlpha = lerp(CTBlendParams.w*1.5f, 0.4, boneMask);  // ⭐ 피부(0.3) → 뼈(0.95)
//
//	return float4(color, finalAlpha);
//}