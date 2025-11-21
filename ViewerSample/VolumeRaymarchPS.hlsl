
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


	float4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
};

Texture3D<float> volumeTex : register(t0);


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




// HU 기반 TF – 연조직/해면골은 거의 안 보이고, 피질골/치아만 강하게
float4 TransferFunctionHU(float hu)
{
	// 1) 공기/연조직/노이즈 – 완전 투명
	//    (윈도우가 -500~1500 같은 거라고 치면 900 정도까지는 날려버림)
	if (hu < 900.0)
		return float4(0, 0, 0, 0);

	// 2) 해면골 (900 ~ 1300) – 아주 살짝만
	if (hu < 1300.0)
	{
		float t = saturate((hu - 900.0) / 400.0);   // 0~1
		float3 col = lerp(float3(0.75, 0.70, 0.65),
			float3(0.88, 0.80, 0.72), t);
		float a = t * 0.15;                         // 🔸 알파 아주 약하게
		return float4(col, a);
	}

	// 3) 피질골/치아 (1300 ~ 3000) – 주 피사체
	float t = saturate((hu - 1300.0) / 1700.0);
	float3 col = lerp(float3(0.92, 0.88, 0.82),
		float3(0.99, 0.97, 0.93), t);
	float a = 0.6 + t * 1.4;                        // 대략 0.6 ~ 2.0
	return float4(col, a);
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

	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);



	


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

		//float3 uvw = (currentPos + 0.75) / 1.5;

	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);

	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;


	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
	
		//float3 uvw = startPos + rayDir * (i * stepSize);


		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;
		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
		float r16 = raw * 65535.0;
		//float hu = r16 * HuParams.x + HuParams.y;
		float hu = raw * (HuParams.w - HuParams.z) + HuParams.z;

		// TransferFunctionHU 호출 전에
		if (i == MaxSteps / 2)  // 중간 샘플만
		{
			return float4(raw, hu / 3000.0, 0, 1);  // raw(R), hu/3000(G) 값 확인
		}

		float4 colorAlpha = TransferFunctionHU(hu);
		float huNorm = raw; // 0~1

	

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
	//	float alpha = colorAlpha.a * stepSize * 8.0;
		float alpha = colorAlpha.a * stepSize;   // 🔹 배율 줄이고 saturate 제거

		if (alpha <= 0.0005)
			continue;  // 너무 작은 건 그냥 스킵



// front-to-back 합성
		acc.rgb += (1.0 - acc.a) * alpha * color;
		acc.a += (1.0 - acc.a) * alpha;

		if (acc.a >= 0.98)
			break;

		//if (alpha > 0.001)
		//{
		//

		//	acc.rgb += (1.0 - acc.a) * alpha * color;

		//	
		//	acc.a += (1.0 - acc.a) * alpha;

		//	if (acc.a >= 0.95)
		//		break;
		//}
	}



	// ✅ 감마 보정만 (선택)
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
	//acc.rgb *= float3(1.0, 0.97, 0.92);  // 아주 약한 따뜻한 톤 (원하면만)
	return float4(acc.rgb, acc.a);


// 후처리 부분 수정
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
	acc.rgb *= 0.95;  // 1.15 → 0.95 (밝기 줄임)
	acc.rgb = (acc.rgb - 0.5) * 1.3 + 0.5;  // 콘트라스트 더 높임
	acc.rgb *= float3(1.0, 0.95, 0.88);
	acc.rgb = saturate(acc.rgb);


	return float4(acc.rgb, 1.0);
}

