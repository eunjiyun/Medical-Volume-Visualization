
cbuffer Crosshair : register(b0)
{
    float2 crossUV;        
    float crossThickness;   
	//float windowCenter;
	//float windowWidth;
	//float padding1;  // 16바이트 정렬
	//float padding2;
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
    bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;

	//o.color0 = isCross ? crossColor : base;

	// 파란색으로 고정: R=0, G=0, B=1, A=1
	o.color0 = isCross ? float4(0.0, 0.0, 1.0, 1.0) : base;


    return o;
}
