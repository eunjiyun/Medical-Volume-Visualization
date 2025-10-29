
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



float4 TransferFunction(float density)
{

	// ? °ø±â/¹è°æ Á¦°Å
	if (density < 0.1)
		return float4(0, 0, 0, 0);

	// ? ¿¬Á¶Á÷

	if (density < 0.3)
	{
		float t = (density - 0.1) / 0.2;
		return float4(0.7, 0.5, 0.4, t * 0.6);
	}


	// ? »À

	if (density < 0.6)
	{
		float t = (density - 0.3) / 0.3;
		return float4(0.9, 0.8, 0.7, 0.5 + t * 0.8);
	}


	// ? Ä¡¾Æ (°¡Àå ¹à°í ºÒÅõ¸í)

	return float4(1.0, 0.95, 0.9, 0.9);
}


float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{


	// --- ê´‘ì„  ?ì„± (ê¸°ì¡´ ì½”ë“œ ? ì?) ---

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



	//return float4(rayPosWS / 10.0, 1);  // 10?¼ë¡œ ?˜ëˆ ???œê°??


	//// CameraPosWS ë¬´ì‹œ?˜ê³  ê³ ì •ê°??¬ìš©
	///*float3*/ rayPosWS = float3(0, 0, -3.0);  // ?˜ë“œì½”ë”©

	//float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//return float4((rayPos + 2.0) / 4.0, 1);






	// ????ì¤??ŒìŠ¤??	//rayDirWS = -rayDirWS;


	//return float4(abs(rayDirWS), 1);


rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;


	//// ??ë²”ìœ„ ì¡°ì •?´ì„œ ?œê°??	//return float4(

	//	rayPos.x / 5.0 + 0.5,  // -2.5~2.5 ??0~1
	//	rayPos.y / 5.0 + 0.5,
	//	rayPos.z / 5.0 + 0.5,
	//	1
	//	);
	




	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//// ??rayDir ?•ì¸
	//return float4(abs(rayDir), 1);



	//// ??ì¹´ë©”???„ì¹˜ ?•ì¸
	//return float4(rayPos, 1);//==>>?¬ê¸°ê°€ ê·¼ë³¸?ì¸ ë¬¸ì œ??
	//rayDir = -rayDir;

	//// --- Ray-box êµì°¨ ---
	//float3 boxMin = float3(0, 0, 0);
	//float3 boxMax = float3(1, 1, 1);

	// ???˜ì • (volumeSize = 1.5 ê¸°ì?)

	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);




	//// ??rayPosê°€ ë°•ìŠ¤ ?ˆì¸ê°€?
	//if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	//	return float4(1, 0, 0, 1);  // ë¹¨ê°• = ??	//else
	//	return float4(0, 1, 0, 1);  // ì´ˆë¡ = ë°?





	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;





	//// ??tMin, tMax ?•ì¸
	////return float4(abs(tMin) / 10.0, 1);  // tMin???‰ìƒ?¼ë¡œ
	//// ?ëŠ”
	//return float4(abs(tMax) / 10.0, 1);  // tMaxë¥??‰ìƒ?¼ë¡œ


	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	//// ??t1 ?•ì¸
	////return float4(abs(t1) / 10.0, 1);
	//// ??t2 ?•ì¸
	// return float4(abs(t2) / 10.0, 1);


	//// ??????ì¶œë ¥
	//return float4(tNear / 10.0, tFar / 10.0, 0, 1);
	//// ë¹¨ê°• = tNear, ì´ˆë¡ = tFar



	//// ???”ë²„ê¹? êµì°¨ ?¬ë?
	//if (tNear > tFar || tFar < 0)
	//	return float4(1, 0, 0, 1);  // ë¹¨ê°• = êµì°¨ ????	//else
	//	return float4(0, 1, 0, 1);  // ì´ˆë¡ = êµì°¨ ??

	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;


	// ??ë°•ìŠ¤ ë²”ìœ„??ë§ê²Œ ?˜ì •
	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  // ì¹´ë©”?¼ê? ë°•ìŠ¤ ??	
	}

	// ??Step ?¬ê¸° ê³„ì‚°
	float stepSize = travelDist / float(MaxSteps);

	// ???œì‘???¤ì •
	float3 startPos = rayPos + rayDir * tNear;

	// ??startPosë¥?ë°•ìŠ¤ ?ˆìœ¼ë¡?ê°•ì œ
	startPos = clamp(startPos, boxMin, boxMax);

	//// ??startPosë¥??‰ìƒ?¼ë¡œ ?œì‹œ

	//return float4((startPos + 0.75) / 1.5, 1);
	//// [-0.75, 0.75] ??[0, 1] ë³€??


	//// ??tNear ê°??•ì¸
	//return float4(tNear / 5.0, tNear / 5.0, tNear / 5.0, 1);
	//// 0~5 ë²”ìœ„ë¥?0~1ë¡??•ê·œ??

	//// ???”ë²„ê¹? ?œì‘ ?„ì¹˜ ?‰ìƒ?¼ë¡œ ?œì‹œ=>ë¬¸ì œ ë°œê²¬
	//return float4(startPos, 1);



	// ??ì²??˜í”Œ ?„ì¹˜
	float3 currentPos = startPos;

	//// ??UV ë³€??	//float3 uvw = (currentPos + 0.75) / 1.5;
	// ???¬ë°”ë¥?UV ë³€??	
	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);


	//// UV ë²”ìœ„ ì²´í¬
	//if (any(uvw < 0.0) || any(uvw > 1.0))
	//	return float4(1, 0, 0, 1);  // ë¹¨ê°„??= ë²”ìœ„ ë°?	//else
	//	return float4(uvw, 1);  // ê·¸ë¼?”ì–¸?¸ì—¬????

	// ëª¨ë“  ?”ë²„ê¹?return ì£¼ì„ì²˜ë¦¬?˜ê³ 
