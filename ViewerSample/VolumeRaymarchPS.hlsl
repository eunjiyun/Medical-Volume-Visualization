
cbuffer CB : register(b0)
{
	matrix View;
	matrix Proj;
	matrix InvView;
	matrix InvProj;
	matrix VolumeWorld;
	matrix InvVolumeWorld;
	float3 CameraPosWS;
	float Step;
	int   MaxSteps;
	float3 Voxel;
	//float Pad0;

	//// 🔽 추가
	//float  HuSlope;        // RescaleSlope
	//float  HuIntercept;    // RescaleIntercept
	//float  HuMin;          // 윈도우/TF용 HU 최소값 (예: -1000)
	//float  HuMax;          // 윈도우/TF용 HU 최대값 (예: 3000)

	float4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
};

Texture3D<float> volumeTex : register(t0);
//Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);


//
float4 TransferFunction(float density)
{
	// ✅ 0.001 이하만 제거 (거의 전부 사용)
	if (density < 0.001)
		return float4(0, 0, 0, 0);

	// ✅ 연조직 - 어두운 베이지
	if (density < 0.01)
	{
		float t = (density - 0.001) / 0.009;
		//return float4(0.6, 0.45, 0.35, t * 0.5);
		return float4(0.5, 0.35, 0.25, t * 0.4);  // 0.5 → 0.4
	}

	// ✅ 뼈 - 밝은 베이지
	if (density < 0.05)
	{
		float t = (density - 0.01) / 0.04;
		//return float4(0.88, 0.78, 0.68, 0.6 + t * 1.0);
		return float4(0.85, 0.70, 0.55, 0.5 + t * 0.8);  // 노란 베이지
	}

	// ✅ 치아 - 아이보리 (매우 밝고 불투명)
	//return float4(0.98, 0.95, 0.90, 2.0);
	return float4(0.98, 0.95, 0.92, 2.2);  // 2.0 → 2.2
}

//
//float4 TransferFunctionHU(float hu, float huNorm)
//{
//	// 1) 공기 : 완전 투명
//	if (hu < -800.0)
//		return float4(0, 0, 0, 0);
//
//	// 2) 연조직 (soft tissue) - 살짝만 보이게
//	if (hu < 150.0)
//	{
//		float t = saturate((hu + 800.0) / 950.0);   // -800 ~ 150
//		float3 col = lerp(float3(0.25, 0.20, 0.18), float3(0.45, 0.35, 0.30), t);
//		float  a = t * 0.3;                       // 알파 약하게
//		return float4(col, a);
//	}
//
//	// 3) 해면골 (trabecular bone)
//	if (hu < 700.0)
//	{
//		float t = saturate((hu - 150.0) / 550.0);   // 150 ~ 700
//		float3 col = lerp(float3(0.75, 0.65, 0.55), float3(0.85, 0.72, 0.60), t);
//		float  a = 0.3 + t * 0.7;
//		return float4(col, a);
//	}
//
//	// 4) 피질골/치아 (cortical bone / enamel)
//	{
//		float t = saturate((hu - 700.0) / 800.0);   // 700 ~ 1500+
//		float3 col = lerp(float3(0.90, 0.85, 0.80), float3(0.98, 0.95, 0.92), t);
//		float  a = 1.2 + t * 1.0;                 // 꽤 불투명
//		return float4(col, a);
//	}
//}

//float4 TransferFunctionHU(float hu, float huNorm)
//{
//	// 공기/배경 제거 - threshold 조정
//	if (hu < 200.0)  // -800 → 200으로 변경
//		return float4(0, 0, 0, 0);
//
//	// 연조직 - 거의 안 보이게
//	if (hu < 500.0)
//	{
//		float t = saturate((hu - 200.0) / 300.0);
//		float3 col = lerp(float3(0.3, 0.25, 0.2), float3(0.5, 0.4, 0.35), t);
//		return float4(col, t * 0.05);  // alpha 매우 낮게
//	}
//
//	// 해면골
//	if (hu < 1200.0)
//	{
//		float t = saturate((hu - 500.0) / 700.0);
//		float3 col = lerp(float3(0.75, 0.65, 0.55), float3(0.88, 0.78, 0.68), t);
//		return float4(col, 0.3 + t * 0.7);
//	}
//
//	// 피질골/치아
//	float t = saturate((hu - 1200.0) / 1800.0);
//	float3 col = lerp(float3(0.90, 0.85, 0.78), float3(0.98, 0.95, 0.90), t);
//	return float4(col, 1.2 + t * 1.0);
//}







