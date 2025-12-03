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

//bool TransferFunction::Initialize(float center, float width, ID3D11Device* device)
//{
//	
//
//	//m_controlPoints.clear();
//
//	//float minHU = center - width / 2.0f;
//	//float maxHU = center + width / 2.0f;
//
//	//std::cout << "Initialize TF - Center:" << center << "Width:" << width << std:: endl;
//	//std::cout << "HU Range:" << minHU << "~" << maxHU << std::endl;
//
//	//// ⭐ 윈도우 범위로 정규화
//	//auto HUtoNorm = [&](float hu) -> float {
//	//	return saturate((hu - minHU) / width);
//	//};
//
//	//// ⭐ 시작점 추가 (0.0)
//	//m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
//
//	//// -400 ~ 200: 연조직
//	//if (maxHU >= -400.0f && minHU <= 200.0f) {
//	//	float t = HUtoNorm(200.0f);
//	//	m_controlPoints.push_back({ t, 0.6f, 0.5f, 0.4f, 0.05f });
//	//	std::cout << "Added soft tissue at t=" << t;
//	//}
//
//	//// 200 ~ 700: 뼈 시작
//	//if (maxHU >= 200.0f && minHU <= 700.0f) {
//	//	float t = HUtoNorm(700.0f);
//	//	m_controlPoints.push_back({ t, 0.85f, 0.75f, 0.65f, 0.4f });
//	//	std::cout << "Added bone start at t=" << t;
//	//}
//
//	//// 700 ~ 1300: 단단한 뼈
//	//if (maxHU >= 700.0f && minHU <= 1300.0f) {
//	//	float t = HUtoNorm(1300.0f);
//	//	m_controlPoints.push_back({ t, 0.92f, 0.88f, 0.82f, 1.1f });
//	//	std::cout << "Added hard bone at t=" << t;
//	//}
//
//	//// 1300 이상: 치아
//	//if (maxHU >= 1300.0f) {
//	//	float t = HUtoNorm(3000.0f);
//	//	m_controlPoints.push_back({ t, 0.98f, 0.95f, 0.90f, 2.0f });
//	//	std::cout << "Added teeth at t=" << t;
//	//}
//
//	//// ⭐ 끝점 추가 (1.0) - 중요!
//	//m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f });
//
//	//std::cout << "Total control points:" << m_controlPoints.size();
//
//	//// ⭐ 정렬 확인
//	//std::sort(m_controlPoints.begin(), m_controlPoints.end(),
//	//	[](const TFPoint& a, const TFPoint& b) { return a.value < b.value; });
//
//	//UpdateTexture(device);
//	//return (m_tfSRV != nullptr);
//
//
//
//	m_controlPoints.clear();
//
//	// ⭐ 절대 HU 기준 (-1000 ~ 3000)
//	auto HUtoNorm = [](float hu) -> float {
//		return saturate((hu + 1000.0f) / 4000.0f);
//	};
//
//	// 배경/공기
//	m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.0f, 0.0f, 0.0f, 0.0f });
//
//	// 연조직 (-100 ~ 100)
//	m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.5f, 0.4f, 0.3f, 0.01f });
//	m_controlPoints.push_back({ HUtoNorm(100.0f), 0.6f, 0.5f, 0.4f, 0.05f });
//
//	// 뼈 시작 (200 ~ 400)
//	m_controlPoints.push_back({ HUtoNorm(200.0f), 0.7f, 0.6f, 0.5f, 0.15f });
//	m_controlPoints.push_back({ HUtoNorm(400.0f), 0.8f, 0.7f, 0.6f, 0.3f });
//
//	// 단단한 뼈 (700 ~ 1000)
//	m_controlPoints.push_back({ HUtoNorm(700.0f), 0.85f, 0.75f, 0.65f, 0.6f });
//	m_controlPoints.push_back({ HUtoNorm(1000.0f), 0.90f, 0.82f, 0.72f, 0.9f });
//
//	// 매우 단단한 뼈 (1300 ~ 1800)
//	m_controlPoints.push_back({ HUtoNorm(1300.0f), 0.93f, 0.88f, 0.80f, 1.2f });
//	m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.96f, 0.92f, 0.85f, 1.5f });
//
//	// 치아 (2000+)
//	m_controlPoints.push_back({ HUtoNorm(2000.0f), 0.98f, 0.95f, 0.90f, 1.8f });
//	m_controlPoints.push_back({ HUtoNorm(3000.0f), 0.99f, 0.97f, 0.93f, 2.2f });
//
//	m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 2.2f });
//
//	UpdateTexture(device);
//	return (m_tfSRV != nullptr);
//}
bool TransferFunction::Initialize(float center, float width, ID3D11Device* device)
{
	m_controlPoints.clear();

	auto HUtoNorm = [](float hu) -> float {
		return saturate((hu + 1000.0f) / 4000.0f);
	};

	//// 공기/배경
	//m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	////m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.0f, 0.0f, 0.0f, 0.0f });


	//// 연조직
	//m_controlPoints.push_back({ HUtoNorm(-500.0f), 0.35f, 0.25f, 0.15f, 1.2f });
	//m_controlPoints.push_back({ HUtoNorm(-200.0f), 0.48f, 0.38f, 0.28f, 1.5f });
	//m_controlPoints.push_back({ HUtoNorm(50.0f),  0.48f, 0.38f, 0.28f, 1.7f });

	////// 연조직 - 더 어두운 갈색
	////m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.35f, 0.25f, 0.15f, 0.01f });
	////m_controlPoints.push_back({ HUtoNorm(100.0f), 0.48f, 0.38f, 0.28f, 0.08f });

	//// 뼈 시작 (300~600) - 베이지 톤
	//m_controlPoints.push_back({ HUtoNorm(300.0f), 0.70f, 0.58f, 0.46f, 0.25f });
	//m_controlPoints.push_back({ HUtoNorm(600.0f), 0.80f, 0.68f, 0.56f, 0.50f });

	//// 뼈 중간 (800~1200) - 밝은 베이지
	//m_controlPoints.push_back({ HUtoNorm(800.0f), 0.86f, 0.76f, 0.66f, 0.75f });
	//m_controlPoints.push_back({ HUtoNorm(1200.0f), 0.90f, 0.83f, 0.74f, 1.00f });

	//// 단단한 뼈 (1400~1800) - 아주 밝은 베이지
	//m_controlPoints.push_back({ HUtoNorm(1400.0f), 0.93f, 0.88f, 0.80f, 1.30f });
	//m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.95f, 0.91f, 0.85f, 1.60f });

	//// 치아 (2000~2500) - 밝은 크림/흰색
	//m_controlPoints.push_back({ HUtoNorm(2000.0f), 0.97f, 0.94f, 0.89f, 2.00f });
	//m_controlPoints.push_back({ HUtoNorm(2500.0f), 0.99f, 0.97f, 0.94f, 2.50f });

	//// 매우 높은 HU (3000+) - 완전 흰색
	//m_controlPoints.push_back({ HUtoNorm(3000.0f), 1.00f, 1.00f, 1.00f, 3.00f });
	//m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 3.00f });



	// ⭐ 시작/공기 - 완전 투명!
	m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	//m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.0f, 0.0f, 0.0f, 0.0f });

	////// ⭐ 경계 영역 (-400 ~ -100) - 어두운 갈색 (입술/콧구멍!)
	////m_controlPoints.push_back({ HUtoNorm(-300.0f), 0.25f, 0.18f, 0.12f, 4.0f });
	////m_controlPoints.push_back({ HUtoNorm(-200.0f), 0.40f, 0.30f, 0.22f, 6.0f });

	//// 연조직 (-100~100)
	//m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.72f, 0.52f, 0.38f, 20.00f });
	//m_controlPoints.push_back({ HUtoNorm(100.0f), 0.82f, 0.62f, 0.45f, 25.00f });

	//// 뼈 (300~1200)
	//m_controlPoints.push_back({ HUtoNorm(300.0f), 0.88f, 0.68f, 0.50f, 18.00f });
	//m_controlPoints.push_back({ HUtoNorm(1200.0f), 0.96f, 0.82f, 0.65f, 24.00f });

	//// 치아 (2000~3000)
	//m_controlPoints.push_back({ HUtoNorm(2000.0f), 0.99f, 0.92f, 0.82f, 30.00f });
	////m_controlPoints.push_back({ HUtoNorm(3000.0f), 1.00f, 0.95f, 0.88f, 35.00f });
	//m_controlPoints.push_back({ 1.0f, 1.00f, 0.95f, 0.88f, 35.00f });

	m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.0f, 0.0f, 0.0f, 0.0f });

	// 연조직 - 더 어두운 갈색
	m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.35f, 0.25f, 0.15f, 20.00f });
	m_controlPoints.push_back({ HUtoNorm(100.0f), 0.48f, 0.38f, 0.28f,25.00f });

	// 뼈 시작 (300~600) - 베이지 톤
	m_controlPoints.push_back({ HUtoNorm(300.0f), 0.70f, 0.58f, 0.46f, 0.25f });
	m_controlPoints.push_back({ HUtoNorm(600.0f), 0.80f, 0.68f, 0.56f, 0.50f });

	// 뼈 중간 (800~1200) - 밝은 베이지
	m_controlPoints.push_back({ HUtoNorm(800.0f), 0.86f, 0.76f, 0.66f, 0.75f });
	m_controlPoints.push_back({ HUtoNorm(1200.0f), 0.90f, 0.83f, 0.74f, 1.00f });

	// 단단한 뼈 (1400~1800) - 아주 밝은 베이지
	m_controlPoints.push_back({ HUtoNorm(1400.0f), 0.93f, 0.88f, 0.80f, 1.30f });
	m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.95f, 0.91f, 0.85f, 1.60f });

	// 치아 (2000~2500) - 밝은 크림/흰색
	m_controlPoints.push_back({ HUtoNorm(2000.0f), 0.97f, 0.94f, 0.89f, 2.00f });
	m_controlPoints.push_back({ HUtoNorm(2500.0f), 0.99f, 0.97f, 0.94f, 2.50f });

	// 매우 높은 HU (3000+) - 완전 흰색
	m_controlPoints.push_back({ HUtoNorm(3000.0f), 1.00f, 1.00f, 1.00f, 3.00f });
	m_controlPoints.push_back({ 1.0f, 1.0f, 1.0f, 1.0f, 3.00f });

	UpdateTexture(device);
	return true;
}



