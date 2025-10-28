
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

Texture3D<float> volumeTex : register(t0);
//Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);



//// Transfer Function: 諛?꾩뿉 ?곕씪 ?됱긽怨??뚰뙆 諛섑솚
//float4 TransferFunction(float density)
//{
//	// 공기
//	//if (density < 0.1)
//	if (density < 0.01)
//		return float4(0, 0, 0, 0);
//
//	//// 연조직
//	//if (density < 0.4)
//	//{
//	//	float t = (density - 0.1) / 0.3;
//	//	return float4(0.7, 0.5, 0.4, t * 0.1);  // ✅ 0.05 → 0.5
//	//}
//	  // ✅ 범위 조정
//	if (density < 0.2)  // 0.4 → 0.2
//	{
//		float t = (density - 0.01) / 0.19;
//		return float4(0.7, 0.5, 0.4, t * 0.5);
//	}
//
//	//// 뼈
//	//if (density < 0.7)
//	//{
//	//	float t = (density - 0.4) / 0.3;
//	//	return float4(1.0, 0.9, 0.8, t * 0.6);  // ✅ 0.3 → 3.0
//	//}
//	if (density < 0.5)  // 0.7 → 0.5
//	{
//		float t = (density - 0.2) / 0.3;
//		return float4(1.0, 0.9, 0.8, t * 3.0);
//	}
//
//	//// 치아, 금속
//	//return float4(1.0, 1.0, 1.0, 1.0);  // ✅ 0.5 → 5.0
//	return float4(1.0, 1.0, 1.0, 5.0);
//}

//float4 TransferFunction(float density)
//{
//	// ✅ 전체 범위 활용
//	if (density < 0.3)
//	{
//		float t = density / 0.3;
//		return float4(0.7, 0.5, 0.4, t * 2.0);  // 베이지
//	}
//
//	if (density < 0.6)
//	{
//		float t = (density - 0.3) / 0.3;
//		return float4(1.0, 0.9, 0.8, 2.0 + t * 3.0);  // 밝은 베이지
//	}
//
//	return float4(1.0, 1.0, 1.0, 5.0);  // 흰색
//}

//float4 TransferFunction(float density)
//{
//	if (density < 0.1)
//		return float4(0, 0, 0, 0);
//
//	if (density < 0.3)
//		return float4(0.7, 0.6, 0.5, 0.1);
//
//	if (density < 0.6)
//		return float4(0.9, 0.8, 0.7, 0.5);
//
//	return float4(1.0, 0.95, 0.9, 1.0);
//}

float4 TransferFunction(float density)
{
	// ✅ 공기/배경 제거
	if (density < 0.1)
		return float4(0, 0, 0, 0);

	// ✅ 연조직
	if (density < 0.3)
	{
		float t = (density - 0.1) / 0.2;
		return float4(0.7, 0.5, 0.4, t * 0.3);
	}

	// ✅ 뼈
	if (density < 0.6)
	{
		float t = (density - 0.3) / 0.3;
		return float4(0.9, 0.8, 0.7, 0.5 + t * 0.4);
	}

	// ✅ 치아 (가장 밝고 불투명)
	return float4(1.0, 0.95, 0.9, 0.9);
}


//float4 TransferFunction(float density)
//{
//	if (density < 0.15)
//		return float4(0, 0, 0, 0);
//
//	if (density < 0.35)
//	{
//		float t = (density - 0.15) / 0.2;
//		return float4(0.7, 0.5, 0.4, t * 0.3);  // ✅ 0.15 → 0.3
//	}
//
//	if (density < 0.65)
//	{
//		float t = (density - 0.35) / 0.3;
//		return float4(0.9, 0.8, 0.7, 0.5 + t * 0.5);  // ✅ 증가
//	}
//
//	return float4(1.0, 1.0, 0.95, 1.5);  // ✅ 0.8 → 1.5
//}


