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


cbuffer DebugCB : register(b1)
{
	float2 ViewSize; // (width, height)
	float2 pad;
};



Texture3D<float> volumeTex        : register(t0);
SamplerState     samp             : register(s0);

Texture1D<float4> transferFunction : register(t1);
SamplerState      tfSampler        : register(s1);

// mesh depth (0~1)
Texture2D<float> SceneDepth : register(t2);
Texture2D<float> MeshViewZTex  : register(t4);


// mesh depth (0~1)
SamplerState pointClamp : register(s5); // 포인트+클램프 추천 (디버그용)
//SamplerState MeshViewZSamp : register(s2); // 포인트+클램프 추천 (디버그용)

RWTexture2D<float> DeltaZTex : register(u1);
Texture2D<float> DebugTex : register(t3);

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


// NOTE:
// deltaZ is written directly from raymarching PS for quick validation.
// This is NOT final architecture.
// Should be refactored to MRT + post pass (or CS) later.


//자동 스케일 계산
//
//릴리즈
//
//수치 비교 기능



float4 main(PSInput input) : SV_Target
{
	// TL viewport 기준 uv → 전체 화면 depth uv
	float2 uvFull;
	uvFull.x = input.uv.x * 0.5;
	uvFull.y = input.uv.y * 0.5;


	//SampleLevel(pointClamp, uvFull, 0);


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


	//로컬 공간에서의 교차
	// Transform ray into volume-local space for intersection ONLY
	float3 rayPosL = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;

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

	

	//screenUV = pixelCoord / ViewSize

	/* ---------------------------
	   Mesh depth (screen-space) -> view Z
	--------------------------- */
	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uvFull, 0);
	/*float d = meshDepth01;
	return float4(d, d, d, 1);
	float meshViewZ = d;*/
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

	int2 pixelCoord = int2(floor(input.pos.xy));

		// ✅ 전체 화면 기준 UV 계산
	float2 screenUV;
	screenUV.x = pixelCoord.x / ViewSize.x;  // ViewSize는 CB로 전달 (1276)
	screenUV.y = pixelCoord.y / ViewSize.y;  // (728)

	//float d = SceneDepth.SampleLevel(pointClamp, screenUV, 0);
	//return float4(d, d, d, 1);

	float2 meshUV;
	meshUV.x = screenUV.x * 2.0;
	meshUV.y = screenUV.y * 2.0;


	//return float4(SceneDepth.Load(int3(10, 10, 0)).xxx, 1);

	//float d = SceneDepth.SampleLevel(pointClamp, uvFull, 0);
	//return float4(d, d, d, 1);





	////	  // ✅ SceneDepth 샘플링
	////float depth01 = SceneDepth.SampleLevel(pointClamp, screenUV, 0);

	//// clip space
	//float z_ndc = meshDepth01 * 2.0f - 1.0f;
	//float4 clip = float4(
	//	screenUV.x * 2 - 1,
	//	1 - screenUV.y * 2,
	//	z_ndc,
	//	1
	//	);

	//// view space
	//float4 view = mul(clip, InvProj);
	//view /= view.w;

	//float meshViewZ = view.z;

	//// 🔴 디버그 출력
	//return float4(abs(meshViewZ) / 500.0, 0, 0, 1);









	//float meshViewZ = SceneDepth.SampleLevel(pointClamp, input.uv, 0);

	//return float4(abs(meshViewZ) / 700, 0, 0, 1);


	//float d01_half = SceneDepth.SampleLevel(pointClamp, input.uv * 0.5, 0);

	// uvFull 없이 직접 샘플링
	//float d01_full = SceneDepth.SampleLevel(pointClamp, input.uv, 0);


	//bool hasMesh = (meshViewZ < 1e8); // background 제외


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
		float tW = tStartW + (i /*+ jitter*/)* stepW;

		// World position along ray
		float3 posWS = rayPosWS + rayDirWS * tW;

		// View Z for mesh-occlusion compare (now consistent!)
		float rayViewZ = mul(float4(posWS, 1), View).z;

		// depth block (tolerance in VIEW units; use a small constant or scale by step)
		// stepW is world units; convert rough tolerance to view by multiplying by |rayDirVS.z|
		float stepV = abs(stepW * rayDirVS.z);
		//float depthDiff = rayViewZ - meshViewZ;

		//if (hasMesh && depthDiff < -stepV * 2.0)
		//	continue;

		// Transform sample position to volume-local for texture lookup
		float3 posL = mul(float4(posWS, 1), InvVolumeWorld).xyz;


	 //  float4 colorMap = float4(0, 0, 0,1);
	 //  if (posL.x >= 0)
		//   colorMap = float4(1, 0, 0,1);           // 빨간색
	 //  else
		//   colorMap = float4(0, 1, 1,1);           // 빨간색의 보수 (시안색)
	 //  if (posL.y >= 0)
		//   colorMap = float4(0, 1, 0,1);           // 초록색
	 //  else
		//   colorMap = float4(1, 0, 1,1);           // 초록색의 보수 (마젠타)
	 //  if (posL.z >= 0)
		//   colorMap = float4(0, 0, 1,1);           // 파란색
	 //  else
		//   colorMap = float4(1, 1, 0,1);           // 파란색의 보수 (노란색)

	 //  return float4(colorMap);





		//return float4(abs(posL) * 0.01, 1);

		// Local -> UVW
		float3 uvw = (posL - boxMinL) / (boxMaxL - boxMinL);
		uvw.y = 1.0 - uvw.y;


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


				


