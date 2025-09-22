cbuffer ColorBuffer : register(b0)
{
    float4 quadColor;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return quadColor; // 색상만 출력
}
