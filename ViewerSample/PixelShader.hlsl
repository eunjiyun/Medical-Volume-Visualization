#include "Common.hlsli"

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D tex3 : register(t3);

SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);
SamplerState samp2 : register(s2);
SamplerState samp3 : register(s3);




PSOutput PSMain(PSInput input)
{
    PSOutput output;
    output.color0 = tex0.Sample(samp0, input.texcoord);
    output.color1 = tex1.Sample(samp1, input.texcoord);
    output.color2 = tex2.Sample(samp2, input.texcoord);
    output.color3 = tex3.Sample(samp3, input.texcoord);

    //output.color0 = float4(1.0f, 0.0f, 0.0f, 1.0f); // 빨강
    //output.color1 = float4(0.0f, 1.0f, 0.0f, 1.0f); // 초록
    //output.color2 = float4(0.0f, 0.0f, 1.0f, 1.0f); // 파랑
    //output.color3 = float4(1.0f, 1.0f, 0.0f, 1.0f); // 노랑

    return output;
}
