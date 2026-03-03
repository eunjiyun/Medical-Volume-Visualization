struct VS_IN {
	float2 pos : POSITION;  // -1~+1
	float2 uv  : TEXCOORD0;
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

VS_OUT main(VS_IN input) {
	VS_OUT o;
	o.pos = float4(input.pos, 0.0, 1.0);
	o.uv = input.uv;
	return o;
}
