
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

Texture3D<float> volumeTex : register(t0);
//Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);



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
	//ndc.x = -ndc.x;
	

	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);
	viewDirVS /= viewDirVS.w;

	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosWS;


	//return float4(rayPosWS / 10.0, 1);  // 10으로 나눠서 시각화



	//// CameraPosWS 무시하고 고정값 사용
	///*float3*/ rayPosWS = float3(0, 0, -3.0);  // 하드코딩

	//float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//return float4((rayPos + 2.0) / 4.0, 1);





	// 이 한 줄 테스트
	//rayDirWS = -rayDirWS;

	//return float4(abs(rayDirWS), 1);


rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;

	//// ✅ 범위 조정해서 시각화
	//return float4(
	//	rayPos.x / 5.0 + 0.5,  // -2.5~2.5 → 0~1
	//	rayPos.y / 5.0 + 0.5,
	//	rayPos.z / 5.0 + 0.5,
	//	1
	//	);
	




	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//// ✅ rayDir 확인
	//return float4(abs(rayDir), 1);


	//// ✅ 카메라 위치 확인
	//return float4(rayPos, 1);//==>>여기가 근본적인 문제임

	//rayDir = -rayDir;

	//// --- Ray-box 교차 ---
	//float3 boxMin = float3(0, 0, 0);
	//float3 boxMax = float3(1, 1, 1);

	// ✅ 수정 (volumeSize = 1.5 기준)
	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);



	//// ✅ rayPos가 박스 안인가?
	//if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	//	return float4(1, 0, 0, 1);  // 빨강 = 안
	//else
	//	return float4(0, 1, 0, 1);  // 초록 = 밖





	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;




	//// ✅ tMin, tMax 확인
	////return float4(abs(tMin) / 10.0, 1);  // tMin을 색상으로
	//// 또는
	//return float4(abs(tMax) / 10.0, 1);  // tMax를 색상으로

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	//// ✅ t1 확인
	////return float4(abs(t1) / 10.0, 1);
	//// ✅ t2 확인
	// return float4(abs(t2) / 10.0, 1);

	//// ✅ 둘 다 출력
	//return float4(tNear / 10.0, tFar / 10.0, 0, 1);
	//// 빨강 = tNear, 초록 = tFar



	//// ✅ 디버깅: 교차 여부
	//if (tNear > tFar || tFar < 0)
	//	return float4(1, 0, 0, 1);  // 빨강 = 교차 안 함
	//else
	//	return float4(0, 1, 0, 1);  // 초록 = 교차 함

	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;

	// ✅ 박스 범위에 맞게 수정
	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  // 카메라가 박스 안
	}

	// ✅ Step 크기 계산
	float stepSize = travelDist / float(MaxSteps);

	// ✅ 시작점 설정
	float3 startPos = rayPos + rayDir * tNear;

	// ✅ startPos를 박스 안으로 강제
	startPos = clamp(startPos, boxMin, boxMax);

	//// ✅ startPos를 색상으로 표시
	//return float4((startPos + 0.75) / 1.5, 1);
	//// [-0.75, 0.75] → [0, 1] 변환


	//// ✅ tNear 값 확인
	//return float4(tNear / 5.0, tNear / 5.0, tNear / 5.0, 1);
	//// 0~5 범위를 0~1로 정규화


	//// ✅ 디버깅: 시작 위치 색상으로 표시=>문제 발견
	//return float4(startPos, 1);



	// ✅ 첫 샘플 위치
	float3 currentPos = startPos;

	//// ✅ UV 변환
	//float3 uvw = (currentPos + 0.75) / 1.5;
	// ✅ 올바른 UV 변환
	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);


	//// UV 범위 체크
	//if (any(uvw < 0.0) || any(uvw > 1.0))
	//	return float4(1, 0, 0, 1);  // 빨간색 = 범위 밖
	//else
	//	return float4(uvw, 1);  // 그라디언트여야 함


	// 모든 디버깅 return 주석처리하고
