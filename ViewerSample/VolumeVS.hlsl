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
	float3 worldPos : WORLDPOS;
};

VS_OUTPUT VSMain(VS_INPUT input) {
	VS_OUTPUT output;

	float4 worldPos = mul(float4(input.position, 1.0f), World);
	float4 viewPos = mul(worldPos, View);
	output.position = mul(viewPos, Projection);

	output.texcoord = input.texcoord;
	output.worldPos = worldPos.xyz;

	return output;
}


//VS_OUTPUT VSMain(VS_INPUT input) {
//	VS_OUTPUT output;
//	output.position = float4(input.position, 1.0f); // NDC 좌표 직접 출력
//	output.texcoord = input.texcoord;
//	output.worldPos = input.position;
//	return output;
//}