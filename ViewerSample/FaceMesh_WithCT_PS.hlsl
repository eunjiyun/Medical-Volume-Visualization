// FaceMesh_WithCT_PS.hlsl

Texture2D faceColorTex : register(t0);  // 얼굴 색상
Texture2D ctTex : register(t1);         // CT 텍스처 (1단계 결과)
SamplerState linearSamp : register(s0);
SamplerState pointClamp : register(s1); // 포인트/클램프 (디버그용)
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

	float4 viewZ : TEXCOORD2;  //  추가: View Space Z
};
float4 main(PSInput input) : SV_Target
{
	/* ============================
	   Input
	============================ */

	float3 faceColor = faceColorTex.Sample(linearSamp, input.uv).rgb;
	float4 ct = ctTex.Sample(linearSamp, input.screenUV); // rgb + a


	float meshDepth = SceneDepth.Sample(pointClamp, input.uv).r;
	float hasFace = step(meshDepth, 0.999); // 메쉬가 있는 픽셀만
	
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

	boneMask = smoothstep(0.12, 0.75, boneMask);
	boneMask = pow(boneMask, 0.7);   //  중심 강화



	boneMask *= hasFace;


	/* ============================
	   2️ CT occlusion field
	   - "어두워질 영역"만 만든다
	============================ */

	// 밝기는 강하게, 영향은 국소적으로
	float ctField = pow(ctLuma, 0.7);

	// 깊을수록 조금 더 영향
	ctField *= lerp(0.7, 1.1, ctAlpha);
	ctField = saturate(ctField);

	/* ============================
	   3️ 국소 음영만 추출 (핵심)
	============================ */

	float ctPresence = smoothstep(0.25, 0.45, ctField);
	// 0.25 이하는 CT 없음 취급, 0.45 이상만 확실

	float localShadow = ctField * boneMask * ctPresence;

	//// CT가 강한 곳만 음영으로
	//float localShadow = ctField * boneMask;

	// 전체 얼굴 덮지 않게 컷
	localShadow = saturate(localShadow - 0.15);

	// 매우 약하게!
	//localShadow *= 0.35;   //  핵심 파라미터
	localShadow *= 0.55;   // 0.35 → 0.55

	/* ============================
	   4️ 피부 레이어 (절대 죽이지 말 것)
	============================ */

	// 피부를 지우지 말고 "얇게"
	float skinAtten = lerp(1.0, 0.55, boneMask);
	float3 skinLayer = faceColor * skinAtten;

	/* ============================
	   5️ Planmeca-style carve
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
	   6️ 미세 구조 대비 보정
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
	   7️ Gamma & Output
	============================ */

	color = pow(saturate(color), 1.0 / 2.2);

	//// 알파는 "합성용" 고정
	//float finalAlpha = 1.0;

	float finalAlpha = lerp(CTBlendParams.w*1.5f, 0.6, boneMask);  //  피부(0.3) → 뼈(0.95)

	finalAlpha *= hasFace;

	//color = float3(0.5f, 0.5f, 0.5f);


	float z = input.viewZ;

	// 더 큰 범위
	if (z < 420) return float4(1, 0, 1, finalAlpha);  // 빨강
	if (z < 450) return float4(1, 1, 0, finalAlpha);  // 노랑
	if (z <490) return float4(0, 1, 0, finalAlpha);  // 초록
	if (z <530) return float4(0, 1, 1, finalAlpha);  // 청록
	return float4(0, 0, 1, finalAlpha);  // 파랑
	
	
	return float4(color, finalAlpha);
}

