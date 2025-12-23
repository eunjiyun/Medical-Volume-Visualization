cbuffer CB : register(b0)
{
	matrix InvView;
	matrix InvProj;
	matrix InvVolumeWorld;

	matrix View;
	matrix Projection;

	float4 CameraPosAndAlpha;   // xyz = camera pos, w = mode
	float4 VoxelAndMaxSteps;    // xyz = voxel dim, w = maxSteps
	float4 HuParams;            // x,y unused / z = HU min / w = HU max
};

Texture3D<float> volumeTex        : register(t0);
SamplerState     samp             : register(s0);

Texture1D<float4> transferFunction : register(t1);
SamplerState      tfSampler        : register(s1);

// mesh depth (0~1)
Texture2D<float> SceneDepth : register(t5);
Texture2D<float> faceColor : register(t6);


// mesh depth (0~1)
SamplerState pointClamp : register(s5); // 포인트+클램프 추천 (디버그용)
SamplerState faceColorSamp : register(s6); // 포인트+클램프 추천 (디버그용)



struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};


// depth01 : 0~1 depth buffer value
// proj    : Projection matrix (same one used for rendering)
float ReconstructViewZ_InvProj(float2 uv, float depth01, matrix proj)
{
	//float2 uv = input.uv;

	//uv.x *= 0.5;
	//uv.y *= 0.5;

	float2 ndc = uv * 2.0f - 1.0f;
	//ndc.y = -ndc.y;
	ndc.y = 1.0f - uv.y * 2.0f; // D3D flip

	float z_ndc = depth01 * 2 - 1;   // ❗ 반드시 필요

	float4 clip = float4(ndc.x, ndc.y, z_ndc, 1.0f);
	float4 view = mul(clip, InvProj);   // 너가 row-vector 스타일이면 mul(v, M) 유지
	view /= max(view.w, 1e-6);

	return view.z; // view-space z
}



