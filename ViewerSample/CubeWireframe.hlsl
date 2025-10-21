// CubeWireframe.hlsl
cbuffer VolumeConstants : register(b0) {
	matrix World;
	matrix View;
	matrix Projection;
	float4 lineColor;
};

struct VS_INPUT {
	float3 position : POSITION;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input) {
	VS_OUTPUT output;
	float4 worldPos = mul(float4(input.position, 1.0f), World);
	float4 viewPos = mul(worldPos, View);
	output.position = mul(viewPos, Projection);
	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET{
	return lineColor;  // ´Ü»ö ¼±
}