Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)
{
	float4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	float3 padding;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		// ✅ ViewPos 사용
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		if (dist < 0)
//		{
//			discard;
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}
//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		// ✅ dist 값을 색으로 표시
//		if (dist < -1.0)
//			return float4(1, 0, 0, 1);  // 빨강: 많이 뒤
//		else if (dist < 0)
//			return float4(1, 1, 0, 1);  // 노랑: 약간 뒤
//		else if (dist < 1.0)
//			return float4(0, 1, 0, 1);  // 초록: 약간 앞
//		else
//			return float4(0, 0, 1, 1);  // 파랑: 많이 앞
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		// ✅ 디버깅
//		if (dist < -2.0) return float4(1, 0, 0, 1);
//		if (dist < -1.0) return float4(1, 1, 0, 1);
//		if (dist < 0) return float4(0, 1, 0, 1);
//		if (dist < 1.0) return float4(0, 1, 1, 1);
//		return float4(0, 0, 1, 1);
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		if (dist > 0)  // ✅ dist > 0 자르기
//		{
//			discard;
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		// ✅ 잘릴 부분을 빨간색으로 표시 (discard 대신)
//		if (dist > 0)
//		{
//			return float4(1, 0, 0, 1);  // 빨강 = 잘릴 부분
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float dist = dot(clipPlane.xyz, input.ViewPos) + clipPlane.w;
//
//		// ✅ dist 값 전체 범위 시각화
//		float normalized = (dist + 2.0) / 4.0;  // -2~2 → 0~1
//		return float4(normalized, normalized, normalized, 1);
//		// 검정 = -2, 회색 = 0, 하얀색 = +2
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		float z = input.ViewPos.z;
//
//		// ✅ 더 세밀한 범위
//		if (z < 0.0) return float4(1, 0, 1, 1);  // 보라: 음수?
//		if (z < 0.1) return float4(1, 0, 0, 1);  // 빨강
//		if (z < 0.2) return float4(1, 1, 0, 1);  // 노랑
//		if (z < 0.3) return float4(0, 1, 0, 1);  // 초록
//		if (z < 0.5) return float4(0, 1, 1, 1);  // 청록
//		return float4(0, 0, 1, 1);  // 파랑
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}


//float4 main(PS_INPUT input) : SV_TARGET
//{
//	// ✅ ViewPos를 RGB로 출력
//	return float4(
//		abs(input.ViewPos.x) * 10,
//		abs(input.ViewPos.y) * 10,
//		abs(input.ViewPos.z) * 10,
//		1
//	);
//}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	if (enableClip == 1)
//	{
//		// ✅ View Z > 0.15 자르기
//		if (input.ViewPos.z > 0.15)
//		{
//			discard;
//		}
//	}
//
//	float4 color = meshTexture.Sample(samplerState, input.Tex);
//	color.a = 0.5;
//	return color;
//}

float4 main(PS_INPUT input) : SV_TARGET
{
	if (enableClip == 1)
	{
		float z = input.ViewPos.z;

		// ✅ 더 큰 범위
		if (z < 2.1) return float4(1, 0, 0, 1);  // 빨강
		if (z < 2.5) return float4(1, 1, 0, 1);  // 노랑
		if (z < 3.0) return float4(0, 1, 0, 1);  // 초록
		if (z < 3.4) return float4(0, 1, 1, 1);  // 청록
		return float4(0, 0, 1, 1);  // 파랑
	}

	float4 color = meshTexture.Sample(samplerState, input.Tex);
	color.a = 0.5;
	return color;
}