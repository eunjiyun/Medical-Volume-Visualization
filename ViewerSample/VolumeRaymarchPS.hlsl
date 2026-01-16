cbuffer CB : register(b0)
{
	matrix VolumeWorld;
	matrix InvView;
	matrix InvProj;
	matrix InvVolumeWorld;
	matrix InvVolumeWorldCorrected;
	matrix View;
	matrix Projection;

	float4 CameraPosAndAlpha;   // xyz = camera pos, w = mode
	float4 VoxelAndMaxSteps;    // xyz = voxel dim, w = maxSteps
	float4 HuParams;            // x,y unused / z = HU min / w = HU max

	float4 volSize;
};


Texture3D<float> volumeTex        : register(t0);
SamplerState     samp             : register(s0);

Texture1D<float4> transferFunction : register(t1);
SamplerState      tfSampler        : register(s1);

// mesh depth (0~1)
Texture2D<float> SceneDepth : register(t2);
Texture2D<float> faceColor : register(t6);


// mesh depth (0~1)
SamplerState pointClamp : register(s5); // 포인트+클램프 추천 (디버그용)
SamplerState faceColorSamp : register(s6); // 포인트+클램프 추천 (디버그용)



struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

struct PSOut
{
	float4 color  : SV_Target0;   // 디버그용 (ΔZ 맵)
	float  hitZ : SV_Target1;   // (옵션) 볼륨 hit viewZ 저장용 (R32_FLOAT RT 필요)
};



// depth01 : 0~1 depth buffer value
// proj    : Projection matrix (same one used for rendering)
float ReconstructViewZ_InvProj(float2 uv,float depth01, matrix proj)
{
	//float2 uv = input.uv;

	//uv.x *= 0.5;
	//uv.y *= 0.5;

	float2 ndc = uv * 2.0f - 1.0f;
	//ndc.y = -ndc.y;
	ndc.y = 1.0f - uv.y * 2.0f; // D3D flip

	float z_ndc = depth01 * 2 - 1;   // ❗ 반드시 필요

	float4 clip = float4(ndc.x,ndc.y, z_ndc, 1.0f);
	float4 view = mul(clip, InvProj);   // 너가 row-vector 스타일이면 mul(v, M) 유지
	view /= max(view.w, 1e-6);

	return view.z; // view-space z
}



//float4 main(PSInput input) : SV_Target
//{
//	float2 uv = input.uv;
//
//	/* ---------------------------
//	   Ray setup (view / world)
//	--------------------------- */
//
//	float2 ndc = uv * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 farClip = float4(ndc, 1, 1);
//	float4 farVS = mul(farClip, InvProj);
//	farVS /= max(farVS.w, 1e-6);
//
//	float3 rayDirVS = normalize(farVS.xyz);
//	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndAlpha.xyz;
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//	//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	/* ---------------------------
//	   Volume bounds
//	--------------------------- */
//
//	//float3 boxMin = float3(-1, -0.75, -0.75);
//	//float3 boxMax = float3(1,  0.75,  0.75);
//
//
//
//	float3 boxMin = float3(-0.5, -0.5, -0.5);
//	float3 boxMax = float3(0.5, 0.5, 0.5);
//
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 t0 = (boxMin - rayPos) * invDir;
//	float3 t1 = (boxMax - rayPos) * invDir;
//
//	float3 tmin = min(t0, t1);
//	float3 tmax = max(t0, t1);
//
//	float tNear = max(max(tmin.x, tmin.y), tmin.z);
//	float tFar = min(min(tmax.x, tmax.y), tmax.z);
//
//	if (tFar < max(tNear, 0.0))
//		return float4(0,0,0,1);
//
//	tNear = max(tNear, 0.0);
//	float stepSize = (tFar - tNear) / VoxelAndMaxSteps.w;
//
//	/* ---------------------------
//	   Mesh depth (once!)
//	--------------------------- */
//
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	bool hasMesh = (meshDepth01 < 0.9999);
//
//	float meshViewZ = hasMesh
//		? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj)
//		: -1e9;
//
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//
//	/* ---------------------------
//	   Accumulation
//	--------------------------- */
//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < VoxelAndMaxSteps.w; ++i)
//	{
//		float t = tNear + (i + 0.5) * stepSize;
//
//		// view space (depth compare)
//		float3 posVS = rayPosVS + rayDirVS * t;
//		float rayViewZ = posVS.z;
//
//		if (hasMesh && rayViewZ < meshViewZ)
//			continue;
//
//
//	/*	float diff = rayViewZ - meshViewZ;
//		return float4(diff < 0 ? 1 : 0, diff > 0 ? 1 : 0, 0, 1);*/
//
//
//
//
//		//// volume space
//		//float3 posVol = rayPos + rayDir * t;
//		//float3 uvw = (posVol - boxMin) / (boxMax - boxMin);
//		//uvw.y = 1.0 - uvw.y;
//
//		//if (any(uvw < 0.0) || any(uvw > 1.0))
//		//	continue;   // ❗ break ❌
//
//
//		float tCurrent = tNear + i * stepSize;
//							float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
//			float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
//				float3 currentPos = startPos + rayDir * (i * stepSize);
//						float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//				uvw.y = 1.0 - uvw.y;
//		
//				if (any(uvw < 0.0) || any(uvw > 1.0))
//					break;
//
//
//		
//		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//		float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//
//		if (col.a < 0.001)
//			continue;
//
//
//
//
//		float alpha = col.a * stepSize * 10.0;
//
//		// --- gradient lighting ---
//		float3 eps = 1.0 / VoxelAndMaxSteps.xyz;
//
//		// central difference gradient
//		float3 g;
//		g.x = volumeTex.SampleLevel(samp, uvw + float3(eps.x, 0, 0), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(eps.x, 0, 0), 0).r;
//		g.y = volumeTex.SampleLevel(samp, uvw + float3(0, eps.y, 0), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(0, eps.y, 0), 0).r;
//		g.z = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps.z), 0).r -
//			volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps.z), 0).r;
//
//		// gradient magnitude (edge detector)
//		float gradMag = length(g);
//
//		// HU 기반 bone gate (먼저!)
//		float boneGate = saturate((hu - 600.0) / 1200.0);
//		boneGate = smoothstep(0.1, 0.6, boneGate);
//
//		// --- edge boost (경계 강조) ---
//		float edgeGate = saturate((gradMag - 0.02) / 0.08);
//		edgeGate = smoothstep(0.0, 1.0, edgeGate);
//		edgeGate *= boneGate;                     // bone 영역에서만
//
//		float edgeBoost = lerp(1.0, 1.08, edgeGate);
//		col.rgb *= edgeBoost;                     // 색만 조정
//
//		// --- soft lighting (view-aligned) ---
//		float3 N = normalize(g + 1e-6);
//		float3 L = normalize(-rayDir);
//
//		float lambert = saturate(dot(N, L));
//		lambert = pow(lambert, 1.5);
//
//		// ⭐ 아주 약한 대비 조명
//		float lighting = lerp(0.96, 1.04, lambert);
//		lighting = lerp(1.0, lighting, boneGate * 0.35);
//
//		// 색에만 곱하기 (알파 ❌)
//		col.rgb *= lighting;
//
//
//
//
//
//
//
//
//		//float distVS = rayViewZ - meshViewZ;
//
//
//
//		////float dbg = saturate(distVS / 5.0); // 0~5mm 기준
//		////return float4(dbg, dbg, dbg, 1);
//
//		//// 안전장치 1: 음수 방지
//		//distVS = max(distVS, 0.0);
//
//		//// 페이드 계산
//		//float skinDepthVS = 2.0;   // mm
//		//float fade = saturate(distVS / skinDepthVS);
//		//fade = fade * fade * (3.0 - 2.0 * fade); // smoothstep
//
//		//// ⭐ 핵심: 최소 기여 보장
//		//float minFade = 0.25;      // 0.2 ~ 0.4 사이 튜닝
//		//fade = max(fade, minFade);
//
//		//alpha *= fade;
//
//		//return float4(fade, fade, fade, 1);
//
//
//		acc.rgb += (1.0 - acc.a) * alpha * col.rgb;
//		acc.a += (1.0 - acc.a) * alpha;
//
//		if (acc.a > 0.98)
//			break;
//	}
//
//
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//	//return float4(acc.rgb, 1.0);
//
//
//	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
//	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
//	return float4(acc.rgb, acc.a);
//}