//float4 TransferFunction(float density)
//{
//	// ✅ 공기/빈 공간 (임계값 높임)
//	if (density < 0.3)
//		return float4(0, 0, 0, 0);
//
//	// ✅ 연조직 - 어두운 베이지
//	if (density < 0.5)
//	{
//		float t = (density - 0.3) / 0.2;
//		return float4(0.7, 0.5, 0.4, t * 0.2);
//	}
//
//	// ✅ 뼈 - 밝은 베이지
//	if (density < 0.8)
//	{
//		float t = (density - 0.5) / 0.3;
//		return float4(0.9, 0.8, 0.7, 0.3 + t * 0.4);
//	}
//
//	// ✅ 치아 - 흰색 (높은 임계값)
//	return float4(1.0, 1.0, 0.95, 0.8);
//}

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- 愿묒꽑 ?앹꽦 (湲곗〈 肄붾뱶 ?좎?) ---
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


	//return float4(rayPosWS / 10.0, 1);  // 10?쇰줈 ?섎닠???쒓컖??


	//// CameraPosWS 臾댁떆?섍퀬 怨좎젙媛??ъ슜
	///*float3*/ rayPosWS = float3(0, 0, -3.0);  // ?섎뱶肄붾뵫

	//float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//return float4((rayPos + 2.0) / 4.0, 1);





	// ????以??뚯뒪??	//rayDirWS = -rayDirWS;

	//return float4(abs(rayDirWS), 1);


rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;

	//// ??踰붿쐞 議곗젙?댁꽌 ?쒓컖??	//return float4(
	//	rayPos.x / 5.0 + 0.5,  // -2.5~2.5 ??0~1
	//	rayPos.y / 5.0 + 0.5,
	//	rayPos.z / 5.0 + 0.5,
	//	1
	//	);
	




	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//// ??rayDir ?뺤씤
	//return float4(abs(rayDir), 1);


	//// ??移대찓???꾩튂 ?뺤씤
	//return float4(rayPos, 1);//==>>?ш린媛 洹쇰낯?곸씤 臾몄젣??
	//rayDir = -rayDir;

	//// --- Ray-box 援먯감 ---
	//float3 boxMin = float3(0, 0, 0);
	//float3 boxMax = float3(1, 1, 1);

	// ???섏젙 (volumeSize = 1.5 湲곗?)
	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);



	//// ??rayPos媛 諛뺤뒪 ?덉씤媛?
	//if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	//	return float4(1, 0, 0, 1);  // 鍮④컯 = ??	//else
	//	return float4(0, 1, 0, 1);  // 珥덈줉 = 諛?




	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;




	//// ??tMin, tMax ?뺤씤
	////return float4(abs(tMin) / 10.0, 1);  // tMin???됱긽?쇰줈
	//// ?먮뒗
	//return float4(abs(tMax) / 10.0, 1);  // tMax瑜??됱긽?쇰줈

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	//// ??t1 ?뺤씤
	////return float4(abs(t1) / 10.0, 1);
	//// ??t2 ?뺤씤
	// return float4(abs(t2) / 10.0, 1);

	//// ??????異쒕젰
	//return float4(tNear / 10.0, tFar / 10.0, 0, 1);
	//// 鍮④컯 = tNear, 珥덈줉 = tFar



	//// ???붾쾭源? 援먯감 ?щ?
	//if (tNear > tFar || tFar < 0)
	//	return float4(1, 0, 0, 1);  // 鍮④컯 = 援먯감 ????	//else
	//	return float4(0, 1, 0, 1);  // 珥덈줉 = 援먯감 ??
	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;

	// ??諛뺤뒪 踰붿쐞??留욊쾶 ?섏젙
	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  // 移대찓?쇨? 諛뺤뒪 ??	
	}

	// ??Step ?ш린 怨꾩궛
	float stepSize = travelDist / float(MaxSteps);

	// ???쒖옉???ㅼ젙
	float3 startPos = rayPos + rayDir * tNear;

	// ??startPos瑜?諛뺤뒪 ?덉쑝濡?媛뺤젣
	startPos = clamp(startPos, boxMin, boxMax);

	//// ??startPos瑜??됱긽?쇰줈 ?쒖떆
	//return float4((startPos + 0.75) / 1.5, 1);
	//// [-0.75, 0.75] ??[0, 1] 蹂??

	//// ??tNear 媛??뺤씤
	//return float4(tNear / 5.0, tNear / 5.0, tNear / 5.0, 1);
	//// 0~5 踰붿쐞瑜?0~1濡??뺢퇋??

	//// ???붾쾭源? ?쒖옉 ?꾩튂 ?됱긽?쇰줈 ?쒖떆=>臾몄젣 諛쒓껄
	//return float4(startPos, 1);



	// ??泥??섑뵆 ?꾩튂
	float3 currentPos = startPos;

	//// ??UV 蹂??	//float3 uvw = (currentPos + 0.75) / 1.5;
	// ???щ컮瑜?UV 蹂??	
	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);


	//// UV 踰붿쐞 泥댄겕
	//if (any(uvw < 0.0) || any(uvw > 1.0))
	//	return float4(1, 0, 0, 1);  // 鍮④컙??= 踰붿쐞 諛?	//else
	//	return float4(uvw, 1);  // 洹몃씪?붿뼵?몄뿬????

	// 紐⑤뱺 ?붾쾭源?return 二쇱꽍泥섎━?섍퀬
