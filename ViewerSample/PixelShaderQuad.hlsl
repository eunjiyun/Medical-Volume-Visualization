#include "Common.hlsli"

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 PSMain(PSInput input) : SV_Target
{
    return tex.Sample(samp, input.texcoord);
}
