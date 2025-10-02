//Texture2D tex : register(t0);
//SamplerState samp : register(s0);
//
//float4 PSMain(float2 uv : TEXCOORD) : SV_Target
//{
//	return tex.Sample(samp, uv);
//
//	//return float4(1, 0, 0, 1); // ??ëª¿è€???ê³—ë®†??//
//}


//Texture2D g_texture : register(t0);
//SamplerState g_sampler : register(s0);
//
//struct PS_INPUT {
//	float4 pos : SV_POSITION;
//	float2 tex : TEXCOORD0;
//};
//
//float4 PSMain(PS_INPUT input) : SV_TARGET{
//	return g_texture.Sample(g_sampler, input.tex);
//// return float4(1,0,0,1);  // ??????ê»ƒì¹°???ë¤¿ì„ ???±ëªµç­???œë˜»??Žì³ž???ëª¿è€?
//}


//Texture2D tex0 : register(t0);
//SamplerState samp0 : register(s0);
//
//float4 PSMain(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
//{
//	return tex0.Sample(samp0, uv);
//}


cbuffer Crosshair : register(b0)
{
    float2 cross0;         // tex0¿ë ½ÊÀÚ¼± À§Ä¡
    float2 cross1;         // tex1¿ë
    float2 cross2;
    float2 cross3;
    float crossThickness;  // ¼± µÎ²² (¿¹: 0.002)
    float4 crossColor;     // ½ÊÀÚ¼± »ö»ó (¿¹: »¡°­ float4(1,0,0,1))
}


Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D tex3 : register(t3);

SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);
SamplerState samp2 : register(s2);
SamplerState samp3 : register(s3);




struct PSOutput {
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
    float4 color2 : SV_Target2;
    float4 color3 : SV_Target3;
};




struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSOutput PSMain(VSOutput input)
{
    //PSOutput o;


    //o.color0 = tex0.Sample(samp0, input.texcoord);
    //o.color1 = tex1.Sample(samp1, input.texcoord);
    //o.color2 = tex2.Sample(samp2, input.texcoord);
    //o.color3 = tex3.Sample(samp3, input.texcoord);



 //   /*float gray0 = tex0.Sample(samp0, input.texcoord).r;
 //   float gray1 = tex1.Sample(samp1, input.texcoord).r;
 //   float gray2 = tex2.Sample(samp2, input.texcoord).r;
 //   float gray3 = tex3.Sample(samp3, input.texcoord).r;

 //   o.color0 = float4(gray0, gray0, gray0, 1.0);
 //   o.color1 = float4(gray1, gray1, gray1, 1.0);
 //   o.color2 = float4(gray2, gray2, gray2, 1.0);
 //   o.color3 = float4(gray3, gray3, gray3, 1.0);*/



    //return o;



    PSOutput o;

    float2 uv = input.texcoord;

    // °¢ ÅØ½ºÃ³ »ùÇÃ¸µ
    float4 base0 = tex0.Sample(samp0, uv);
    float4 base1 = tex1.Sample(samp1, uv);
    float4 base2 = tex2.Sample(samp2, uv);
    float4 base3 = tex3.Sample(samp3, uv);

    // ½ÊÀÚ¼± Á¶°Ç
    bool isCross0 = abs(uv.x - cross0.x) < crossThickness || abs(uv.y - cross0.y) < crossThickness;
    bool isCross1 = abs(uv.x - cross1.x) < crossThickness || abs(uv.y - cross1.y) < crossThickness;
    bool isCross2 = abs(uv.x - cross2.x) < crossThickness || abs(uv.y - cross2.y) < crossThickness;
    bool isCross3 = abs(uv.x - cross3.x) < crossThickness || abs(uv.y - cross3.y) < crossThickness;

    // ½ÊÀÚ¼± Æ÷ÇÔ »ö»ó Ãâ·Â
    o.color0 = isCross0 ? crossColor : base0;
    o.color1 = isCross1 ? crossColor : base1;
    o.color2 = isCross2 ? crossColor : base2;
    o.color3 = isCross3 ? crossColor : base3;

    return o;

}



//float box(float2 uv, float2 center, float2 size)
//{
//	float2 diff = abs(uv - center);
//	return step(diff.x, size.x) * step(diff.y, size.y);
//}
//
//PSOutput PSMain(VSOutput input)
//{
//	PSOutput o;
//
//	float2 uv = input.texcoord;
//
//	o.color0 = float4(box(uv, float2(0.5, 0.5), float2(0.2, 0.1)), 0, 0, 1); // ??ëª¿è€??„ì…ë²??//	o.color1 = float4(0, box(uv, float2(0.3, 0.3), float2(0.1, 0.1)), 0, 1); // ?Î»?„ä»¥??„ì…ë²??//	o.color2 = float4(0, 0, box(uv, float2(0.7, 0.7), float2(0.15, 0.15)), 1); // ??????„ì…ë²??//	o.color3 = float4(box(uv, float2(0.2, 0.8), float2(0.05, 0.05))); // ?ï§ê»‹???????//
//	return o;
//}
