//
//cbuffer CB : register(b0)
//{
//	matrix InvView;
//	matrix InvProj;
//	matrix InvVolumeWorld;
//
//	matrix View;           // ✅ 추가
//	matrix Projection;     // ✅ 추가
//
//	float4 CameraPosAndAlpha;  // xyz=pos, w=alpha
//	float4 VoxelAndMaxSteps;   // xyz=voxel, w=maxSteps
//	float4 HuParams;
//};
//
//Texture3D<float> volumeTex : register(t0);
//SamplerState samp : register(s0);
//
//
//// ⭐ Transfer Function 추가
//Texture1D<float4> transferFunction : register(t1);
//SamplerState tfSampler : register(s1);
//
//
//// ========== 추가: Depth 텍스처 ==========
//Texture2D<float> SceneDepth : register(t5);  // ← 메쉬 depth
//
//
//struct PSInput
//{
//	float4 pos : SV_POSITION; // ⭐ 이게 화면 픽셀 좌표
//	float2 uv       : TEXCOORD0;
//};
//
//
//
//float4 main(PSInput input) : SV_Target
//{
//	float2 offset = float2(0.0, 0.0);
//	float2 scale = float2(0.5, 0.5);
//	float2 localUV = (input.uv - offset) / scale;
//
//	float2 screenUV = input.uv;
//
//	float2 ndc = screenUV * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 ndcPos = float4(ndc, 1, 1);
//	float4 viewDirVS = mul(ndcPos, InvProj);
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndAlpha.xyz;
//
//
//	//rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//
//	// ---- 의료용 연출 파라미터 ----
//	float skinBias = 0.0025;   // 0.001~0.006 사이에서 튜닝 (화면/near/far에 따라 달라짐)
//	float fadeWidth = 0.006;    // 경계 페이드 폭
//
//
//
//
//	// ✅ 현재 픽셀의 메쉬 depth 읽기
//	float meshDepthNDC = SceneDepth.Load(int3(input.pos.xy, 0));
//	//float depth01 = meshDepth;   // ⭐ 이 줄이 빠졌던 것
//	//// NDC Z
//	//float ndcZ = depth01 * 2.0f - 1.0f;
//
//	//// NDC position
//	//float4 clipPos = float4(ndc.x, ndc.y, ndcZ, 1.0f);
//
//	//// View space
//	//float4 meshViewPos = mul(clipPos, InvProj);
//	//meshViewPos /= meshViewPos.w;
//
//	//float meshDepthVS = meshViewPos.z;
//
//
//	//////   // 디버그: mesh depth 시각화
//	//return float4(meshDepthNDC, meshDepthNDC, meshDepthNDC, 1.0);
//
//	// meshDepthNDC는 0~1이라고 가정
//	float meshDepth = meshDepthNDC - skinBias; // ✅ 의도적으로 메쉬를 "앞"으로 당김 => CT가 뒤로 밀려 보임
//
//	
//
//	
//
//
//	float3 boxMin = float3(-1, -0.75, -0.75);
//	float3 boxMax = float3(1, 0.75, 0.75);
//
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 tMin = (boxMin - rayPos) * invDir;
//	float3 tMax = (boxMax - rayPos) * invDir;
//
//
//	float3 t1 = min(tMin, tMax);
//	float3 t2 = max(tMin, tMax);
//
//	float tNear = max(max(t1.x, t1.y), t1.z);
//	float tFar = min(min(t2.x, t2.y), t2.z);
//	tNear = max(tNear, 0.0);
//
//	float travelDist = tFar - tNear;
//
//	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
//	{
//		tNear = 0.0;
//	}
//
//
//	float stepSize = travelDist / float(VoxelAndMaxSteps.w);
//
//	// ✅ Jittering
//	float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
//	// 시작점에 랜덤 오프셋
//	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
//
//
//	startPos = clamp(startPos, boxMin, boxMax);
//
//	/*float3 currentPos = startPos;
//
//	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);*/
//
//	float4 acc = float4(0, 0, 0, 0);
//
//
//	// ✅✅✅ View space ray 정보 (depth 계산용) ✅✅✅
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//	float3 rayDirVS = normalize(mul(float4(rayDirWS, 0), View).xyz);
//
//
//
//
//	//int sampleCount = 0;
//
//
//
//[loop]
//for (int i = 0; i < VoxelAndMaxSteps.w; i++)
//{
//
//	//// ========== Depth 체크 ==========
//	//	// 현재 ray 위치를 클립 공간으로 변환
//	//float4 worldPos = float4(rayPos, 1.0f);
//	//float4 viewPos = mul(worldPos, View);
//	//float4 clipPos = mul(viewPos, Projection);
//
//	////// Perspective divide
//	////float currentDepth = clipPos.z / clipPos.w;
//
//
//	//// Perspective divide
//	//float currentDepth = viewPos.z;
//
//	//// ✅ 메쉬보다 뒤에 있으면 중단
//	//if (currentDepth > meshDepth)
//	//{
//	//	break;  // 더 이상 샘플링 안 함
//	//}
//
//
//	////return float4(meshDepth, meshDepth, meshDepth, 1);
//
//
//
//
//	float3 currentPos = startPos + rayDir * (i * stepSize);
//	float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//
//	uvw.y = 1.0 - uvw.y;  // ✅ 추가
//
//	if (any(uvw < 0.0) || any(uvw > 1.0))
//		break;
//
//
//
//	// ✅✅✅ Depth 체크 (메쉬보다 뒤면 안 그림!) ✅✅✅
//	float tCurrent = tNear + i * stepSize;
//	float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
//	float4 clipPos = mul(float4(currentPosVS, 1.0), Projection);
//	float currentDepthNDC = clipPos.z / clipPos.w;  // NDC depth (0~1)
////
////	// currentDepthNDC가 mesh보다 얼마나 앞/뒤인지
////	float d = meshDepth - currentDepthNDC;     // d>0 : 볼륨이 메쉬 "앞"(가까움), d<0 : 볼륨이 메쉬 "뒤"(가려져야 함)
////
////
////	// ---- Soft occlusion factor ----
////// d가 0 근처(경계)면 서서히 사라지고, 충분히 뒤면 0
////	float occ = saturate((d) / fadeWidth);   // 0..1
////	// occ = 1  => 메쉬보다 앞이므로 정상 표시
////	// occ ~ 0  => 메쉬 뒤쪽이므로 투명해짐
//
//	// 1) Raw 기반 density
//	float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//
//	//// 2) DICOM HU 로 변환
//	float hu = raw/* * HuParams.x + HuParams.y*/;   // -1000 ~ 3000 같은 범위
//
//
//	// 3) 윈도우/레벨 범위로 정규화 (0~1)
//	float huNorm = (hu - HuParams.z) / (HuParams.w - HuParams.z);
//	huNorm = saturate(huNorm);
//
//	// Volume shader
//	if (hu < 400 && CameraPosAndAlpha.w == 2.0) continue;  // Threshold 높이기
//
//	//float4 colorAlpha = transferFunction.Sample(tfSampler, huNorm);
//	float4 colorAlpha = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//
//	// ⭐ Window로 알파만 조절 (조직 분리 유지)
//	float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
//	if (huInWindow < 0.0 || huInWindow > 1.0) {
//		colorAlpha.a *= 0.05;  // Window 밖은 투명하게
//	}
//
//
//	if (colorAlpha.a < 0.001)
//		continue;
//
//
//	// 조명 계산
//	float3 eps = float3(1.0 / VoxelAndMaxSteps.x, 1.0 / VoxelAndMaxSteps.y, 1.0 / VoxelAndMaxSteps.z);
//
//	float dx = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
//		volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
//	float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
//		volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
//	float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
//		volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;
//
//	float3 N = normalize(float3(dx, dy, dz) + 1e-6);
//
//
//	float3 L = normalize(float3(0.5, 0.7, -0.5));
//	float3 V = -rayDir;
//	float3 H = normalize(L + V);
//
//	float lambert = max(dot(N, L), 0.0);
//	float spec = pow(max(dot(N, H), 0.0), 48.0);
//
//	float lighting = 0.88 + lambert * 0.12;
//	colorAlpha.rgb *= lighting;
//	colorAlpha.rgb += spec * float3(0.08, 0.07, 0.06);
//
//	//colorAlpha.a *= occ;
//	//if (acc.a < 0.001) continue;
//
//
//	// ✅ Depth margin 추가
//	float depthMargin = 0.001;  // 약간의 여유
//
//	//if (currentDepthNDC > meshDepthNDC) {
//	if (currentDepthNDC > meshDepthNDC + depthMargin) {
//		break;  // ✅ Volume이 mesh 뒤에 있으면 중단!
//	}
//
//	//if (occ <= 0.0) break;
//
//
//	//// 경계 근처(occ가 낮아질수록) CT 존재감 더 낮추기
//	//float boundarySoft = pow(occ, 1.5);     // 1~3 정도로 취향 튜닝
//	//colorAlpha.rgb *= lerp(0.7, 1.0, boundarySoft);
//	//colorAlpha.a *= boundarySoft;
//
//	float3 color = colorAlpha.rgb;
//	float alpha = colorAlpha.a * stepSize * 8.0;
//
//	if (alpha > 0.001) {
//		acc.rgb += (1.0 - acc.a) * alpha * color;
//		acc.a += (1.0 - acc.a) * alpha;
//		if (acc.a >= 0.95) break;
//	}
//
//
//}
//
//	// 후처리 (간단하게!)
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//	//acc.a = 0.0;
//
//
//
//	if(CameraPosAndAlpha.w==1.0)
//		return float4(acc.rgb, 1.0);
//	else if(CameraPosAndAlpha.w == 0.0)
//		return float4(acc.rgb, 0.0);    // ← RGB는 같지만 alpha=0 (투명)
//	else
//		return float4(acc.rgb, acc.a);
//}
//




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

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
	/* ===============================
	   Screen → Ray setup
	=============================== */

	//return float4(1, 0, 1, 1); // 자홍

	float2 screenUV = input.uv;
	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;

	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);
	viewDirVS /= viewDirVS.w;

	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosAndAlpha.xyz;

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	/* ===============================
	   Volume box
	=============================== */

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

	/* ===============================
	   Mesh depth (0~1)
	=============================== */

	float meshDepth01 = SceneDepth.Load(int3(input.pos.xy, 0));

	//return float4(meshDepth01, meshDepth01, meshDepth01, 1);

	// 의료용 연출 파라미터
	float skinBias01 = 0.003;   // 피부 두께
	float fadeWidth01 = 0.012;   // 경계 soft width

	//meshDepth01 -= skinBias01;

	/* ===============================
	   View-space ray (depth compare)
	=============================== */

	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
	float3 rayDirVS = normalize(mul(float4(rayDirWS, 0), View).xyz);

	/* ===============================
	   Accumulation
	=============================== */

	float4 acc = float4(0,0,0,0);

	// jitter
	float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
	startPos = clamp(startPos, boxMin, boxMax);

	/* ===============================
	   Raymarch
	=============================== */

	[loop]
	for (int i = 0; i < VoxelAndMaxSteps.w; i++)
	{
		float tCurrent = tNear + i * stepSize;
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
		uvw.y = 1.0 - uvw.y;

		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;

		/* ---------- depth compare ---------- */

		float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
		float4 clipPos = mul(float4(currentPosVS, 1), Projection);
		float currentDepth01 = (clipPos.z / clipPos.w) * 0.5 + 0.5;

		//return float4(currentDepth01, currentDepth01, currentDepth01, 1);

		float d = meshDepth01 - currentDepth01;

		// 🔥 soft clamp
		float occ = saturate(d / fadeWidth01);

		// 완전 차단 금지
		occ = max(occ, 0.15);

		//if (occ <= 0.0)
		//	break;

		occ = saturate(occ + 0.05); // 최소 보장

		/* ---------- sample volume ---------- */

		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
		float hu = raw;

		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));

		if (CameraPosAndAlpha.w == 2.0 && hu < 400)
			continue;

		float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);
		if (ca.a < 0.001)
			continue;

		ca.a *= occ;

		/* ---------- lighting (soft) ---------- */

		float3 eps = 1.0 / VoxelAndMaxSteps.xyz;
		float3 g;
		g.x = volumeTex.SampleLevel(samp, uvw + float3(eps.x,0,0),0).r -
			  volumeTex.SampleLevel(samp, uvw - float3(eps.x,0,0),0).r;
		g.y = volumeTex.SampleLevel(samp, uvw + float3(0,eps.y,0),0).r -
			  volumeTex.SampleLevel(samp, uvw - float3(0,eps.y,0),0).r;
		g.z = volumeTex.SampleLevel(samp, uvw + float3(0,0,eps.z),0).r -
			  volumeTex.SampleLevel(samp, uvw - float3(0,0,eps.z),0).r;

		float3 N = normalize(g + 1e-6);
		float3 L = normalize(float3(0.5,0.7,-0.5));
		float3 V = -rayDir;
		float3 H = normalize(L + V);

		float lambert = max(dot(N,L),0.0);
		float spec = pow(max(dot(N,H),0.0), 48.0) * occ;

		float lighting = (CameraPosAndAlpha.w == 2.0)
						 ? (0.92 + lambert * 0.08)
						 : (0.88 + lambert * 0.12);

		ca.rgb *= lighting;
		ca.rgb += spec * float3(0.06,0.05,0.04);

		/* ---------- accumulate ---------- */

		//float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 4.5 : 8.0;
		//float alpha = ca.a * stepSize * alphaScale;

		float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 12.0 : 18.0; // 기존 4.5/8.0 → 크게



		ca.a = saturate(ca.a * 2.5);   // 1.5~4 사이 튜닝


		//float alpha = ca.a * stepSize * alphaScale;
		float alpha = ca.a * max(stepSize, 0.002) * alphaScale;

		acc.rgb += (1.0 - acc.a) * alpha * ca.rgb;
		acc.a += (1.0 - acc.a) * alpha;

		if (acc.a >= 0.95)
			break;
	}

	/* ===============================
	   Post
	=============================== */

	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);

	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
	return float4(acc.rgb, acc.a);
}
