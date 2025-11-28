#include "TransferFunction.h"
#include<algorithm>

TransferFunction::TransferFunction()
	: m_tfTexture(nullptr)
	, m_tfSRV(nullptr)
{
}

TransferFunction::~TransferFunction()
{
	if (m_tfSRV) m_tfSRV->Release();
	if (m_tfTexture) m_tfTexture->Release();
}

float saturate(float x) {
	if (x < 0.0f) return 0.0f;
	if (x > 1.0f) return 1.0f;
	return x;
}

bool TransferFunction::Initialize(float center, float width, ID3D11Device* device)
{
	

	//m_controlPoints.clear();

	//float minHU = center - width / 2.0f;
	//float maxHU = center + width / 2.0f;

	//std::cout << "Initialize TF - Center:" << center << "Width:" << width << std:: endl;
	//std::cout << "HU Range:" << minHU << "~" << maxHU << std::endl;

	//// ⭐ 윈도우 범위로 정규화
	//auto HUtoNorm = [&](float hu) -> float {
	//	return saturate((hu - minHU) / width);
	//};

	//// ⭐ 시작점 추가 (0.0)
	//m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });

	//// -400 ~ 200: 연조직
	//if (maxHU >= -400.0f && minHU <= 200.0f) {
	//	float t = HUtoNorm(200.0f);
	//	m_controlPoints.push_back({ t, 0.6f, 0.5f, 0.4f, 0.05f });
	//	std::cout << "Added soft tissue at t=" << t;
	//}

	//// 200 ~ 700: 뼈 시작
	//if (maxHU >= 200.0f && minHU <= 700.0f) {
	//	float t = HUtoNorm(700.0f);
	//	m_controlPoints.push_back({ t, 0.85f, 0.75f, 0.65f, 0.4f });
	//	std::cout << "Added bone start at t=" << t;
	//}

	//// 700 ~ 1300: 단단한 뼈
	//if (maxHU >= 700.0f && minHU <= 1300.0f) {
	//	float t = HUtoNorm(1300.0f);
	//	m_controlPoints.push_back({ t, 0.92f, 0.88f, 0.82f, 1.1f });
	//	std::cout << "Added hard bone at t=" << t;
	//}

	//// 1300 이상: 치아
	//if (maxHU >= 1300.0f) {
	//	float t = HUtoNorm(3000.0f);
	//	m_controlPoints.push_back({ t, 0.98f, 0.95f, 0.90f, 2.0f });
	//	std::cout << "Added teeth at t=" << t;
	//}

	//// ⭐ 끝점 추가 (1.0) - 중요!
	//m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f });

	//std::cout << "Total control points:" << m_controlPoints.size();

	//// ⭐ 정렬 확인
	//std::sort(m_controlPoints.begin(), m_controlPoints.end(),
	//	[](const TFPoint& a, const TFPoint& b) { return a.value < b.value; });

	//UpdateTexture(device);
	//return (m_tfSRV != nullptr);



	m_controlPoints.clear();

	// ⭐ 절대 HU 기준 (-1000 ~ 3000)
	auto HUtoNorm = [](float hu) -> float {
		return saturate((hu + 1000.0f) / 4000.0f);
	};

	// 배경/공기
	m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.0f, 0.0f, 0.0f, 0.0f });

	// 연조직 (-100 ~ 100)
	m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.5f, 0.4f, 0.3f, 0.01f });
	m_controlPoints.push_back({ HUtoNorm(100.0f), 0.6f, 0.5f, 0.4f, 0.05f });

	// 뼈 시작 (200 ~ 400)
	m_controlPoints.push_back({ HUtoNorm(200.0f), 0.7f, 0.6f, 0.5f, 0.15f });
	m_controlPoints.push_back({ HUtoNorm(400.0f), 0.8f, 0.7f, 0.6f, 0.3f });

	// 단단한 뼈 (700 ~ 1000)
	m_controlPoints.push_back({ HUtoNorm(700.0f), 0.85f, 0.75f, 0.65f, 0.6f });
	m_controlPoints.push_back({ HUtoNorm(1000.0f), 0.90f, 0.82f, 0.72f, 0.9f });

	// 매우 단단한 뼈 (1300 ~ 1800)
	m_controlPoints.push_back({ HUtoNorm(1300.0f), 0.93f, 0.88f, 0.80f, 1.2f });
	m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.96f, 0.92f, 0.85f, 1.5f });

	// 치아 (2000+)
	m_controlPoints.push_back({ HUtoNorm(2000.0f), 0.98f, 0.95f, 0.90f, 1.8f });
	m_controlPoints.push_back({ HUtoNorm(3000.0f), 0.99f, 0.97f, 0.93f, 2.2f });

	m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 2.2f });

	UpdateTexture(device);
	return (m_tfSRV != nullptr);
}



void TransferFunction::SetHUWindow(float center, float width, ID3D11Device* g_pd3dDevice)
{
	// ⭐ Transfer Function은 항상 고정
	m_controlPoints.clear();
	m_controlPoints.push_back({ 0.0f,   0.0f, 0.0f, 0.0f, 0.0f });
	m_controlPoints.push_back({ 0.176f, 0.6f, 0.5f, 0.4f, 0.05f });
	m_controlPoints.push_back({ 0.324f, 0.85f, 0.75f, 0.65f, 0.4f });
	m_controlPoints.push_back({ 0.5f,   0.92f, 0.88f, 0.82f, 1.1f });
	m_controlPoints.push_back({ 1.0f,   0.98f, 0.95f, 0.90f, 2.0f });

	UpdateTexture(g_pd3dDevice);
}

void TransferFunction::UpdateTexture(ID3D11Device* device)
{
	// 기존 텍스처 해제
	if (m_tfSRV) { m_tfSRV->Release(); m_tfSRV = nullptr; }
	if (m_tfTexture) { m_tfTexture->Release(); m_tfTexture = nullptr; }

	// 256개 샘플 데이터 생성
	float* tfData = new float[TF_SIZE * 4]; // RGBA

	for (int i = 0; i < TF_SIZE; i++) {
		float t = (float)i / (TF_SIZE - 1);

		float r = 1.0f, g = 1.0f, b = 1.0f, a = 0.0f;

		// 컨트롤 포인트 사이 선형 보간
		for (size_t j = 0; j < m_controlPoints.size() - 1; j++) {
			TFPoint& p1 = m_controlPoints[j];
			TFPoint& p2 = m_controlPoints[j + 1];

			if (t >= p1.value && t <= p2.value) {
				float localT = (t - p1.value) / (p2.value - p1.value);

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

	// D3D11 1D 텍스처 생성
	D3D11_TEXTURE1D_DESC desc = {};
	desc.Width = TF_SIZE;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = tfData;
	initData.SysMemPitch = TF_SIZE * 4 * sizeof(float);

	HRESULT hr = device->CreateTexture1D(&desc, &initData, &m_tfTexture);
	if (FAILED(hr)) {
		delete[] tfData;
		return;
	}

	// Shader Resource View 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = 1;
	srvDesc.Texture1D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(m_tfTexture, &srvDesc, &m_tfSRV);

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