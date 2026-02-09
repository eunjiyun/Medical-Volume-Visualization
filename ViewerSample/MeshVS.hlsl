cbuffer MeshConstantBuffer : register(b0)
{
	matrix WVP;
	matrix World;  //  추가
	matrix WorldView;  //  추가!
}


struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};



struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : TEXCOORD0;  //  View 좌표
	float2 Tex : TEXCOORD1;
	float WorldY : TEXCOORD2;  //  추가!
};

VS_OUTPUT main(VS_INPUT input)
{
	//VS_OUTPUT o;
	//o.Pos = mul(float4(input.Pos, 1.0), WVP);
	//o.ViewPos = mul(float4(input.Pos, 1.0), WorldView).xyz;  // ✅ View 좌표
	//o.Tex = input.Tex;
	//return o;


	VS_OUTPUT output;

	//벡터를 왼쪽에 곱함
	//순서는 S>R>T
	output.Pos = mul(float4(input.Pos, 1), WVP);
	output.Tex = input.Tex;


	//  World Y 계산
	float4 worldPos = mul(float4(input.Pos, 1), World);
	output.WorldY = worldPos.y;

	return output;
}