// 蹂쇰ⅷ ?곷텇 ?꾩뿉 異붽?:

//// ???띿뒪泥?以묒븰媛??뺤씤
//	float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;
//	return float4(testDensity * 10.0, testDensity * 10.0, testDensity * 10.0, 1);


	//float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;

	//// ??利앺룺 ?놁씠 ?먮낯 ?뺤씤
	//return float4(testDensity, testDensity, testDensity, 1);


	// --- 蹂쇰ⅷ ?곷텇 ---
	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

	//// 猷⑦봽 ?꾩뿉
	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		//// ???꾩옱 ?꾩튂 怨꾩궛 (i???곕씪 ?꾩쭊)
		//float3 uvw = startPos + rayDir * (i * stepSize);

		// ???щ컮瑜?怨꾩궛
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		uvw.y = 1.0 - uvw.y;  // ✅ 추가




		// 踰붿쐞 泥댄겕
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		// 諛???섑뵆留?		
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;

		//// ✅ 원본 값 확인
		//return float4(density / 10.0, density / 10.0, density / 10.0, 1);


		//// ✅ 밀도 분포 확인
		//if (density < 0.3)
		//	return float4(0, 0, 1, 1);  // 파랑 = 공기
		//else if (density < 0.5)
		//	return float4(0, 1, 0, 1);  // 초록 = 연조직
		//else if (density < 0.8)
		//	return float4(1, 1, 0, 1);  // 노랑 = 뼈
		//else
		//	return float4(1, 0, 0, 1);  // 빨강 = 치아



		// ✅ 밀도 범위 확인 후 정규화
		density = saturate(density / 255.0);  // 0~255 → 0~1




		//// 諛?꾨? 10諛?利앺룺?댁꽌 ?쒖떆
		//return float4(density * 10.0, density * 10.0, density * 10.0, 1);


		//  // ??諛??議곗젙 (?꾧퀎媛???텛湲?
		//density = saturate((density - 0.05) * 2.0);  // 0.05 ?댄븯 ?쒓굅


		//// ??諛앷린 利앷?
		////density = saturate((density - 0.1) * 2.0);  // 0.1 ?댄븯 ?쒓굅, 2諛?利앺룺


		//// ??Transfer Function ?곸슜
		//float4 colorAlpha = TransferFunction(density);

