cbuffer CB : register(b0)
{
	matrix WorldViewProj;
};

VS_OUT main(VS_IN input)
{
	VS_OUT o;
	o.pos = mul(float4(input.pos, 1), WorldViewProj);
	o.color = input.color;
	return o;
}
