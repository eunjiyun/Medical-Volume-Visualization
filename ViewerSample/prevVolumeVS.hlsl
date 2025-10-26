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


	// ?îπ ?§Ï????âÎ†¨ ?ùÏÑ± (0.5Î°?Ï∂ïÏÜå)
	float4x4 scale = float4x4(
		0.9, 0.0, 0.0, 0.0,
		0.0, 0.9, 0.0, 0.0,
		0.0, 0.0, 0.9, 0.0,
		0.0, 0.0, 0.0, 1.0
		);



	//// ?îπ ?§Ï????âÎ†¨ ?ùÏÑ± (0.5Î°?Ï∂ïÏÜå)
	//float4x4 scale = float4x4(
	//	1.0, 0.0, 0.0, 0.0,
	//	0.0, 1.0, 0.0, 0.0,
	//	0.0, 0.0, 1.0, 0.0,
	//	0.0, 0.0, 0.0, 1.0
	//	);



	//output.position = mul(float4(input.position, 1.0f), mul(scale, World));
	//output.position = mul(output.position, View);
	//output.position = mul(output.position, Projection);
	//output.texcoord = input.texcoord;


	output.position = mul(float4(input.position, 1.0f), World);
	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);
	output.texcoord = input.texcoord;
	return output;


}
