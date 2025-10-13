cbuffer Crosshair : register(b0)
{
    float2 crossUV;
    float crossThickness;
    float4 crossColor;
}

Texture2D texAxial : register(t0);
SamplerState sampAxial : register(s0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 PSMain_Axial(VSOutput input) : SV_Target
{
    float2 uv = input.texcoord;
    float4 base = texAxial.Sample(sampAxial, uv);

    bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;
    return isCross ? crossColor : base;
}
