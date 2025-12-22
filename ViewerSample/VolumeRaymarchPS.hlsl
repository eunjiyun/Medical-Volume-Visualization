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
Texture2D<float> faceColor : register(t6);


// mesh depth (0~1)
SamplerState pointClamp : register(s5); // 포인트+클램프 추천 (디버그용)
SamplerState faceColorSamp : register(s6); // 포인트+클램프 추천 (디버그용)



struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};


// depth01 : 0~1 depth buffer value
// proj    : Projection matrix (same one used for rendering)
float ReconstructViewZ_InvProj(float2 uv, float depth01, matrix proj)
{
	//float2 uv = input.uv;

	//uv.x *= 0.5;
	//uv.y *= 0.5;

	float2 ndc = uv * 2.0f - 1.0f;
	//ndc.y = -ndc.y;
	ndc.y = 1.0f - uv.y * 2.0f; // D3D flip

	float z_ndc = depth01 * 2 - 1;   // ❗ 반드시 필요

	float4 clip = float4(ndc.x, ndc.y, z_ndc, 1.0f);
	float4 view = mul(clip, InvProj);   // 너가 row-vector 스타일이면 mul(v, M) 유지
	view /= max(view.w, 1e-6);

	return view.z; // view-space z
}

float4 main(PSInput input) : SV_Target
{
	/* ===============================
	   Screen → Ray setup
	=============================== */

	//return float4(1, 0, 1, 1); // 자홍

	float2 screenUV = input.uv;

	float2 uv = input.uv;

	// 예: 좌상단 쿼드만 depth가 있을 경우
	uv.x *= 0.5;
	uv.y *= 0.5;







	float2 ndc = screenUV * 2.0 - 1.0;
	ndc.y = -ndc.y;






	float4 ndcPos = float4(ndc, 1, 1);
	float4 viewDirVS = mul(ndcPos, InvProj);

	float4 farClip = float4(ndc, 1, 1);
	float4 farVS = mul(farClip, InvProj);
	//viewDirVS /= viewDirVS.w;
	farVS /= max(farVS.w, 1e-6);

	float3 rayDirVS = normalize(farVS.xyz); // view space 방향

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




	float d = SceneDepth.SampleLevel(pointClamp, uv, 0);


	float z = ReconstructViewZ_InvProj(uv, d, InvProj);

	//// 범위 넉넉하게
	//float v = saturate(-z / 1000.0);
	//return float4(v, v, v, 1);


	/* ===============================
	   Mesh depth (0~1)
	=============================== */

	//float2 pixel = input.pos.xy;   // SV_POSITION (screen pixel)
	//float meshDepth01 = SceneDepth.Load(int3(pixel, 0));


	//float meshDepth01 = SceneDepth.Load(int3(input.pos.xy, 0));
	//return float4(saturate((1 - meshDepth01) * 50), 0, 0, 1);
	//return float4(1 - meshDepth01, 1 - meshDepth01, 1 - meshDepth01, 1);


	//float d = SceneDepth.SampleLevel(pointClamp, uv, 0);
	//return float4(d, d, d, 1);

	float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0);
	//return float4(meshDepth01, meshDepth01, meshDepth01, 1);








	//if (meshDepth01 >= 0.9999)
	//	return float4(0, 0, 0, 1);   // 배경


	//float g = frac((-z) * 0.01); // 줄무늬로 변화 보이게
	//return float4(g, g, g, 1);

	//// 단순 시각화 (회색 얼굴 나와야 정상)
	//return float4(
	//	saturate(-meshViewZ / 500.0),
	//	saturate(-meshViewZ / 500.0),
	//	saturate(-meshViewZ / 500.0),
	//	1
	//	);

	//return float4(meshDepth01, meshDepth01, meshDepth01, 1);

	// 의료용 연출 파라미터
	float skinBias01 = 0.003;   // 피부 두께
	float fadeWidth01 = 0.012;   // 경계 soft width



	//meshDepth01 -= skinBias01;

	/* ===============================
	   View-space ray (depth compare)
	=============================== */


	float3 rayPosVS = mul(float4(rayPosWS, 1), View).xyz;


	// View space 강제 Ray (무조건 앞)
	farClip = float4(ndc, 1.0, 1.0);
	farVS = mul(farClip, InvProj);
	farVS /= max(farVS.w, 1e-6);
	//float3 rayDirVS = normalize(farVS.xyz);



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
		float3 currentPosVS = rayPosVS + rayDirVS * tCurrent;
		//float rayViewZ = currentPosVS.z;

		//if (hasMesh && rayViewZ < meshViewZ)
		//	continue; // mesh 앞이면 차단



		float3 currentPos = startPos + rayDir * (i * stepSize);
		// 2) ray depth
		//float3 currentPosVS = rayDirVS * tCurrent;

		float4 clipPos = mul(float4(currentPosVS, 1), Projection);
		float rayDepth01 = clipPos.z / clipPos.w * 0.5 + 0.5;

		// 3) 비교 결과 시각화
		// R = ray가 mesh 앞 (❌)
		// G = 거의 동일 (표면)
		// B = ray가 mesh 뒤 (✅)
	//	float diff = rayDepth01 - meshDepth01;

		/*return float4(
			diff < 0 ? 1 : 0,
			abs(diff) < 0.002 ? 1 : 0,
			diff > 0 ? 1 : 0,
			1
			);*/
		bool hasMesh = (meshDepth01 < 0.9999);  // 또는 1.0에 가깝지 않으면

		float rayViewZ = currentPosVS.z;        // z < 0
		float meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;

		// ray가 mesh 뒤?
		bool behind = rayViewZ < meshViewZ;  // (더 음수면 더 앞/뒤 상황에 따라 조정)


		float diff = rayViewZ - meshViewZ;

		/*float*/ rayDepth01 = 1.0 - (clipPos.z / clipPos.w * 0.5 + 0.5);

		/*	return float4(
				diff < 0 ? 1 : 0,
				abs(diff) < 1.0 ? 1 : 0,
				diff > 0 ? 1 : 0,
				1
				);
	*/






			float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
			uvw.y = 1.0 - uvw.y;

			if (any(uvw < 0.0) || any(uvw > 1.0))
				break;

			/* ---------- depth compare ---------- */

			float meshDepth01 = SceneDepth.SampleLevel(pointClamp, uv, 0); // ✅ 이걸 meshDepth01로 사용




			/*float*/ //meshViewZ = hasMesh ? ReconstructViewZ_InvProj(uv, meshDepth01, InvProj) : 1e9;


			//if (hasMesh && rayViewZ < meshViewZ)
			//{
			//	// 메쉬 앞: CT 완전 차단
			//	continue;
			//}

			// View-space Z 복원

			//return float4(meshDepth01, meshDepth01, meshDepth01, 1);

			//// view space Z 시각화 (스케일링해서)
			//return float4(saturate(-meshViewZ / 500.0), 0, 0, 1);





			currentPosVS = rayPosVS + rayDirVS * tCurrent;

			/*	return float4(
					saturate(-currentPosVS.z / 500.0),
					0,
					0,
					1
					);*/


					//float4 clipPos = mul(float4(currentPosVS, 1), Projection);

					//float rayDepth01 = clipPos.z / clipPos.w * 0.5 + 0.5;

					//return float4(rayDepth01, rayDepth01, rayDepth01, 1);


					float d = clipPos.z / clipPos.w;     // ✅ D3D
					//return float4(d, d, d, 1);


					float currentDepth01 = (clipPos.z / clipPos.w) * 0.5 + 0.5;

					//return float4(currentDepth01, currentDepth01, currentDepth01, 1);

					//float d = meshDepth01 - currentDepth01;

					//float dist = currentDepth01 - meshDepth01;


					//// 메쉬 기준 거리 (mm 스케일 유지됨)
					//float distVS = meshViewZ - currentViewZ;

					//float skinDepth = 0.01; // 0.005~0.02 튜닝
					//float fade = saturate(dist / skinDepth);
					//fade = pow(fade, 2.0); // ★ 핵심

					//if (d >0)
					//	discard; // 메쉬 앞

					//bool hasMesh = (meshDepth01 < 0.9999);  // 또는 1.0에 가깝지 않으면


					//if (!hasMesh) return float4(0, 0, 0, 1); // 배경은 검정으로
					//return float4(saturate(meshDepth01), saturate(meshDepth01), saturate(meshDepth01), 1);


					float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
					float hu = raw;

					float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));



					float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);

					// HU 기준 치아 영역
					bool isTooth = (hu > 2000.5);

					// 치아는 알파 상한 제한
					if (isTooth)
					{
						ca.a = min(ca.a, 0.15);   // ⭐ 핵심
					}




					float dist = currentDepth01 - meshDepth01;
					float distVS = rayViewZ - meshViewZ;   // > 0 이면 메쉬 뒤

					float hardBlock = -1.0;   // 1mm 앞까지만 완전 차단

				/*	if (hasMesh && distVS < hardBlock)
						continue;
			*/

			//if (hasMesh)
			//{
			//	

			//	// 메쉬 앞이면 CT 절대 금지
			//	if (dist < 0.0)
			//		continue;

				// 2) skin zone fade
				//float skinDepth = 0.01;        // 0.005~0.02

				// // 2) 피부 구간
				//float skinDepth = 0.02;
				//float fade = smoothstep(0.0, skinDepth, dist);

			bool isBone = (hu > 700.0 && hu < 1800.0);


			float boneRecovery = 1.0;

			//if (isBone)
			//{
			//	// 얼굴 바로 뒤 bone은 억제
			//	if (hasMesh)
			//	{
			//		float d = saturate(distVS / 4.0);   // 0~4mm
			//		boneRecovery = d;                   // 앞면 bone 얇게
			//	}

			//	// ⭐ 깊어질수록 bone 다시 살리기
			//	float depthBoost = saturate(distVS / 20.0); // 0~20mm
			//	boneRecovery = max(boneRecovery, depthBoost);
			//}




				float skinDepthVS = 1.5;   // 5mm
				float boneFogStart = 6.0;  // 뼈 안개 시작


				float skinFade = saturate(distVS / skinDepthVS);

				//// 부드럽게
				skinFade = skinFade * skinFade; // 또는 smoothstep


	// CT를 빨리 살리기
				skinFade = pow(skinFade, 0.5);    // ⭐ 핵심 (기존 fade*fade 반대)

				//// 3) 알파 억제 + 피부색 중화
				//ca.a *= fade;

				float3 skinTint = float3(0.78, 0.62, 0.55);

				ca.rgb = lerp(skinTint, ca.rgb, skinFade);
				ca.a *= skinFade;
				//}

				//return float4(saturate(dist * 50), 0, 0, 1);





				//// 🔥 soft clamp
				//float occ = saturate(d / fadeWidth01);

				//// 완전 차단 금지
				//occ = max(occ, 0.15);

				////if (occ <= 0.0)
				////	break;

				//occ = saturate(occ + 0.05); // 최소 보장

				/* ---------- sample volume ---------- */

				//float raw = volumeTex.SampleLevel(samp, uvw, 0).r;
				//float hu = raw;

				//float huNorm = saturate((hu - HuParams.z) / (HuParams.w - HuParams.z));

				if (CameraPosAndAlpha.w == 2.0 && hu < 400)
					continue;

				//float4 ca = transferFunction.SampleLevel(tfSampler, huNorm, 0);
				if (ca.a < 0.001)
					continue;

				//ca.a *= occ;

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
				float spec = pow(max(dot(N,H),0.0), 48.0) /** occ*/;

				float lighting = (CameraPosAndAlpha.w == 2.0)
								 ? (0.92 + lambert * 0.08)
								 : (0.88 + lambert * 0.12);

				ca.rgb *= lighting;
				ca.rgb += spec * float3(0.06,0.05,0.04);



				//float3 skinTint = float3(0.78, 0.62, 0.55); // 임시값

		// 메쉬 바로 뒤에서는 피부색, 안쪽으로 갈수록 CT
				//ca.rgb = lerp(skinTint, ca.rgb, fade);



				/* ---------- accumulate ---------- */

				//float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 4.5 : 8.0;
				//float alpha = ca.a * stepSize * alphaScale;

				float alphaScale = (CameraPosAndAlpha.w == 2.0) ? 12.0 : 18.0; // 기존 4.5/8.0 → 크게



				ca.a = saturate(ca.a * 2.5);   // 1.5~4 사이 튜닝
			//	ca.a *= fade;


				//float alpha = ca.a * stepSize * alphaScale;




				float mediumTransparency = 0.35; // ⭐ 0.25 ~ 0.45 권장

				if (isBone)   mediumTransparency = 0.3;
				if (isTooth)  mediumTransparency = 0.2;

				//float fog = saturate((distVS - boneFogStart) / 30.0);
				//fog = fog * 0.35;   // ⭐ 아주 약하게

				float fog = saturate((distVS - 0.01) / 0.08);          // 1%~9% 구간



				float3 boneColor = float3(0.95, 0.95, 0.95);
				ca.rgb = lerp(ca.rgb, boneColor, fog * 0.6);


				float alpha = ca.a
					* max(stepSize, 0.002)
					* alphaScale
					* mediumTransparency;

				alpha *= boneRecovery;

				if (isBone)
				{
					float frontAtten = hasMesh ? saturate(distVS / 4.0) : 1.0;
					float deepRecover = saturate(distVS / 20.0);

					float recovery = max(frontAtten, deepRecover);

					// ⭐ 핵심: 0으로 깎지 말고 최소 밀도 보장
					float baseBone = 0.55;          // 0.45 ~ 0.65 추천
					alpha *= lerp(baseBone, 1.0, recovery);
				}







				//float alpha = ca.a * max(stepSize, 0.002) * alphaScale;
				// HU가 낮아도 살려줌
				//float fogAlpha = fog * stepSize * 6.0;

				float fogAlpha = fog * max(stepSize, 0.003) * 25.0;  // 체감용
				//float fogAlpha = fog * max(stepSize, 0.003) * 25.0;
				//float fogAlpha = fog * 0.02;

				acc.rgb += (1.0 - acc.a) * alpha * ca.rgb;
				acc.a += (1.0 - acc.a) * alpha;

				if (acc.a >= 0.95)
					break;
			}

	//float currentViewZ = currentPosVS.z;   // ✅ view space
		// 시각화
	//return float4(
	//	saturate(-meshViewZ / 500.0),
	//	saturate(-meshViewZ / 500.0),
	//	saturate(-meshViewZ / 500.0),
	//	1
	//	);

	///* ===============================
	//   Post
	//=============================== */




	float4 face = faceColor.SampleLevel(faceColorSamp, input.uv, 0);
	//float4 face = faceColor.Sample(faceColorSamp, float2(0.5, 0.5));


	// CT 구조 밝기
	float ctLuma = dot(acc.rgb, float3(0.299, 0.587, 0.114));
	// 얼굴 위에 CT 구조를 “더하기” (강도 튜닝)
	float ctStrength = 0.45; // 0.25~0.7 튜닝
	float3 outRgb = face.rgb + ctLuma * ctStrength;


	//// 얼굴 알파(또는 face.a)로 최종 결정
	//return float4(saturate(outRgb), 1.0);


	//return float4(face.rgb, 1);


	acc.rgb = pow(saturate(acc.rgb), 1.0 / 2.2);

	if (CameraPosAndAlpha.w == 1.0) return float4(acc.rgb, 1.0);
	if (CameraPosAndAlpha.w == 0.0) return float4(acc.rgb, 0.0);
	return float4(acc.rgb, acc.a);
}