//float4 main(PSInput input) : SV_Target
//{
//	//return float4(1,0,0,1);
//	float2 uv = input.uv;
//
//	/* ---------------------------
//	   Ray setup (view / world)
//	--------------------------- */
//
//	float2 ndc = uv * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 farClip = float4(ndc, 1, 1);
//	float4 farVS = mul(farClip, InvProj);
//	farVS /= max(farVS.w, 1e-6);
//
//	float3 rayDirVS = normalize(farVS.xyz);
//	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndAlpha.xyz;
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//	//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	/* ---------------------------
//	   Volume bounds
//	--------------------------- */
//
//	//float3 boxMin = float3(-1, -0.75, -0.75);
//	//float3 boxMax = float3(1,  0.75,  0.75);
//
//	//	float3 boxMin = float3(-0.5, -0.5, -0.5);
//	//float3 boxMax = float3(0.5, 0.5, 0.5);
//
//
//
//
//	// CT 물리 비율 반영한 박스
//	float3 boxMin = float3(
//		-volSize.x/ volSize.w * 0.5,
//		-volSize.y / volSize.w * 0.5,
//		-volSize.z / volSize.w * 0.5);
//
//	float3 boxMax = -boxMin;
//
//
//
//	//float3 boxMin = float3(-0.75, -0.75, -0.75);
//	//float3 boxMax = float3(0.75, 0.75, 0.75);
//
//
//	//float margin = 0.01;
//
//	//float3 boxMin = float3(-0.5 - margin, -0.5 - margin, -0.5 - margin);
//	//float3 boxMax = float3(0.5 + margin, 0.5 + margin, 0.5 + margin);
//
//
////	float3 volBoxMin = float3(-0.5, -0.5, -0.5);
////float3 volBoxMax = float3( 0.5,  0.5,  0.5);
//
//
//
//
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 t0 = (boxMin - rayPos) * invDir;
//	float3 t1 = (boxMax - rayPos) * invDir;
//
//	float3 tmin = min(t0, t1);
//	float3 tmax = max(t0, t1);
//
//	float tNear = max(max(tmin.x, tmin.y), tmin.z);
//	float tFar = min(min(tmax.x, tmax.y), tmax.z);
//
//	if (tFar < max(tNear, 0.0))
//		return float4(0,0,0,1);
//
//	tNear = max(tNear, 0.0);
//
//
//
//	
//
//	float stepSize = (tFar - tNear) / VoxelAndMaxSteps.w;
//
//
//	//// 교차 후
//	//tNear += stepSize * 2.0;
//	//tFar -= stepSize * 2.0;
//
//
//
//	//// ⭐ 여기서 한 번만!
//	//float entryBias = stepSize * 2.0;   // 또는 0.002
//	//tNear += entryBias;
//
//
//	/* ---------------------------
//	   Mesh depth (once!)
//	--------------------------- */
//
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	bool hasMesh = (meshDepth01 < 0.9999);
//
//	float meshViewZ = hasMesh
//		? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj)
//		: -1e9;
//
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//
//	/* ---------------------------
//	   Accumulation
//	--------------------------- */
//
//	float4 acc = float4(0,0,0,0);
//
//
//	float entryBias = stepSize * 2.0;
//	tNear += entryBias;
//
//	[loop]
//	for (int i = 0; i < VoxelAndMaxSteps.w; ++i)
//	{
//		float t = tNear + i * stepSize;
//
//		float3 posVS = rayPosVS + rayDirVS * t;
//		float rayViewZ = posVS.z;
//
////		//// ⭐ HARD DEPTH BLOCK
////		if (hasMesh && rayViewZ < meshViewZ)
////			continue;
////
////		//		//if (hasMesh && rayViewZ < meshViewZ)
//////		//	continue; // mesh 앞이면 차단
//
//		float depthDiff = rayViewZ - meshViewZ;
//
//		// 메쉬보다 앞이면 차단
//		if (hasMesh && depthDiff < -stepSize * 2.0)
//			continue;
//
//
//
//
//		float3 pos = rayPos + rayDir * t;
//		//float3 uvw = (pos - boxMin) / (boxMax - boxMin);
//		//uvw.y = 1.0 - uvw.y;
//
//		//if (any(uvw < 0.0) || any(uvw > 1.0))
//		//	break;
//
//				float tCurrent = tNear + i * stepSize;
//					float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
//
//					//jitter = 0;
//
//	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//				float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//		uvw.y = 1.0 - uvw.y;
//
//		if (any(uvw < 0.02) || any(uvw > 0.99))
//			continue;
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			continue;
//
//
//		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//		float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//
//		if (col.a < 0.001)
//			continue;
//
//		float alpha = col.a * stepSize * 10.0;
//		acc.rgb += (1.0 - acc.a) * alpha * col.rgb;
//		acc.a += (1.0 - acc.a) * alpha;
//
//		if (acc.a > 0.98)
//			break;
//	}
//
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//
//
//		if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
//	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
//
//
//	//return float4(acc.rgb, acc.a);
////	return float4(acc.rgb, 1.0);
//	return float4(acc.rgb, 0.6);
//}










// 샘플 위치 흐름 (좌표계 흐름 요약)

//Screen UV
//화면 픽셀 좌표를 졍규화한 값(0~1 범위)

//→ NDC
//Normalized Device Coordinates
//Screen UV를 -1, 1로 변환한 좌표
//GPU 렌더링 파이프라인에서 표준화된 공간.

//→ View Space(ray origin)
//카메라 기준 좌표계
//레이마칭에서는 카메라 위치가 레이의 시작점, 
//NDC를 역투영해 레이 방향을 얻음.

//→ World Space(rayPosWS)
//View Space에서 월드 좌표계로 변환
//실제 씬의 오브젝트와 동일한 좌표계에서 레이가 어디로 향하는지 계산.

//→ Local Volume Space(posL)
//특정 볼륨의 로컬 좌표계
//월드 공간의 레이을 해당 오브젝트의 로컬 공간으로 변환.

//→ UVW(0~1)
//로컬 좌표를 텍스처 좌표로 정규화.
//볼륨 텍스처는 3D 이미지이므로 (u, v, w) 형태

//→ volumeTex.Sample
//최종적으로 3D 텍스처에서 샘플링.
//레이마칭은 이 샘플링을 레이 경로를 따라 여러 번 반복해서 누적(적분)하는 과정.
//결과적으로 픽셀 색상은 레이 경로상의 밀도/색상 값들의 합성으로 결정됨.




//화면 픽셀(Screen UV)
//→ GPU 표준 좌표(NDC)
//→ 카메라 기준(View Space)
//→ 씬 좌표(World Space)
//→ 오브젝트 기준(Local Volume Space)
//→ 텍스처 좌표(UVW)
//→ 3D 텍스처 샘플링(volumeTex.Sample)

//즉, 픽셀 → 레이 → 월드 → 오브젝트 → 텍스처로 좌표계를 계속 변환하면서, 
//최종적으로 레이 경로를 따라 3D 텍스처를 샘플링하는 게 볼륨 레이마칭

//std::cout << "floatData check: "
//<< floatData[0] << " "
//<< floatData[100] << " "
//<< floatData[10000] << std::endl;




