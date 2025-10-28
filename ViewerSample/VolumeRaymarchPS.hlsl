//cbuffer CB : register(b0)
//{
//	matrix View;
//	matrix Proj;
//	matrix InvView;
//	matrix InvProj;
//	matrix VolumeWorld;
//	matrix InvVolumeWorld;
//	float3 CameraPosWS;
//	float Step;
//	int   MaxSteps;
//	float Opacity;
//	float pad0;
//	float pad1;
//};
//
//Texture3D<float> volumeTex : register(t0);
//SamplerState samp : register(s0);
//
//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// ??NDC ??View space
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 rayStartVS = mul(ndc, InvProj);
//	rayStartVS /= rayStartVS.w;
//
//	/*float3 rayDirWS = normalize(mul(float4(rayStartVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosWS;*/
//
//
//	// ?ㅽ뿕?? 蹂쇰ⅷ 濡쒖뺄 醫뚰몴媛 0~1?대씪 媛??
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // 移대찓?쇰? 蹂쇰ⅷ ?욎そ??諛곗튂
//	float3 rayDirWS = normalize(float3(0, 0, 1));
//
//
//
//
//	//// ???붾뱶 ??蹂쇰ⅷ 濡쒖뺄濡?蹂??
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// ??Raymarch loop
//	float4 acc = 0;
//	for (int i = 0; i < MaxSteps; i++) {
//	/*	float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0)) break;*/
//
//		////float3 uvw = mul(float4(worldPos, 1.0), InvVolumeWorld).xyz;
//		//float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		//if (any(uvw < 0.0) || any(uvw > 1.0))
//		//	return float4(1, 0, 0, 1); // ?덉씠 諛뺤뒪 諛???鍮④컙??
//
//
//		float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1); // 諛???鍮④컯
//
//		float d = volumeTex.SampleLevel(samp, uvw, 0);
//		if (d <= 0.001) return float4(0, 0, 1, 1); // ?섑뵆媛믪씠 嫄곗쓽 ?놁쓬 ???뚮옉
//
//
//		 d = volumeTex.SampleLevel(samp, uvw, 0);
//		float4 color = float4(d, d, d, d * Opacity);
//		acc.rgb += (1 - acc.a) * color.a * color.rgb;
//		acc.a += (1 - acc.a) * color.a;
//		if (acc.a >= 1.0) break;
//	}
//
//	return acc;
//	//return float4(1, 0, 0, 1);
//}


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
	float Opacity;
	float pad0;
	float pad1;
};

//Texture3D<float> volumeTex : register(t0);
Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);

//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// ???ㅽ겕由겸넂酉겸넂?붾뱶
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 viewDirVS = mul(ndc, InvProj);
//	float4 viewPos = mul(ndc, InvProj);
//	viewPos /= viewPos.w;
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirVS = normalize(viewPos.xyz);
//	//float3 rayPosWS = CameraPosWS;
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // 移대찓?쇰? 蹂쇰ⅷ ?욎そ??諛곗튂
//	//float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	//float3 rayDirWS = normalize(float3(0, 0, 1));
//	float3 rayPosWS = CameraPosWS;
//
//	// ???붾뱶 ??蹂쇰ⅷ 濡쒖뺄
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// ?붾뱶 ??濡쒖뺄
//	float3 rayPosLocal = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDirLocal = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//
//	// ???덉씠留덉묶
//	float4 acc = 0;
//	[loop]
//	for (int i = 0; i < MaxSteps; i++) {
//		//float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		float3 uvw = rayPosLocal + rayDirLocal * (i * Step);
//
//
//
//		/*if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;*/
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			continue; // return???꾨땲??continue
//
//
//		float d = volumeTex.SampleLevel(samp, uvw, 0);
//		float4 color = float4(d, d, d, d * Opacity);
//		acc.rgb += (1 - acc.a) * color.a * color.rgb;
//		acc.a += (1 - acc.a) * color.a;
//		if (acc.a >= 1.0) break;
//	}
//
//	return acc;
//}




//float4 TransferFunction(float d)
//{
//	// HU normalized → [0,1]
//	if (d < 0.15) return float4(0, 0, 0, 0);               // Air
//	if (d < 0.35) return float4(0.7, 0.6, 0.6, 0.03);      // Soft tissue
//	if (d < 0.6)  return float4(1.0, 0.85, 0.8, 0.1);      // Bone
//	return float4(1.0, 1.0, 1.0, 0.2);                     // Dense bone
//}


