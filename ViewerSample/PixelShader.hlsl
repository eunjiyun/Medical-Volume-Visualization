
cbuffer Crosshair : register(b0)
{
    float2 crossUV;        
    float crossThickness;   
	//float windowCenter;
	//float windowWidth;
	//float padding1;  // 16바이트 정렬
	//float padding2;

	float sharpness;        // ⭐ 추가
	float4 crossColor;  

	//float2 textureSize;  // ⭐ 텍스처 크기 추가
	//float2 padding;
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

 //   PSOutput o;

	////// ⭐ sharpness 값 시각화 (임시 테스트)
	////o.color0 = float4(sharpness, sharpness, sharpness, 1.0);
	////return o;

 //   float2 uv = input.texcoord;
 //   float4 base = tex.Sample(samp0, uv);


	//// ⭐ Sharpness 적용
	//if (sharpness > 0.01) {
	//	float2 ts = float2(1.0 / 794.0, 1.0 / 794.0);  // 텍스처 크기에 맞게 조정
	//	float4 blur = (
	//		tex.Sample(samp0, uv + float2(-1, -1)*ts) +
	//		tex.Sample(samp0, uv + float2(0, -1)*ts) +
	//		tex.Sample(samp0, uv + float2(1, -1)*ts) +
	//		tex.Sample(samp0, uv + float2(-1, 0)*ts) +
	//		tex.Sample(samp0, uv + float2(1, 0)*ts) +
	//		tex.Sample(samp0, uv + float2(-1, 1)*ts) +
	//		tex.Sample(samp0, uv + float2(0, 1)*ts) +
	//		tex.Sample(samp0, uv + float2(1, 1)*ts)
	//		) / 8.0;
	//	base = base + (base - blur) * sharpness;
	//}



 //   bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;

	////o.color0 = isCross ? crossColor : base;

	//// 파란색으로 고정: R=0, G=0, B=1, A=1
	//o.color0 = isCross ? float4(0.0, 1.0, 0.0, 1.0) : base;


 //   return o;



	PSOutput o;
	float2 uv = input.texcoord;

	// ⭐ 텍스처 크기 자동으로 구하기
	float width, height;
	tex.GetDimensions(width, height);

	float4 base = tex.Sample(samp0, uv);

	// ⭐ Sharpness 적용
	if (sharpness > 0.01) {
		float2 ts = 1.0 / float2(width, height);

		float4 blur = (
			tex.Sample(samp0, uv + float2(-1, -1)*ts) +
			tex.Sample(samp0, uv + float2(0, -1)*ts) +
			tex.Sample(samp0, uv + float2(1, -1)*ts) +
			tex.Sample(samp0, uv + float2(-1, 0)*ts) +
			tex.Sample(samp0, uv + float2(1, 0)*ts) +
			tex.Sample(samp0, uv + float2(-1, 1)*ts) +
			tex.Sample(samp0, uv + float2(0, 1)*ts) +
			tex.Sample(samp0, uv + float2(1, 1)*ts)
			) / 8.0;

		base = base + (base - blur) * sharpness*3.0;
	}

	bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;
	o.color0 = isCross ? float4(0.0, 1.0, 0.0, 1.0) : base;

	return o;
}
