// Pixel Shader 
Texture2D sliceTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer VolumeConstants : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 color;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 worldPos : WORLDPOS;
};


float4 PSMain(VS_OUTPUT input) : SV_TARGET{
//	////�ܼ� �ؽ�ó ���ø� (�����̽� ����)
//	//float4 color = sliceTexture.Sample(samplerState, input.texcoord);
//	//return color;
//
//
//
//	//   return float4(input.texcoord, 0.0f, 1.0f); // �ؽ�ó ��ǥ �ð�ȭ
//
//	////return float4(1.0f, 0.0f, 0.0f, 1.0f); // ������
//
 //return float4(abs(input.texcoord.x), abs(input.texcoord.y), 0.0f, 1.0f);
//	float4 color = sliceTexture.Sample(samplerState, input.texcoord);
//return color;


return color;


}

//float4 PSMain(VS_OUTPUT input) : SV_TARGET{
//	return float4(abs(input.texcoord.x), abs(input.texcoord.y), 0.0f, 1.0f);
//}