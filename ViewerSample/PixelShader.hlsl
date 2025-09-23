//Texture2D tex : register(t0);
//SamplerState samp : register(s0);
//
//float4 PSMain(float2 uv : TEXCOORD) : SV_Target
//{
//	return tex.Sample(samp, uv);
//
//	//return float4(1, 0, 0, 1); // 빨간색 출력
//
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
//// return float4(1,0,0,1);  // ← 이렇게 되어있으면 무조건 빨강
//}


//Texture2D tex0 : register(t0);
//SamplerState samp0 : register(s0);
//
//float4 PSMain(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
//{
//	return tex0.Sample(samp0, uv);
//}

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

//PSOutput PSMain(float2 uv : TEXCOORD)
//{
//	PSOutput o;
//	o.color0 = tex0.Sample(samp0, uv);
//	o.color1 = tex1.Sample(samp1, uv);
//	o.color2 = tex2.Sample(samp2, uv);
//	o.color3 = tex3.Sample(samp3, uv);
//	return o;
//}


struct VSOutput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

PSOutput PSMain(VSOutput input)
{
	PSOutput o;
	o.color0 = tex0.Sample(samp0, input.texcoord);
	o.color1 = tex1.Sample(samp1, input.texcoord);
	o.color2 = tex2.Sample(samp2, input.texcoord);
	o.color3 = tex3.Sample(samp3, input.texcoord);
	return o;
}