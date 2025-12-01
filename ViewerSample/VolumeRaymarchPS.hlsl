
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



// ⭐ Transfer Function 추가
Texture1D<float4> transferFunction : register(t1);
SamplerState tfSampler : register(s1);


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

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
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


    rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;


	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//float3 boxMin = float3(-0.75, -0.75, -0.75);
	//float3 boxMax = float3(0.75, 0.75, 0.75);


	float3 boxMin = float3(-1, -0.75, -0.75);
	float3 boxMax = float3(1, 0.75, 0.75);

	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;


	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);


	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;

	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  
	}


	float stepSize = travelDist / float(MaxSteps);

	// ✅ Jittering
	float jitter = frac(sin(dot(uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
	// 시작점에 랜덤 오프셋
	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);


	startPos = clamp(startPos, boxMin, boxMax);

	float3 currentPos = startPos;

	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);

	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//
//		uvw.y = 1.0 - uvw.y;  // ✅ 추가
//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//
//		// 1) Raw 기반 density
//		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
//		//float density = raw / 255.0;    // TF 전용
//
//
//		// ⭐ 패딩 체크
//		if (raw > 60000.0 || raw < 1.0) {
//			continue;  // 완전히 스킵
//		}
//
//
//		//// 2) DICOM HU 로 변환
//		float hu = raw * HuParams.x + HuParams.y;   // -1000 ~ 3000 같은 범위
//
//
//
//
//	//// ⭐ Raw를 4095배 해서 확인 (12bit)
//	//	float scaledRaw = raw * 4095.0;
//	//	return float4(scaledRaw / 4095.0, scaledRaw / 4095.0, scaledRaw / 4095.0, 1.0);
//
//		// 3) 윈도우/레벨 범위로 정규화 (0~1)
//		float huNorm = (hu - HuParams.z) / (HuParams.w - HuParams.z);
//		if (huNorm < 0.0 || huNorm > 1.0) {
//			continue;  // ✅ 완전히 스킵!
//		}
//
//		huNorm = saturate(huNorm);
//
//		// ⭐ 절대 HU 기준 (전체 범위 -1000~3000)
//		float tfCoord = saturate((hu + 1000.0f) / 4000.0f);
//
//		//// Transfer Function에서 색상/투명도 가져오기
//		//float4 tfValue = transferFunction.Sample(tfSampler, huNorm);
//
//		//// ⭐ Transfer Function에서 색상/투명도 가져오기 (하나만 사용!)
//		//float4 colorAlpha = transferFunction.Sample(tfSampler, huNorm);
//		float4 colorAlpha = transferFunction.Sample(tfSampler, tfCoord);
//
//
//		// ⭐ Window로 알파만 조절 (조직 분리 유지)
//		float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
//		if (huInWindow < 0.0 || huInWindow > 1.0) {
//			colorAlpha.a *= 0.05;  // Window 밖은 투명하게
//		}
//
//
//		// ⭐ 기존 TransferFunctionHU() 삭제 - tfValue 하나로 통일!
//
//		if (colorAlpha.a < 0.001)
//			continue;
//
//		//float4 colorAlpha = TransferFunctionHU(hu/*, huNorm*/);
//
//
//
//		// 3) eps 계산
//		float3 eps = float3(1.0 / Voxel.x, 1.0 / Voxel.y, 1.0 / Voxel.z);
//
//		// 4) raw 기반 gradient
//		float dx =
//			volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
//
//		float dy =
//			volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
//
//		float dz =
//			volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;
//
//		// 조명 계산 부분에서
//		float3 N = normalize(float3(dx, dy, dz) + 1e-6);
//		float3 L = normalize(float3(0.5, 0.7, -0.5));
//		float3 V = -rayDir;  // 뷰 방향
//		float3 H = normalize(L + V);  // 하프 벡터
//
//		float lambert = max(dot(N, L), 0.0);
//		float spec = pow(max(dot(N, H), 0.0), 48.0);  // 광택
//
//	// ⭐ 조명을 훨씬 약하게
//		float lighting = 0.85 + lambert * 0.15;  // 0.7 + 0.3 → 0.85 + 0.15
//
//
//		colorAlpha.rgb *= lighting;
//		colorAlpha.rgb += spec * float3(0.12, 0.10, 0.08);  // 하이라이트도 약하게
//
//
//		float3 color = colorAlpha.rgb;
//		float alpha = colorAlpha.a * stepSize * 8.0;
//
//
//		if (alpha > 0.001)
//		{
//			acc.rgb += (1.0 - acc.a) * alpha * color;
//			acc.a += (1.0 - acc.a) * alpha;
//
//			if (acc.a >= 0.95)
//				break;
//		}
//	}
//
//
//	// ✅ 감마 보정만 (선택)
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//
//
//// 후처리 부분 수정
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//	acc.rgb *= 0.95;  // 1.15 → 0.95 (밝기 줄임)
//	acc.rgb = (acc.rgb - 0.5) * 1.3 + 0.5;  // 콘트라스트 더 높임
//	//acc.rgb *= float3(1.0, 0.95, 0.88);
//	acc.rgb = saturate(acc.rgb);
//
//
//	return float4(acc.rgb, 1.0);



[loop]
for (int i = 0; i < MaxSteps; i++)
{
	float3 currentPos = startPos + rayDir * (i * stepSize);
	float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

	uvw.y = 1.0 - uvw.y;  // ✅ 추가

	if (any(uvw < 0.0) || any(uvw > 1.0))
		break;


	// 1) Raw 기반 density
	float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
	//float density = raw / 255.0;    // TF 전용


	// ⭐ 패딩 체크
	if (raw > 60000.0) {
		continue;  // 완전히 스킵
	}


	//// 2) DICOM HU 로 변환
	float hu = raw * HuParams.x + HuParams.y;   // -1000 ~ 3000 같은 범위




//// ⭐ Raw를 4095배 해서 확인 (12bit)
//	float scaledRaw = raw * 4095.0;
//	return float4(scaledRaw / 4095.0, scaledRaw / 4095.0, scaledRaw / 4095.0, 1.0);

	// 3) 윈도우/레벨 범위로 정규화 (0~1)
	float huNorm = (hu - HuParams.z) / (HuParams.w - HuParams.z);
	huNorm = saturate(huNorm);

	// ⭐ 절대 HU 기준 (전체 범위 -1000~3000)
	float tfCoord = saturate((hu + 1000.0f) / 4000.0f);

	//// Transfer Function에서 색상/투명도 가져오기
	//float4 tfValue = transferFunction.Sample(tfSampler, huNorm);

	//// ⭐ Transfer Function에서 색상/투명도 가져오기 (하나만 사용!)
	//float4 colorAlpha = transferFunction.Sample(tfSampler, huNorm);
	float4 colorAlpha = transferFunction.Sample(tfSampler, tfCoord);


	// ⭐ Window로 알파만 조절 (조직 분리 유지)
	float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
	if (huInWindow < 0.0 || huInWindow > 1.0) {
		colorAlpha.a *= 0.05;  // Window 밖은 투명하게
	}


	// ⭐ 기존 TransferFunctionHU() 삭제 - tfValue 하나로 통일!

	if (colorAlpha.a < 0.001)
		continue;

	//float4 colorAlpha = TransferFunctionHU(hu/*, huNorm*/);



	// 조명 계산
	float3 eps = float3(1.0 / Voxel.x, 1.0 / Voxel.y, 1.0 / Voxel.z);

	float dx = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
	float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
	float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;

	float3 N = normalize(float3(dx, dy, dz) + 1e-6);
	float3 L = normalize(float3(0.5, 0.7, -0.5));
	float3 V = -rayDir;
	float3 H = normalize(L + V);

	float lambert = max(dot(N, L), 0.0);
	float spec = pow(max(dot(N, H), 0.0), 48.0);

	float lighting = 0.88 + lambert * 0.12;

	colorAlpha.rgb *= lighting;
	colorAlpha.rgb += spec * float3(0.08, 0.07, 0.06);

	float3 color = colorAlpha.rgb;
	float alpha = colorAlpha.a * stepSize * 8.0;

	if (alpha > 0.001) {
		acc.rgb += (1.0 - acc.a) * alpha * color;
		acc.a += (1.0 - acc.a) * alpha;
		if (acc.a >= 0.95) break;
	}
}

// 후처리
acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
acc.rgb *= 1.0;
acc.rgb = (acc.rgb - 0.5) * 1.4 + 0.5;
acc.rgb = saturate(acc.rgb);

return float4(acc.rgb, 1.0);

}

