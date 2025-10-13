cbuffer Crosshair : register(b0)
{
    float2 crossUV;
    float crossThickness;
    float4 crossColor;
}

Texture2D texCoronal : register(t0);
SamplerState sampCoronal : register(s0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 PSMain_Coronal(VSOutput input) : SV_Target
{
    float2 uv = input.texcoord;
    float4 base = texCoronal.Sample(sampCoronal, uv);

    bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;
    return isCross ? crossColor : base;
}
