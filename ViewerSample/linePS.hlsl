
struct PS_IN
{
	float4 color : SV_POSITION;
	
};



float4 main(PS_IN input) : SV_Target
{
	return input.color;
}