// 볼륨 적분 전에 추가:

//// ✅ 텍스처 중앙값 확인
//	float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;
//	return float4(testDensity * 10.0, testDensity * 10.0, testDensity * 10.0, 1);


	//float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;

	//// ✅ 증폭 없이 원본 확인
	//return float4(testDensity, testDensity, testDensity, 1);


	// --- 볼륨 적분 ---
	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

	//// 루프 전에
	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		//// ✅ 현재 위치 계산 (i에 따라 전진)
		//float3 uvw = startPos + rayDir * (i * stepSize);

		// ✅ 올바른 계산
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		// 범위 체크
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		// 밀도 샘플링
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;

		//// 밀도를 10배 증폭해서 표시
		//return float4(density * 10.0, density * 10.0, density * 10.0, 1);


		  // ✅ 밀도 조정 (임계값 낮추기)
		density = saturate((density - 0.05) * 2.0);  // 0.05 이하 제거


		// ✅ 밝기 증가
		//density = saturate((density - 0.1) * 2.0);  // 0.1 이하 제거, 2배 증폭


		// ✅ Transfer Function 적용
		float4 colorAlpha = TransferFunction(density);

		  


		float3 color = colorAlpha.rgb;
		float alpha = colorAlpha.a * stepSize;  // stepSize 곱하기!


	

		if (alpha > 0.001)
		{
			//float3 color = float3(density, density, density);

			// Front-to-back 블렌딩
			acc.rgb += (1.0 - acc.a) * alpha * color;
			acc.a += (1.0 - acc.a) * alpha;

			// Early termination
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


	return float4(acc.rgb, acc.a);

}


//
//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// --- 광선 생성 ---
//	float2 offset = float2(0.0, 0.0);
//	float2 scale = float2(0.5, 0.5);
//	float2 localUV = (uv - offset) / scale;
//	float2 screenUV = localUV;
//
//	float2 ndc = screenUV * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 ndcPos = float4(ndc, 1, 1);
//	float4 viewDirVS = mul(ndcPos, InvProj);
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//
//	// ✅ 하드코딩 (임시)
//	float3 rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// --- Ray-box 교차 ---
//	float3 boxMin = float3(-0.75, -0.75, -0.75);
//	float3 boxMax = float3(0.75, 0.75, 0.75);
//
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 tMin = (boxMin - rayPos) * invDir;
//	float3 tMax = (boxMax - rayPos) * invDir;
//
//	float3 t1 = min(tMin, tMax);
//	float3 t2 = max(tMin, tMax);
//
//	float tNear = max(max(t1.x, t1.y), t1.z);
//	float tFar = min(min(t2.x, t2.y), t2.z);
//
//	if (tNear > tFar || tFar < 0)
//		return float4(0, 0, 0, 1);
//
//	tNear = max(tNear, 0.0);
//
//	float travelDist = tFar - tNear;
//	float stepSize = travelDist / float(MaxSteps);
//	float3 startPos = rayPos + rayDir * tNear;
//
//	// --- 볼륨 적분 ---
//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//
//		// ✅ UV 변환
//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//
//		// UV 범위 체크
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1);  // 빨간색 = 범위 밖
//		else
//			return float4(uvw, 1);  // 그라디언트여야 함
//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		float density = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		// ✅ 밀도 증폭
//		density = saturate(density * 3.0);
//
//		// ✅ 알파 계산
//		float alpha = density * 10.0 * stepSize;
//
//		if (alpha > 0.001)
//		{
//			float value = density * 2.0;
//			float3 color = float3(value, value, value);
//
//			acc.rgb += (1.0 - acc.a) * alpha * color;
//			acc.a += (1.0 - acc.a) * alpha;
//
//			if (acc.a >= 0.95)
//				break;
//		}
//	}
//
//	return float4(acc.rgb, 1.0);
//}