float4 main(PSInput input) : SV_Target
{
	float2 uv = input.uv;
	float2 ndc = uv * 2.0 - 1.0;
	ndc.y = -ndc.y;



	// 1. ray origin (view space)
	float4 rayOriginVS4 = mul(float4(ndc, 0.0, 1.0), InvProj);

	// 각 픽셀의 월드로 나가는 시작점
	float3 rayOriginVS = rayOriginVS4.xyz;

	//모든 레이가 동일한 방향
	// 2. ray direction (view space, fixed)
	//레이 이동 방향이 z축인듯 하지만
	float3 rayDirVS = float3(0, 0, 1);

	// 3. view -> world
	//월드 공간 레이 시작점
	float3 rayPosWS = mul(float4(rayOriginVS, 1), InvView).xyz;

	//월드 공간 레이 방향
	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);

	// (옵션) view-space origin (mesh depth 비교용)
	float3 rayPosVS = rayOriginVS;



	/* ---------------------------
	   Volume bounds (LOCAL space)
	   - still define the box in volume-local normalized space
	   - using physical aspect ratio (volSize.xyz / volSize.w)
	--------------------------- */
	//볼륨 로컬 좌표계가 mm단위라는 전제가 있음
	float3 boxMinL = float3(
		-volSize.x * 0.5,
		-volSize.y  * 0.5,
		-volSize.z* 0.5);

	float3 boxMaxL = -boxMinL;

	/* ---------------------------
	   Intersect in LOCAL space (stable),
	   but convert entry/exit to WORLD t
	--------------------------- */



	//float4x4 flipYZ = {
	//	1.0, 0.0, 0.0, 0.0,
	//	0.0, -1.0, 0.0, 0.0,
	//	0.0, 0.0, -1.0, 0.0,
	//	0.0, 0.0, 0.0, 1.0
	//};

	////float4 worldPos = mul(float4(input.position, 1.0f), mul(flipYZ, InVolumeWorld));
	//float 4x4 InvVolumeWorld=



	//로컬 공간에서의 교차
	// Transform ray into volume-local space for intersection ONLY
	float3 rayPosL = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//float3 rayPosL = mul(float4(rayPosWS, 1), mul(flipYZ, InVolumeWorld)).xyz;

	//정규화 안 한 것
	float3 rayDirL = mul(float4(rayDirWS, 0), InvVolumeWorld).xyz;   // NOTE: no normalize here
	//return float4(abs(normalize(rayDirL)),1);
	//InvVolumeWorld가 월드 축을 볼륨 로컬 축으로 어떻게 매핑할지를 정함.


	//DICOM 슬라이스 적재 순서

	//	볼륨을 w × h × d로 해석한 방식
	//	volSize.x / y / z에 어떤 물리 축을 넣었는지
	//	boxMinL / boxMaxL를 어떤 축 기준으로 만들었는지
	//	uvw.y = 1 - uvw.y 같은 보정이 어느 축에 적용됐는지




	float3 invDirL = 1.0 / (rayDirL + 1e-6);


	float3 t0L = (boxMinL - rayPosL) * invDirL;
	float3 t1L = (boxMaxL - rayPosL) * invDirL;

	float3 tminL = min(t0L, t1L);
	float3 tmaxL = max(t0L, t1L);

	//로컬 파라미터
	float tNearL = max(max(tminL.x, tminL.y), tminL.z);
	float tFarL = min(min(tmaxL.x, tmaxL.y), tmaxL.z);

	if (tFarL < max(tNearL, 0.0))
		return float4(0, 0, 0, 1);

	tNearL = max(tNearL, 0.0);

	// Compute entry/exit points in LOCAL
	float3 entryL = rayPosL + rayDirL * tNearL;
	float3 exitL = rayPosL + rayDirL * tFarL;


	//월드 기준으로 레이마칭 변경
	// Transform entry/exit to WORLD
	// (requires VolumeWorld in your CB)
	//로컬 -> 월드 거리 변환
	float3 entryWS = mul(float4(entryL, 1), VolumeWorld).xyz;
	float3 exitWS = mul(float4(exitL,  1), VolumeWorld).xyz;

	// Convert to WORLD t (distance along world ray)
	float tNearW = dot(entryWS - rayPosWS, rayDirWS);
	float tFarW = dot(exitWS - rayPosWS, rayDirWS);



	// Safety
	if (tFarW < max(tNearW, 0.0))
		return float4(0, 0, 0, 1);

	tNearW = max(tNearW, 0.0);


	// World step size (THIS is the big fix: marching distance is in world units)
	float maxSteps = VoxelAndMaxSteps.w;

	//월드 단위 stepSize
	float stepW = (tFarW - tNearW) / maxSteps;



	//float2 uvDepth = (uv - float2(0.0, 0.0)) * 0.5;

	// TL viewport 기준 uv → 전체 화면 depth uv
	float2 uvFull;
	uvFull.x = input.uv.x * 0.5;
	uvFull.y = input.uv.y * 0.5;

	/* ---------------------------
	   Mesh depth (screen-space) -> view Z
	--------------------------- */
	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uvFull, 0);
	//bool hasMesh = (meshDepth01 < 0.9999);
	//hasMesh = false;

	////float meshViewZ = hasMesh
	////	? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj)
	////	: -1e9;



	//float meshViewZ = -1e9;


	



	//////바인딩 패스 문제
	////검은 화면
	//float d = SceneDepth.SampleLevel(pointClamp, uvFull, 0);
	////float d = SceneDepth.SampleLevel(faceColorSamp, uv, 0);
	//return float4(d, d, d, 1);


	////빨간색 정사각형
	//float d01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
	//float z = ReconstructViewZ_InvProj(uv, d01, InvProj);
	////return float4(abs(z) / 500.0, 0, 0, 1);


	//float meshViewZ = SceneDepth.SampleLevel(pointClamp, uvFull, 0);

	float d01 = SceneDepth.SampleLevel(pointClamp, uvFull, 0);
	float meshViewZ = ReconstructViewZ_InvProj(uvFull, d01, InvProj);

	//return float4(abs(meshViewZ) / 700, 0, 0, 1);





	bool hasMesh = (meshViewZ < 1e8); // background 제외


	/* ---------------------------
	   Accumulation
	--------------------------- */
	float4 acc = float4(0,0,0,0);

	// entry bias in WORLD units
	//float entryBiasW = stepW * 2.0;
	float entryBiasW = min(stepW * 0.5, 0.5);


	float tStartW = tNearW + entryBiasW;

	//// jitter in WORLD step
	//float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);

	float hitViewZ = 0.0;
	bool  hasHit = false;
	float deltaZ = 0.0f;

	[loop]
	for (int i = 0; i < (int)maxSteps; ++i)
	{
		//return float4(0, 1, 0, 1);
		float tW = tStartW + (i /*+ jitter*/) * stepW;

		// World position along ray
		float3 posWS = rayPosWS + rayDirWS * tW;

		// View Z for mesh-occlusion compare (now consistent!)
		float rayViewZ = mul(float4(posWS, 1), View).z;

		// depth block (tolerance in VIEW units; use a small constant or scale by step)
		// stepW is world units; convert rough tolerance to view by multiplying by |rayDirVS.z|
		float stepV = abs(stepW * rayDirVS.z);
		float depthDiff = rayViewZ - meshViewZ;

		//if (hasMesh && depthDiff < -stepV * 2.0)
		//	continue;

		// Transform sample position to volume-local for texture lookup
		float3 posL = mul(float4(posWS, 1), InvVolumeWorldCorrected).xyz;
		//float3 posL = mul(float4(posWS, 1), InvVolumeWorld).xyz;

		//return float4(abs(posL) * 0.01, 1);

		// Local -> UVW
		float3 uvw = (posL - boxMinL) / (boxMaxL - boxMinL);
		//uvw.y = 1.0 - uvw.y;
		uvw.x = 1.0 - uvw.x;
		uvw.z = 1.0 - uvw.z;


		////3D볼륨 로컬축이 
		////u(x) : 좌우, v(y) : 앞뒤, w(z) 
		////: 위아래(axial 적층)으로 쌓여있는걸 확인
		//// Sagittal 단면 (X 고정)
		//float3 uvw = float3( input.uv.x,0.5,input.uv.y);
		//float v = volumeTex.Sample(samp, uvw, 0).r;
		//return float4(v, v, v, 1);

		////=>볼륨 3D 텍스처의 W(Z)축은 Axial 방향으로 정의되어 있으며,
		////현재 볼륨 데이터는 Axial 기준으로 정상 적재됨을 확인함.







		//return float4(uvw, 1);

	/*	if (any(uvw < 0.0) || any(uvw > 1.0))
			continue;*/

		//// (optional) keep your margin/edge skip
		//if (any(uvw < 0.02) || any(uvw > 0.99))
		//	continue;


		uint dimX, dimY, dimZ;
		volumeTex.GetDimensions(dimX, dimY, dimZ);

		// voxel index
		float3 voxelIdx = uvw * float3(dimX,dimY, dimZ);
		voxelIdx = floor(voxelIdx) + 0.5;

		// back to uvw
		float3 uvwVoxel = voxelIdx / float3(dimX, dimY, dimZ);

		//float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
		//return float4(hu* 2000.0, hu* 2000.0, hu* 2000.0, 1);


		//float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
		////return float4(hu, hu, hu, 1);
		//return float4(hu * 0.001, hu * 0.001, hu * 0.001, 1);


		//float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
		////return float4(hu, hu, hu, 1);

		////float hu = volumeTex.SampleLevel(samp, uvwVoxel, 0).r;
		//////return float4(hu, hu, hu, 1);

		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
		float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);


	/*	float density = volumeTex.SampleLevel(samp, uvw, 0).r;
		return float4(density, density, density, 1);

		float4 col = transferFunction.SampleLevel(tfSampler, density, 0);*/



		//return float4(huNorm, huNorm, huNorm, 1);

		//return float4(col.rgb, 1);

		if (col.a < 0.001)
			continue;


		if (!hasHit)
		{
			hitViewZ = mul(float4(posWS, 1), View).z;


			//hitViewZ = tW;   // ray parameter (world distance)
			hasHit = true;


			if (hasMesh)
				deltaZ = abs(meshViewZ - hitViewZ);
		}


		//// alpha integrate (step in WORLD units now)
		////float alpha = col.a * stepW * 10.0;

		//// voxelSizeMM: 평균 voxel spacing (CB로 전달)
		////float alpha = col.a * (stepW / voxelSizeMM) * densityScale;
		////densityScale ≈ 0.05 ~ 0.2
		//float alpha = col.a * (stepW / 0.2f) * 0.1;


		float densityScale = 0.02;

		float sigma = col.a * densityScale; // densityScale ≈ 0.02 ~ 0.05
		float alpha = 1.0 - exp(-sigma * stepW);


		acc.rgb += (1.0 - acc.a) * alpha * col.rgb;
		acc.a += (1.0 - acc.a) * alpha;

		if (acc.a > 0.98)
			break;
	}

	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);



	//if (hasHit)
	//{
	//	//// 보기 좋게 스케일 (예: 100mm 기준)
	//	////float viz = saturate(abs(hitViewZ) / 500.0);

	//	////float viz = saturate(hitViewZ / 500.0); // 200mm 기준
	//	////return float4(viz, viz, viz, 1);

	//	////float viz = saturate((abs(hitViewZ) - 150.0) / 300.0);

	//	////float viz = saturate((abs(hitViewZ) - 250.0) / 250.0);


	//	////return float4(viz, viz, viz, 1);


	//	////return float4(deltaZ, 0, 0, 1);

	//	//return float4(abs(meshViewZ) / 500.0, 0, 0, 1);

	//	////return float4(abs(hitViewZ) / 500.0, 0, 0, 1);


	//	float deltaZ = abs(meshViewZ - hitViewZ);
	//	return float4(deltaZ / 10.0, 0, 0, 1); // 시각화용
	//}

	//return float4(0, 0, 0, 1);


	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);

	return float4(acc.rgb, 0.6);
}




