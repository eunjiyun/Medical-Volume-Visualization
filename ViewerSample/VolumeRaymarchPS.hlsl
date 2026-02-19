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
SamplerState pointClamp : register(s5); // 포인트/클램프  (디버그용)
//SamplerState MeshViewZSamp : register(s2); // 포인트/클램프  (디버그용)

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
//픽셀 셰이더 entry
//화면 픽셀 하나당 1번 실행
//반환값 = 최종 픽셀 색
// : SV_Target는 Semantic을 지정하는 부분이고, 반환되는 값이 렌더 타겟
//(최종 출력 프레임 버퍼의 색상 값)에 쓰인다는 의미
//SV_Target0 첫 번째 렌더 타겟에 출력
//SV_Target1 두 번째 렌더 타겟에 출력(MRT, Multiple Render Targets)

//Direct3D의 셰이더 모델에서는 단순히 float4를 반환한다는 사실만으로는 
//그 값이 어디로 가야하는지 알 수가 없음.
//Sementic을 붙여서 GPU에 이 값은 최종 픽셀 색상으로 쓰라고 알려줌.
//그래서  : SV_Target는 픽셀 셰이더에서 출력이 화면에 그려질 색상 값임을
//명시하는 역할
{
	// TL viewport 기준 uv → 전체 화면 depth uv
	float2 uvFull;
	uvFull.x = input.uv.x * 0.5;
	uvFull.y = input.uv.y * 0.5;

	float2 uv = input.uv;

	//<픽셀 -> 레이 생성>
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



	//<레이를 실제 씬으로 이동>
	//실제 CT 볼륨 탐험 가능 상태
	// 3. view -> world
	//월드 공간 레이 시작점
	float3 rayPosWS = mul(float4(rayOriginVS, 1), InvView).xyz;
	//월드 공간 레이 방향
	float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
	//카메라 기준 좌표 => 실제 월드 좌표
	



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


	//<레이 vs 볼륨 박스 교차 계산>
	/*레이가 볼륨에
		언제 들어가고
		언제 나오는지 계산*/
	/* 볼륨 밖은 탐색할 필요 없음
	=>	속도 핵심 최적화*/
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
	//레이가 볼륨에 언제 들어가고 언제 나오는지 계산
	//볼륨 밖은 탐색할 필요 없음
	//속도 핵심 최적화





	//<레이마칭 준비>
	/*탐험 시작점
		탐험 끝점
		걸음 크기*/
	/*어디서부터 어디까지
		얼마씩 이동할지*/
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
	//탐험 시작점, 탐험 끝점, 걸음 크기


	/* ---------------------------
	   Mesh depth (screen-space) -> view Z
	--------------------------- */
	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uvFull, 0);

	int2 pixelCoord = int2(floor(input.pos.xy));

	//  전체 화면 기준 UV 계산
	float2 screenUV;
	screenUV.x = pixelCoord.x / ViewSize.x;  // ViewSize는 CB로 전달 (1276)
	screenUV.y = pixelCoord.y / ViewSize.y;  // (728)


	float2 meshUV;
	meshUV.x = screenUV.x * 2.0;
	meshUV.y = screenUV.y * 2.0;


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

	//레이마칭 루프 (핵심)
	/*for (i)
	{
		posWS = ray 따라 이동
			posL 변환
			uvw 계산
			volume 샘플
	}*/
	//공간 속을 한 걸음씩 걸어감
	//카메라에서 쏜 레이를 따라 공간을 조금씩 전진하면서
	//그 위치의 CT 값을 읽는 과정
	[loop]
	for (int i = 0; i < (int)maxSteps; ++i)
	{
		//레이 따라 이동
		//현재 샘플 위치 계산
		float tW = tStartW + (i)* stepW;
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


		//볼륨 로컬로 변환
		//볼륨 텍스처는 자기 기준 좌표계로 저장됨
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
		//mm 좌표 → 0~1 텍스처 좌표
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



		   //색 누적 / hit 탐지
		 /*  hu -> transfer function -> col
			   accumulate*/


		   float hu = volumeTex.SampleLevel(samp, uvw, 0).r;
		   float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));
		   float4 col = transferFunction.SampleLevel(tfSampler, huNorm, 0);

		   if (col.a < 0.001)
			   continue;

		   //메쉬 전용 패스에서 미리 계산해 둔 View Space Z 텍스처
		   //이 픽셀에서 메쉬 표면이 
		   //카메라로부터 얼마나 떨어져 있는지
		   float meshViewZ = MeshViewZTex.Load(int3(pixelCoord, 0));

		   //볼륨에서 hit 지점 찾는 부분
		   //활성화된 볼륨 투명도 기준으로 충돌 지점 설정
		   //if (!hasHit && col.a > 0.001)



		   //hu 300 이상(해면골) 기준으로 충돌 지점 설정
		   if (!hasHit && hu > 300)
		   {
		     // hitViewZ 계산
		     //볼륨 hit 지점의 View Z 계산
			 float4 posView = mul(float4(posWS, 1), View);
		     posView /= max(abs(posView.w), 1e-6);

			  //view space 변환
			  //정규화
			  float hitViewZ = posView.z;

		      // 깊이 차이
			  if (meshDepth01 > 0.001f && meshDepth01 < 0.9999f)  // 메시 있음
				 {
					   //최종 깊이 변화량 계산
					   //카메라 좌표계에서 피부-뼈 두께를 직접 측정하는
					   //의료 계측식
					   float deltaZ = abs(meshViewZ - hitViewZ);

					   //UAV : GPU 계산 결과를 CPU에서 읽을 수 있는 리소스
					   //DeltaZTex가 UAV로 사용됨.

					   //메쉬와 볼륨 간 깊이 차이는 
					   //얼굴 전체 픽셀에 대해 계산해야 하므로 
					   //GPU에서 수행
					   //이후 UAV에 저장된 깊이 차이 값을 CPU에서 통계 계산하는 순서
					   
					   //메쉬 <-> 볼륨 거리 기록
					   DeltaZTex[pixelCoord] = deltaZ;
				 }
				 else
				 {
					   DeltaZTex[pixelCoord] = -1.0f;    // 메시 없음 표시
				 }

				 hasHit = true;
		   }


			 float densityScale = 0.02;

			 float sigma = col.a * densityScale; // densityScale ≈ 0.02 ~ 0.05
			 float alpha = 1.0 - exp(-sigma * stepW);

			 //볼륨 렌더링에서의 front-to-Back Alpha Compositing
			 //색 누적 / 광학 적분 근사
			 //색 물리적 누적
			 acc.rgb += (1.0 - acc.a) * alpha * col.rgb;

			 //누적 투명도
			 acc.a += (1.0 - acc.a) * alpha;


			 //레이마칭 속도 최적화
			 //볼륨이 거의 불투명해졌으면 뒤 볼 필요 없음
			 //레이마칭 루프를 끝까지 돌면 모든 샘플을 계산해야하는데
			 //중간에 누적하다가 불투명해지는 순간에는 뒤쪽은 어차피 안 보이니까
			 //계산을 생략함
			 //이 처리로 인해 불필요한 연산을 줄여 성능을 향상시킴
			 //밀도가 높은 볼륨에서 특히 효과가 큼.
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

	//감마 보정
	//모니터 표시 보정
	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);


	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);


	//return float4(acc.rgb, 0.6);
	return float4(acc.rgb, 1.0);
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


