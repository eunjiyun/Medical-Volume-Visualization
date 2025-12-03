
cbuffer CB : register(b0)
{
	matrix View;
	matrix Proj;
	matrix InvView;
	matrix InvProj;
	matrix VolumeWorld;
	matrix InvVolumeWorld;
	float3 CameraPosWS;
	float alphaScale;
	int   MaxSteps;
	float3 Voxel;


	float4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
};

Texture3D<float> volumeTex : register(t0);
//Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);



// ⭐ Transfer Function 추가
Texture1D<float4> transferFunction : register(t1);
SamplerState tfSampler : register(s1);


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





	//// ⭐ 패딩 체크
	//if (raw > 60000.0) {
	//	continue;  // 완전히 스킵
	//}


	//// 2) DICOM HU 로 변환
	float hu = raw/* * HuParams.x + HuParams.y*/;   // -1000 ~ 3000 같은 범위


	//	// ⭐ HU 복원 (-1500 ~ 3500)
	//float hu = raw * 5000.0 - 1500.0;


	//return float4(hu, hu, hu, 1.0);


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
	float4 colorAlpha = transferFunction.Sample(tfSampler, huNorm);
	//float4 colorAlpha = transferFunction.Sample(tfSampler, tfCoord);


	// ⭐ Window로 알파만 조절 (조직 분리 유지)
	float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
	if (huInWindow < 0.0 || huInWindow > 1.0) {
		colorAlpha.a *= 0.05;  // Window 밖은 투명하게
	}


	//// 뼈/치아 제거: HU가 300 이상이면 완전 투명
	//if (hu > 300.0) {
	//	colorAlpha.a = 0.0;
	//}

	// ⭐ 기존 TransferFunctionHU() 삭제 - tfValue 하나로 통일!

	if (colorAlpha.a < 0.001)
		continue;

	//float4 colorAlpha = TransferFunctionHU(hu/*, huNorm*/);

	//// 연조직 boost 적용
	//colorAlpha *= alphaScale;

	// 조명 계산
	float3 eps = float3(1.0 / Voxel.x, 1.0 / Voxel.y, 1.0 / Voxel.z);

	float dx = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
	float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
	float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
		volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;

	float3 N = normalize(float3(dx, dy, dz) + 1e-6);
	//float gradMag = length(float3(dx, dy, dz));

	////colorAlpha.a *= saturate(gradMag * 50.0);  // ⭐ 이 한 줄!

	////// ✅ 대신 이렇게!
	////float gradientOpacity = saturate(gradMag * 30.0);
	////colorAlpha.a *= (0.3 + gradientOpacity * 0.7);  // 최소 30%, 최대 100%

	//// 기존
	//float gradientOpacity = saturate(gradMag * 30.0);
	//colorAlpha.a *= (0.3 + gradientOpacity * 0.7);

	//// 디버그: 연조직 확인용 최소 알파 높이기
	//colorAlpha.a *= 1.0; // 또는 0.8 이상으로 고정해서 쌓이게

	////// ⭐ 경계 감지 및 강조
	////if (gradMag > 0.02) {
	////	// 경계를 어둡게 (입술/콧구멍처럼)
	////	colorAlpha.rgb *= 0.4;  // 60% 어둡게
	////	colorAlpha.a *= 1.5;    // 더 불투명
	////}


	float3 L = normalize(float3(0.5, 0.7, -0.5));
	float3 V = -rayDir;
	float3 H = normalize(L + V);

	float lambert = max(dot(N, L), 0.0);
	float spec = pow(max(dot(N, H), 0.0), 48.0);

	float lighting = 0.88 + lambert * 0.12;
	//colorAlpha.rgb *= lighting;

	//float lighting = 0.4 + lambert * 0.6;  // 0.88 + 0.12 → 0.4 + 0.6 (더 강하게)
	//float lighting = 0.5 + lambert * 0.5;
	colorAlpha.rgb *= lighting;


	colorAlpha.rgb += spec * float3(0.08, 0.07, 0.06);

	float3 color = colorAlpha.rgb;
	float alpha = colorAlpha.a * stepSize * 8.0;

	if (alpha > 0.001) {
		acc.rgb += (1.0 - acc.a) * alpha * color;
		acc.a += (1.0 - acc.a) * alpha;
		if (acc.a >= 0.95) break;
	}

	//acc.rgb += (1.0 - acc.a) * alpha * colorAlpha.rgb;
	//acc.a += (1.0 - acc.a) * alpha;

	//if (acc.a >= 0.95) break;
}