// 네 코드의 boxMinL/boxMaxL, intersect 로직은 그대로 쓴다고 가정
// 여기서는 "tNearW~tFarW"가 이미 구해졌다고 가정하고 최소만 보여줌.

//PSOut main(PSInput input)
//{
//	PSOut o;
//	o.color = float4(0, 0, 0, 1);
//	o.hitZ = -1e9;
//
//	float2 uv = input.uv;
//
//	// ---- mesh depth -> viewZ
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	bool hasMesh = (meshDepth01 < 0.9999);
//	float meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : -1e9;
//
//	// ---- (1) Ray setup (Ortho 가정: rayDirVS 고정, rayOriginVS 픽셀별)
//	float2 ndc = uv * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 rayOriginVS4 = mul(float4(ndc, 0.0, 1.0), InvProj);
//	float3 rayPosWS = mul(float4(rayOriginVS4.xyz, 1), InvView).xyz;
//
//	float3 rayDirVS = float3(0, 0, 1);
//	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//
//	// ---- (2) Intersect volume -> get tNearW, tFarW (네 기존 코드 그대로)
//	float tNearW, tFarW;
//	// ... (네가 이미 구현한 local intersect + world t 변환 부분) ...
//
//	// 교차 없으면
//	// if (!hitBox) return black
//	// 여기서는 hitBox true라고 가정하고 진행
//
//	float maxSteps = VoxelAndMaxSteps.w;
//	float stepW = (tFarW - tNearW) / maxSteps;
//
//	float hitAlpha = 0.2;   // “첫 hit” 기준 (0.1~0.3 정도 스윕 가능)
//	float accA = 0.0;
//
//	float tStartW = tNearW + stepW * 0.5;
//
//	[loop]
//	for (int i = 0; i < (int)maxSteps; ++i)
//	{
//		float tW = tStartW + i * stepW;
//		float3 posWS = rayPosWS + rayDirWS * tW;
//
//		// world -> volume local
//		float3 posL = mul(float4(posWS, 1), InvVolumeWorld).xyz;
//
//		// local -> uvw
//		float3 boxMinL = float3(-volSize.x*0.5, -volSize.y*0.5, -volSize.z*0.5);
//		float3 boxMaxL = -boxMinL;
//
//		float3 uvw = (posL - boxMinL) / (boxMaxL - boxMinL);
//		uvw.y = 1.0 - uvw.y;
//
//		// 경계 밖 skip (필수)
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			continue;
//
//		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//		float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//
//		// “hit” 판단은 col.a 기반 (너 볼륨 잘 나오면 이게 가장 쉬움)
//		float sigma = col.a * 0.02;             // 너 쓰던 densityScale
//		float alpha = 1.0 - exp(-sigma * stepW);
//
//		accA += (1.0 - accA) * alpha;
//
//		if (accA >= hitAlpha)
//		{
//			// ✅ 첫 hit 지점 viewZ
//			float volViewZ = mul(float4(posWS, 1), View).z;
//			o.hitZ = volViewZ;
//
//			// ΔZ 맵 출력 (디버그)
//			float dz = hasMesh ? abs(volViewZ - meshViewZ) : 0.0;
//
//			// 보기 좋게 스케일링 (적당히 조절)
//			float viz = saturate(dz / 20.0); // 20mm 기준으로 흰색
//			o.color = float4(viz, viz, viz, 1);
//
//			return o;
//		}
//	}
//
//	// hit 못 찾음
//	o.color = float4(0, 0, 0, 1);
//	o.hitZ = -1e9;
//
//
//
//
//	if (hasHit)
//	{
//		// 보기 좋게 스케일 (예: 100mm 기준)
//		float viz = saturate(abs(hitViewZ) / 100.0);
//		return float4(viz, viz, viz, 1);
//	}
//
//	return float4(0, 0, 0, 1);
//
//
//
//
//
//	return o;
//}




