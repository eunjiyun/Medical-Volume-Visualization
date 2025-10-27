//cbuffer CB : register(b0)
//{
//	matrix View;
//	matrix Proj;
//	matrix InvView;
//	matrix InvProj;
//	matrix VolumeWorld;
//	matrix InvVolumeWorld;
//	float3 CameraPosWS;
//	float Step;
//	int   MaxSteps;
//	float Opacity;
//	float pad0;
//	float pad1;
//};
//
//Texture3D<float> volumeTex : register(t0);
//SamplerState samp : register(s0);
//
//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// ??NDC ??View space
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 rayStartVS = mul(ndc, InvProj);
//	rayStartVS /= rayStartVS.w;
//
//	/*float3 rayDirWS = normalize(mul(float4(rayStartVS.xyz, 0), InvView).xyz);
//	float3 rayPosWS = CameraPosWS;*/
//
//
//	// ?§Ìóò?? Î≥ºÎ•® Î°úÏª¨ Ï¢åÌëúÍ∞Ä 0~1?¥Îùº Í∞Ä??
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // Ïπ¥Î©î?ºÎ? Î≥ºÎ•® ?ûÏ™Ω??Î∞∞Ïπò
//	float3 rayDirWS = normalize(float3(0, 0, 1));
//
//
//
//
//	//// ???îÎìú ??Î≥ºÎ•® Î°úÏª¨Î°?Î≥Ä??
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// ??Raymarch loop
//	float4 acc = 0;
//	for (int i = 0; i < MaxSteps; i++) {
//	/*	float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0)) break;*/
//
//		////float3 uvw = mul(float4(worldPos, 1.0), InvVolumeWorld).xyz;
//		//float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		//if (any(uvw < 0.0) || any(uvw > 1.0))
//		//	return float4(1, 0, 0, 1); // ?àÏù¥ Î∞ïÏä§ Î∞???Îπ®Í∞Ñ??
//
//
//		float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			return float4(1, 0, 0, 1); // Î∞???Îπ®Í∞ï
//
//		float d = volumeTex.SampleLevel(samp, uvw, 0);
//		if (d <= 0.001) return float4(0, 0, 1, 1); // ?òÌîåÍ∞íÏù¥ Í±∞Ïùò ?ÜÏùå ???åÎûë
//
//
//		 d = volumeTex.SampleLevel(samp, uvw, 0);
//		float4 color = float4(d, d, d, d * Opacity);
//		acc.rgb += (1 - acc.a) * color.a * color.rgb;
//		acc.a += (1 - acc.a) * color.a;
//		if (acc.a >= 1.0) break;
//	}
//
//	return acc;
//	//return float4(1, 0, 0, 1);
//}


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

//Texture3D<float> volumeTex : register(t0);
Texture3D<min16float> volumeTex : register(t0);

SamplerState samp : register(s0);

//float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
//{
//	// ???§ÌÅ¨Î¶∞‚ÜíÎ∑∞‚Üí?îÎìú
//	float4 ndc = float4(uv * 2 - 1, 0, 1);
//	float4 viewDirVS = mul(ndc, InvProj);
//	float4 viewPos = mul(ndc, InvProj);
//	viewPos /= viewPos.w;
//	viewDirVS /= viewDirVS.w;
//
//	float3 rayDirVS = normalize(viewPos.xyz);
//	//float3 rayPosWS = CameraPosWS;
//	float3 rayPosWS = float3(0.5, 0.5, -1.0);   // Ïπ¥Î©î?ºÎ? Î≥ºÎ•® ?ûÏ™Ω??Î∞∞Ïπò
//	//float3 rayDirWS = normalize(mul(float4(rayDirVS, 0), InvView).xyz);
//	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);
//	//float3 rayDirWS = normalize(float3(0, 0, 1));
//	float3 rayPosWS = CameraPosWS;
//
//	// ???îÎìú ??Î≥ºÎ•® Î°úÏª¨
//	//rayPosWS = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	//rayDirWS = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//	// ?îÎìú ??Î°úÏª¨
//	float3 rayPosLocal = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
//	float3 rayDirLocal = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);
//
//
//	// ???àÏù¥ÎßàÏπ≠
//	float4 acc = 0;
//	[loop]
//	for (int i = 0; i < MaxSteps; i++) {
//		//float3 uvw = rayPosWS + rayDirWS * (i * Step);
//		float3 uvw = rayPosLocal + rayDirLocal * (i * Step);
//
//
//
//		/*if (any(uvw < 0.0) || any(uvw > 1.0))
//			break;*/
//		if (any(uvw < 0.0) || any(uvw > 1.0))
//			continue; // return???ÑÎãà??continue
//
//
//		float d = volumeTex.SampleLevel(samp, uvw, 0);
//		float4 color = float4(d, d, d, d * Opacity);
//		acc.rgb += (1 - acc.a) * color.a * color.rgb;
//		acc.a += (1 - acc.a) * color.a;
//		if (acc.a >= 1.0) break;
//	}
//
//	return acc;
//}

