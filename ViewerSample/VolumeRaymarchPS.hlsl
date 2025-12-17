
cbuffer CB : register(b0)
{
	matrix InvView;
	matrix InvProj;
	matrix InvVolumeWorld;

	matrix View;           // ✅ 추가
	matrix Projection;     // ✅ 추가

	float4 CameraPosAndAlpha;  // xyz=pos, w=alpha
	float4 VoxelAndMaxSteps;   // xyz=voxel, w=maxSteps
	float4 HuParams;
};

Texture3D<float> volumeTex : register(t0);
SamplerState samp : register(s0);


// ⭐ Transfer Function 추가
Texture1D<float4> transferFunction : register(t1);
SamplerState tfSampler : register(s1);


// ========== 추가: Depth 텍스처 ==========
Texture2D<float> SceneDepth : register(t5);  // ← 메쉬 depth


struct PSInput
{
	float4 pos : SV_POSITION; // ⭐ 이게 화면 픽셀 좌표
	float2 uv       : TEXCOORD0;
};



float4 main(PSInput input) : SV_Target
{
	float2 offset = float2(0.0, 0.0);
	float2 scale = float2(0.5, 0.5);
	float2 localUV = (input.uv - offset) / scale;

	float2 screenUV = input.uv;

	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;

	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);
	viewDirVS /= viewDirVS.w;

	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
	float3 rayPosWS = CameraPosAndAlpha.xyz;


	//rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);



	// ✅ 현재 픽셀의 메쉬 depth 읽기
	float meshDepthNDC = SceneDepth.Load(int3(input.pos.xy, 0));
	//float depth01 = meshDepth;   // ⭐ 이 줄이 빠졌던 것
	//// NDC Z
	//float ndcZ = depth01 * 2.0f - 1.0f;

	//// NDC position
	//float4 clipPos = float4(ndc.x, ndc.y, ndcZ, 1.0f);

	//// View space
	//float4 meshViewPos = mul(clipPos, InvProj);
	//meshViewPos /= meshViewPos.w;

	//float meshDepthVS = meshViewPos.z;


	//////   // 디버그: mesh depth 시각화
	//return float4(meshDepthNDC, meshDepthNDC, meshDepthNDC, 1.0);





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


	float stepSize = travelDist / float(VoxelAndMaxSteps.w);

	// ✅ Jittering
	float jitter = frac(sin(dot(input.uv * 1000.0, float2(12.9898, 78.233))) * 43758.5453);
	// 시작점에 랜덤 오프셋
	float3 startPos = rayPos + rayDir * (tNear + jitter * stepSize);


	startPos = clamp(startPos, boxMin, boxMax);

	/*float3 currentPos = startPos;

	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);*/

	float4 acc = float4(0, 0, 0, 0);


	// ✅✅✅ View space ray 정보 (depth 계산용) ✅✅✅
	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;
	float3 rayDirVS = normalize(mul(float4(rayDirWS, 0), View).xyz);




	//int sampleCount = 0;



[loop]
for (int i = 0; i < VoxelAndMaxSteps.w; i++)
{

	//// ========== Depth 체크 ==========
	//	// 현재 ray 위치를 클립 공간으로 변환
	//float4 worldPos = float4(rayPos, 1.0f);
	//float4 viewPos = mul(worldPos, View);
	//float4 clipPos = mul(viewPos, Projection);

	////// Perspective divide
	////float currentDepth = clipPos.z / clipPos.w;


	//// Perspective divide
	//float currentDepth = viewPos.z;

	//// ✅ 메쉬보다 뒤에 있으면 중단
	//if (currentDepth > meshDepth)
	//{
	//	break;  // 더 이상 샘플링 안 함
	//}


	////return float4(meshDepth, meshDepth, meshDepth, 1);




	float3 currentPos = startPos + rayDir * (i * stepSize);
	float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

	uvw.y = 1.0 - uvw.y;  // ✅ 추가

	if (any(uvw < 0.0) || any(uvw > 1.0))
		break;



	// ✅✅✅ Depth 체크 (메쉬보다 뒤면 안 그림!) ✅✅✅
	float tCurrent = tNear + i * stepSize;
	float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
	float4 clipPos = mul(float4(currentPosVS, 1.0), Projection);
	float currentDepthNDC = clipPos.z / clipPos.w;  // NDC depth (0~1)


	// ✅ Depth margin 추가
	float depthMargin = 0.001;  // 약간의 여유

	//if (currentDepthNDC > meshDepthNDC) {
	if (currentDepthNDC > meshDepthNDC + depthMargin) {
		break;  // ✅ Volume이 mesh 뒤에 있으면 중단!
	}








	// 1) Raw 기반 density
	float raw = volumeTex.SampleLevel(samp, uvw, 0).r;


	//// 2) DICOM HU 로 변환
	float hu = raw/* * HuParams.x + HuParams.y*/;   // -1000 ~ 3000 같은 범위


	// 3) 윈도우/레벨 범위로 정규화 (0~1)
	float huNorm = (hu - HuParams.z) / (HuParams.w - HuParams.z);
	huNorm = saturate(huNorm);


	//float4 colorAlpha = transferFunction.Sample(tfSampler, huNorm);
	float4 colorAlpha = transferFunction.SampleLevel(tfSampler, huNorm, 0);

	// ⭐ Window로 알파만 조절 (조직 분리 유지)
	float huInWindow = (hu - HuParams.z) / (HuParams.w - HuParams.z);
	if (huInWindow < 0.0 || huInWindow > 1.0) {
		colorAlpha.a *= 0.05;  // Window 밖은 투명하게
	}


	if (colorAlpha.a < 0.001)
		continue;


	// 조명 계산
	float3 eps = float3(1.0 / VoxelAndMaxSteps.x, 1.0 / VoxelAndMaxSteps.y, 1.0 / VoxelAndMaxSteps.z);

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

	// 후처리 (간단하게!)
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);
	//acc.a = 0.0;

	if(CameraPosAndAlpha.w==1.0)
		return float4(acc.rgb, 1.0);
	else if(CameraPosAndAlpha.w == 0.0)
		return float4(acc.rgb, 0.0);    // ← RGB는 같지만 alpha=0 (투명)
	else
		return float4(acc.rgb, acc.a);
}