//float4 main(PSInput input) : SV_Target
//{
//	float2 uv = input.uv;
//
//	/* ===============================
//	   Ray setup (너 코드 그대로)
//	=============================== */
//
//	float2 ndc = uv * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 farClip = float4(ndc, 1, 1);
//	float4 farVS = mul(farClip, InvProj);
//	farVS /= max(farVS.w, 1e-6);
//
//	float3 rayDirVS = normalize(farVS.xyz);
//	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndAlpha.xyz;
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	/* ===============================
//	   Volume bounds (너 코드 그대로)
//	=============================== */
//	//float3 boxMin = float3(-1, -0.75, -0.75);
//	//float3 boxMax = float3(1,  0.75,  0.75);
//
//	float3 boxMin = float3(-0.5, -0.5, -0.5);
//	float3 boxMax = float3(0.5, 0.5, 0.5);
//
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 t0 = (boxMin - rayPos) * invDir;
//	float3 t1 = (boxMax - rayPos) * invDir;
//
//	float3 tmin = min(t0, t1);
//	float3 tmax = max(t0, t1);
//
//	float tNear = max(max(tmin.x, tmin.y), tmin.z);
//	float tFar = min(min(tmax.x, tmax.y), tmax.z);
//
//	if (tFar < max(tNear, 0.0))
//		return float4(0,0,0,1);
//
//	tNear = max(tNear, 0.0);
//
//	float travelDist = tFar - tNear;
//	float stepSize = travelDist / VoxelAndMaxSteps.w;
//
//	/* ===============================
//	   Mesh depth (once)
//	=============================== */
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	bool  hasMesh = (meshDepth01 < 0.9999);
//
//	float meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//
//	/* ===============================
//	   Accum: density + edge only
//	=============================== */
//	float densityAcc = 0.0;
//	float edgeAcc = 0.0;
//	float coverageAcc = 0.0;   // ⭐ 추가
//
//	// jitter (너 코드 유지)
//	float jitter = frac(sin(dot(uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
//
//	// gradient eps (볼륨 해상도 기반)
//	float3 eps = 1.0 / VoxelAndMaxSteps.xyz;
//	float alpha;
//	[loop]
//	for (int i = 0; i < (int)VoxelAndMaxSteps.w; ++i)
//	{
//		float t = tNear + (i + jitter) * stepSize;
//
//		// view-space depth compare (너 코드 방식 유지)
//		float3 posVS = rayPosVS + rayDirVS * t;
//		float  rayViewZ = posVS.z;
//
//		if (hasMesh && rayViewZ < meshViewZ)
//			continue;
//
//		// volume space
//		float3 posVol = rayPos + rayDir * t;
//		float3 uvw = (posVol - boxMin) / (boxMax - boxMin);
//		uvw.y = 1.0 - uvw.y;
//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		// sample HU
//		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		// TF alpha만 사용 (색은 버림)
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//		float aTF = transferFunction.SampleLevel(tfSampler, huNorm, 0).a;
//
//		if (aTF < 0.001)
//			continue;
//
//
//		//// coverage는 "존재 여부"만 본다
//		////float coverageStep = (aTF > 0.02) ? 0.25 : 0.0;
//		////coverageAcc += (1.0 - coverageAcc) * coverageStep;
//		//float coverageStep = (aTF > 0.02) ? 1.0 : 0.0;
//		////coverageAcc = max(coverageAcc, coverageStep);
//		//coverageAcc += (1.0 - coverageAcc) * coverageStep * 0.2;
//
//
//
//		//return float4(coverageAcc.xxx, 1);
//
//
//		/* ===============================
//		   Depth gate (피부 뒤 억제/깊이 회복) - 최소형
//		   - mesh가 있을 때만 적용
//		=============================== */
//		float gate = 1.0;
//		if (hasMesh)
//		{
//			float distVS = rayViewZ - meshViewZ;   // mesh 뒤면 > 0 (너 코드 컨벤션)
//			distVS = max(distVS, 0.0);
//
//			// 피부층 0~skinDepthVS 구간은 약하게, 안쪽은 빠르게 회복
//			float skinDepthVS = 1.5;   // mm (튜닝)
//			gate = saturate(distVS / skinDepthVS);
//			gate = pow(gate, 0.6);     // 빠르게 회복(Planmeca 느낌)
//		}
//
//		/* ===============================
//		   density accumulate
//		   - "존재 에너지"만 적분
//		=============================== */
//		float alpha = aTF * stepSize * 6.0;
//		alpha *= gate;
//
//		// 🔥 핵심: 너무 작은 기여는 무시
//		if (alpha < 0.003)
//			continue;
//
//		densityAcc += (1.0 - densityAcc) * alpha;
//
//		coverageAcc = densityAcc;
//
//
//		//return float4(densityAcc.xxx, 1);
//
//		/* ===============================
//		   edge accumulate
//		   - gradient magnitude + HU gate + depth gate
//		=============================== */
//		// central diff gradient
//		float gx = volumeTex.SampleLevel(samp, uvw + float3(eps.x,0,0), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(eps.x,0,0), 0).r;
//		float gy = volumeTex.SampleLevel(samp, uvw + float3(0,eps.y,0), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(0,eps.y,0), 0).r;
//		float gz = volumeTex.SampleLevel(samp, uvw + float3(0,0,eps.z), 0).r -
//				   volumeTex.SampleLevel(samp, uvw - float3(0,0,eps.z), 0).r;
//
//		float gradMag = length(float3(gx,gy,gz));
//
//		// HU bone gate
//		float boneGate = saturate((hu - 600.0) / 1200.0);
//		boneGate = smoothstep(0.1, 0.6, boneGate);
//
//		// edge gate (너가 쓰던 방식)
//		float edgeGate = saturate((gradMag - 0.02) / 0.08);
//		edgeGate = smoothstep(0.0, 1.0, edgeGate);
//
//		float edge = edgeGate * boneGate * gate;
//
//		// edge도 front-to-back 누적(“겹치면 더해지는” 느낌 방지)
//		edgeAcc += (1.0 - edgeAcc) * (edge * alpha);
//
//		//return float4(edgeAcc.xxx, 1);
//
//		// termination (density 기반)
//		if (densityAcc > 0.98)
//			break;
//	}
//
//
//	//if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
//	if (CameraPosAndAlpha.w == 0.0) 
//		return float4(0,0,0,0);
//
//	// 최종 출력: R=density, G=edge
//	return float4(saturate(densityAcc), saturate(edgeAcc), 0, saturate(coverageAcc));   // A : CT presence / confidence);
//}
//
//
//
//