// 볼륨 레이마칭은 
// 화면의 픽셀마다 3D 공간 속을 탐색해서
// 그 픽셀의 색을 결정하는 과정

// 픽셀 => 레이 생성 => 볼륨 진입 구간 찾기 => 레이 이동
//=> 볼륨 샘플링 => 색 누적 => 픽셀 출력

// 화면 픽셀 위치가
// 카메라 공간 => 월드 공간 => 볼륨 로컬 => 텍스처 좌표
// 이렇게 계속 변환됨

//Screen
//→ NDC
//→ View
//→ World
//→ Volume Local
//→ UVW


//Screen Space
//input.uv
//화면 픽셀 위치 0~1
//그냥 모니터 위치, 3D 정보 없음

//NDC (정규화 화면)
//ndc = uv*2 - 1
//0~1 => -1~1
//GPU 표준 카메라 평면 좌표
//카메라 앞 평면의 위치, 아직 3D 아님

//View Space (카메라 공간)
//rayOriginVS = mul(ndc, InvProj)
//픽셀이 카메라 기준에서 어느 방향인지 계산
//카메라 = (0,0,0)
//이제 레이 시작점 생김

//World Space
//rayPosWS = mul(rayOriginVS, InvView)
//rayDirWS = mul(rayDirVS, InvView)
//카메라 기준 => 실제 씬 기준
//이제 레이가 탐험이 가능한 상태
//볼륨, 메쉬, 모든 오브젝트 다 월드에 있음.