float4 main(PSInput input) : SV_Target
{
	float2 screenUV = input.uv;

	// ⭐ UV 수정 (좌상단 쿼드용)
	float2 uv = input.uv;
	uv.x *= 0.5;
	uv.y *= 0.5;

	// NDC 계산
	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;

	// Ray setup
	float4 farVS = mul(float4(ndc, 1, 1), InvProj);
	farVS /= max(farVS.w, 1e-6);
	float3 rayDirVS = normalize(farVS.xyz);

	float3 rayDirWS = normalize(mul(float4(farVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosAndAlpha.xyz;

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	// Volume box intersection
	float3 boxMin = float3(-1, -0.75, -0.75);
	float3 boxMax = float3(1,  0.75,  0.75);

	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	tNear = max(tNear, 0.0);

	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
		tNear = 0.0;

	float travelDist = tFar - tNear;
	float stepSize = travelDist / VoxelAndMaxSteps.w;

	// ========== Mesh depth ==========
	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
	bool hasMesh = (meshDepth01 < 0.9999);
	float meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;

	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;

	// ========== Accumulation ==========
	float4 acc = float4(0, 0, 0, 0);


	// 🔽 여기!!!
	float3 toothAccumRGB = float3(0, 0, 0);
	float  toothAccumA = 0.0;
	int    toothCount = 0;
	float  bestToothScore = 0.0;
	float3 bestToothRGB = 0;
	float  bestToothA = 0.0;
	bool   foundTooth = false;

	// tooth confidence (0~1)
	float toothW = 0.0;


	float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
	float hu;
	// tooth outline candidate (best edge on this pixel)
	float  bestEdgeW = 0.0;
	float3 bestEdgeRGB = 0;
	bool   foundEdge = false;
	float  bestEdgeT = 1e9;   // (선택) 가장 앞쪽(edge) 선택용
	// ========== Ray marching ==========
	[loop]
	for (int i = 0; i < VoxelAndMaxSteps.w; i++)
	{
		float tCurrent = tNear + i * stepSize;
		float3 currentPos = startPos + rayDir * (i * stepSize);

		// UVW
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
		uvw.y = 1.0 - uvw.y;

		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;

		// View space position
		float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
		float rayViewZ = currentPosVS.z;

		// ========== Depth comparison ==========
		float distVS = rayViewZ - meshViewZ;



		// ========== Hard block (매우 좁게) ==========
		if (hasMesh && distVS < -0.2)  // ⭐ -2.0 → -0.5 (거의 차단 안 함)
			continue;

		// ========== Volume sampling ==========
		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
		hu = raw;

		//if (hu > 2000 &&input.uv.y>0.3f)
		//	return float4(1, 0, 0, 0);

		if (CameraPosAndAlpha.w == 2.0 && hu < 400)
			continue;

		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
		float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);

		if (ca.a < 0.001)
			continue;

	

		// ========== Material classification ==========
		bool isBone = (hu > 700.0 && hu < 1800.0);


		float3 eps = 1.0 / VoxelAndMaxSteps.xyz;
		float3 g;
		g.x = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
		g.y = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
		g.z = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;
		float gradMag = length(g);
		// CT 기준: 경험적으로 이 범위가 맞음
		float gradN = saturate(gradMag * 0.0002);
		//float gradN = gradMag * 0.0002;   // saturate 제거
	/*	return float4(
			gradN > 0.25 && gradN < 0.6 ? 1 : 0,
			0,
			0,
			1
			);*/
		float y = currentPosVS.y;
		float yN = saturate((y + 10.0) / 20.0); // [-10 ~ +10] 가정
	//	return float4(yN, yN, yN, 1);

		bool huCandidate = (hu > 2000);
		bool edgeGate = (gradN > 0.25 && gradN < 0.6);
		bool nearSkin = (distVS > 0.0 && distVS < 6.0);
	//	bool frontFace = (distVS > 0.0 && distVS < 2.5);
		bool frontTeeth = (uvw.z > 0.55);   // 값은 CT마다 조정
		
		float yVS = currentPosVS.y;
		bool lowerFace = (yVS < -5.0);   // 값은 디버그로 튜닝

		bool toothEdge =
			hu > 2000.0 &&
			gradN > 0.25 && gradN < 0.6 &&
			distVS > 0.2 && distVS < 1.2 &&
			uvw.y < 0.45 &&uvw.y > 0.2 &&
			uvw.z > 0.55;

	//	return float4(toothEdge ? 1 : 0, 0, 0, 1);

		

		bool toothHU = (hu > 1200.0);

	//	float edge = smoothstep(0.4, 0.7, gradN);
		//float contour = edge * (toothHU ? 1.0 : 0.0) * (nearSkin ? 1.0 : 0.0);
		//float contourAlpha = contour * 0.25;
		float3 contourColor = float3(0.95, 0.92, 0.88);

//		acc.rgb += (1.0 - acc.a) * contourAlpha * contourColor;
		//acc.a += (1.0 - acc.a) * contourAlpha;
		


		bool isToothCandidate = (hu >3000);
		
		bool isTooth =
			isToothCandidate &&
			gradMag > 0.04 &&          // ★ 경계가 날카로움
			gradN > 0.25 && gradN < 0.6 &&
			distVS > 0.0 &&            // 피부 안쪽
			distVS < 1.5;              // 피부 바로 아래 (앞니!)
		//bool isTooth = (hu > 3000);



		// ⭐ Base unified color (베이지/황토색)
		float3 unifiedColor = float3(0.82, 0.72, 0.64);  // 전체 기본 색상

		//// HU별 색상 변화 (미묘하게)
		//if (toothEdge)
		//{
		//	/*float lineAlpha = 0.4;
		//	float3 lineColor = float3(0.95, 0.92, 0.88);

		//	acc.rgb += (1.0 - acc.a) * lineAlpha * lineColor;
		//	acc.a += (1.0 - acc.a) * lineAlpha;*/

		//	// 경계 강조 (정면보단 측면에서 더 잘 보이게)
		//	float edge = saturate((gradMag - 0.03) * 25.0); // 0~1

		//	// 색을 바꾸지 말고 "밝기"만 살짝
		//	ca.rgb *= (1.0 + edge * 0.35);   // 0.25~0.45 튜닝

		//	// 알파도 아주 소폭만
		//	ca.a *= (1.0 + edge * 0.25);

		//	return float4(1,0,0,0);
		//}
		if (isBone)
		{
			// 뼈: 기본 색상과 비슷
			unifiedColor = float3(0.85, 0.75, 0.68);
		}
		else
		{
			// 연조직: 약간 더 붉은 톤
			unifiedColor = float3(0.80, 0.68, 0.60);
		}

		// ⭐ Transfer function 색상을 통일 색상으로 대체
		float originalLuminance = dot(ca.rgb, float3(0.299, 0.587, 0.114));
		ca.rgb = unifiedColor * (0.8 + originalLuminance * 0.2);  // 밝기만 약간 반영


		// ========== Skin transition (수정!) ==========
		float skinDepthVS = 12;  // ⭐ 1.5 → 12로 증가 (12mm)
		float skinFade = saturate(distVS / skinDepthVS);



	
		// HU별 색상 변화 (미묘하게)
		if (isTooth)
		{
			// 1) gradient -> edge strength (0~1)
			float gradScaled = gradMag * 20.0;
			float edge = smoothstep(0.25, 0.75, gradScaled); // 범위만 살짝 안정적으로

			// 2) surface proximity (피부 바로 아래만)
			// distVS가 0~1.2mm 근처에서 strongest
			//float depthW = smoothstep(1.6, 0.2, distVS);     // ✅ "가까울수록 1" 형태

			//float depthW = smoothstep(3.0, 0.3, distVS);  // ← 무조건 이걸로
			float depthW = smoothstep(1.2, 0.3, abs(distVS));  // ★ 핵심

			float w = edge * depthW;                         // 0~1

			// 3) 점 노이즈 제거용: 아주 약한 값은 컷
			if (w > 0.12)  // ✅ 이 threshold가 "점->선" 만드는 핵심
			{
				// (선택) 앞쪽 것을 우선: distVS가 더 작은게 더 표면
				float t = distVS;

				// best 후보만 저장 (누적 X)
				if (!foundEdge || (w > bestEdgeW) || (abs(w - bestEdgeW) < 0.02 && t < bestEdgeT))
				{
					foundEdge = true;
					bestEdgeW = w;
					bestEdgeT = t;

					// 선 색: 밝은 아이보리(치아 느낌)
					//bestEdgeRGB = float3(0.96, 0.95, 0.93);

					bestEdgeRGB = float3(1.0, 0.98, 0.95);  // 거의 흰색
				}
			}

		}
		else
			skinFade = pow(skinFade, 0.2);   // ⭐ 부드러운 전환



		


		//// Skin color tinting
		//if (hasMesh && distVS < skinDepthVS)
		//{
		//	// ⭐ 피부색도 통일 색상과 조화
		//	float3 skinTint = float3(0.85, 0.73, 0.65);  // 전체 톤과 비슷
		//	ca.rgb = lerp(skinTint, ca.rgb, skinFade);

		//	// ⭐ Alpha 최소 억제
		//	ca.a *= lerp(0.85, 1.0, skinFade);  // 0.7 → 0.85 (최소 85%)
		//}


		//if (hasMesh && distVS < skinDepthVS)
		//{
		//	float3 skinTint = float3(0.85, 0.73, 0.65);  // 전체 톤과 비슷
		//	ca.rgb = lerp(skinTint, ca.rgb, skinFade);

		//	float keep = isTooth ? 0.95 : 0.85; // ⭐ 앞니 보호
		//	ca.a *= lerp(keep, 1.0, skinFade);
		//}

		float skinW = saturate(distVS / skinDepthVS);

		
		//if (isTooth)
		//{
		//	float gradMag = length(g);  // 이미 계산한 g 사용
		//	// HU 경계선 강조
		//	float edge = smoothstep(0.02, 0.08, gradMag);

		//	// 색상: 밝기만 살짝
		//	ca.rgb *= (1.0 + edge * 0.35);

		//	// 알파: 경계에서만 증가
		//	ca.a *= (1.0 + edge * 0.5);

		//	// 피부 혼합 억제
		//	skinW *= 0.25;
		//}
	
		gradMag *= 20.0;                         // (1) 스케일 보정
	//	float edge = smoothstep(0.3, 0.9, gradMag);

	//	float contourAlpha = edge * 0.6;         // (2) 일단 세게
	//	float3 contourColor = float3(0.95, 0.92, 0.88);

		if (isTooth && hasMesh && distVS > 0.0 && distVS < 0.15)  // (3) 표면만
		{
		//	acc.rgb += (1.0 - acc.a) * contourAlpha * contourColor;
			//acc.a += (1.0 - acc.a) * contourAlpha;

		
		}

		////float contourAlpha = contour * 0.2;
		//float contourAlpha = contour * 0.5;


		///*if (isTooth && distVS < 2.0)
		//{
		//	ca.a *= 1.2;
		//}*/

		//if (isTooth && distVS < 1.0)
		//	contourAlpha *= 1.5;



		// ========== Bone fog (수정!) ==========
		if (isBone && hasMesh)
		{
			float boneFogStart = 1.0;
			float boneFogRange = 20;  // ⭐ 20 → 8 (범위 축소)

			float fog = saturate((distVS - boneFogStart) / boneFogRange);
			fog = 1.0 - fog;

			// ⭐ Bone 색상 약간만 조정
			float3 fadedBone = float3(0.90, 0.82, 0.75);  // 통일 톤
			ca.rgb = lerp(ca.rgb, fadedBone, fog * 0.2);  // 0.3 → 0.2 (더 미묘)

			// ⭐ Alpha는 거의 약화 안 함
			ca.a *= lerp(0.85, 1.0, 1.0 - fog);  // 0.4 → 0.85 (최소 85% 유지)
		}

	

		// ========== Lighting ==========

		

		float3 N = normalize(g + 1e-6);
		float3 L = normalize(float3(0.5, 0.7, -0.5));
		float3 V = -rayDir;
		float3 H = normalize(L + V);

		float lambert = max(dot(N, L), 0.0);
		float spec = pow(max(dot(N, H), 0.0), 64.0);  // 48 → 64 (더 sharp)


			// ========== Tooth handling ==========


		//if (isTooth)
		//{
		//	float edgeBoost = pow(1.0 - lambert, 2.0); // 경계 강조
		//	ca.rgb *= (1.0 + edgeBoost * 0.25);
		//	ca.a = min(ca.a, 0.75);   // 0.2 → 0.35
		//}
	//	float skinW = saturate(distVS / skinDepthVS);
		


		float lighting = (CameraPosAndAlpha.w == 2.0)
			? (0.92 + lambert * 0.08)  // ⭐ 약간 밝게
			: (0.85 + lambert * 0.15);

		ca.rgb *= lighting;
		ca.rgb += spec * float3(0.03, 0.03, 0.03);  // 강도 감소

		// ========== Accumulation ==========
		float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 15.0 : 22.0;  // ⭐ 증가
		ca.a = saturate(ca.a * 3.0);  // ⭐ 2.0 → 3.0 (더 강하게)

		float mediumTransparency = 0.5;  // ⭐ 0.4 → 0.5
		if (isBone)  mediumTransparency = 0.45;  // ⭐ 0.35 → 0.45
		if (isTooth) mediumTransparency = 0.8;  // 0.25 → 0.3 (치아 더 보이게)

	

		float alpha = ca.a * max(stepSize, 0.002) * alphaScale * mediumTransparency;

		acc.rgb += (1.0 - acc.a) * alpha * ca.rgb;
		acc.a += (1.0 - acc.a) * alpha;


		if (acc.a >= 0.95)
			break;
	}

	//if (foundTooth && bestToothScore > 0.0)
	//{
	///*	acc.rgb = bestToothRGB;
	//	acc.a = bestToothA;*/

	//	float softness = saturate(bestToothScore * 2.0); // 0~1
	//	acc.rgb = lerp(acc.rgb, bestToothRGB, softness);
	//	acc.a = lerp(acc.a, bestToothA, softness);
	//}

	// -------------------------
// Apply tooth outline (thin overlay)
// -------------------------
	if (foundEdge)
	{
		//float outlineA = saturate(bestEdgeW * 0.4);   // ★ 조금 세게

		float outlineA = saturate(bestEdgeW * 0.55); // 0.4 → 0.55

		   // 🔥 기존 볼륨 위에 "선"을 덮어씀
		acc.rgb = acc.rgb * (1.0 - outlineA) + bestEdgeRGB * outlineA;

		// 알파는 건드리지 마라 (중요)
	}


	// ========== Post-processing ==========
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
	
	
		if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
		if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
		if (CameraPosAndAlpha.w == 2.0) return float4(acc.rgb, 0.2);
		return float4(acc.rgb, acc.a);
	
}