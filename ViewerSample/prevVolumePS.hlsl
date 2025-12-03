
Texture2D sliceTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer VolumeConstants : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 color;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 worldPos : WORLDPOS;
};

float4 PSVolume(VS_OUTPUT input) : SV_TARGET
{
	float gray = sliceTexture.Sample(samplerState, input.texcoord).r;

// === HU ?�도???�벨 (조금 ????? 중심) ===
float windowCenter = 0.32; // ?�두??중심?�로
float windowWidth = 0.5;  // ?�짝 좁힘
gray = saturate((gray - (windowCenter - windowWidth * 0.5)) / windowWidth);

// === 명암�?강화 (???�렷?�게) ===
gray = pow(gray, 1.6);

// === ?�상 보간 (?�뜻???�렌지 계열) ===
float3 darkTone = float3(0.25, 0.18, 0.12);
float3 midTone = float3(0.8, 0.6, 0.45);
float3 lightTone = float3(1.0, 0.85, 0.65);
float3 finalColor = lerp(darkTone, lightTone, gray);
finalColor = lerp(finalColor, midTone, 0.25); // ?�간 중간 ???�기

float alpha = smoothstep(0.3, 0.7, gray); 
alpha = pow(alpha, 2.0) * 0.65;           

// === 감마 보정 ===
finalColor = pow(finalColor, 1.0 / 1.8);

return float4(finalColor, alpha);
}
