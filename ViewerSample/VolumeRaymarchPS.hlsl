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
//	// ① NDC → View space
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 rayStartVS = mul(ndc, InvProj);
//	rayStartVS /= rayStartVS.w;
//
//	/*float3 rayDirWS = normalize(mul(float4(rayStartVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosWS;*/
//
//
//	// 실험용: 볼륨 로컬 좌표가 0~1이라 가정
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // 카메라를 볼륨 앞쪽에 배치
//	float3 rayDirWS = normalize(float3(0, 0, 1));
//
//
//
//
//	//// ② 월드 → 볼륨 로컬로 변환
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// ③ Raymarch loop
//	float4 acc = 0;
//	for (int i = 0; i < MaxSteps; i++) {
//	/*	float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0)) break;*/
//
//		////float3 uvw = mul(float4(worldPos, 1.0), InvVolumeWorld).xyz;
//		//float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		//if (any(uvw < 0.0) || any(uvw > 1.0))
//		//	return float4(1, 0, 0, 1); // 레이 박스 밖 → 빨간색
//
//
//		float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1); // 밖 → 빨강
//
//		float d = volumeTex.SampleLevel(samp, uvw, 0);
//		if (d <= 0.001) return float4(0, 0, 1, 1); // 샘플값이 거의 없음 → 파랑
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
//	// ① 스크린→뷰→월드
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 viewDirVS = mul(ndc, InvProj);
//	float4 viewPos = mul(ndc, InvProj);
//	viewPos /= viewPos.w;
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirVS = normalize(viewPos.xyz);
//	//float3 rayPosWS = CameraPosWS;
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // 카메라를 볼륨 앞쪽에 배치
//	//float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	//float3 rayDirWS = normalize(float3(0, 0, 1));
//	float3 rayPosWS = CameraPosWS;
//
//	// ② 월드 → 볼륨 로컬
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// 월드 → 로컬
//	float3 rayPosLocal = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDirLocal = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//
//	// ③ 레이마칭
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
//			continue; // return이 아니라 continue
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

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
	// 1️⃣ 픽셀 기준 시야 레이 계산
	float4 ndc = float4(uv * 2 - 1, 1, 1);
	float4 viewDirVS = mul(ndc, InvProj);
	viewDirVS /= viewDirVS.w;
	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);

	float3 rayPosWS = CameraPosWS;













	// 2️⃣ 월드 → 볼륨 로컬
	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);







	// 볼륨 경계 (0~1 박스 기준)
	float3 boxMin = float3(0, 0, 0);
	float3 boxMax = float3(1, 1, 1);

	// Ray-box 교차 계산
	float3 invDir = 1.0 / rayDir;
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;
	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	// 교차 없으면 검정색
	if (tNear > tFar || tFar < 0)
		return float4(0, 0, 0, 1);

	// Ray 시작점을 박스 진입점으로 이동
	rayPos += rayDir * max(tNear, 0.0);








	// 3️⃣ 누적 시작
	float4 acc = 0;
	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		float3 uvw = rayPos + rayDir * (i * Step);
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;

		float d = volumeTex.SampleLevel(samp, uvw, 0);
	/*	float windowCenter = 0.3;
		float windowWidth = 0.4;
		d = saturate((d - (windowCenter - windowWidth * 0.5)) / windowWidth);*/


		float4 col = float4(d, d, d, d * Opacity);

		acc.rgb += (1 - acc.a) * col.a * col.rgb;
		acc.a += (1 - acc.a) * col.a;

		if (acc.a >= 1.0)
			break;
	}

	return acc;
}
