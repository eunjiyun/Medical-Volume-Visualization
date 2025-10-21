
cbuffer Crosshair : register(b0)
{
    float2 crossUV;         // 십자선 위치 (0~1)
    float crossThickness;   // 선 두께
    float4 crossColor;      // 십자선 색상
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

	o.color0 = isCross ? crossColor : base;

    return o;
}