// ë³¼ë¥¨ ?ë¶„ ?„ì— ì¶”ê?:

//// ???ìŠ¤ì²?ì¤‘ì•™ê°??•ì¸

//	float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;
//	return float4(testDensity * 10.0, testDensity * 10.0, testDensity * 10.0, 1);


	//float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;


	//// ??ì¦í­ ?†ì´ ?ë³¸ ?•ì¸
	//return float4(testDensity, testDensity, testDensity, 1);


	// --- ë³¼ë¥¨ ?ë¶„ ---

	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;


	//// ë£¨í”„ ?„ì—

	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{

		//// ???„ì¬ ?„ì¹˜ ê³„ì‚° (i???°ë¼ ?„ì§„)
		//float3 uvw = startPos + rayDir * (i * stepSize);

		// ???¬ë°”ë¥?ê³„ì‚°
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		uvw.y = 1.0 - uvw.y;  // ? Ãß°¡






		// ë²”ìœ„ ì²´í¬
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		// ë°€???˜í”Œë§?		
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;

		//// ? ¿øº» °ª È®ÀÎ
		//return float4(density / 10.0, density / 10.0, density / 10.0, 1);


		//// ? ¹Ğµµ ºĞÆ÷ È®ÀÎ
		//if (density < 0.3)
		//	return float4(0, 0, 1, 1);  // ÆÄ¶û = °ø±â
		//else if (density < 0.5)
		//	return float4(0, 1, 0, 1);  // ÃÊ·Ï = ¿¬Á¶Á÷
		//else if (density < 0.8)
		//	return float4(1, 1, 0, 1);  // ³ë¶û = »À
		//else
		//	return float4(1, 0, 0, 1);  // »¡°­ = Ä¡¾Æ



		// ? ¹Ğµµ ¹üÀ§ È®ÀÎ ÈÄ Á¤±ÔÈ­
		density = saturate(density / 255.0);  // 0~255 ¡æ 0~1






		//// ë°€?„ë? 10ë°?ì¦í­?´ì„œ ?œì‹œ
		//return float4(density * 10.0, density * 10.0, density * 10.0, 1);


		//  // ??ë°€??ì¡°ì • (?„ê³„ê°???¶”ê¸?
		//density = saturate((density - 0.05) * 2.0);  // 0.05 ?´í•˜ ?œê±°


		//// ??ë°ê¸° ì¦ê?
		////density = saturate((density - 0.1) * 2.0);  // 0.1 ?´í•˜ ?œê±°, 2ë°?ì¦í­


		//// ??Transfer Function ?ìš©
		//float4 colorAlpha = TransferFunction(density);


// ¶Ç´Â ? ¾àÇÏ°Ô
		//density = saturate(density *1.2);


		float4 colorAlpha = TransferFunction(density);



		//// ? °í¹Ğµµ(»À/Ä¡¾Æ)¿¡¸¸ ¶óÀÌÆÃ Ãß°¡
		//if (density > 0.4)
		//{
		//	// °£´ÜÇÑ ±×¶óµğ¾ğÆ® °è»ê

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


		//	// ? ÃÖ¼Ò°ªÀ» ³ôÀÓ (0.3 ¡æ 0.7)

		//	float lighting = max(0.7, dot(normal, lightDir));

		//	colorAlpha.rgb *= lighting;
		//}
		//




		//// ? »ö»ó¸¸ ¹Ù·Î ¸®ÅÏ (¾ËÆÄ ¹«½Ã)

		//return float4(colorAlpha.rgb, 1.0);



		float3 color = colorAlpha.rgb;








		//float alpha = colorAlpha.a * stepSize;  // stepSize ê³±í•˜ê¸?
		float alpha = colorAlpha.a * stepSize * 4.0;  // ? Åõ¸íµµ °­È­ ¹èÀ²



		//float4 colorAlpha = TransferFunction(density);
		//float3 color = colorAlpha.rgb;

		//float alpha = colorAlpha.a;  // ? stepSize °öÇÏÁö ¾ÊÀ½!



	

		if (alpha > 0.001)
		{
			////float3 color = float3(density, density, density);


			//// Front-to-back ë¸”ë Œ??			

			//acc.rgb += (1.0 - acc.a) * alpha * color;
			//acc.a += (1.0 - acc.a) * alpha;

			//// Early termination
			//if (acc.a >= 0.95)
			//	break;

			acc.rgb += (1.0 - acc.a) * alpha * color;

			//acc.rgb = pow(acc.rgb, 1.0 / 2.2); // °¨¸¶ º¸Á¤

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

//	// --- ê´‘ì„  ?ì„± ---

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

//	// ???˜ë“œì½”ë”© (?„ì‹œ)

//	float3 rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//

//	// --- Ray-box êµì°¨ ---

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

//	// --- ë³¼ë¥¨ ?ë¶„ ---

//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//
//		// ??UV ë³€??//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//

//		// UV ë²”ìœ„ ì²´í¬
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1);  // ë¹¨ê°„??= ë²”ìœ„ ë°?//		else
//			return float4(uvw, 1);  // ê·¸ë¼?”ì–¸?¸ì—¬????//

//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		float density = volumeTex.SampleLevel(samp, uvw, 0).r;
//

//		// ??ë°€??ì¦í­
//		density = saturate(density * 3.0);
//
//		// ???ŒíŒŒ ê³„ì‚°
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