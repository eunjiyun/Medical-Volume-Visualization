#include "TransferFunction.h"


void TransferFunction::SetHUWindow(float center, float width, ID3D11Device* g_pd3dDevice)
{
	// HU 윈도우 레벨 설정
	float minHU = center - width / 2.0f;
	float maxHU = center + width / 2.0f;

	m_controlPoints.clear();

	//// 3개 포인트로 간단한 TF 생성
	//m_controlPoints.push_back({ 0.0f, 0, 0, 0, 0 });      // 최소값: 투명
	//m_controlPoints.push_back({ 0.5f, 1, 1, 1, 0.8f });   // 중간값: 불투명
	//m_controlPoints.push_back({ 1.0f, 1, 1, 1, 1.0f });   // 최대값: 완전 불투명

	 // 실제 의료 영상에서 많이 쓰는 설정
	m_controlPoints.push_back({ 0.0f,  0.0f, 0.0f, 0.0f, 0.0f });   // 최소값: 완전 투명
	m_controlPoints.push_back({ 0.2f,  0.3f, 0.3f, 0.3f, 0.1f });   // 어두운 부분: 약간 보임
	m_controlPoints.push_back({ 0.4f,  0.8f, 0.8f, 0.7f, 0.4f });   // 중간 부분
	m_controlPoints.push_back({ 0.6f,  1.0f, 0.9f, 0.8f, 0.7f });   // 밝은 부분
	m_controlPoints.push_back({ 1.0f,  1.0f, 1.0f, 1.0f, 1.0f });   // 최대값: 완전 불투명


	UpdateTexture(g_pd3dDevice);
}

void TransferFunction::UpdateTexture(ID3D11Device* g_pd3dDevice)
{
	// 256개 샘플로 보간된 1D 텍스처 생성
	const int TF_SIZE = 256;
	float* tfData = new float[TF_SIZE * 4]; // RGBA

	for (int i{}; i < TF_SIZE; ++i) {
		float t = (float)i / (TF_SIZE - 1); // 0.0 ~ 1.0

		// control points 사이를 선형 보간
		float r = 1.0f, g = 1.0f, b = 1.0f, a = 0.0f;

		// 어느 두 control point 사이에 있는지 찾기
		for (size_t j = 0; j < m_controlPoints.size() - 1; ++j) {
			TFPoint& p1 = m_controlPoints[j];
			TFPoint& p2 = m_controlPoints[j + 1];

			if (t >= p1.value && t <= p2.value) {
				// p1과 p2 사이의 위치 (0~1)
				float localT = (t - p1.value) / (p2.value - p1.value);

				// 선형 보간
				r = p1.r + localT * (p2.r - p1.r);
				g = p1.g + localT * (p2.g - p1.g);
				b = p1.b + localT * (p2.b - p1.b);
				a = p1.a + localT * (p2.a - p1.a);
				break;
			}
		}

		tfData[i * 4 + 0] = r;
		tfData[i * 4 + 1] = g;
		tfData[i * 4 + 2] = b;
		tfData[i * 4 + 3] = a;
	}

	// D3D11 텍스처 생성
	D3D11_TEXTURE1D_DESC desc = {};
	desc.Width = TF_SIZE;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = tfData;
	initData.SysMemPitch = TF_SIZE * 4 * sizeof(float);

	g_pd3dDevice->CreateTexture1D(&desc, &initData, &m_tfTexture);
	g_pd3dDevice->CreateShaderResourceView(m_tfTexture, nullptr, &m_tfSRV);

	delete[] tfData;
}

void TransferFunction::SetBonePreset(ID3D11Device* g_pd3dDevice)
{
	m_controlPoints.clear();

	// 뼈 보기 최적화
	m_controlPoints.push_back({ 0.0f,  0.0f, 0.0f, 0.0f, 0.0f });   // 공기/배경: 투명
	m_controlPoints.push_back({ 0.3f,  0.5f, 0.3f, 0.2f, 0.0f });   // 연조직: 거의 투명
	m_controlPoints.push_back({ 0.6f,  0.9f, 0.8f, 0.7f, 0.3f });   // 뼈 시작
	m_controlPoints.push_back({ 0.8f,  1.0f, 0.95f, 0.9f, 0.8f });  // 단단한 뼈
	m_controlPoints.push_back({ 1.0f,  1.0f, 1.0f, 1.0f, 1.0f });   // 최고 밀도: 완전 불투명

	UpdateTexture(g_pd3dDevice);
}