// 또는 ✅ 약하게
		//density = saturate(density *1.2);

		float4 colorAlpha = TransferFunction(density);


		//// ✅ 고밀도(뼈/치아)에만 라이팅 추가
		//if (density > 0.4)
		//{
		//	// 간단한 그라디언트 계산
		//	float eps = 0.01;
		//	float dx = volumeTex.SampleLevel(samp, uvw + float3(eps, 0, 0), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(eps, 0, 0), 0).r;
		//	float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps, 0), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(0, eps, 0), 0).r;
		//	float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps), 0).r;

		//	float3 normal = normalize(float3(dx, dy, dz) + 1e-6);
		//	float3 lightDir = normalize(float3(1, 1, -1));
		//	float lighting = max(0.3, dot(normal, lightDir));

		//	colorAlpha.rgb *= lighting;
		//}


		//if (density > 0.4)
		//{
		//	float eps = 0.01;
		//	float dx = volumeTex.SampleLevel(samp, uvw + float3(eps, 0, 0), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(eps, 0, 0), 0).r;
		//	float dy = volumeTex.SampleLevel(samp, uvw + float3(0, eps, 0), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(0, eps, 0), 0).r;
		//	float dz = volumeTex.SampleLevel(samp, uvw + float3(0, 0, eps), 0).r
		//		- volumeTex.SampleLevel(samp, uvw - float3(0, 0, eps), 0).r;

		//	float3 normal = normalize(float3(dx, dy, dz) + 1e-6);
		//	float3 lightDir = normalize(float3(1, 1, -1));

		//	// ✅ 최소값을 높임 (0.3 → 0.7)
		//	float lighting = max(0.7, dot(normal, lightDir));

		//	colorAlpha.rgb *= lighting;
		//}
		//



		//// ✅ 색상만 바로 리턴 (알파 무시)
		//return float4(colorAlpha.rgb, 1.0);



		float3 color = colorAlpha.rgb;







		//float alpha = colorAlpha.a * stepSize;  // stepSize 怨깊븯湲?
		float alpha = colorAlpha.a * stepSize * 4.0;  // ✅ 투명도 강화 배율


		//float4 colorAlpha = TransferFunction(density);
		//float3 color = colorAlpha.rgb;
		//float alpha = colorAlpha.a;  // ✅ stepSize 곱하지 않음!


	

		if (alpha > 0.001)
		{
			////float3 color = float3(density, density, density);

			//// Front-to-back 釉붾젋??			
			//acc.rgb += (1.0 - acc.a) * alpha * color;
			//acc.a += (1.0 - acc.a) * alpha;

			//// Early termination
			//if (acc.a >= 0.95)
			//	break;

			acc.rgb += (1.0 - acc.a) * alpha * color;

			//acc.rgb = pow(acc.rgb, 1.0 / 2.2); // 감마 보정

			acc.a += (1.0 - acc.a) * alpha;

			if (acc.a >= 0.95)
				break;
		}


		/*if (alpha > 0.001)
		{
			float value = density;
			float3 color = float3(value, value, value);

			acc.rgb += (1.0 - acc.a) * alpha * color;
			acc.a += (1.0 - acc.a) * alpha;

			if (acc.a >= 0.95)
				break;
		}*/
	}


	return float4(acc.rgb, acc.a);

}


//
//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// --- 愿묒꽑 ?앹꽦 ---
//	float2 offset = float2(0.0, 0.0);
//	float2 scale = float2(0.5, 0.5);
//	float2 localUV = (uv - offset) / scale;
//	float2 screenUV = localUV;
//
//	float2 ndc = screenUV * 2.0 - 1.0;
//	ndc.y = -ndc.y;
//
//	float4 ndcPos = float4(ndc, 1, 1);
//	float4 viewDirVS = mul(ndcPos, InvProj);
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//
//	// ???섎뱶肄붾뵫 (?꾩떆)
//	float3 rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// --- Ray-box 援먯감 ---
//	float3 boxMin = float3(-0.75, -0.75, -0.75);
//	float3 boxMax = float3(0.75, 0.75, 0.75);
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
//	if (tNear > tFar || tFar < 0)
//		return float4(0, 0, 0, 1);
//
//	tNear = max(tNear, 0.0);
//
//	float travelDist = tFar - tNear;
//	float stepSize = travelDist / float(MaxSteps);
//	float3 startPos = rayPos + rayDir * tNear;
//
//	// --- 蹂쇰ⅷ ?곷텇 ---
//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//
//		// ??UV 蹂??//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//
//		// UV 踰붿쐞 泥댄겕
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1);  // 鍮④컙??= 踰붿쐞 諛?//		else
//			return float4(uvw, 1);  // 洹몃씪?붿뼵?몄뿬????//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		float density = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		// ??諛??利앺룺
//		density = saturate(density * 3.0);
//
//		// ???뚰뙆 怨꾩궛
//		float alpha = density * 10.0 * stepSize;
//
//		if (alpha > 0.001)
//		{
//			float value = density * 2.0;
//			float3 color = float3(value, value, value);
//
//			acc.rgb += (1.0 - acc.a) * alpha * color;
//			acc.a += (1.0 - acc.a) * alpha;
//
//			if (acc.a >= 0.95)
//				break;
//		}
//	}
//
//	return float4(acc.rgb, 1.0);
//}