float4 TransferFunction(float d)
{
	// HU normalized °Ê [0,1]
	if (d < 0.15) return float4(0, 0, 0, 0);               // Air
	if (d < 0.35) return float4(0.7, 0.6, 0.6, 0.03);      // Soft tissue
	if (d < 0.6)  return float4(1.0, 0.85, 0.8, 0.1);      // Bone
	return float4(1.0, 1.0, 1.0, 0.2);                     // Dense bone
}

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{

	// --- ªÁ∫–«“ ∫∏¡§ ---
// ∫º∑˝∫‰∞° ¿¸√º √¢¿« ¡¬ªÛ¥‹ 1/4 øµø™¿Ã∂Û∏È:
float2 offset = float2(0.0, 0.0);   // ¡¬ªÛ¥‹
float2 scale = float2(0.5, 0.5);   // ¿¸√º¿« ¿˝π› ≈©±‚


// uv ∫∏¡§ (∫‰∆˜∆Æ ≥ª∫Œ ¡¬«•∑Œ ¡§±‘»≠)
float2 localUV = (uv - offset) / scale;


	// 1Ô∏è‚É£ ?ΩÏ? Í∏∞Ï? ?úÏïº ?àÏù¥ Í≥ÑÏÇ∞
	float4 ndc = float4(uv * 2 - 1, 1, 1);
	float4 viewDirVS = mul(ndc, InvProj);
	viewDirVS /= viewDirVS.w;
	float3 rayDirWS = normalize(mul(float4(viewDirVS.xyz, 0), InvView).xyz);

	float3 rayPosWS = CameraPosWS;













	// 2Ô∏è‚É£ ?îÎìú ??Î≥ºÎ•® Î°úÏª¨
	float3 rayPos = mul(float4(rayPosWS, 1), InvVolumeWorld).xyz;
	float3 rayDir = normalize(mul(float4(rayDirWS, 0), InvVolumeWorld).xyz);







	// Î≥ºÎ•® Í≤ΩÍ≥Ñ (0~1 Î∞ïÏä§ Í∏∞Ï?)
	float3 boxMin = float3(0, 0, 0);
	float3 boxMax = float3(1, 1, 1);

	// Ray-box ÍµêÏ∞® Í≥ÑÏÇ∞
	float3 invDir = 1.0 / rayDir;
	float3 tMin = (boxMin - rayPos) * invDir;
	float3 tMax = (boxMax - rayPos) * invDir;
	float3 t1 = min(tMin, tMax);
	float3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	// ÍµêÏ∞® ?ÜÏúºÎ©?Í≤Ä?ïÏÉâ
	if (tNear > tFar || tFar < 0)
		return float4(0, 0, 0, 1);

	// Ray ?úÏûë?êÏùÑ Î∞ïÏä§ ÏßÑÏûÖ?êÏúºÎ°??¥Îèô
	rayPos += rayDir * max(tNear, 0.0);








	// 3Ô∏è‚É£ ?ÑÏ†Å ?úÏûë
	float4 acc = 0;
	[loop]
	for (int i = 0; i < MaxSteps; i++)
	{
		float3 uvw = rayPos + rayDir * (i * Step);
		if (any(uvw < 0.0) || any(uvw > 1.0))
			break;

	float d = volumeTex.SampleLevel(samp, uvw, 0);
	///*	float windowCenter = 0.3;
	//	float windowWidth = 0.4;
	//	d = saturate((d - (windowCenter - windowWidth * 0.5)) / windowWidth);*/


	//if (d > 0.0 && d < 0.001) return float4(1, 0, 0, 1);
	//if (d >= 0.001 && d < 0.01) return float4(0, 1, 0, 1);
	//if (d >= 0.01) return float4(0, 0, 1, 1);


		d= saturate((d - 0.25) * 2.0);

		//if (d < 0.01f) discard;  // π–µµ 0.01 ¿Ã«œ∞™¿∫ ∞¯±‚∑Œ √Î±ﬁ


		float4 col = float4(d, d, d, d * Opacity);

		acc.rgb += (1 - acc.a) * col.a * col.rgb;
		acc.a += (1 - acc.a) * col.a;

		if (acc.a >= 1.0)
			break;
	}

	return acc;
}
