//Texture2D meshTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
////cbuffer PassBuffer : register(b1)
////{
////	int renderPass; // 0 = 1단계(불투명), 1 = 2단계(반투명)
////}
//
//struct PS_OUTPUT
//{
//	float4 Accumulation : SV_Target0;
//	float Revealage : SV_Target1;
//};
//
////PS_OUTPUT  main(PS_INPUT input) : SV_TARGET
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	////////return meshTexture.Sample(samplerState, input.Tex);
//	////////// return float4(1.0f, 0.0f, 0.0f, 1.0f);  // ✅ 빨간색으로 강제 출력
//
//
//	//////float4 color = meshTexture.Sample(samplerState, input.Tex);
//
//
//	////////color.a = 0.5;   // 투명도 절반
//	////////color.a *= 0.5;
//
//
//
//	//////if (renderPass == 0) {
//	//////	// 1단계: 완전 불투명
//	//////	//color.a = 1.0;
//	//////	//color.a = 0.5;
//
//	//////	color.a = 0.7; // 0.5보다 조금 더 불투명하게
//	//////}
//	//////else {
//	//////	// 2단계: 반투명
//	//////	//color.a *= 0.5;
//
//
//	//////	color.a = 0.7; // 0.5보다 조금 더 불투명하게
//	//////}
//
//
//	////color.a = 0.5;   // 투명도 절반
//
//	////return color;
//
//
//
//
//	// PS_OUTPUT output;
//
//	//float4 color = meshTexture.Sample(samplerState, input.Tex);
//	//color.a = 0.5; // 반투명
//
//	//// ✅ Weighted Blended OIT 공식
//	//float z = input.Pos.z; // depth
//
//
//
//	////float weight = color.a * max(0.01, min(3000.0,
//	////	10.0 / (0.00001 + pow(abs(z) / 200.0, 4.0))));
//
//	//  // ✅ 강도 높이기 (depth 차이를 더 강하게)
//	//float weight = color.a * max(0.01, min(10000.0, 100.0 / (0.00001 + pow(abs(z) / 100.0, 6.0))));
//
//	//// Accumulation: 가중치 적용된 색상
//	//output.Accumulation = float4(color.rgb * color.a, color.a) * weight;
//
//	//// Revealage: 투명도
//	//output.Revealage = color.a;
//
//	//return output;
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//
//	color.a = 0.5;   // 투명도 절반
//	return color;
//
//}


//
//
//
//
//Texture2D meshTexture : register(t0);
//Texture2D prevDepthTexture : register(t1);
//SamplerState samplerState : register(s0);
//
//cbuffer MeshConstantBuffer : register(b0)
//{
//	matrix WVP;
//	int peelLayer;
//	float viewportWidth;   // ✅ 분리!
//	float viewportHeight;  // ✅ 분리!
//	float padding;
//}
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//
//
//	// 사용할 때:
//	float2 viewportSize = float2(viewportWidth, viewportHeight);
//
//	//return float4(peelLayer, peelLayer, peelLayer, 0.5f);
//
//	if (peelLayer > 0)
//	{
//		float2 screenUV = input.Pos.xy / viewportSize;
//		float prevDepth = prevDepthTexture.Sample(samplerState, screenUV).r;
//		float currentDepth = input.Pos.z;
//
//		// ✅ 미세한 epsilon
//		float epsilon = 0.00000000001;
//
//		if (currentDepth <= prevDepth + epsilon)
//		{
//			discard;
//		}
//	}
//
//	// 디버그 색상
//	if (peelLayer == 0)
//		return float4(1, 0, 0, 0.5); // 빨강
//	else if (peelLayer == 1)
//		return float4(0, 1, 0, 0.5); // 초록
//	else if (peelLayer == 2)
//		return float4(0, 0, 1, 0.5); // 파랑
//	else
//		return float4(1, 1, 0, 0.5); // 노랑
//}



//Texture2D meshTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//cbuffer ClipSettings : register(b0)
//{
//	float4 clipPlane;  // (nx, ny, nz, d)
//	int enableClip;
//	float3 padding;
//}
//
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float3 WorldPos : TEXCOORD1;
//	float2 Tex : TEXCOORD0;
//	float3 Normal : NORMAL;
//};
//
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	// ✅ Clipping
//	if (enableClip)
//	{
//		float distance = dot(clipPlane.xyz, input.WorldPos) + clipPlane.w;
//		if (distance < 0)
//		{
//			discard;
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	return color;  // ✅ 불투명!
//}


Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)  // ✅ b1 (두 번째 상수 버퍼)
{
	float4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	float3 padding;
}

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : TEXCOORD1;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
};

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	// ✅ Clipping
//	if (enableClip == 1)
//	{
//		float distance = dot(clipPlane.xyz, input.WorldPos) + clipPlane.w;
//		if (distance < 0)
//		{
//			discard;  // 평면 뒤쪽 픽셀 버리기
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	return color;
//}
float4 main(PS_INPUT input) : SV_TARGET
{
//	if (enableClip == 1)
//	{
//		float distance = dot(clipPlane.xyz, input.WorldPos) + clipPlane.w;
//
//		// ✅ 음수면 discard (잘라내기)
//		if (distance < 0)
//		{
//			discard;
//		}
//	}
//
//// 원래 텍스처 표시
//float4 color = meshTexture.Sample(samplerState, input.Tex);
//return color;


	//  // ✅ WorldPos를 색으로 표시
	//float3 pos = input.WorldPos;

	//// X, Y, Z 좌표를 색으로 (음수는 0, 양수는 스케일)
	//float r = (pos.x + 100) / 200;  // X: -100~100 → 0~1
	//float g = (pos.y + 100) / 200;  // Y: -100~100 → 0~1
	//float b = (pos.z + 100) / 200;  // Z: -100~100 → 0~1

	//return float4(r, g, b, 1);




	/*if (enableClip == 1)
	{
		float distance = dot(clipPlane.xyz, input.WorldPos) + clipPlane.w;

		if (distance < 0)
		{
			discard;
		}
	}

	float4 color = meshTexture.Sample(samplerState, input.Tex);
	return color;*/



if (enableClip == 1)
	{
		float distance = dot(clipPlane.xyz, input.WorldPos) + clipPlane.w;
		if (distance < 0)
		{
			discard;
		}
	}

	float4 color = meshTexture.Sample(samplerState, input.Tex);
	color.a = 0.5;
	return color;
}