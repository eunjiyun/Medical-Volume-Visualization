// ToothMaskPS.hlsl
float4 main(PSInput input) : SV_Target
{
	// 기존 raymarch 거의 그대로
	// BUT acc / lighting 다 제거

	bool isTooth =
		hu > 1200.0 &&        // HU 조건 (너가 검증한 값)
		gradMag > 0.04 &&     // 경계 있음
		distVS > 0.0 &&
		distVS < 2.0 &&       // 피부 바로 아래
		uvw.y < 0.45 &&       // 하악
		uvw.z > 0.55;         // 앞니

	if (isTooth)
		return float4(1, 0, 0, 1); // R=1
	else
		return float4(0, 0, 0, 1);
}