//float4 main(PSInput input) : SV_Target
//{
//	/* ===============================
//	   Screen → Ray setup
//	=============================== */
//
//	//return float4(1, 0, 1, 1); // 
//
//	float2 screenUV = input.uv;
//
//	float2 uv = input.uv;
//
//	//// 예: 좌상단 쿼드만 depth가 있을 경우
//	//uv.x *= 0.5;
//	//uv.y *= 0.5;
//
//	//return float4(1, 0, 0, 1);
//
//
//
//
//
//	float2 ndc = screenUV * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	
//
//	
//
//
//	float4 ndcPos = float4(ndc, 1, 1);
//	float4 viewDirVS = mul(ndcPos, InvProj);
//
//	float4 farClip = float4(ndc, 1, 1);
//	float4 farVS = mul(farClip, InvProj);
//	//viewDirVS /= viewDirVS.w;
//	farVS /= max(farVS.w, 1e-6);
//
//	float3 rayDirVS = normalize(farVS.xyz); // view space 방향
//
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndAlpha.xyz;
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	/* ===============================
//	   Volume box
//	=============================== */
//
//	//float3 boxMin = float3(-1, -0.75, -0.75);
//	//float3 boxMax = float3(1,  0.75,  0.75);
//
//	float3 boxMin = float3(-0.5, -0.5, -0.5);
//	float3 boxMax = float3(0.5, 0.5, 0.5);
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
//	tNear = max(tNear, 0.0);
//
//	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
//		tNear = 0.0;
//
//	float travelDist = tFar - tNear;
//	float stepSize = travelDist / VoxelAndMaxSteps.w;
//
//
//	
//
//	float d = SceneDepth.SampleLevel(pointClamp, uv, 0);
//
//	
//	float z = ReconstructViewZ_InvProj(uv, d, InvProj);
//
//	//// 범위 넉넉하게
//	//float v = saturate(-z / 1000.0);
//	//return float4(v, v, v, 1);
//
//
//	/* ===============================
//	   Mesh depth (0~1)
//	=============================== */
//
//	//float2 pixel = input.pos.xy;   // SV_POSITION (screen pixel)
//	//float meshDepth01 = SceneDepth.Load(int3(pixel, 0));
//
//
//	//float meshDepth01 = SceneDepth.Load(int3(input.pos.xy, 0));
//	//return float4(saturate((1 - meshDepth01) * 50), 0, 0, 1);
//	//return float4(1 - meshDepth01, 1 - meshDepth01, 1 - meshDepth01, 1);
//
//
//	//float d = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	//return float4(d, d, d, 1);
//
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	//return float4(meshDepth01, meshDepth01, meshDepth01, 1);
//
//
//
//
//
//
//
//
//	//if (meshDepth01 >= 0.9999)
//	//	return float4(0, 0, 0, 1);   // 배경
//
//
//	//float g = frac((-z) * 0.01); // 줄무늬로 변화 보이게
//	//return float4(g, g, g, 1);
//
//	//// 단순 시각화 (회색 얼굴 나와야 정상)
//	//return float4(
//	//	saturate(-meshViewZ / 500.0),
//	//	saturate(-meshViewZ / 500.0),
//	//	saturate(-meshViewZ / 500.0),
//	//	1
//	//	);
//
//	//return float4(meshDepth01, meshDepth01, meshDepth01, 1);
//
//	// 의료용 연출 파라미터
//	float skinBias01 = 0.003;   // 피부 두께
//	float fadeWidth01 = 0.012;   // 경계 soft width
//
//	
//
//	//meshDepth01 -= skinBias01;
//
//	/* ===============================
//	   View-space ray (depth compare)
//	=============================== */
//
//
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//
//
//	// View space 강제 Ray (무조건 앞)
//	farClip = float4(ndc, 1.0, 1.0);
//	farVS = mul(farClip, InvProj);
//	farVS /= max(farVS.w, 1e-6);
//	//float3 rayDirVS = normalize(farVS.xyz);
//
//
//	
//	/* ===============================
//	   Accumulation
//	=============================== */
//
//	float4 acc = float4(0,0,0,0);
//
//	// jitter
//	float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
//	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);
//	startPos = clamp(startPos, boxMin, boxMax);
//
//	/* ===============================
//	   Raymarch
//	=============================== */
//
//	[loop]
//	for (int i = 0; i < VoxelAndMaxSteps.w; i++)
//	{
//	
//
//
//		float tCurrent = tNear + i * stepSize;
//		float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
//		//float rayViewZ = currentPosVS.z;
//
//		//if (hasMesh && rayViewZ < meshViewZ)
//		//	continue; // mesh 앞이면 차단
//
//
//
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//		// 2) ray depth
//		//float3 currentPosVS = rayDirVS * tCurrent;
//
//		float4 clipPos = mul(float4(currentPosVS, 1), Projection);
//		float rayDepth01 = clipPos.z / clipPos.w * 0.5 + 0.5;
//
//		// 3) 비교 결과 시각화
//		// R = ray가 mesh 앞 (❌)
//		// G = 거의 동일 (표면)
//		// B = ray가 mesh 뒤 (✅)
//	//	float diff = rayDepth01 - meshDepth01;
//
//		/*return float4(
//			diff < 0 ? 1 : 0,
//			abs(diff) < 0.002 ? 1 : 0,
//			diff > 0 ? 1 : 0,
//			1
//			);*/
//		bool hasMesh = (meshDepth01 < 0.9999);  // 또는 1.0에 가깝지 않으면
//
//		float rayViewZ = currentPosVS.z;        // z < 0
//		float meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;
//
//		// ray가 mesh 뒤?
//		bool behind = rayViewZ < meshViewZ;  // (더 음수면 더 앞/뒤 상황에 따라 조정)
//
//
//		float diff = rayViewZ - meshViewZ;
//
//		/*float*/ rayDepth01 = 1.0 - (clipPos.z / clipPos.w * 0.5 + 0.5);
//
//	/*	return float4(
//			diff < 0 ? 1 : 0,
//			abs(diff) < 1.0 ? 1 : 0,
//			diff > 0 ? 1 : 0,
//			1
//			);
//*/
//
//
//	
//
//	
//
//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//		uvw.y = 1.0 - uvw.y;
//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		/* ---------- depth compare ---------- */
//
//		float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0); // ✅ 이걸 meshDepth01로 사용
//	
//
//	
//
//		/*float*/ //meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;
//
//
//		if (hasMesh && rayViewZ < meshViewZ)
//		{
//			// 메쉬 앞: CT 완전 차단
//			continue;
//		}
//
//		// View-space Z 복원
//
//		//return float4(meshDepth01, meshDepth01, meshDepth01, 1);
//
//		//// view space Z 시각화 (스케일링해서)
//		//return float4(saturate(-meshViewZ / 500.0), 0, 0, 1);
//
//
//
//
//
//		currentPosVS = rayPosVS + rayDirVS * tCurrent;
//
//	/*	return float4(
//			saturate(-currentPosVS.z / 500.0),
//			0,
//			0,
//			1
//			);*/
//
//	
//		//float4 clipPos = mul(float4(currentPosVS, 1), Projection);
//
//		//float rayDepth01 = clipPos.z / clipPos.w * 0.5 + 0.5;
//
//		//return float4(rayDepth01, rayDepth01, rayDepth01, 1);
//
//
//		float d = clipPos.z / clipPos.w;     // ✅ D3D
//		//return float4(d, d, d, 1);
//
//
//		float currentDepth01 = (clipPos.z / clipPos.w) * 0.5 + 0.5;
//
//		//return float4(currentDepth01, currentDepth01, currentDepth01, 1);
//
//		//float d = meshDepth01 - currentDepth01;
//
//		//float dist = currentDepth01 - meshDepth01;
//
//
//		//// 메쉬 기준 거리 (mm 스케일 유지됨)
//		//float distVS = meshViewZ - currentViewZ;
//
//		//float skinDepth = 0.01; // 0.005~0.02 튜닝
//		//float fade = saturate(dist / skinDepth);
//		//fade = pow(fade, 2.0); // ★ 핵심
//
//		//if (d >0)
//		//	discard; // 메쉬 앞
//
//		//bool hasMesh = (meshDepth01 < 0.9999);  // 또는 1.0에 가깝지 않으면
//
//
//		//if (!hasMesh) return float4(0, 0, 0, 1); // 배경은 검정으로
//		//return float4(saturate(meshDepth01), saturate(meshDepth01), saturate(meshDepth01), 1);
//
//
//		float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
//		float hu = raw;
//
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//
//	
//
//		float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//
//		// HU 기준 치아 영역
//		bool isTooth = (hu > 2000.5);
//
//		// 치아는 알파 상한 제한
//		if (isTooth)
//		{
//			ca.a = min(ca.a, 0.15);   // ⭐ 핵심
//		}
//
//	
//
//
//		float dist = currentDepth01 - meshDepth01;
//		float distVS = rayViewZ - meshViewZ;   // > 0 이면 메쉬 뒤
//
//		float hardBlock = -1.0;   // 1mm 앞까지만 완전 차단
//
//	/*	if (hasMesh && distVS < hardBlock)
//			continue;
//*/
//
//		//if (hasMesh)
//		//{
//		//	
//
//		//	// 메쉬 앞이면 CT 절대 금지
//		//	if (dist < 0.0)
//		//		continue;
//
//			// 2) skin zone fade
//			//float skinDepth = 0.01;        // 0.005~0.02
//
//			// // 2) 피부 구간
//			//float skinDepth = 0.02;
//			//float fade = smoothstep(0.0, skinDepth, dist);
//
//		bool isBone = (hu > 700.0 && hu < 1800.0);
//
//
//		float boneRecovery = 1.0;
//
//		//if (isBone)
//		//{
//		//	// 얼굴 바로 뒤 bone은 억제
//		//	if (hasMesh)
//		//	{
//		//		float d = saturate(distVS / 4.0);   // 0~4mm
//		//		boneRecovery = d;                   // 앞면 bone 얇게
//		//	}
//
//		//	// ⭐ 깊어질수록 bone 다시 살리기
//		//	float depthBoost = saturate(distVS / 20.0); // 0~20mm
//		//	boneRecovery = max(boneRecovery, depthBoost);
//		//}
//		
//
//
//
//			float skinDepthVS = 1.5;   // 5mm
//			float boneFogStart = 6.0;  // 뼈 안개 시작
//
//
//			float skinFade = saturate(distVS / skinDepthVS);
//
//			//// 부드럽게
//			skinFade = skinFade * skinFade; // 또는 smoothstep
//
//
//// CT를 빨리 살리기
//			skinFade = pow(skinFade, 0.5);    // ⭐ 핵심 (기존 fade*fade 반대)
//
//			//// 3) 알파 억제 + 피부색 중화
//			//ca.a *= fade;
//
//			float3 skinTint = float3(0.78, 0.62, 0.55);
//
//		//	ca.rgb = lerp(skinTint, ca.rgb, skinFade);
//		//	ca.a *= skinFade;
//		//}
//
//		//return float4(saturate(dist * 50), 0, 0, 1);
//
//
//
//
//
//		//// 🔥 soft clamp
//		//float occ = saturate(d / fadeWidth01);
//
//		//// 완전 차단 금지
//		//occ = max(occ, 0.15);
//
//		////if (occ <= 0.0)
//		////	break;
//
//		//occ = saturate(occ + 0.05); // 최소 보장
//
//		/* ---------- sample volume ---------- */
//
//		//float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
//		//float hu = raw;
//
//		//float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//
//		if (CameraPosAndAlpha.w == 2.0 && hu < 400)
//			continue;
//
//		//float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//		if (ca.a < 0.001)
//			continue;
//
//		//ca.a *= occ;
//
//		/* ---------- lighting (soft) ---------- */
//
//		float3 eps = 1.0 / VoxelAndMaxSteps.xyz;
//		float3 g;
//		g.x = volumeTex.SampleLevel(samp, uvw + float3(eps.x,0,0),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(eps.x,0,0),0).r;
//		g.y = volumeTex.SampleLevel(samp, uvw + float3(0,eps.y,0),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(0,eps.y,0),0).r;
//		g.z = volumeTex.SampleLevel(samp, uvw + float3(0,0,eps.z),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(0,0,eps.z),0).r;
//
//		float3 N = normalize(g + 1e-6);
//		float3 L = normalize(float3(0.5,0.7,-0.5));
//		float3 V = -rayDir;
//		float3 H = normalize(L + V);
//
//		float lambert = max(dot(N,L),0.0);
//		float spec = pow(max(dot(N,H),0.0), 48.0) /** occ*/;
//
//		float lighting = (CameraPosAndAlpha.w == 2.0)
//						 ? (0.92 + lambert * 0.08)
//						 : (0.88 + lambert * 0.12);
//		spec *= 0.05;   // 지금보다 훨씬 줄여
//		ca.rgb *= lighting;
//		ca.rgb += spec * float3(0.06,0.05,0.04);
//
//
//
//		//float3 skinTint = float3(0.78, 0.62, 0.55); // 임시값
//
//// 메쉬 바로 뒤에서는 피부색, 안쪽으로 갈수록 CT
//		//ca.rgb = lerp(skinTint, ca.rgb, fade);
//
//
//
//		/* ---------- accumulate ---------- */
//
//		//float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 4.5 : 8.0;
//		//float alpha = ca.a * stepSize * alphaScale;
//
//		float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 12.0 : 18.0; // 기존 4.5/8.0 → 크게
//
//
//
//		ca.a = saturate(ca.a * 2.5);   // 1.5~4 사이 튜닝
//	//	ca.a *= fade;
//
//
//		//float alpha = ca.a * stepSize * alphaScale;
//
//
//
//
//		float mediumTransparency = 0.35; // ⭐ 0.25 ~ 0.45 권장
//
//		if (isBone)   mediumTransparency = 0.3;
//		if (isTooth)  mediumTransparency = 0.2;
//
//		//float fog = saturate((distVS - boneFogStart) / 30.0);
//		//fog = fog * 0.35;   // ⭐ 아주 약하게
//
//		float fog = saturate((distVS - 0.01) / 0.08);          // 1%~9% 구간
//
//
//
//		float3 boneColor = float3(0.95, 0.95, 0.95);
//		ca.rgb = lerp(ca.rgb, boneColor, fog * 0.6);
//
//
//		float alpha = ca.a
//			* max(stepSize, 0.002)
//			* alphaScale
//			* mediumTransparency;
//
//		alpha *= boneRecovery;
//
//		if (isBone)
//		{
//			float frontAtten = hasMesh ? saturate(distVS / 4.0) : 1.0;
//			float deepRecover = saturate(distVS / 20.0);
//
//			float recovery = max(frontAtten, deepRecover);
//
//			// ⭐ 핵심: 0으로 깎지 말고 최소 밀도 보장
//			float baseBone = 0.55;          // 0.45 ~ 0.65 추천
//			alpha *= lerp(baseBone, 1.0, recovery);
//		}
//
//
//
//
//
//
//
//		//float alpha = ca.a * max(stepSize, 0.002) * alphaScale;
//		// HU가 낮아도 살려줌
//		//float fogAlpha = fog * stepSize * 6.0;
//
//		float fogAlpha = fog * max(stepSize, 0.003) * 25.0;  // 체감용
//		//float fogAlpha = fog * max(stepSize, 0.003) * 25.0;
//		//float fogAlpha = fog * 0.02;
//
//		acc.rgb += (1.0 - acc.a) * alpha * ca.rgb;
//		acc.a += (1.0 - acc.a) * alpha;
//
//		if (acc.a >= 0.95)
//			break;
//	}
//
//	//float currentViewZ = currentPosVS.z;   // ✅ view space
//		// 시각화
//	//return float4(
//	//	saturate(-meshViewZ / 500.0),
//	//	saturate(-meshViewZ / 500.0),
//	//	saturate(-meshViewZ / 500.0),
//	//	1
//	//	);
//
//	///* ===============================
//	//   Post
//	//=============================== */
//
//
//
//
//	float4 face = faceColor.SampleLevel(faceColorSamp, input.uv, 0);
//	//float4 face = faceColor.Sample(faceColorSamp, float2(0.5, 0.5));
//
//
//	// CT 구조 밝기
//	float ctLuma = dot(acc.rgb, float3(0.299, 0.587, 0.114));
//	// 얼굴 위에 CT 구조를 “더하기” (강도 튜닝)
//	float ctStrength = 0.45; // 0.25~0.7 튜닝
//	float3 outRgb = face.rgb + ctLuma * ctStrength;
//
//
//	//// 얼굴 알파(또는 face.a)로 최종 결정
//	//return float4(saturate(outRgb), 1.0);
//
//
//	//return float4(face.rgb, 1);
//
//
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//
//	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
//	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
//	acc.a *= 7;
//	return float4(acc.rgb, acc.a);
//	//return float4(acc.rgb, 1.0);
//}

