// Common.hlsli

struct VSInput {
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

//struct VSOutput {
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    uint   rtIndex  : TEXCOORD1;
//};

//struct GSOutput {
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    uint   rtIndex  : SV_RenderTargetArrayIndex;
//};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;

};

struct PSOutput {
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
    float4 color2 : SV_Target2;
    float4 color3 : SV_Target3;
};

