
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



//// Transfer Function: è«›Â€?ê¾©ë¿‰ ?ê³•ì”ª ??±ê¸½????°ë™† è«›ì„‘??//float4 TransferFunction(float density)
//{
//	// ê³µê¸°
//	//if (density < 0.1)
//	if (density < 0.01)
//		return float4(0, 0, 0, 0);
//
//	//// ?°ì¡°ì§?//	//if (density < 0.4)
//	//{
//	//	float t = (density - 0.1) / 0.3;
//	//	return float4(0.7, 0.5, 0.4, t * 0.1);  // ??0.05 ??0.5
//	//}
//	  // ??ë²”ìœ„ ì¡°ì •
//	if (density < 0.2)  // 0.4 ??0.2
//	{
//		float t = (density - 0.01) / 0.19;
//		return float4(0.7, 0.5, 0.4, t * 0.5);
//	}
//
//	//// ë¼?//	//if (density < 0.7)
//	//{
//	//	float t = (density - 0.4) / 0.3;
//	//	return float4(1.0, 0.9, 0.8, t * 0.6);  // ??0.3 ??3.0
//	//}
//	if (density < 0.5)  // 0.7 ??0.5
//	{
//		float t = (density - 0.2) / 0.3;
//		return float4(1.0, 0.9, 0.8, t * 3.0);
//	}
//
//	//// ì¹˜ì•„, ê¸ˆì†
//	//return float4(1.0, 1.0, 1.0, 1.0);  // ??0.5 ??5.0
//	return float4(1.0, 1.0, 1.0, 5.0);
//}

//float4 TransferFunction(float density)
//{
//	// ???„ì²´ ë²”ìœ„ ?œìš©
//	if (density < 0.3)
//	{
//		float t = density / 0.3;
//		return float4(0.7, 0.5, 0.4, t * 2.0);  // ë² ì´ì§€
//	}
//
//	if (density < 0.6)
//	{
//		float t = (density - 0.3) / 0.3;
//		return float4(1.0, 0.9, 0.8, 2.0 + t * 3.0);  // ë°ì? ë² ì´ì§€
//	}
//
//	return float4(1.0, 1.0, 1.0, 5.0);  // ?°ìƒ‰
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
	// ??ê³µê¸°/ë°°ê²½ ?œê±°
	if (density < 0.1)
		return float4(0, 0, 0, 0);

	// ???°ì¡°ì§?
	if (density < 0.3)
	{
		float t = (density - 0.1) / 0.2;
		return float4(0.7, 0.5, 0.4, t * 0.6);
	}

	// ??ë¼?
	if (density < 0.6)
	{
		float t = (density - 0.3) / 0.3;
		return float4(0.9, 0.8, 0.7, 0.5 + t * 0.8);
	}

	// ??ì¹˜ì•„ (ê°€??ë°ê³  ë¶ˆíˆ¬ëª?
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
//		return float4(0.7, 0.5, 0.4, t * 0.3);  // ??0.15 ??0.3
//	}
//
//	if (density < 0.65)
//	{
//		float t = (density - 0.35) / 0.3;
//		return float4(0.9, 0.8, 0.7, 0.5 + t * 0.5);  // ??ì¦ê?
//	}
//
//	return float4(1.0, 1.0, 0.95, 1.5);  // ??0.8 ??1.5
//}


