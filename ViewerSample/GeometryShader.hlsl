
//struct VSOutput {
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    uint rtIndex : SV_RenderTargetArrayIndex;
//};
//
//// GS: 입력은 VSOutput, 출력은 GSOutput (GSOutput 에서만 SV_RenderTargetArrayIndex 사용)
//struct GSOutput {
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    uint  rtIndex   : SV_RenderTargetArrayIndex; // GS -> PS 로 보낼 때 system semantic 사용
//};

#include "Common.hlsli"


float4 ComputeViewportPosition(int index)
{
    float2 offset = float2((index % 2) * 0.5f, (index / 2) * 0.5f);
    return float4(offset * 2.0f - 1.0f, 0.0f, 1.0f); // NDC 좌표로 변환
}


[maxvertexcount(4)]
void GSMain(triangle VSOutput input[3], inout TriangleStream<GSOutput> stream)
{
    for (int i = 0; i < 4; ++i)
    {
        GSOutput o;
        o.position = ComputeViewportPosition(i);
        o.texcoord = input[0].texcoord;
        o.rtIndex = i; // 이제 SV_RenderTargetArrayIndex에 실제 값 설정
        stream.Append(o);
    }
}