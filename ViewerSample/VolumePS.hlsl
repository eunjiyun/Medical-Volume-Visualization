// Pixel Shader 
Texture2D sliceTexture : register(t0);
SamplerState samplerState : register(s0);

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 worldPos : WORLDPOS;
};


float4 PSMain(VS_OUTPUT input) : SV_TARGET{
	////단순 텍스처 샘플링 (슬라이스 평면용)
	//float4 color = sliceTexture.Sample(samplerState, input.texcoord);
	//return color;



	//   return float4(input.texcoord, 0.0f, 1.0f); // 텍스처 좌표 시각화

	////return float4(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색

	   return float4(abs(input.texcoord.x), abs(input.texcoord.y), 0.0f, 1.0f);
}

//float4 PSMain(VS_OUTPUT input) : SV_TARGET{
//	return float4(abs(input.texcoord.x), abs(input.texcoord.y), 0.0f, 1.0f);
//}