//float4 TransferFunction(float density)
//{
//	// ??ê³µê¸°/ë¹?ê³µê°„ (?„ê³„ê°??’ì„)
//	if (density < 0.3)
//		return float4(0, 0, 0, 0);
//
//	// ???°ì¡°ì§?- ?´ë‘??ë² ì´ì§€
//	if (density < 0.5)
//	{
//		float t = (density - 0.3) / 0.2;
//		return float4(0.7, 0.5, 0.4, t * 0.2);
//	}
//
//	// ??ë¼?- ë°ì? ë² ì´ì§€
//	if (density < 0.8)
//	{
//		float t = (density - 0.5) / 0.3;
//		return float4(0.9, 0.8, 0.7, 0.3 + t * 0.4);
//	}
//
//	// ??ì¹˜ì•„ - ?°ìƒ‰ (?’ì? ?„ê³„ê°?
//	return float4(1.0, 1.0, 0.95, 0.8);
//}

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- ?¿ë¬’ê½???¹ê½¦ (æ¹²ê³—???„ë¶¾ë±??ì¢?) ---
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


	//return float4(rayPosWS / 10.0, 1);  // 10??°ì¤ˆ ??ë‹ ????“ì»–??


	//// CameraPosWS ?¾ëŒ???í€??¨ì¢?™åª›?????	///*float3*/ rayPosWS = float3(0, 0, -3.0);  // ??ë±¶?„ë¶¾ëµ?
	//float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	//return float4((rayPos + 2.0) / 4.0, 1);





	// ????ä»????’ª??	//rayDirWS = -rayDirWS;

	//return float4(abs(rayDirWS), 1);


rayPosWS = float3(0, 0, -3.0);

	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;

	//// ??è¸°ë¶¿??è­°ê³—???ê½Œ ??“ì»–??	//return float4(
	//	rayPos.x / 5.0 + 0.5,  // -2.5~2.5 ??0~1
	//	rayPos.y / 5.0 + 0.5,
	//	rayPos.z / 5.0 + 0.5,
	//	1
	//	);
	




	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);

	//// ??rayDir ?ëº¤ì”¤
	//return float4(abs(rayDir), 1);


	//// ??ç§»ë?ì°???ê¾©íŠ‚ ?ëº¤ì”¤
	//return float4(rayPos, 1);//==>>??ë¦°åª›? æ´¹ì‡°??ê³¸ì”¤ ?¾ëª„???
	//rayDir = -rayDir;

	//// --- Ray-box ?´ë¨¯ê°?---
	//float3 boxMin = float3(0, 0, 0);
	//float3 boxMax = float3(1, 1, 1);

	// ????ì ™ (volumeSize = 1.5 æ¹²ê³—?)
	float3 boxMin = float3(-0.75, -0.75, -0.75);
	float3 boxMax = float3(0.75, 0.75, 0.75);



	//// ??rayPosåª›Â€ è«›ëº¤????‰ì”¤åª›Â€?
	//if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	//	return float4(1, 0, 0, 1);  // ??‘£ì»?= ??	//else
	//	return float4(0, 1, 0, 1);  // ?¥ëˆì¤?= è«?




	float3 invDir = 1.0 / (rayDir + 1e-6);
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;




	//// ??tMin, tMax ?ëº¤ì”¤
	////return float4(abs(tMin) / 10.0, 1);  // tMin????±ê¸½??°ì¤ˆ
	//// ?ë¨?’—
	//return float4(abs(tMax) / 10.0, 1);  // tMax????±ê¸½??°ì¤ˆ

	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	//// ??t1 ?ëº¤ì”¤
	////return float4(abs(t1) / 10.0, 1);
	//// ??t2 ?ëº¤ì”¤
	// return float4(abs(t2) / 10.0, 1);

	//// ???????°ì’•??	//return float4(tNear / 10.0, tFar / 10.0, 0, 1);
	//// ??‘£ì»?= tNear, ?¥ëˆì¤?= tFar



	//// ???ë¶¾ì¾­æº? ?´ë¨¯ê°????
	//if (tNear > tFar || tFar < 0)
	//	return float4(1, 0, 0, 1);  // ??‘£ì»?= ?´ë¨¯ê°?????	//else
	//	return float4(0, 1, 0, 1);  // ?¥ëˆì¤?= ?´ë¨¯ê°???
	tNear = max(tNear, 0.0);

	float travelDist = tFar - tNear;

	// ??è«›ëº¤??è¸°ë¶¿???ï§ìšŠì¾???ì ™
	if (all(rayPos >= boxMin) && all(rayPos <= boxMax))
	{
		tNear = 0.0;  // ç§»ë?ì°??? è«›ëº¤????	
	}

	// ??Step ??ë¦??¨ê¾©ê¶?	
	float stepSize = travelDist / float(MaxSteps);

	// ????–ì˜‰????¼ì ™
	float3 startPos = rayPos + rayDir * tNear;

	// ??startPos??è«›ëº¤????‰ì‘æ¿?åª›ëº¤??	startPos = clamp(startPos, boxMin, boxMax);

	//// ??startPos????±ê¸½??°ì¤ˆ ??–ë–†
	//return float4((startPos + 0.75) / 1.5, 1);
	//// [-0.75, 0.75] ??[0, 1] è¹‚Â€??

	//// ??tNear åª??ëº¤ì”¤
	//return float4(tNear / 5.0, tNear / 5.0, tNear / 5.0, 1);
	//// 0~5 è¸°ë¶¿?ç‘œ?0~1æ¿??ëº?‡‹??

	//// ???ë¶¾ì¾­æº? ??–ì˜‰ ?ê¾©íŠ‚ ??±ê¸½??°ì¤ˆ ??–ë–†=>?¾ëª„??è«›ì’“ê»?	//return float4(startPos, 1);



	// ??ï§???‘ëµ† ?ê¾©íŠ‚
	float3 currentPos = startPos;

	//// ??UV è¹‚Â€??	//float3 uvw = (currentPos + 0.75) / 1.5;
	// ????ì»?‘œ?UV è¹‚Â€??	
	float3 uvw = (startPos - boxMin) / (boxMax - boxMin);


	//// UV è¸°ë¶¿??ï§£ëŒ„ê²?	//if (any(uvw < 0.0) || any(uvw > 1.0))
	//	return float4(1, 0, 0, 1);  // ??‘£ì»??= è¸°ë¶¿??è«?	//else
	//	return float4(uvw, 1);  // æ´¹ëªƒ??ë¶¿ë¼µ?ëª„ë¿¬????

	// ï§â‘¤ë±??ë¶¾ì¾­æº?return äºŒì‡±ê½ï§£?â”??í€?// è¹‚ì‡°???ê³·í…‡ ?ê¾©ë¿‰ ?°ë¶½?:

//// ????¿ë’ªï§?ä»¥ë¬’ë¸°åª›??ëº¤ì”¤
//	float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;
//	return float4(testDensity * 10.0, testDensity * 10.0, testDensity * 10.0, 1);


	//float testDensity = volumeTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0).r;

	//// ??ï§ì•ºë£???ì”  ?ë¨?‚¯ ?ëº¤ì”¤
	//return float4(testDensity, testDensity, testDensity, 1);


	// --- è¹‚ì‡°???ê³·í…‡ ---
	float4 acc = float4(0, 0, 0, 0);

	int sampleCount = 0;

	//// ?·â‘¦ë´??ê¾©ë¿‰
	//return float4(MaxSteps / 256.0, 0, 0, 1);

	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		//// ???ê¾©ì˜± ?ê¾©íŠ‚ ?¨ê¾©ê¶?(i???ê³•ì”ª ?ê¾©ì­Š)
		//float3 uvw = startPos + rayDir * (i * stepSize);

		// ????ì»?‘œ??¨ê¾©ê¶?		
		float3 currentPos = startPos + rayDir * (i * stepSize);
		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);

		uvw.y = 1.0 - uvw.y;  // ??ì¶”ê?




		// è¸°ë¶¿??ï§£ëŒ„ê²?		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;


		// è«›Â€????‘ëµ†ï§?		
		float density = volumeTex.SampleLevel(samp, uvw, 0).r;

		//// ???ë³¸ ê°??•ì¸
		//return float4(density / 10.0, density / 10.0, density / 10.0, 1);


		//// ??ë°€??ë¶„í¬ ?•ì¸
		//if (density < 0.3)
		//	return float4(0, 0, 1, 1);  // ?Œë‘ = ê³µê¸°
		//else if (density < 0.5)
		//	return float4(0, 1, 0, 1);  // ì´ˆë¡ = ?°ì¡°ì§?		//else if (density < 0.8)
		//	return float4(1, 1, 0, 1);  // ?¸ë‘ = ë¼?		//else
		//	return float4(1, 0, 0, 1);  // ë¹¨ê°• = ì¹˜ì•„



		// ??ë°€??ë²”ìœ„ ?•ì¸ ???•ê·œ??		density = saturate(density / 255.0);  // 0~255 ??0~1




		//// è«›Â€?ê¾? 10è«?ï§ì•ºë£??ê½Œ ??–ë–†
		//return float4(density * 10.0, density * 10.0, density * 10.0, 1);


		//  // ??è«›Â€??è­°ê³—??(?ê¾§í€åª›????›æ¹²?
		//density = saturate((density - 0.05) * 2.0);  // 0.05 ??„ë¸¯ ??“êµ…


		//// ??è«›ì•·ë¦?ï§ì•·?
		////density = saturate((density - 0.1) * 2.0);  // 0.1 ??„ë¸¯ ??“êµ…, 2è«?ï§ì•ºë£?

		//// ??Transfer Function ?ê³¸ìŠœ
		//float4 colorAlpha = TransferFunction(density);