// Transfer Function: 밀도에 따라 색상과 알파 반환
float4 TransferFunction(float density)
{
	// 공기 (매우 낮은 밀도)
	if (density < 0.1)
		return float4(0, 0, 0, 0);

	// 연조직 (피부, 근육)
	if (density < 0.4)
	{
		float t = (density - 0.1) / 0.3;  // 0~1 정규화
		return float4(0.7, 0.5, 0.4, t * 0.05);  // 살색, 약한 알파
	}

	// 뼈 (중간 밀도)
	if (density < 0.7)
	{
		float t = (density - 0.4) / 0.3;
		return float4(1.0, 0.9, 0.8, t * 0.3);  // 밝은 베이지, 강한 알파
	}

	// 치아, 금속 (높은 밀도)
	return float4(1.0, 1.0, 1.0, 0.5);  // 흰색
}

//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//
//	// --- 사분할 보정 ---
//// 볼륨뷰가 전체 창의 좌상단 1/4 영역이라면:
//float2 offset = float2(0.0, 0.0);   // 좌상단
//float2 scale = float2(0.5, 0.5);   // 전체의 절반 크기
//
//
//// uv 보정 (뷰포트 내부 좌표로 정규화)
//float2 localUV = (uv - offset) / scale;
//
//
//	// 1截뤴깵 ?쎌? 湲곗? ?쒖빞 ?덉씠 怨꾩궛
//	float4 ndc = float4(localUV * 2 - 1, 1, 1);
//	ndc.y = -ndc.y;  // DirectX는 Y축 반전 필요!
//
//	float4 viewDirVS = mul(ndc, InvProj);
//	viewDirVS /= viewDirVS.w;
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//
//	float3 rayPosWS = CameraPosWS;
//
//
//
//
//
//
//
//
//
//
//
//
//
//	// 2截뤴깵 ?붾뱶 ??蹂쇰ⅷ 濡쒖뺄
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//
//
//
//
//
//
//	// 蹂쇰ⅷ 寃쎄퀎 (0~1 諛뺤뒪 湲곗?)
//	float3 boxMin = float3(0, 0, 0);
//	float3 boxMax = float3(1, 1, 1);
//
//	// Ray-box 援먯감 怨꾩궛
//	float3 invDir = 1.0 / rayDir;
//	float3 tMin = (boxMin - rayPos) * invDir;
//	float3 tMax = (boxMax - rayPos) * invDir;
//	float3 t1 = min(tMin, tMax);
//	float3 t2 = max(tMin, tMax);
//
//	float tNear = max(max(t1.x, t1.y), t1.z);
//	float tFar = min(min(t2.x, t2.y), t2.z);
//
//	// 援먯감 ?놁쑝硫?寃?뺤깋
//	if (tNear > tFar || tFar < 0)
//		return float4(0, 0, 0, 1);
//
//	// Ray ?쒖옉?먯쓣 諛뺤뒪 吏꾩엯?먯쑝濡??대룞
//	rayPos += rayDir * max(tNear, 0.0);
//
//
//
//
//
//
//
//
//	// 3截뤴깵 ?꾩쟻 ?쒖옉
//	float4 acc = 0;
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 uvw = rayPos + rayDir * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			//break;
//			return float4(1, 0, 0, 1);
//
//		//if (any(uvw < 0) || any(uvw > 1)) return float4(1, 0, 0, 1);  // 빨간색이면 밖임
//
//
//	float d = volumeTex.SampleLevel(samp, uvw, 0);
//	///*	float windowCenter = 0.3;
//	//	float windowWidth = 0.4;
//	//	d = saturate((d - (windowCenter - windowWidth * 0.5)) / windowWidth);*/
//
//
//	//if (d > 0.0 && d < 0.001) return float4(1, 0, 0, 1);
//	//if (d >= 0.001 && d < 0.01) return float4(0, 1, 0, 1);
//	//if (d >= 0.01) return float4(0, 0, 1, 1);
//
//
//		d= saturate((d - 0.25) * 2.0);
//
//		//if (d < 0.01f) discard;  // 밀도 0.01 이하값은 공기로 취급
//
//
//		float4 col = float4(d, d, d, d * Opacity);
//
//		acc.rgb += (1 - acc.a) * col.a * col.rgb;
//		acc.a += (1 - acc.a) * col.a;
//
//		if (acc.a >= 1.0)
//			break;
//	}
//
//	return acc;
//}