// ============================================================
// Minimal Clean Volume Raymarch PS (Mesh-depth occlusion)
// - Stable under rotation
// - Depth compare in View-Z only
// - Optional: soft skin fade near mesh surface
// - Outputs: CT-only (rgb,a) or rgb-only
// ============================================================

//cbuffer CB : register(b0)
//{
//	matrix InvView;
//	matrix InvProj;
//	matrix InvVolumeWorld;
//
//	matrix View;
//	matrix Projection;
//
//	float4 CameraPosAndMode;   // xyz = camera pos WS, w = mode
//	float4 VoxelAndMaxSteps;   // xyz = volume dims (or voxel dims), w = maxSteps
//	float4 HuParams;           // z = HU min, w = HU max
//};
//
//Texture3D<float>   volumeTex        : register(t0);
//SamplerState       samp             : register(s0);
//
//Texture1D<float4>  transferFunction : register(t1);
//SamplerState       tfSampler        : register(s1);
//
//Texture2D<float>   SceneDepth       : register(t5);
//SamplerState       pointClamp       : register(s5);
//
//struct PSInput
//{
//	float4 pos : SV_POSITION;
//	float2 uv  : TEXCOORD0;
//};
//
//// -----------------------------
//// Depth01 -> ViewZ via InvProj
//// (D3D NDC y flip 고려)
//// -----------------------------
//float ReconstructViewZ(float2 uv, float depth01, matrix invProj)
//{
//	float2 ndc;
//	ndc.x = uv.x * 2.0f - 1.0f;
//	ndc.y = 1.0f - uv.y * 2.0f;          // D3D flip
//
//	float z_ndc = depth01 * 2.0f - 1.0f; // [0,1] -> [-1,1]
//
//	float4 clip = float4(ndc.x, ndc.y, z_ndc, 1.0f);
//	float4 view = mul(clip, invProj);
//	view /= max(view.w, 1e-6);
//
//	return view.z; // view-space z (D3D LH면 보통 +z forward, 네 코드 기준은 z가 음수로 가는 듯)
//}
//
//// -----------------------------
//// Ray-box intersection (AABB)
//// returns tNear/tFar in "volume local space"
//// -----------------------------
//bool IntersectBox(float3 rayPos, float3 rayDir, float3 bmin, float3 bmax, out float tNear, out float tFar)
//{
//	float3 invDir = 1.0 / (rayDir + 1e-6);
//	float3 t0 = (bmin - rayPos) * invDir;
//	float3 t1 = (bmax - rayPos) * invDir;
//
//	float3 tmin = min(t0, t1);
//	float3 tmax = max(t0, t1);
//
//	tNear = max(max(tmin.x, tmin.y), tmin.z);
//	tFar = min(min(tmax.x, tmax.y), tmax.z);
//
//	return (tFar >= max(tNear, 0.0));
//}
//
//float4 main(PSInput input) : SV_Target
//{
//	float2 uv = input.uv;
//
//	// =========================================================
//	// 1) Ray setup in View/World
//	// =========================================================
//	float2 ndc = uv * 2.0 - 1.0;
//	ndc.y = -ndc.y; // 너 기존 방식 유지 (아래 ViewZ 복원은 별도 flip 처리)
//
//	float4 farClip = float4(ndc, 1, 1);
//	float4 farVS4 = mul(farClip, InvProj);
//	farVS4 /= max(farVS4.w, 1e-6);
//
//	float3 rayDirVS = normalize(farVS4.xyz);
//
//	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosAndMode.xyz;
//
//	// Volume local
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// View-space ray origin
//	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
//
//	// =========================================================
//	// 2) Volume bounds + intersection
//	// =========================================================
//	// 너가 쓰던 박스 그대로
//	const float3 boxMin = float3(-1, -0.75, -0.75);
//	const float3 boxMax = float3(1,  0.75,  0.75);
//
//	float tNear, tFar;
//	if (!IntersectBox(rayPos, rayDir, boxMin, boxMax, tNear, tFar))
//		return float4(0,0,0,0);
//
//	tNear = max(tNear, 0.0);
//
//	int   maxSteps = (int)VoxelAndMaxSteps.w;
//	float stepSize = (tFar - tNear) / max(maxSteps, 1);
//
//	// =========================================================
//	// 3) Mesh depth (sample once) + mesh viewZ
//	// =========================================================
//	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
//	bool  hasMesh = (meshDepth01 < 0.9999);
//
//	// NOTE:
//	// - depth01 -> viewZ는 "View space z" 정의가 코드/프로젝션에 따라 부호가 달라질 수 있음.
//	// - 너는 rayViewZ < meshViewZ에서 continue가 잘 맞는다고 했으니 그 관계 유지.
//	float meshViewZ = hasMesh ? ReconstructViewZ(uv, meshDepth01, InvProj) : -1e9;
//
//	// =========================================================
//	// 4) Accumulation
//	// =========================================================
//	float4 acc = float4(0,0,0,0);
//
//	// Optional: small jitter (안정적이지만 디더링)
//	float jitter = frac(sin(dot(uv * 1000.0, float2(12.9898,78.233))) * 43758.5453);
//	float t0 = tNear + jitter * stepSize;
//
//	[loop]
//	for (int i = 0; i < maxSteps; ++i)
//	{
//		float t = t0 + (i + 0.5) * stepSize;
//
//		// View-space depth for this sample (depth compare only)
//		float3 posVS = rayPosVS + rayDirVS * t;
//		float  rayViewZ = posVS.z;
//
//		// =====================================================
//		// 4-A) Mesh occlusion (stable)
//		// =====================================================
//		// Hard block (필요 시)
//		//if (hasMesh && rayViewZ < meshViewZ)
//		//    continue;
//
//		// Soft skin fade near mesh surface (추천: 깜빡임 방지)
//		float alphaMul = 1.0;
//		if (hasMesh)
//		{
//			float distVS = rayViewZ - meshViewZ; // >0: mesh 뒤(네가 쓰던 정의 유지)
//			if (distVS < 0.0)
//			{
//				// mesh 앞은 차단
//				continue;
//			}
//
//			// 아주 얇은 피부 구간에서만 살짝 억제 (옵션)
//			// NOTE: 숫자는 "viewZ 단위"에 좌우됨. 너가 mm로 맞췄으면 1~3mm 권장.
//			const float skinDepthVS = 2.0; // 튜닝
//			float fade = saturate(distVS / skinDepthVS);
//			fade = fade * fade * (3.0 - 2.0 * fade); // smoothstep
//
//			// 최소 기여 보장 (완전 소실 방지)
//			fade = max(fade, 0.25);
//
//			alphaMul *= fade;
//		}
//
//		// =====================================================
//		// 4-B) Volume sample (uvw)
//		// =====================================================
//		float3 posVol = rayPos + rayDir * t;
//		float3 uvw = (posVol - boxMin) / (boxMax - boxMin);
//		uvw.y = 1.0 - uvw.y;
//
//		// ✅ 여기서 break하면 "중간에 갑자기 사라짐"이 나올 수 있음
//		//    그래서 최소 버전에서는 continue로 유지.
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			continue;
//
//		float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
//		float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);
//		if (col.a < 0.001)
//			continue;
//
//		// =====================================================
//		// 4-C) Optional: soft gradient lighting (very mild)
//		// =====================================================
//		// VoxelAndMaxSteps.xyz를 "volume resolution"로 쓰는 가정.
//		// 만약 xyz가 voxel dim이 아니라면 별도 cb로 VolumeDim 넣는 게 정석.
//		float3 eps = 1.0 / max(VoxelAndMaxSteps.xyz, 1.0.xxx);
//
//		float3 g;
//		g.x = volumeTex.SampleLevel(samp, uvw + float3(eps.x,0,0),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(eps.x,0,0),0).r;
//		g.y = volumeTex.SampleLevel(samp, uvw + float3(0,eps.y,0),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(0,eps.y,0),0).r;
//		g.z = volumeTex.SampleLevel(samp, uvw + float3(0,0,eps.z),0).r -
//			  volumeTex.SampleLevel(samp, uvw - float3(0,0,eps.z),0).r;
//
//		float3 N = normalize(g + 1e-6);
//		float3 L = normalize(-rayDir);          // view-aligned light
//		float  lambert = saturate(dot(N, L));
//		lambert = pow(lambert, 1.5);
//
//		// HU bone gate
//		float boneGate = saturate((hu - 600.0) / 1200.0);
//		boneGate = smoothstep(0.1, 0.6, boneGate);
//
//		// tiny lighting only on bone-ish
//		float lighting = lerp(0.96, 1.04, lambert);
//		lighting = lerp(1.0, lighting, boneGate * 0.35);
//		col.rgb *= lighting;
//
//		// edge tiny boost
//		float gradMag = length(g);
//		float edgeGate = saturate((gradMag - 0.02) / 0.08);
//		edgeGate = smoothstep(0.0, 1.0, edgeGate);
//		edgeGate *= boneGate;
//		col.rgb *= lerp(1.0, 1.08, edgeGate);
//
//		// =====================================================
//		// 4-D) Accumulate
//		// =====================================================
//		float alpha = col.a * stepSize * 12.0;   // 튜닝(너 기존 alphaScale 정리)
//		alpha *= alphaMul;
//
//		acc.rgb += (1.0 - acc.a) * alpha * col.rgb;
//		acc.a += (1.0 - acc.a) * alpha;
//
//		if (acc.a > 0.98)
//			break;
//	}
//
//	// gamma
//	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
//
//		if (CameraPosAndMode.w == 1.0) return float4(acc.rgb, 1.0);
//	if (CameraPosAndMode.w == 0.0) return float4(acc.rgb, 0.0);
//
//	// CT-only 결과는 "합성용"이면 alpha를 살려서 내보내는 게 좋음
//	// (그 다음 패스에서 faceColor와 blend)
//	return float4(acc.rgb, acc.a);
//}
//