// ?ëŠ” ???½í•˜ê²?		
density = saturate(density *1.2);

		float4 colorAlpha = TransferFunction(density);


		//// ??ê³ ë???ë¼?ì¹˜ì•„)?ë§Œ ?¼ì´??ì¶”ê?
		//if (density > 0.4)
		//{
		//	// ê°„ë‹¨??ê·¸ë¼?”ì–¸??ê³„ì‚°
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

		//	// ??ìµœì†Œê°’ì„ ?’ì„ (0.3 ??0.7)
		//	float lighting = max(0.7, dot(normal, lightDir));

		//	colorAlpha.rgb *= lighting;
		//}
		//



		//// ???‰ìƒë§?ë°”ë¡œ ë¦¬í„´ (?ŒíŒŒ ë¬´ì‹œ)
		//return float4(colorAlpha.rgb, 1.0);



		float3 color = colorAlpha.rgb;







		//float alpha = colorAlpha.a * stepSize;  // stepSize ?¨ê¹Šë¸?¹²?
		float alpha = colorAlpha.a * stepSize * 4.0;  // ???¬ëª…??ê°•í™” ë°°ìœ¨


		//float4 colorAlpha = TransferFunction(density);
		//float3 color = colorAlpha.rgb;
		//float alpha = colorAlpha.a;  // ??stepSize ê³±í•˜ì§€ ?ŠìŒ!


	

		if (alpha > 0.001)
		{
			////float3 color = float3(density, density, density);

			//// Front-to-back ?‰ë¶¾???			
			//acc.rgb += (1.0 - acc.a) * alpha * color;
			//acc.a += (1.0 - acc.a) * alpha;

			//// Early termination
			//if (acc.a >= 0.95)
			//	break;

			acc.rgb += (1.0 - acc.a) * alpha * color;

			//acc.rgb = pow(acc.rgb, 1.0 / 2.2); // ê°ë§ˆ ë³´ì •

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
//	// --- ?¿ë¬’ê½???¹ê½¦ ---
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
//	// ????ë±¶?„ë¶¾ëµ?(?ê¾©ë–†)
//	float3 rayPosWS = float3(0, 0, -3.0);
//
//	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// --- Ray-box ?´ë¨¯ê°?---
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
//	// --- è¹‚ì‡°???ê³·í…‡ ---
//	float4 acc = float4(0, 0, 0, 0);
//
//	[loop]
//	for (int i = 0; i < MaxSteps; i++)
//	{
//		float3 currentPos = startPos + rayDir * (i * stepSize);
//
//		// ??UV è¹‚Â€??//		float3 uvw = (currentPos - boxMin) / (boxMax - boxMin);
//
//		// UV è¸°ë¶¿??ï§£ëŒ„ê²?//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1);  // ??‘£ì»??= è¸°ë¶¿??è«?//		else
//			return float4(uvw, 1);  // æ´¹ëªƒ??ë¶¿ë¼µ?ëª„ë¿¬????//
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;
//
//		float density = volumeTex.SampleLevel(samp, uvw, 0).r;
//
//		// ??è«›Â€??ï§ì•ºë£?//		density = saturate(density * 3.0);
//
//		// ????°ë™† ?¨ê¾©ê¶?//		float alpha = density * 10.0 * stepSize;
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