
cbuffer Crosshair : register(b0)
{
	float2 crossUV;
	float crossThickness;

	float sharpness;        // ⭐ 추가
	float4 crossColor;
}

SamplerState samp0 : register(s0);
Texture2D tex : register(t0);       // tex[0] = Axial, tex[1] = Coronal, tex[2] = Sagittal

struct PSOutput {
	float4 color0 : SV_Target0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

PSOutput PSMain(VSOutput input) 
{
	PSOutput o;
	float2 uv = input.texcoord;
	float4 base = tex.Sample(samp0, uv);

	//  Sharpness 적용 (5x5 커널)
	if (sharpness > 0.01) {
		float width, height;
		tex.GetDimensions(width, height);
		float2 ts = 1.0 / float2(width, height);

		// 5x5 가우시안 블러 (가중치 적용)
		float4 blur = float4(0, 0, 0, 0);

		// 중심에서 거리에 따른 가중치
		// 1  4  6  4  1
		// 4 16 24 16  4
		// 6 24 36 24  6
		// 4 16 24 16  4
		// 1  4  6  4  1
		// 총합 = 256




		blur += tex.Sample(samp0, uv + float2(-2, -2)*ts) * 1.0;
		blur += tex.Sample(samp0, uv + float2(-1, -2)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(0, -2)*ts) * 6.0;
		blur += tex.Sample(samp0, uv + float2(1, -2)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(2, -2)*ts) * 1.0;

		blur += tex.Sample(samp0, uv + float2(-2, -1)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(-1, -1)*ts) * 16.0;
		blur += tex.Sample(samp0, uv + float2(0, -1)*ts) * 24.0;
		blur += tex.Sample(samp0, uv + float2(1, -1)*ts) * 16.0;
		blur += tex.Sample(samp0, uv + float2(2, -1)*ts) * 4.0;

		blur += tex.Sample(samp0, uv + float2(-2, 0)*ts) * 6.0;
		blur += tex.Sample(samp0, uv + float2(-1, 0)*ts) * 24.0;
		blur += tex.Sample(samp0, uv + float2(0, 0)*ts) * 36.0;
		blur += tex.Sample(samp0, uv + float2(1, 0)*ts) * 24.0;
		blur += tex.Sample(samp0, uv + float2(2, 0)*ts) * 6.0;

		blur += tex.Sample(samp0, uv + float2(-2, 1)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(-1, 1)*ts) * 16.0;
		blur += tex.Sample(samp0, uv + float2(0, 1)*ts) * 24.0;
		blur += tex.Sample(samp0, uv + float2(1, 1)*ts) * 16.0;
		blur += tex.Sample(samp0, uv + float2(2, 1)*ts) * 4.0;

		blur += tex.Sample(samp0, uv + float2(-2, 2)*ts) * 1.0;
		blur += tex.Sample(samp0, uv + float2(-1, 2)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(0, 2)*ts) * 6.0;
		blur += tex.Sample(samp0, uv + float2(1, 2)*ts) * 4.0;
		blur += tex.Sample(samp0, uv + float2(2, 2)*ts) * 1.0;

		blur /= 256.0;  // 가중치 총합으로 나누기

		// Unsharp mask
		base = base + (base - blur) * sharpness*3.0;
	}

	bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;
	o.color0 = isCross ? float4(0.0, 0.0, 1.0, 1.0) : base;

	return o;
}