//float4 TransferFunctionHU(float hu, float huNorm)
//{
//	// 공기/배경 완전 제거
//	if (hu < 300.0)
//		return float4(0, 0, 0, 0);
//
//	// 연조직 - 거의 투명
//	if (hu < 600.0)
//	{
//		float t = saturate((hu - 300.0) / 300.0);
//		return float4(0.4, 0.35, 0.3, t * 0.02);  // 거의 안 보이게
//	}
//
//	// 해면골 - 반투명
//	if (hu < 1200.0)
//	{
//		float t = saturate((hu - 600.0) / 600.0);
//		float3 col = lerp(float3(0.78, 0.68, 0.58), float3(0.88, 0.78, 0.68), t);
//		return float4(col, 0.5 + t * 1.0);  // alpha 높임
//	}
//
//	// 피질골/치아 - 불투명
//	float t = saturate((hu - 1200.0) / 1500.0);
//	float3 col = lerp(float3(0.92, 0.87, 0.80), float3(0.98, 0.95, 0.90), t);
//	return float4(col, 2.0 + t * 1.5);  // alpha 더 높임
//}



// 1. TF에서 alpha 더 높이기
float4 TransferFunctionHU(float hu, float huNorm)
{
	if (hu < 450.0)  // 400 → 450 (노이즈 더 제거)
		return float4(0, 0, 0, 0);

	if (hu < 750.0)
	{
		float t = saturate((hu - 450.0) / 300.0);
		return float4(0.5, 0.42, 0.36, t * 0.005);  // 거의 투명
	}

	if (hu < 1300.0)
	{
		float t = saturate((hu - 750.0) / 550.0);
		float3 col = lerp(float3(0.82, 0.72, 0.62), float3(0.92, 0.82, 0.72), t);
		return float4(col, 0.8 + t * 1.5);  // alpha 높임
	}

	float t = saturate((hu - 1300.0) / 1200.0);
	float3 col = lerp(float3(0.94, 0.89, 0.82), float3(0.99, 0.96, 0.92), t);
	return float4(col, 3.0 + t * 2.0);  // 더 불투명
}
float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- 愿묒꽑 ?앹꽦 (湲곗〈 肄붾뱶 ?좎?) ---
	float2 offset = float2(0.0, 0.0);
	float2 scale = float2(0.5, 0.5);
	float2 localUV = (uv - offset) / scale;
	//float2 screenUV = localUV;
	float2 screenUV = uv;

	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;
	//ndc.x = -ndc.x;
	

	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);
	viewDirVS /= viewDirVS.w;

	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosWS;


	//return float4(rayPosWS / 10.0, 1);  // 10?쇰줈 ?섎닠???쒓컖??


	//// CameraPosWS 臾댁떆?섍퀬 怨좎젙媛??ъ슜
	///*float3*/ rayPosWS = float3(0, 0, -3.0);  // ?섎뱶肄붾뵫

	//float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//return float4((rayPos + 2.0) / 4.0, 1);





	// ????以??뚯뒪??	//rayDirWS = -rayDirWS;

	//return float4(abs(rayDirWS), 1);


rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;

	//// ??踰붿쐞 議곗젙?댁꽌 ?쒓컖??	//return float4(
	//	rayPos.x / 5.0 + 0.5,  // -2.5~2.5 ??0~1
	//	rayPos.y / 5.0 + 0.5,
	//	rayPos.z / 5.0 + 0.5,
	//	1
	//	);
	




	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//// ??rayDir ?뺤씤
	//return float4(abs(rayDir), 1);


	//// ??移대찓???꾩튂 ?뺤씤
	//return float4(rayPos, 1);//==>>?ш린媛 洹쇰낯?곸씤 臾몄젣??
	//rayDir = -rayDir;

	//// --- Ray-box 援먯감 ---
	//float3 boxMin = float3(0, 0, 0);
	//float3 boxMax = float3(1, 1, 1);

	// ???섏젙 (volumeSize = 1.5 湲곗?)
	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);



	//// ??rayPos媛 諛뺤뒪 ?덉씤媛?
	//if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	//	return float4(1, 0, 0, 1);  // 鍮④컯 = ??	//else
	//	return float4(0, 1, 0, 1);  // 珥덈줉 = 諛?




	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;




	//// ??tMin, tMax ?뺤씤
	////return float4(abs(tMin) / 10.0, 1);  // tMin???됱긽?쇰줈
	//// ?먮뒗
	//return float4(abs(tMax) / 10.0, 1);  // tMax瑜??됱긽?쇰줈

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	//// ??t1 ?뺤씤
	////return float4(abs(t1) / 10.0, 1);
	//// ??t2 ?뺤씤
	// return float4(abs(t2) / 10.0, 1);

	//// ??????異쒕젰
	//return float4(tNear / 10.0, tFar / 10.0, 0, 1);
	//// 鍮④컯 = tNear, 珥덈줉 = tFar



	//// ???붾쾭源? 援먯감 ?щ?
	//if (tNear > tFar || tFar < 0)
	//	return float4(1, 0, 0, 1);  // 鍮④컯 = 援먯감 ????	//else
	//	return float4(0, 1, 0, 1);  // 珥덈줉 = 援먯감 ??
	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;

	// ??諛뺤뒪 踰붿쐞??留욊쾶 ?섏젙
	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  // 移대찓?쇨? 諛뺤뒪 ??	
	}

	// ??Step ?ш린 怨꾩궛
	float stepSize = travelDist / float(MaxSteps);

	// ✅ Jittering
	float jitter = frac(sin(dot(uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
	// 시작점에 랜덤 오프셋
	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);

	// ??startPos瑜?諛뺤뒪 ?덉쑝濡?媛뺤젣
	startPos = clamp(startPos, boxMin, boxMax);

	//// ??startPos瑜??됱긽?쇰줈 ?쒖떆
	//return float4((startPos + 0.75) / 1.5, 1);
	//// [-0.75, 0.75] ??[0, 1] 蹂??

	//// ??tNear 媛??뺤씤
	//return float4(tNear / 5.0, tNear / 5.0, tNear / 5.0, 1);
	//// 0~5 踰붿쐞瑜?0~1濡??뺢퇋??

	//// ???붾쾭源? ?쒖옉 ?꾩튂 ?됱긽?쇰줈 ?쒖떆=>臾몄젣 諛쒓껄
	//return float4(startPos, 1);



	// ??泥??섑뵆 ?꾩튂
	float3 currentPos = startPos;

	//// ??UV 蹂??	//float3 uvw = (currentPos + 0.75) / 1.5;
	// ???щ컮瑜?UV 蹂??	
	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);


	//// UV 踰붿쐞 泥댄겕
	//if (any(uvw < 0.0) || any(uvw > 1.0))
	//	return float4(1, 0, 0, 1);  // 鍮④컙??= 踰붿쐞 諛?	//else
	//	return float4(uvw, 1);  // 洹몃씪?붿뼵?몄뿬????

	// 紐⑤뱺 ?붾쾭源?return 二쇱꽍泥섎━?섍퀬
// 蹂쇰ⅷ ?곷텇 ?꾩뿉 異붽?:

//// ???띿뒪泥?以묒븰媛??뺤씤
//	float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;
//	return float4(testDensity * 10.0, testDensity * 10.0, testDensity * 10.0, 1);


	//float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;

	//// ??利앺룺 ?놁씠 ?먮낯 ?뺤씤
	//return float4(testDensity, testDensity, testDensity, 1);


	// --- 蹂쇰ⅷ ?곷텇 ---
	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

	//// 猷⑦봽 ?꾩뿉
	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		//// ???꾩옱 ?꾩튂 怨꾩궛 (i???곕씪 ?꾩쭊)
		//float3 uvw = startPos + rayDir * (i * stepSize);

		// ???щ컮瑜?怨꾩궛
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		uvw.y = 1.0 - uvw.y;  // ✅ 추가




		// 踰붿쐞 泥댄겕
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		// 1) Raw 기반 density
		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
		float density = raw / 255.0;    // TF 전용





		//// 2) DICOM HU 로 변환
		float hu = raw * HuParams.x + HuParams.y;   // -1000 ~ 3000 같은 범위


		//// 루프 전에
		//float3 testUVW = float3(0.5, 0.5, 0.5);
		//float testRaw = volumeTex.SampleLevel(samp, testUVW, 0).r;
		//float testHU = testRaw * HuParams.x + HuParams.y;

		//// HU 범위 확인 (정상이면 -1000 ~ 3000)
		//return float4(
		//	saturate(testRaw / 65535.0),      // R: raw가 큰 값인지
		//	saturate((testHU + 1000.0) / 4000.0),  // G: HU가 정상 범위인지
		//	0,
		//	1
		//	);



		// 3) 윈도우/레벨 범위로 정규화 (0~1)
		float huNorm = (hu - HuParams.z) / (HuParams.w - HuParams.z);
		huNorm = saturate(huNorm);


		// 2) TF 적용
	//	float4 colorAlpha = TransferFunction(density);
		// 4) TF에 넘길 값으로 사용
		float4 colorAlpha = TransferFunctionHU(hu, huNorm);


		//// 색상이 있는지 확인 - 빨강 채널만 강조
		//if (colorAlpha.a > 0.1)
		//{
		//	colorAlpha.rgb = float3(0.9, 0.7, 0.5);  // 강제로 베이지색
		//}

		// 3) eps 계산
		float3 eps = float3(1.0 / Voxel.x, 1.0 / Voxel.y, 1.0 / Voxel.z);

		// 4) raw 기반 gradient
		float dx =
			volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;

		float dy =
			volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;

		float dz =
			volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
			volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;

		// 조명 계산 부분에서
		float3 N = normalize(float3(dx, dy, dz) + 1e-6);
		float3 L = normalize(float3(0.5, 0.7, -0.5));
		float3 V = -rayDir;  // 뷰 방향
		float3 H = normalize(L + V);  // 하프 벡터

		float lambert = max(dot(N, L), 0.0);
		float spec = pow(max(dot(N, H), 0.0), 48.0);  // 광택

		float lighting = 0.45 + lambert * 0.75;
		colorAlpha.rgb *= lighting;
		colorAlpha.rgb += spec * float3(0.2, 0.18, 0.15);  // 따뜻한 하이라이트


		float3 color = colorAlpha.rgb;
		float alpha = colorAlpha.a * stepSize * 8.0;
	



	//	float3 color = colorAlpha.rgb;







		////float alpha = colorAlpha.a * stepSize;  // stepSize 怨깊븯湲?
		//float alpha = colorAlpha.a * stepSize * 4.0;  // ✅ 투명도 강화 배율




		//// 변경
		//float alpha = colorAlpha.a * stepSize * 8.0;   // 4.0 → 8.0


		//float4 colorAlpha = TransferFunction(density);
		//float3 color = colorAlpha.rgb;
		//float alpha = colorAlpha.a;  // ✅ stepSize 곱하지 않음!

		//// 2. 공기 threshold 더 높이기 (노이즈 제거)
		//if (hu < 400.0)  // 300 → 400
		//	return float4(0, 0, 0, 0);


		//// 3. 후처리에서 콘트라스트 추가
		//acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
		//acc.rgb = (acc.rgb - 0.5) * 1.2 + 0.5;  // 콘트라스트
		//acc.rgb *= float3(1.0, 0.95, 0.88);
		//acc.rgb = saturate(acc.rgb);
	

		if (alpha > 0.001)
		{
			////float3 color = float3(density, density, density);

			//// Front-to-back 釉붾젋??			
			//acc.rgb += (1.0 - acc.a) * alpha * color;
			//acc.a += (1.0 - acc.a) * alpha;

			//// Early termination
			//if (acc.a >= 0.95)
			//	break;

			acc.rgb += (1.0 - acc.a) * alpha * color;

			//acc.rgb = pow(acc.rgb, 1.0 / 2.2); // 감마 보정




			acc.a += (1.0 - acc.a) * alpha;

			if (acc.a >= 0.95)
				break;
		}


		/*if (alpha > 0.001)
		{
			float value = density;
			float3 color = float3(value, value, value);

			acc.rgb += (1.0 - acc.a) * alpha * color;
			acc.a += (1.0 - acc.a) * alpha;

			if (acc.a >= 0.95)
				break;
		}*/
	}


	//// Post-processing
	//acc.rgb *= 1.2;  // ✅ 1.4 → 1.2 (약간만)
	//acc.rgb = (acc.rgb - 0.5) * 1.25 + 0.5;
	//acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);

	//return float4(acc.rgb, acc.a);

	// ✅ 감마 보정만 (선택)
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);


	//// 베이지 색조 추가
	//acc.rgb *= float3(1.0, 0.95, 0.88);  // 약간 따뜻한 톤

// 후처리 부분 수정
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
	acc.rgb *= 0.95;  // 1.15 → 0.95 (밝기 줄임)
	acc.rgb = (acc.rgb - 0.5) * 1.3 + 0.5;  // 콘트라스트 더 높임
	acc.rgb *= float3(1.0, 0.95, 0.88);
	acc.rgb = saturate(acc.rgb);


	return float4(acc.rgb, 1.0);



//// 1단계: Raw 값 자체 확인
//	float3 testUVW = float3(0.5, 0.5, 0.5);
//	float testRaw = volumeTex.SampleLevel(samp, testUVW, 0).r;
//	return float4(testRaw / 65535.0, testRaw / 65535.0, testRaw / 65535.0, 1);

	//// 2단계: HuParams 값 확인
	//return float4(
	//	HuParams.x * 100.0,  // Slope (보통 1.0)
	//	abs(HuParams.y) / 1000.0,  // Intercept (보통 -1024)
	//	HuParams.w / 3000.0,  // Max
	//	1
	//	);

}