//bool TransferFunction::Initialize(float center, float width, ID3D11Device* device)
//{
//	m_controlPoints.clear();
//	
//
//	auto HUtoNorm = [center, width](float hu) -> float {
//		return saturate((hu - center + width / 2.0f) / width);
//	};
//
//	// ⭐ 공기 - 좁게 투명 처리 (실제 공기는 -1200~-900)
//	m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(-800.0f), 0.0f, 0.0f, 0.0f, 0.0f });
//
//	// ⭐ 공기/연조직 경계 (-800~-300) - Median -301 포함!
//	m_controlPoints.push_back({ HUtoNorm(-600.0f), 0.35f, 0.28f, 0.22f, 0.1f });
//	m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.48f, 0.36f, 0.28f, 0.3f });
//	m_controlPoints.push_back({ HUtoNorm(-200.0f), 0.58f, 0.43f, 0.33f, 0.5f });
//
//	// ⭐ 연조직 (-200~200) - 0~100 포함!
//	m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.66f, 0.48f, 0.36f, 0.8f });
//	m_controlPoints.push_back({ HUtoNorm(-50.0f),  0.70f, 0.52f, 0.38f, 1.0f });
//	m_controlPoints.push_back({ HUtoNorm(0.0f),    0.74f, 0.56f, 0.40f, 1.3f });
//	m_controlPoints.push_back({ HUtoNorm(50.0f),   0.78f, 0.60f, 0.44f, 1.6f });
//	m_controlPoints.push_back({ HUtoNorm(100.0f),  0.82f, 0.64f, 0.47f, 2.0f });
//
//	// 연조직/뼈 경계 (100~300)
//	m_controlPoints.push_back({ HUtoNorm(200.0f),  0.85f, 0.68f, 0.51f, 2.5f });
//
//	// 뼈 (300~1000)
//	m_controlPoints.push_back({ HUtoNorm(400.0f),  0.88f, 0.73f, 0.57f, 3.5f });
//	m_controlPoints.push_back({ HUtoNorm(600.0f),  0.90f, 0.77f, 0.62f, 5.0f });
//	m_controlPoints.push_back({ HUtoNorm(800.0f),  0.92f, 0.81f, 0.67f, 7.0f });
//
//	// 단단한 뼈 (1000~1500)
//	m_controlPoints.push_back({ HUtoNorm(1000.0f), 0.94f, 0.84f, 0.72f, 9.5f });
//	m_controlPoints.push_back({ HUtoNorm(1500.0f), 0.96f, 0.88f, 0.78f, 13.0f });
//
//	// 치아 (1500~3000)
//	m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.97f, 0.91f, 0.84f, 17.0f });
//	m_controlPoints.push_back({ HUtoNorm(2200.0f), 0.98f, 0.94f, 0.89f, 21.0f });
//	m_controlPoints.push_back({ 1.0f, 0.99f, 0.97f, 0.93f, 25.0f });
//
//	UpdateTexture(device);
//	return true;
//}