//// 후처리
//acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//acc.rgb *= 1.0;
//acc.rgb = (acc.rgb - 0.5) * 1.4 + 0.5;
//acc.rgb = saturate(acc.rgb);
//
//return float4(acc.rgb, 1.0);

// 후처리 (간단하게!)
acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
return float4(acc.rgb, 1.0);
}

//cbuffer CB : register(b0)
//{
//	matrix View;
//	matrix Proj;
//	matrix InvView;
//	matrix InvProj;
//	matrix VolumeWorld;
//	matrix InvVolumeWorld;
//	float3 CameraPosWS;
//	float alphaScale;
//	int   MaxSteps;
//	float3 Voxel;
//
//	float4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
//};
//
//Texture3D<float> volumeTex : register(t0);
//SamplerState samp : register(s0);
//
//Texture1D<float4> transferFunction : register(t1);
//SamplerState tfSampler : register(s1);
//
//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	float2 screenUV = uv;
//
//	float2 ndc = screenUV * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 ndcPos = float4(ndc, 1, 1);
//	float4 viewDirVS = mul(ndcPos, InvProj);
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosWS;
//
//	rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	float3 boxMin = float3(-1, -0.75, -0.75);
//	float3 boxMax = float3(1, 0.75, 0.75);
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
//	tNear = max(tNear, 0.0);
//	float travelDist = tFar - tNear;
//
//	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
//	{
//		tNear = 0.0;
//	}
//
//	float stepSize = travelDist / float(MaxSteps);
//
//	// Jittering
//	float jitter = frac(sin(dot(uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
//	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
//	startPos = clamp(startPos, boxMin, boxMax);
//
//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//		uvw.y = 1.0 - uvw.y;
//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		// ⭐⭐⭐ 1. Raw 값 샘플링 (0~1 범위)
//		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		// ⭐⭐⭐ 2. Raw → HU 변환 (Rescale 제거!)
//		// floatData에 이미 HU가 저장되어 있으므로 그대로 사용!
//		float hu = raw * 4024.0 - 1024.0;  // 0~1 → -1024~3000
//
//		// ⭐⭐⭐ 3. Transfer Function 좌표 계산 (절대 HU)
//		float tfCoord = saturate((hu + 1024.0) / 4024.0);  // -1024~3000 → 0~1
//
//		// ⭐⭐⭐ 4. Transfer Function 샘플링
//		float4 colorAlpha = transferFunction.Sample(tfSampler, tfCoord);
//
//		// ⭐ 5. Window 필터 (선택적)
//		float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
//		if (huInWindow < 0.0 || huInWindow > 1.0) {
//			colorAlpha.a *= 0.05;  // Window 밖은 약하게
//		}
//
//		if (colorAlpha.a < 0.001)
//			continue;
//
//		// ⭐ 6. Gradient 계산 (조명용)
//		float3 eps = float3(1.0 / Voxel.x, 1.0 / Voxel.y, 1.0 / Voxel.z);
//
//		float dx = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
//		float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
//		float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;
//
//		float3 N = normalize(float3(dx, dy, dz) + 1e-6);
//		float gradMag = length(float3(dx, dy, dz));
//
//		// ⭐ 7. Gradient Opacity (경계 강조)
//		float gradientOpacity = saturate(gradMag * 30.0);
//		colorAlpha.a *= (0.3 + gradientOpacity * 0.7);
//
//		// ⭐ 8. 조명 계산
//		float3 L = normalize(float3(0.5, 0.7, -0.5));
//		float3 V = -rayDir;
//		float3 H = normalize(L + V);
//
//		float lambert = max(dot(N, L), 0.0);
//		float spec = pow(max(dot(N, H), 0.0), 48.0);
//
//		float lighting = 0.35 + lambert * 0.65;
//		colorAlpha.rgb *= lighting;
//		colorAlpha.rgb += spec * float3(0.25, 0.22, 0.20);
//
//		// ⭐ 9. Alpha Blending
//		float alpha = colorAlpha.a * stepSize * 8.0;
//
//		acc.rgb += (1.0 - acc.a) * alpha * colorAlpha.rgb;
//		acc.a += (1.0 - acc.a) * alpha;
//
//		if (acc.a >= 0.95) break;
//	}
//
//	// ⭐ 10. 감마 보정
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//	return float4(acc.rgb, 1.0);
//}