//Volume Local Space
//posL = mul(posWS, InvVolumeWorld)
//씬 좌표 => 볼륨 자체 좌표
//이 단계가 필요한 이유는 볼륨 텍스처는 자기 기준 좌표로 저장돼서.
//월드 => 오브젝트 내부 좌표

//UAV Texture Space
//uvw = normalize(boxMinL ~ boxMaxL)
//실제 길이(mm) => 0~1
//텍스처 샘플링 좌표
//GPU가 읽을 수 있음

//Texture Sampling
//volumeTex.Sample(uvw)
//실제 CT 데이터 읽음


//[픽셀 위치]
//Screen
//↓
//NDC
//↓
//카메라 기준 위치
//View Space
//↓
//씬 기준 위치
//World Space
//↓
//볼륨 내부 좌표
//Local Space
//↓
//텍스처 좌표
//UVW
//↓
//CT 값 읽기

//그래픽스에서 보통 흐름은
//월드=>뷰=>프로젝션=>스크린 
//이렇게 앞으로 가는데 

//레이마칭은 반대로
//screen에서 시작해서 거꾸로 올라감
//스크린=>프로젝션의 역행렬=>뷰의 역행렬=>월드

//프로젝션 행렬은 
//projection은 카메라 렌즈 역할
//3D 공간 => 2D 평면
//멀리 있으면 작아짐, 원근 적용

//프로젝션 역행렬은
//화면 평면 => 3D 방향 복원
//2D 위치에서 그 픽셀이 어디 방향인지 계산
//rayOriginVS = mul(float4(ndc,0,1), InvProj);
//카메라 평면 위치 => 카메라 기준 공간 위치
//레이 방향 생성

//Projection : 카메라가 찍음 : 3D방향=>화면
//InvProj : 사진 보고 광선 역추적 : 화면=>3D방향

//View 행렬
//이건 카메라 위치/회전 반영
//월드 => 카메라 기준 변환
//씬 좌표 => 카메라 좌표

//InvView 행렬
//카메라 기준 => 실제 씬 좌표
//카메라에서 본 방향 => 실제 공간 방향
//카메라 기준 레이를 실제 월드 공간 레이로 변환

//뷰 행렬은 세상을 카메라 기준으로 재배치
//뷰 역행렬은 카메라 기준 좌표를 세상 좌표로 되돌림

//<레이 생성 전체 흐름>
//화면 픽셀(uv) => NDC(ndc) => InvProj(픽셀->카메라 기준 방향)
//=>InvView(카메라 기준->실제 공간 방향) : rayDirWS


//🎥 렌더링 엔진
//
//raymarch
//
//transfer function
//
//accumulation


//📏 분석 엔진
//
//HU 기반 hit 탐지
//
//mesh depth 비교
//
//DeltaZ 저장


//> 픽셀마다 광선을 쏴서
//> 볼륨을 탐색하며 색을 만들고
//> 동시에 구조 위치를 측정한다