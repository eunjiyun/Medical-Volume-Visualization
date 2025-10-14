//struct VSInput
//{
//    float3 position : POSITION;
//    float2 texcoord : TEXCOORD;
//};
//
//struct VSOutput
//{
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD;
//};
//
//VSOutput VSMain(VSInput input)
//{
//    VSOutput output;
//    output.position = float4(input.position, 1.0f);
//    output.texcoord = input.texcoord;
//    return output;
//}
struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD; // 원본 전체 화면 UV
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv_axial   : TEXCOORD0; // 뷰포트0 UV
    float2 uv_coronal : TEXCOORD1; // 뷰포트1 UV
    float2 uv_sagittal: TEXCOORD2; // 뷰포트2 UV
    float2 uv_mixview : TEXCOORD3; // 뷰포트3 UV
};

VSOutput VSMain(VSInput input)
{
    //VSOutput output;
    //output.position = float4(input.position, 1.0f);

    //// 화면 전체 UV에서 각 뷰포트 UV 계산
    //float2 fullUV = input.texcoord;

    //// 화면을 2x2로 나누었다고 가정
    //float2 half = float2(0.5, 0.5);

    //// Top-left 뷰포트 (Axial)
    //output.uv_mixview = fullUV * half;

    //// Top-right 뷰포트 (Coronal)
    //output.uv_axial = float2(fullUV.x * half.x + half.x, fullUV.y * half.y);

    //// Bottom-left 뷰포트 (Sagittal)
    //output.uv_coronal = float2(fullUV.x * half.x, fullUV.y * half.y + half.y);

    //// Bottom-right 뷰포트 (Mix)
    //output.uv_sagittal = fullUV * half + half;

    //return output;
    VSOutput output;
    output.position = float4(input.position, 1.0f);

    // 화면 전체 UV 그대로 전달
    output.uv_mixview = input.texcoord;
    output.uv_axial = input.texcoord;
    output.uv_coronal = input.texcoord;
    output.uv_sagittal = input.texcoord;

    return output;
}
