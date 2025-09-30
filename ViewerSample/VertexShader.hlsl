//struct VSInput
//{
//    float3 position : POSITION;
//    float2 texcoord : TEXCOORD0;
//};
//
//struct VSOutput {
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    uint rtIndex : SV_RenderTargetArrayIndex;
//};

#include "Common.hlsli"


PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0f); // 월드/뷰/프로젝션 행렬 적용 필요 시 여기에
    output.texcoord = input.texcoord;
    return output;
}