float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- 광선 생성 (기존 코드 유지) ---
	float2 offset = float2(0.0, 0.0);
	float2 scale = float2(0.5, 0.5);
	float2 localUV = (uv - offset) / scale;
	float2 screenUV = localUV;
	//screenUV = uv;

	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;
	ndc.x = -ndc.x;
	

	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);
	viewDirVS /= viewDirVS.w;

	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosWS;

	// 이 한 줄 테스트
	//rayDirWS = -rayDirWS;


	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	// 디버그: 내부인지 확인
	if (all(rayPos > 0) && all(rayPos < 1))
		return float4(1, 0, 0, 1); // 내부에 있으면 빨강으로
	//return float4(rayPos.xyz, 1);  // RGB에 로컬좌표 표시

	//rayDir = -rayDir;

	// --- Ray-box 교차 ---
	float3 boxMin = float3(0, 0, 0);
	float3 boxMax = float3(1, 1, 1);

	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	if (tNear > tFar || tFar < 0)
		return float4(0, 0, 0, 1);  // 배경

	tNear = max(tNear, 0.0);
	float travelDist = tFar - tNear;

	// ✅ Step 크기 계산
	float stepSize = travelDist / float(MaxSteps);

	// ✅ 시작점 설정
	float3 startPos = rayPos + rayDir * tNear;

	// --- 볼륨 적분 ---
	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		// ✅ 현재 위치 계산 (i에 따라 전진)
		float3 uvw = startPos + rayDir * (i * stepSize);

		// 범위 체크
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		//sampleCount++;

		// 밀도 샘플링
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;

		//// ✅ Windowing 적용
		//float windowCenter = 0.3;
		//float windowWidth = 0.6;  // 더 넓게
		//density = saturate((density - (windowCenter - windowWidth * 0.5)) / windowWidth);

		//// 알파 계산
		////float alpha = density * 0.5 * stepSize;  // stepSize 곱해서 스케일 조정
		//float alpha = density * 1.0 * stepSize;


		// ✅ Transfer Function 적용
		float4 colorAlpha = TransferFunction(density);

		float3 color = colorAlpha.rgb;
		float alpha = colorAlpha.a * stepSize;  // stepSize 곱하기!


		if (alpha > 0.001)
		{
			float3 color = float3(density, density, density);

			// Front-to-back 블렌딩
			acc.rgb += (1.0 - acc.a) * alpha * color;
			acc.a += (1.0 - acc.a) * alpha;

			// Early termination
			if (acc.a >= 0.95)
				break;
		}
	}

	//return float4(sampleCount / 256.0, 0, 0, 1);  // 빨간색 밝기로 샘플 수 표시

	return float4(acc.rgb, 1.0);




	//if (tNear > tFar || tFar < 0)
	//	return float4(0, 0, 0, 1);

	//tNear = max(tNear, 0.0);
	//float travelDist = tFar - tNear;
	//float stepSize = travelDist / float(MaxSteps);
	//float3 startPos = rayPos + rayDir * tNear;

	//// ✅ 디버깅: 최대/최소 밀도값 추적
	//float minDensity = 1.0;
	//float maxDensity = 0.0;

	//[loop]
	//for (int i = 0; i < MaxSteps; i++)
	//{
	//	float3 uvw = startPos + rayDir * (i * stepSize);

	//	if (any(uvw < 0.0) || any(uvw > 1.0))
	//		break;

	//	float density = volumeTex.SampleLevel(samp, uvw, 0).r;

	//	minDensity = min(minDensity, density);
	//	maxDensity = max(maxDensity, density);
	//}

	//// ✅ 결과 출력
	//return float4(minDensity, maxDensity, 0, 1);
	//// 녹색 = 최대값, 빨간색 = 최소값
	//// 완전 검은색 = 모두 0
	//// 녹색 밝기로 최대값 확인 가능
}