//bool TransferFunction::Initialize(float center, float width, ID3D11Device* device)
//{
//	m_controlPoints.clear();
//
//	auto HUtoNorm = [center, width](float hu) -> float {
//		return saturate((hu - center + width / 2.0f) / width);
//	};
//
//	// 공기: 완전히 투명
//	m_controlPoints.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(-800.0f), 0.0f, 0.0f, 0.0f, 0.0f });
//
//	// 공기/연조직 경계 (-800~-300)
//	m_controlPoints.push_back({ HUtoNorm(-600.0f), 0.35f, 0.28f, 0.22f, 0.1f });
//	m_controlPoints.push_back({ HUtoNorm(-400.0f), 0.48f, 0.36f, 0.28f, 0.3f });
//	m_controlPoints.push_back({ HUtoNorm(-200.0f), 0.58f, 0.43f, 0.33f, 0.5f });
//
//	// 연조직 (-200~200) → 알파를 높여서 잘 보이게
//	m_controlPoints.push_back({ HUtoNorm(-100.0f), 0.66f, 0.48f, 0.36f, 0.8f });
//	m_controlPoints.push_back({ HUtoNorm(-50.0f),  0.70f, 0.52f, 0.38f, 1.0f });
//	m_controlPoints.push_back({ HUtoNorm(0.0f),    0.74f, 0.56f, 0.40f, 1.3f });
//	m_controlPoints.push_back({ HUtoNorm(50.0f),   0.78f, 0.60f, 0.44f, 1.6f });
//	m_controlPoints.push_back({ HUtoNorm(100.0f),  0.82f, 0.64f, 0.47f, 2.0f });
//	m_controlPoints.push_back({ HUtoNorm(200.0f),  0.85f, 0.68f, 0.51f, 2.5f });
//
//	// 뼈/치아 구간 → 알파를 0으로 해서 완전히 투명 처리
//	m_controlPoints.push_back({ HUtoNorm(400.0f),  0.88f, 0.73f, 0.57f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(600.0f),  0.90f, 0.77f, 0.62f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(800.0f),  0.92f, 0.81f, 0.67f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(1000.0f), 0.94f, 0.84f, 0.72f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(1500.0f), 0.96f, 0.88f, 0.78f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(1800.0f), 0.97f, 0.91f, 0.84f, 0.0f });
//	m_controlPoints.push_back({ HUtoNorm(2200.0f), 0.98f, 0.94f, 0.89f, 0.0f });
//	m_controlPoints.push_back({ 1.0f,              0.99f, 0.97f, 0.93f, 0.0f });
//
//	UpdateTexture(device);
//	return true;
//}



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