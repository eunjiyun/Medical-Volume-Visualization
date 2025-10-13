cbuffer Crosshair : register(b0)
{
    float2 crossUV;
    float crossThickness;
    float4 crossColor;
}

Texture2D texSagittal : register(t0);
SamplerState sampSagittal : register(s0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 PSMain_Sagittal(VSOutput input) : SV_Target
{
    float2 uv = input.texcoord;
    float4 base = texSagittal.Sample(sampSagittal, uv);

    bool isCross = abs(uv.x - crossUV.x) < crossThickness || abs(uv.y - crossUV.y) < crossThickness;
    return isCross ? crossColor : base;
}