//if (!hasHit && col.a > 0.001)
//{
//	// hitViewZ 계산
//	float4 posView = mul(float4(posWS, 1), View);
//	posView /= max(abs(posView.w), 1e-6);
//	float hitViewZ = posView.z;
//
//	// meshViewZ 읽기
//	//float meshViewZ = SceneDepth.SampleLevel(pointClamp, input.uv, 0);
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
//	//float nearZ = 205.4f;          // 카메라 바로 앞
//	//float farZ = 794.6f; // 볼륨 깊이 + 여유
//
//	//float meshViewZ = ReconstructViewZ_InvProj(depth01,-1000,1000);
//	//float meshViewZ = ReconstructViewZ_InvProj(depth01, nearZ, farZ);
//
//	
//
//	
//	// ✅ 깊이 차이
//	if (meshDepth01 > 0.001f && meshDepth01 < 0.9999f)  // 메시 있음
//	{
//		float deltaZ = abs(meshViewZ - hitViewZ);
//		
//		//if (deltaZ < 500.0f)  // 50cm 이상 차이는 무시
//		{
//			DeltaZTex[pixelCoord] = hitViewZ;
//		}
//		//else
//		//{
//		//	DeltaZTex[pixelCoord] = -1.0f;  // 이상치
//		//}
//
//	}
//	else
//	{
//		DeltaZTex[pixelCoord] = -1.0f;    // 메시 없음 표시
//
//	}
//
//	hasHit = true;
//
//}
//else
//{
//	int2 pixelCoord = int2(floor(input.pos.xy));
//	DeltaZTex[pixelCoord] = -1.0f;  // 메시 없음
//}


				//half viewport, uvFull 보정
				//depth copy 구조 보정, MRT/UAV 혼용
				//이 상태에서 역투영은 비추천


				////Depth 값(NDC 변환)
				//float z_ndc = meshDepth01 * 2.0f - 1.0f;


				////Clip Space 좌표 구성
				//float4 clip = float4(
				//	screenUV.x * 2.0f - 1.0f,
				//	1.0f - screenUV.y * 2.0f, // D3D Y flip
				//	z_ndc,
				//	1.0f
				//	);


				////View Space로 역투영
				//float4 view = mul(clip, InvProj);
				//view /= view.w;

				////View Space Z 추출 
				//float meshViewZ = view.z;


				float meshViewZ = MeshViewZTex.Load(int3(pixelCoord, 0));

		// meshViewZ = mul(float4(meshPosWS, 1), View).z;


				//// meshViewZ만 보기
				//float viz = saturate(abs(meshViewZ) / 500.0);
				//return float4(viz, viz, viz, 1);


			
				
				if (!hasHit && col.a > 0.001)
				//if (!hasHit && hu > 300)
				{
				
					// hitViewZ 계산
					float4 posView = mul(float4(posWS, 1), View);
					posView /= max(abs(posView.w), 1e-6);

					//view space 변환
					//정규화

					//
					float hitViewZ = posView.z;

					// meshViewZ 읽기
					//float meshViewZ = SceneDepth.SampleLevel(pointClamp, input.uv, 0);



			/*		float viz = saturate(abs(hitViewZ) / 500.0);
					return float4(viz, 0, 0, 1);*/

					//float nearZ = 205.4f;          // 카메라 바로 앞
					//float farZ = 794.6f; // 볼륨 깊이 + 여유

					//float meshViewZ = ReconstructViewZ_InvProj(depth01,-1000,1000);
					//float meshViewZ = ReconstructViewZ_InvProj(depth01, nearZ, farZ);




					// ✅ 깊이 차이
					if (meshDepth01 > 0.001f && meshDepth01 < 0.9999f)  // 메시 있음
					{
						float deltaZ = abs(meshViewZ - hitViewZ);

						//if (deltaZ < 500.0f)  // 50cm 이상 차이는 무시
						{
							DeltaZTex[pixelCoord] = deltaZ;
						}
						//else
						//{
						//	DeltaZTex[pixelCoord] = -1.0f;  // 이상치
						//}

					}
					//else
					//{
					//	DeltaZTex[pixelCoord] = -1.0f;    // 메시 없음 표시
					//}

					hasHit = true;

				}
				//else
				//{
				//	//여기가 주로 row 값 실패값으로 출력됨.
				//	int2 pixelCoord = int2(floor(input.pos.xy));
				//	DeltaZTex[pixelCoord] = -1.0f;  // 메시 없음
			
				//}



				float densityScale = 0.02;

				float sigma = col.a * densityScale; // densityScale ≈ 0.02 ~ 0.05
				float alpha = 1.0 - exp(-sigma * stepW);


				acc.rgb += (1.0 - acc.a) * alpha * col.rgb;
				acc.a += (1.0 - acc.a) * alpha;

				if (acc.a > 0.98)
					break;


				posL = mul(float4(posWS, 1), InvVolumeWorld).xyz;

				//float4 colorMap = float4(0, 0, 0, 1);

				//if (posL.z >= 0)
				//	acc.rgb = float3((0, 0, 1);// 파란색
				//else
				//	acc.rgb = float3((1, 1, 0);// 파란색의 보수 (노란색)
				//if (posL.y >= 0)
				//	acc.rgb = float3(0, 1, 0);// 초록색
				//else
				//	acc.rgb = float3((1, 0, 1);// 초록색의 보수 (마젠타)

				//if (posL.x >= 0)
				//	colorMap.rgb = float3(1, 0, 0);// 빨간색
				//else
				//	colorMap.rgb = float3(0, 1, 1);// 빨간색의 보수 (시안색)
				//return float4(colorMap);

			}


			// 루프 끝나고
			if (!hasHit)
			{
				DeltaZTex[pixelCoord] = -1;
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



			//// 마지막에 디버그 오버레이 (선택)
			//if (meshDepth01 > 0.001f && meshDepth01 < 0.9999f)
			//{
			//	float v = DebugTex.Sample(pointClamp, uvFull, 0);
			//	acc.rgb = lerp(acc.rgb, float3(v, v, v), 0.3);  // 반투명 오버레이
			//}

		/*	float v = DebugTex.Sample(pointClamp, uvFull, 0);
			return float4(v, v, 0, 1);
*/



			

			if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
			if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);

			return float4(acc.rgb, 0.6);
}




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




