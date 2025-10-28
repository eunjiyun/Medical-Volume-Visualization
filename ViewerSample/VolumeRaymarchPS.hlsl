
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



// Transfer Function: 諛?꾩뿉 ?곕씪 ?됱긽怨??뚰뙆 諛섑솚
float4 TransferFunction(float density)
{
	// 怨듦린 (留ㅼ슦 ??? 諛??
	if (density < 0.1)
		return float4(0, 0, 0, 0);

	// ?곗“吏?(?쇰?, 洹쇱쑁)
	if (density < 0.4)
	{
		float t = (density - 0.1) / 0.3;  // 0~1 ?뺢퇋??		
		return float4(0.7, 0.5, 0.4, t * 0.05);  // ?댁깋, ?쏀븳 ?뚰뙆
	}

	// 堉?(以묎컙 諛??
	if (density < 0.7)
	{
		float t = (density - 0.4) / 0.3;
		return float4(1.0, 0.9, 0.8, t * 0.3);  // 諛앹? 踰좎씠吏, 媛뺥븳 ?뚰뙆
	}

	// 移섏븘, 湲덉냽 (?믪? 諛??
	return float4(1.0, 1.0, 1.0, 0.5);  // ?곗깋
}



float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- 愿묒꽑 ?앹꽦 (湲곗〈 肄붾뱶 ?좎?) ---
	float2 offset = float2(0.0, 0.0);
	float2 scale = float2(0.5, 0.5);
	float2 localUV = (uv - offset) / scale;
	float2 screenUV = localUV;
	//screenUV = uv;

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

		//// 諛?꾨? 10諛?利앺룺?댁꽌 ?쒖떆
		//return float4(density * 10.0, density * 10.0, density * 10.0, 1);


		  // ??諛??議곗젙 (?꾧퀎媛???텛湲?
		density = saturate((density - 0.05) * 2.0);  // 0.05 ?댄븯 ?쒓굅


		// ??諛앷린 利앷?
		//density = saturate((density - 0.1) * 2.0);  // 0.1 ?댄븯 ?쒓굅, 2諛?利앺룺


		// ??Transfer Function ?곸슜
		float4 colorAlpha = TransferFunction(density);

		  


		float3 color = colorAlpha.rgb;
		float alpha = colorAlpha.a * stepSize;  // stepSize 怨깊븯湲?


	

		if (alpha > 0.001)
		{
			//float3 color = float3(density, density, density);

			// Front-to-back 釉붾젋??			
			acc.rgb += (1.0 - acc.a) * alpha * color;
			acc.a += (1.0 - acc.a) * alpha;

			// Early termination
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