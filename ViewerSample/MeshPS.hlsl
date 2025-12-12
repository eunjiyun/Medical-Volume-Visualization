Texture2D meshTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ClipSettings : register(b1)
{
	float4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	float3 padding;
};

// ========== ✨ 추가: Camera 버퍼 ==========
cbuffer CameraBuffer : register(b2)
{
	float3 CameraPos;       // 카메라 위치
	float padding2;
};

// ========== ✨ 추가: Light 버퍼 ==========
cbuffer LightBuffer : register(b3)
{
	float3 LightDir;        // 빛 방향 (예: (0.5, -0.7, -0.5))
	float padding3;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : TEXCOORD0;
	float2 Tex : TEXCOORD1;

	float3 WorldPos : TEXCOORD2;    // ✨ 추가
	float3 Normal : TEXCOORD3;      // ✨ 추가
};



float4 main(PS_INPUT input) : SV_TARGET
{
	if (enableClip == 1)
	{
		// ✅ 파랑(z > 3.5) 자르기
		if (input.ViewPos.z > 3.0)
		{
			discard;

			//return float4(1, 0, 0, 1);  // 빨강 = 잘릴 부분
		}
		
	}

	float4 color = meshTexture.Sample(samplerState, input.Tex);


	// ========== ✨ Fresnel 디버깅 ==========
	float3 viewDir = normalize(CameraPos - input.WorldPos);
	float3 normal = normalize(input.Normal);
	float fresnel = 1.0 - saturate(dot(normal, viewDir));

	// ========== ✨ Fresnel 시각화 (임시) ==========
	return float4(fresnel, fresnel, fresnel, 1.0);
	// 정면: 검정 (fresnel = 0)
	// 측면: 흰색 (fresnel = 1)

	//여기

	// ========== ✨ 추가: Fresnel Effect ==========
// 1. View Direction 계산
	float3 viewDir = normalize(CameraPos - input.WorldPos);

	// 2. Normal 정규화
	float3 normal = normalize(input.Normal);

	// 3. Fresnel 값 계산
	float fresnel = 1.0 - saturate(dot(normal, viewDir));

	// 4. Alpha 조절 (정면: 0.4, 측면: 0.9)
	float baseAlpha = 0.65;
	float finalAlpha = lerp(baseAlpha, 0.85, pow(fresnel, 2.0));

	// ========== ✨ 추가: Lighting ==========
// 5. Ambient (기본 밝기)
	//float3 ambient = float3(0.35, 0.35, 0.35);
	// 수정
	float3 ambient = float3(0.6, 0.6, 0.6);  // 60%로 올림

	// 6. Diffuse (빛 받는 정도)
	float ndotl = saturate(dot(normal, normalize(-LightDir)));
	//float3 diffuse = ndotl * float3(0.65, 0.65, 0.65);
	// 수정
	float3 diffuse = ndotl * float3(0.8, 0.8, 0.8);  // 65% → 80%

	//// 7. 최종 라이팅 적용
	//color.rgb *= (ambient + diffuse);

	// 추가: 전체적으로 밝게
	color.rgb *= 1.3;  // 1.5배 밝게

	// ========== ✨ 수정: Alpha 적용 ==========
	color.a = finalAlpha;  // 기존 0.65 대신 Fresnel 기반

	return color;
}

