
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


		// 밀도 샘플링
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;


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
	}


	return float4(acc.rgb, acc.a);

}