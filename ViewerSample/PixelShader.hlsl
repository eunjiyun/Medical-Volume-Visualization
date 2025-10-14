
cbuffer Crosshair : register(b0)
{
    float2 crossUVs[4];     // 0: Mix, 1: Axial, 2: Coronal, 3: Sagittal
    float crossThickness;
    float4 crossColor;
}

SamplerState samp0 : register(s0);
Texture2D tex[4] : register(t0);

struct VSOutput
{
    float4 position      : SV_POSITION;
    float2 uv_mix        : TEXCOORD0;
    float2 uv_axial      : TEXCOORD1;
    float2 uv_coronal    : TEXCOORD2;
    float2 uv_sagittal   : TEXCOORD3;
};

struct PSOutput
{
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
    float4 color2 : SV_Target2;
    float4 color3 : SV_Target3;
};

PSOutput PSMain(VSOutput input)
{
    PSOutput o;

    // MRT용 텍스처 샘플링
    float4 mix = tex[0].Sample(samp0, input.uv_mix);
    float4 axial = tex[1].Sample(samp0, input.uv_axial);
    float4 coronal = tex[2].Sample(samp0, input.uv_coronal);
    float4 sagittal = tex[3].Sample(samp0, input.uv_sagittal);

    // 십자선 판단
    bool cross_mix = abs(input.uv_mix.x - crossUVs[0].x) < crossThickness ||
        abs(input.uv_mix.y - crossUVs[0].y) < crossThickness;

    bool cross_axial = abs(input.uv_axial.x - crossUVs[1].x) < crossThickness ||
        abs(input.uv_axial.y - crossUVs[1].y) < crossThickness;

    bool cross_coronal = abs(input.uv_coronal.x - crossUVs[2].x) < crossThickness ||
        abs(input.uv_coronal.y - crossUVs[2].y) < crossThickness;

    bool cross_sagittal = abs(input.uv_sagittal.x - crossUVs[3].x) < crossThickness ||
        abs(input.uv_sagittal.y - crossUVs[3].y) < crossThickness;

    // MRT 출력
    o.color0 = cross_mix ? crossColor : mix;
    o.color1 = cross_axial ? crossColor : axial;
    o.color2 = cross_coronal ? crossColor : coronal;
    o.color3 = cross_sagittal ? crossColor : sagittal;

    return o;
}



//cbuffer Crosshair : register(b0)
//{
//    float2 crossUV[4];         // 십자선 위치 (0~1)
//    float crossThickness;   // 선 두께
//    float4 crossColor;      // 십자선 색상
//}
//
//cbuffer ViewInfo : register(b1)
//{
//    int viewIndex;          // 0: Axial, 1: Coronal, 2: Sagittal
//}
//
//
//SamplerState samp0 : register(s0);
//SamplerState samp1 : register(s1);
//SamplerState samp2 : register(s2);
//SamplerState samp3 : register(s3);
//
//Texture2D tex[4] : register(t0);       // tex[0] = Axial, tex[1] = Coronal, tex[2] = Sagittal
//
//
//struct PSOutput {
//    float4 color0 : SV_Target0;
//    float4 color1 : SV_Target1;
//    float4 color2 : SV_Target2;
//    float4 color3 : SV_Target3;
//};
//
//
//
//
//struct VSOutput
//{
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD;
//};
//
//PSOutput PSMain(VSOutput input)
//{
//
//
//    PSOutput o;
//    float2 uv = input.texcoord;
//
//    // 텍스처 샘플링
//    float4 base0 = tex[0].Sample(samp0, uv);
//    float4 base1 = tex[1].Sample(samp1, uv);
//    float4 base2 = tex[2].Sample(samp2, uv);
//    float4 base3 = tex[3].Sample(samp3, uv);
//
//    // 각 뷰별로 개별 crossUV 적용
//    bool isCross0 = abs(uv.x - crossUVs[0].x) < crossThickness || abs(uv.y - crossUVs[0].y) < crossThickness;
//    bool isCross1 = abs(uv.x - crossUVs[1].x) < crossThickness || abs(uv.y - crossUVs[1].y) < crossThickness;
//    bool isCross2 = abs(uv.x - crossUVs[2].x) < crossThickness || abs(uv.y - crossUVs[2].y) < crossThickness;
//    bool isCross3 = abs(uv.x - crossUVs[3].x) < crossThickness || abs(uv.y - crossUVs[3].y) < crossThickness;
//
//    o.color0 = isCross0 ? crossColor : base0;
//    o.color1 = isCross1 ? crossColor : base1;
//    o.color2 = isCross2 ? crossColor : base2;
//    o.color3 = isCross3 ? crossColor : base3;
//
//    return o;
//
//}


//cbuffer Crosshair : register(b0)
//{
//    float2 crossUVs[4];
//    float crossThickness;
//    float4 crossColor;
//};
//
//SamplerState samp0 : register(s0);
//Texture2D tex[4]   : register(t0);
//
//struct VSOutput
//{
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD;
//};
//
//struct PSOutput
//{
//    float4 color0 : SV_Target0;
//    float4 color1 : SV_Target1;
//    float4 color2 : SV_Target2;
//    float4 color3 : SV_Target3;
//};
//
//PSOutput PSMain(VSOutput input)
//{
//    PSOutput o;
//    float2 uv = input.texcoord;
//
//    float4 base0 = tex[0].Sample(samp0, uv);
//    float4 base1 = tex[1].Sample(samp0, uv);
//    float4 base2 = tex[2].Sample(samp0, uv);
//    float4 base3 = tex[3].Sample(samp0, uv);
//
//    // bool 방식으로 십자선 판단
//    bool isCross0 = abs(uv.x - crossUVs[0].x) < crossThickness || abs(uv.y - crossUVs[0].y) < crossThickness;
//    bool isCross1 = abs(uv.x - crossUVs[1].x) < crossThickness || abs(uv.y - crossUVs[1].y) < crossThickness;
//    bool isCross2 = abs(uv.x - crossUVs[2].x) < crossThickness || abs(uv.y - crossUVs[2].y) < crossThickness;
//    bool isCross3 = abs(uv.x - crossUVs[3].x) < crossThickness || abs(uv.y - crossUVs[3].y) < crossThickness;
//
//    // 선명하게 덮어쓰기
//    o.color0 = isCross0 ? crossColor : base0;
//
//    o.color0 = float4(crossUVs[0].x, crossUVs[0].x, crossUVs[0].x, 1.0);
//    o.color0 = float4(crossUVs[0].x, crossUVs[0].x, crossUVs[0].x, 1.0);
//    o.color0 = float4(crossUVs[0].x, crossUVs[0].x, crossUVs[0].x, 1.0);
//    o.color0 = float4(crossUVs[0].x, crossUVs[0].x, crossUVs[0].x, 1.0);
//
//
//    o.color1 = isCross1 ? crossColor : base1;
//    o.color2 = isCross2 ? crossColor : base2;
//    o.color3 = isCross3 ? crossColor : base3;
//
//    return o;
//}
