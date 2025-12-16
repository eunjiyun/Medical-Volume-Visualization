cbuffer VolumeConstants : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 color;
};

struct VS_INPUT {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VS_OUTPUT VSVolume(VS_INPUT input)
{
	VS_OUTPUT output;

	// ✅ 0.9 스케일 제거!
	output.position = mul(float4(input.position, 1.0f), World);
	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);
	output.texcoord = input.texcoord;

	return output;
}