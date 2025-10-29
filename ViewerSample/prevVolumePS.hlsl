//Texture2D sliceTexture : register(t0);
//SamplerState sliceSampler : register(s0);
//
//cbuffer VolumeConstants : register(b0)
//{
//	matrix World;
//	matrix View;
//	matrix Projection;
//	float4 color;
//};
//
//struct VS_OUTPUT {
//	float4 position : SV_POSITION;
//	float2 texcoord : TEXCOORD;
//};
//
//float4 PSVolume(VS_OUTPUT input) : SV_TARGET
//{
//	float4 texColor = sliceTexture.Sample(sliceSampler, input.texcoord);
//	texColor.a *= color.a;   // C++?�서 지?�한 ?�파 반영
//	return texColor;
//}


//Texture2D sliceTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//cbuffer VolumeConstants : register(b0)
//{
//	matrix World;
//	matrix View;
//	matrix Projection;
//	float4 color;
//};
//
//struct VS_OUTPUT {
//	float4 position : SV_POSITION;
//	float2 texcoord : TEXCOORD;
//	float3 worldPos : WORLDPOS;
//};
//
//float4 PSVolume(VS_OUTPUT input) : SV_TARGET
//{
//	// ?�플�?
//	float gray = sliceTexture.Sample(samplerState, input.texcoord).r;
//
//// 명암�?강화 ??중간값을 ?�게 밀?�줌
//gray = saturate((gray - 0.08) * 3.2);
//
//// ?�두??갈색 ??밝�? ?�렌지 ??
//float3 darkTone = float3(0.22, 0.16, 0.10);
//float3 lightTone = float3(1.0, 0.82, 0.6);
//float3 finalColor = lerp(darkTone, lightTone, gray);
//
//// ?�명??좀 ??강하�?
//float alpha = pow(gray, 2.0) * 0.6;
//
//// 감마 ?�짝 (???�뜻?�게)
//finalColor = pow(finalColor, 1.0 / 1.9);
//
//return float4(finalColor, alpha);
//}


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

// === ?�명?? ?�한 밀??soft tissue)???�전 ?�명, 뼈는 ?�렷 ===
float alpha = smoothstep(0.3, 0.7, gray); // 0.3?�하 ?�전 ?�명, 0.7?�상 ?�전 불투�?
alpha = pow(alpha, 2.0) * 0.65;           // ?�체 ?�적 ?�명??조절

// === 감마 보정 ===
finalColor = pow(finalColor, 1.0 / 1.8);

return float4(finalColor, alpha);
}

//Texture2D sliceTexture : register(t0);
//SamplerState samplerState : register(s0);
//
//cbuffer VolumeConstants : register(b0)
//{
//	matrix World;
//	matrix View;
//	matrix Projection;
//	float4 color;
//};
//
//struct VS_OUTPUT {
//	float4 position : SV_POSITION;
//	float2 texcoord : TEXCOORD;
//	float3 worldPos : WORLDPOS;
//};
//
//float4 PSVolume(VS_OUTPUT input) : SV_TARGET
//{
//	float gray = sliceTexture.Sample(samplerState, input.texcoord).r;
//
//// === HU ?�도???�벨 조정 ===
//float windowCenter = 0.30;
//float windowWidth = 0.48;
//gray = saturate((gray - (windowCenter - windowWidth * 0.5)) / windowWidth);
//gray = pow(gray, 1.8);
//
//// === ?�상 매핑 (?�뜻???�렌지 ?? ===
//float3 darkTone = float3(0.28, 0.18, 0.10);
//float3 midTone = float3(0.92, 0.65, 0.40);
//float3 lightTone = float3(1.0, 0.82, 0.55);
//float3 baseColor = lerp(darkTone, lightTone, gray);
//float3 finalColor = lerp(baseColor, midTone, 0.3);
//
//// === Lambert 조명 (밝기 보정 ?�함) ===
//float3 dx = ddx(input.worldPos);
//float3 dy = ddy(input.worldPos);
//float3 normal = normalize(cross(dx, dy));
//
//// 광원 방향 (?�면+??
//float3 lightDir = normalize(float3(-0.2, 0.6, -0.5));
//float diffuse = saturate(dot(normal, lightDir));
//
//// 조명 밸런??조정
//diffuse = diffuse * 0.6 + 0.5;  // 기본 밝기 0.5, 조명 ?�과 60%
//
//finalColor *= diffuse;
//
//// === ?�명??===
//float alpha = smoothstep(0.5, 0.85, gray);
//alpha = pow(alpha, 2.3) * 0.4;
//
//// === 감마 보정 ===
//finalColor = pow(finalColor, 1.0 / 1.8);
//
//return float4(finalColor, alpha);
//}
