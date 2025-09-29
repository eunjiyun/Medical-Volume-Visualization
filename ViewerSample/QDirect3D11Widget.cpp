/*
 *
 */
#pragma comment(lib, "d3d11.lib")



#include "QDirect3D11Widget.h"

#include <QDebug>

 //Qt?????濚????룸Ŧ爾???????袁⑸즵獒뺣뎾???嚥▲꺂痢????濚?嶺? ????猿??嚥▲꺂痢???좊즵??꼯??
#include <QEvent>

//Qt?????癲ル슢????????モ섌???嶺뚮ㅎ??????곕뜤 ???袁⑹뵫????筌?痢⑼┼?논맋?? 癲ル슪?ｇ몭??????????嚥▲꺂痢?????밸쭬
#include <QWheelEvent>



#include "stdafx.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshader.h"
#include "graphicsclass.h"
#include <wrl/client.h>
#include <vector>
#include "FileReader.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

using Microsoft::WRL::ComPtr;



constexpr int FPS_LIMIT = 60.0f;
constexpr int MS_PER_FRAME = (int)((1.0f / FPS_LIMIT) * 1000.0f);

QDirect3D11Widget::QDirect3D11Widget(QWidget* parent)
	: QWidget(parent)
	, m_pDevice(Q_NULLPTR)
	, m_pDeviceContext(Q_NULLPTR)
	, m_pSwapChain(Q_NULLPTR)
	, m_RTViews(4, Q_NULLPTR)
	, m_hWnd(reinterpret_cast<HWND>(winId()))
	, m_bDeviceInitialized(false)
	, m_bRenderActive(false)
	, m_bStarted(false)
	, m_BackColor{ 0.0f, 0.135f, 0.481f, 1.0f }
{
	qDebug() << "[QDirect3D11Widget::QDirect3D11Widget] - Widget Handle: " << m_hWnd;

	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_NativeWindow);

	// Setting these attributes to our widget and returning null on paintEngine event
	// tells Qt that we'll handle all drawing and updating the widget ourselves.
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
}

QDirect3D11Widget::~QDirect3D11Widget() {}

void QDirect3D11Widget::release()
{
	m_bDeviceInitialized = false;
	disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
	m_qTimer.stop();

	for (auto& view : m_RTViews)
		ReleaseObject(view);

	ReleaseObject(m_pSwapChain);
	ReleaseObject(m_pDeviceContext);
	ReleaseObject(m_pDevice);
}

void QDirect3D11Widget::run()
{
	m_qTimer.start(MS_PER_FRAME);
	m_bRenderActive = m_bStarted = true;
}

void QDirect3D11Widget::pauseFrames()
{
	if (!m_qTimer.isActive() || !m_bStarted) return;

	disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
	m_qTimer.stop();
	m_bRenderActive = false;
}

void QDirect3D11Widget::continueFrames()
{
	if (m_qTimer.isActive() || !m_bStarted) return;

	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
	m_qTimer.start(MS_PER_FRAME);
	m_bRenderActive = true;
}

void QDirect3D11Widget::showEvent(QShowEvent* event)
{
	if (!m_bDeviceInitialized)
	{
		m_bDeviceInitialized = init();
		emit deviceInitialized(m_bDeviceInitialized);
	}

	QWidget::showEvent(event);

	//LoadDICOMSeries();  // 최초 표시 시 DICOM 로드


    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    HWND hwnd = (HWND)this->winId(); // QWidget 기반이라면 this->winId()로 HWND 확보

    ImGui_ImplWin32_Init(hwnd); // Qt에서 가져온 HWND
    ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

}

bool QDirect3D11Widget::init()
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;

	qDebug() << "qDebug : width : " << width() << endl;
	//qDebug() << "hwnd : " << hwnd << endl;


	sd.BufferDesc.Width = width();


	sd.BufferDesc.Height = height();
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	iCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
										 D3D_FEATURE_LEVEL_10_0 };

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, iCreateFlags, featureLevels,
		_countof(featureLevels), D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice,
		&featureLevel, &m_pDeviceContext);
	if (hr != S_OK)
	{
		DXCall(D3D11CreateDeviceAndSwapChain(
			NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, iCreateFlags, featureLevels,
			_countof(featureLevels), D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice,
			&featureLevel, &m_pDeviceContext));
	}

	resetEnvironment();




    LoadDICOMSeries();  // 최초 표시 시 DICOM 로드
	initializeRenderTargets();

	createSwapChainRTV();
	// ???怨쀬뵠???λ뜃由??
	InitShaders();

	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);

	return true;
}

void QDirect3D11Widget::LoadDICOMSeries()
{
	//std::string folderPath = "D:\\Data\\sez\\DICOM"; // 또는 UI에서 선택된 경로

	//// FileReader 인스턴스 생성
	//fileReader = new FileReader;

	//// DICOM 시리즈 로드
	//if (!fileReader->LoadDICOMSeries(folderPath)) {
	//	qDebug() << "LoadDICOMSeries : Failed to load DICOM series from folder:" << QString::fromStdString(folderPath);
	//	return;
	//}

	////	UINT16 m_width;
	////UINT16 m_height;
	//// 첫 번째 슬라이스 생성 및 텍스처로 변환
	//std::vector<uint8_t> slice = fileReader->GenerateAxialSlice(0);
	//ID3D11ShaderResourceView* texture = fileReader->CreateTextureFromSlice(slice, fileReader->m_width, fileReader->m_height, m_pDevice);

	//// 텍스처를 위젯에 적용 (예시)
	//m_texture = (ID3D11Texture2D*)texture;
	////m_fileReader = std::move(fileReader); // 멤버 변수로 저장하고 싶다면


	fileReader = new FileReader();
	//LoadDICOMSeries
	//fileReader->ParseSlice((std::string)"D:\\Data\\sez\\DICOM",0);
	fileReader->LoadDICOMSeries((std::string)"D:\\Data\\sez\\DICOM", m_pDevice);
}

void QDirect3D11Widget::onFrame()
{
	if (m_bRenderActive) tick();

	//beginScene();
	//render();
	RenderAllQuads();
	endScene();
}

void QDirect3D11Widget::beginScene()
{
	//???????濡ろ뜑?? ???類??袁?맪????源놁젳
	//DX11?? 癲ル슔?됭짆? 8??좊즵獒?????????濡ろ뜑???????덈빰???袁⑸즴?????됰씭彛??????源낆쓱


	/*m_pDeviceContext->OMSetRenderTargets
	(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), NULL);*/

	for (int i{}; i < m_RTViews.size(); ++i) {


		D3D11_VIEWPORT vp = {};
		vp.Width = width() / 2.0f;
		vp.Height = height() / 2.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		if (0 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ??몿而?

		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ?λ뜄以?


		}
		else if (2 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ???삂

		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?紐껋삂

		}

		
		m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 揶쏆뮆????쇱젟

	
		m_pDeviceContext->RSSetViewports(1, &vp);
		
		m_pDeviceContext->ClearRenderTargetView(m_RTViews[i],
			reinterpret_cast<const float*>(&m_BackColor));
	}
}

void QDirect3D11Widget::endScene()
{
	if (FAILED(m_pSwapChain->Present(1, 0))) { onReset(); }
}


void QDirect3D11Widget::tick()
{
	// TODO: Update your scene here. For aesthetics reasons, only do it here if it's an
	// important component, otherwise do it in the MainWindow.
	// m_pCamera->Tick();

	emit ticked();
}



void QDirect3D11Widget::initializeRenderTargets()
{
	m_RTViews.clear();
	m_SRViews.clear();
	m_samplerState.clear();




    for (int i{}; i < 4; ++i) {
		// 1. ??용뮞筌???밴쉐
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width() / 2;
		texDesc.Height = height() / 2;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		ID3D11Texture2D* pTexture = nullptr;

		//250922  texture
		DXCall(m_pDevice->CreateTexture2D(&texDesc, nullptr, &pTexture));

		//// 2. RenderTargetView ??밴쉐
		//ID3D11RenderTargetView* pRTV = nullptr;
		//DXCall(m_pDevice->CreateRenderTargetView(pTexture, nullptr, &pRTV));
		//m_RTViews.push_back(pRTV);

        if (0== i) {

            // 2. RenderTargetView ??밴쉐
            ID3D11RenderTargetView* pRTV = nullptr;
            DXCall(m_pDevice->CreateRenderTargetView(pTexture, nullptr, &pRTV));
            m_RTViews.push_back(pRTV);


            // 3. ShaderResourceView ??밴쉐
            ID3D11ShaderResourceView* pSRV = nullptr;
            DXCall(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pSRV));
            m_SRViews.push_back(pSRV);
        }
        else if(1==i){

            // 2. RenderTargetView ??밴쉐
            ID3D11RenderTargetView* pRTV = nullptr;
            DXCall(m_pDevice->CreateRenderTargetView(fileReader->axialTexture, nullptr, &pRTV));
            m_RTViews.push_back(pRTV);


            // 3. ShaderResourceView ??밴쉐
            ID3D11ShaderResourceView* pSRV = nullptr;
            DXCall(m_pDevice->CreateShaderResourceView(fileReader->axialTexture, nullptr, &pSRV));
            m_SRViews.push_back(pSRV);

        }
        else if (2 == i) {
            // 2. RenderTargetView ??밴쉐
            ID3D11RenderTargetView* pRTV = nullptr;
            DXCall(m_pDevice->CreateRenderTargetView(fileReader->coronalTexture, nullptr, &pRTV));
            m_RTViews.push_back(pRTV);

            // 3. ShaderResourceView ??밴쉐
            ID3D11ShaderResourceView* pSRV = nullptr;
            DXCall(m_pDevice->CreateShaderResourceView(fileReader->coronalTexture, nullptr, &pSRV));
            m_SRViews.push_back(pSRV);
        }
        else if (3 == i) {
            // 2. RenderTargetView ??밴쉐
            ID3D11RenderTargetView* pRTV = nullptr;
            DXCall(m_pDevice->CreateRenderTargetView(fileReader->sagittalTexture, nullptr, &pRTV));
            m_RTViews.push_back(pRTV);

            // 3. ShaderResourceView ??밴쉐
            ID3D11ShaderResourceView* pSRV = nullptr;
            DXCall(m_pDevice->CreateShaderResourceView(fileReader->sagittalTexture, nullptr, &pSRV));
            m_SRViews.push_back(pSRV);
        }


		// 4. 샘플러 상태 생성
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* pSampler = nullptr;
		DXCall(m_pDevice->CreateSamplerState(&sampDesc, &pSampler));
		m_samplerState.push_back(pSampler);



		// 4. ??용뮞筌???곸젫
		pTexture->Release();
	}
}

void QDirect3D11Widget::createSwapChainRTV()
{
	ID3D11Texture2D* pBackBuffer = nullptr;
	DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pSwapChainRTV));
	ReleaseObject(pBackBuffer);
}



void QDirect3D11Widget::render()
{
	// TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
	// important component, otherwise do it in the MainWindow.
	// m_pCamera->Apply();




	//m_pDeviceContext->OMSetRenderTargets(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), nullptr);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);


	// ?됯퀬猷????쇱젟 (?袁⑷퍥 ?遺얇늺)
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(width());
	vp.Height = static_cast<float>(height());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 揶????쐭 ??野껋옕??ShaderResourceView嚥??遺얇늺???곗뮆??
	for (int i{}; i < m_SRViews.size(); ++i) {
		// ?? DrawQuadWithTexture(m_SRViews[i], viewport[i]);
		// ???봔?브쑴? ?怨쀬뵠?遺? ?類ㅼ젎 甕곌쑵?곫에??닌뗭겱??곷튊 ??곸뒄


		if (0 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ??몿而?
		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ?λ뜄以?
		}
		else if (2 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ???삂
		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?紐껋삂
		}




		m_pDeviceContext->RSSetViewports(1, &vp); // ?됯퀬猷????쇱젟

		DrawQuadWithTexture(m_SRViews[i], vp); // viewport[i]???袁⑺뒄 ?類ｋ궖

        
	}


	emit rendered();
}
void QDirect3D11Widget::UpdateColorBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	m_pDeviceContext->Map(m_colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &m_BackColor, sizeof(XMFLOAT4));
	m_pDeviceContext->Unmap(m_colorBuffer, 0);

	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);
}

void QDirect3D11Widget::DrawColoredQuad(const D3D11_VIEWPORT& vp)
{
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 텍스처 바인딩 (여기 추가!)
	//m_pDeviceContext->PSSetShaderResources(0, 1, &m_textureSRV);
	//m_pDeviceContext->PSSetSamplers(0, 1, &m_samplerState); // 샘플러도 함께 바인딩


	m_pDeviceContext->IASetInputLayout(m_inputLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	//m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	// Constant Buffer 바인딩
	//m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);



	//// 6. ConstantBuffer ?怨몄뒠 (??깃맒 ?袁⑤뼎)
	//m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);

	m_pDeviceContext->Draw(4, 0);
}

UINT QDirect3D11Widget::BytesPerPixel(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8_UNORM: return 1;
	case DXGI_FORMAT_R8G8_UNORM: return 2;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
	case DXGI_FORMAT_R16_UNORM: return 2;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	default: return 0; // 알 수 없는 포맷
	}
}

ID3D11Texture2D* QDirect3D11Widget::CreateTexture2D(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, const void* initData)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.Format = format;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = initData;
	data.SysMemPitch = width * BytesPerPixel(format); // 포맷에 따라 계산

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = device->CreateTexture2D(&desc, initData ? &data : nullptr, &texture);
	if (FAILED(hr)) {
		// 로그 출력 또는 예외 처리
		return nullptr;
	}

	return texture;
}

ID3D11ShaderResourceView* QDirect3D11Widget::CreateTextureSRV(ID3D11Device* device, ID3D11Texture2D* texture)
{
	ID3D11ShaderResourceView* srv = nullptr;

	HRESULT hr = device->CreateShaderResourceView(texture, nullptr, &srv); 
	if (FAILED(hr)) {
		// 로그 출력 또는 예외 처리
		return nullptr;
	}

	return srv;
}

void QDirect3D11Widget::InitTextures(UINT width, UINT height)
{
	////const void* initData = dicomSliceBuffer; // 예: uint8_t* 또는 uint16_t* 포인터
	//std::vector<uint32_t> testImage(width * height, 0xFF0000FF); // 파란색 RGBA
	//const void* initData = testImage.data();


	//m_texture = CreateTexture2D(m_pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, initData);
	//m_textureSRV = CreateTextureSRV(m_pDevice, m_texture);



	const UINT pixelCount = width * height;
	uint8_t* rawImage = new uint8_t[pixelCount * 4]; // 4 bytes per pixel (R,G,B,A)

	// 파란색 RGBA로 초기화
	for (UINT i = 0; i < pixelCount; ++i) {
		rawImage[i * 4 + 0] = 0x00; // R
		rawImage[i * 4 + 1] = 0x00; // G
		rawImage[i * 4 + 2] = 0xFF; // B
		rawImage[i * 4 + 3] = 0xFF; // A
	}

	const void* initData = rawImage;

	ID3D11Texture2D* tex = nullptr;
	tex = CreateTexture2D(m_pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, initData);
	m_textureSRV[0] = CreateTextureSRV(m_pDevice, tex);

	delete[] rawImage; // 메모리 해제

}

//void QDirect3D11Widget::InitTextures(UINT width, UINT height)
//{
//	m_texture = CreateTexture2D(m_pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, initData);
//	m_textureSRV = CreateTextureSRV(m_pDevice, m_texture);
//
//	// 샘플러 생성
//	D3D11_SAMPLER_DESC sampDesc = {};
//	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
//	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
//	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
//	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
//	sampDesc.MinLOD = 0;
//	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
//
//	HRESULT hr = m_pDevice->CreateSamplerState(&sampDesc, &m_samplerState);
//	if (FAILED(hr)) {
//		throw std::runtime_error("샘플러 생성 실패");
//	}
//}

void QDirect3D11Widget::InitSampler()
{
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = m_pDevice->CreateSamplerState(&sampDesc, &m_samplerState[0]);
	if (FAILED(hr)) {
		throw std::runtime_error("샘플러 생성 실패");
	}
}


void QDirect3D11Widget::InitializeGraphics()
{
	InitShaders();
	InitTextures(width()/2,height()/2);
	InitSampler();       // ← 여기서 샘플러 생성

	initializeRenderTargets();
}



void QDirect3D11Widget::InitShaders()
{
	using Microsoft::WRL::ComPtr;

	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
	ComPtr<ID3DBlob> errorBlob;

	// 1. Vertex Shader ?뚮똾???
	HRESULT hr = D3DCompileFromFile(
		L"VertexShader.hlsl", nullptr, nullptr,
		"VSMain", "vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&vsBlob, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Vertex Shader ?뚮똾?????쎈솭");
	}

	// 2. Pixel Shader ?뚮똾???
	hr = D3DCompileFromFile(
		L"PixelShader.hlsl", nullptr, nullptr,
		"PSMain", "ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&psBlob, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Pixel Shader ?뚮똾?????쎈솭");
	}

	// 3. ?怨쀬뵠??揶쏆빘猿???밴쉐
	DXCall(m_pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		nullptr, &m_vertexShader));
	DXCall(m_pDevice->CreatePixelShader(
		psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		nullptr, &m_pixelShader));

	// 4. ??낆젾 ??됱뵠?袁⑹뜍 ??밴쉐
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	DXCall(m_pDevice->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&m_inputLayout));

	// 5. Constant Buffer ??밴쉐
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(XMFLOAT4);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DXCall(m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_colorBuffer));

	struct Vertex {
		float x, y, z;
		float u, v;
	};

	// 6. ?類ㅼ젎 甕곌쑵????밴쉐
	Vertex vertices[] = {
		{ -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }, // ?ル슣湲?
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f }, // ?怨쀪맒
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, // ?ル슦釉?
		{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f }  // ?怨좊릭
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	DXCall(m_pDevice->CreateBuffer(&bd, &initData, &m_vertexBuffer));
}


D3D11_VIEWPORT QDirect3D11Widget::CreateViewport(int index)
{
	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(width()) / 2;
	vp.Height = static_cast<float>(height()) / 2;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	switch (index) {
	case 0: vp.TopLeftX = 0; vp.TopLeftY = 0; break;
	case 1: vp.TopLeftX = vp.Width; vp.TopLeftY = 0; break;
	case 2: vp.TopLeftX = 0; vp.TopLeftY = vp.Height; break;
	case 3: vp.TopLeftX = vp.Width; vp.TopLeftY = vp.Height; break;
	}

	return vp;
}
void QDirect3D11Widget::SetBackgroundColor(int index)
{
	switch (index) {
	case 0: m_BackColor = { 1.0f, 0.0f, 0.0f, 1.0f }; break; // ??몿而?
	case 1: m_BackColor = { 0.0f, 1.0f, 0.0f, 1.0f }; break; // ?λ뜄以?
	case 2: m_BackColor = { 0.0f, 0.0f, 1.0f, 1.0f }; break; // ???삂
	case 3: m_BackColor = { 1.0f, 1.0f, 0.0f, 1.0f }; break; // ?紐껋삂
	}
}


void QDirect3D11Widget::RenderSceneToTarget(int i)
{
	m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr);

	//float clearColor[4] = GetClearColorForIndex(i);


	//for (int i{}; i < m_RTViews.size(); ++i)
		SetBackgroundColor(i);

	m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], reinterpret_cast<const float*>(&m_BackColor));

	//SetViewportForTarget(i);
	//BindShadersForTarget(i);
	//BindResourcesForTarget(i);

	//m_pDeviceContext->Draw(...); // quad 또는 모델 출력



	 // 3. 해당 뷰포트 설정 (사분할)
	D3D11_VIEWPORT vp = CreateViewport(i);
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 4. 셰이더 바인딩
	m_pDeviceContext->IASetInputLayout(m_inputLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	// 5. 버텍스 버퍼 설정
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 6. 텍스처/샘플러 바인딩 (MPR 볼륨 or 현재 더미 텍스처)
	/*if (m_textureSRV) {
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_textureSRV);
	}*/
	if (m_samplerState[0]) {
		m_pDeviceContext->PSSetSamplers(0, 1, &m_samplerState[0]);
	}

	// 7. 드로우콜 (quad 출력)
	m_pDeviceContext->Draw(4, 0);
}


void QDirect3D11Widget::RenderAllQuads()
{
	//// 1. ???쐭 ??野???쇱젟
	//m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);

	//// 2. ?袁⑷퍥 ?遺얇늺 ?λ뜃由??(野꺜??獄쏄퀗瑗?
	//m_BackColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	//m_pDeviceContext->ClearRenderTargetView(m_pSwapChainRTV, reinterpret_cast<float*>(&m_BackColor));

	//// 3. ????醫딆쨮 ??깃맒 quad ?곗뮆??
	//for (int i = 0; i < 4; ++i) {
	//	D3D11_VIEWPORT vp = CreateViewport(i);  // ?됯퀬猷????쇱젟
	//	SetBackgroundColor(i);                  // ??깃맒 ??쇱젟
	//	UpdateColorBuffer();                    // ConstantBuffer????깃맒 ?袁⑤뼎
	//	DrawColoredQuad(vp);                    // ??깃맒 quad ?곗뮆??
	//}


    //250926
    //int currentZ = fileReader->m_depth / 2;
    //std::vector<uint8_t> axialSlice = fileReader->GenerateAxialSlice(currentZ);
    //axialTextureSRV = fileReader->CreateTextureFromSlice(axialSlice,
    //    fileReader->m_width, fileReader->m_height, m_pDevice);


	m_pDeviceContext->OMSetRenderTargets(4, m_RTViews.data(), nullptr);

	//// 1. 각 렌더 타겟에 개별 콘텐츠 렌더링
	//for (int i = 0; i < 4; ++i)
	//{
	////	m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], m_pDepthStencilView);
	//	

	//	float clearColor[4] = { 0,0,1.0f,1.0f };
	//	m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);
	//	RenderSceneToTarget(i); // ← 각 타겟에 그릴 내용
	//}


    //250926
    //axial texture 바인딩
    //



    //m_pDeviceContext->PSSetShaderResources(0, 1, &axialTextureSRV);
    //m_pDeviceContext->PSSetSamplers(0, 1, &m_samplerState[0]);


   


    


	m_pDeviceContext->PSSetShaderResources(0, 4, m_SRViews.data());     // tex0~tex3


  //  m_pDeviceContext->PSSetShaderResources(0, 4, axialTextureSRV.data());     // tex0~tex3
	m_pDeviceContext->PSSetSamplers(0, 4, m_samplerState.data());       // samp0~samp3


    //m_pDeviceContext->PSSetShaderResources(0, 1, &axialTextureSRV);


   
	//m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);

	//for (int i = 0; i < 4; ++i)
	//{
	//	D3D11_VIEWPORT vp = CreateViewport(i); // 사분할 영역
	//	DrawQuadWithTexture(m_SRViews[i], vp); // RTV 결과를 quad로 출력
	//}


	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pDeviceContext->IASetInputLayout(m_inputLayout);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	m_pDeviceContext->Draw(4, 0); // 4개의 정점으로 quad 출력



	//// 2. 백버퍼에 출력할 준비
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);
	////m_pDeviceContext->ClearRenderTargetView(m_pSwapChainRTV, reinterpret_cast<float*>(&m_BackColor));

	// 3. 각 렌더 타겟 텍스처를 quad로 출력
	for (int i{}; i < 4; ++i)
	{
		D3D11_VIEWPORT vp = CreateViewport(i); // ← 4분할 뷰포트 계산
		DrawQuadWithTexture(m_SRViews[i], vp);      // ← 여기서 호출!
	}


  

    ImGuiIO& io = ImGui::GetIO();

 //   //C:\\Projects\\sample\\Qt3DViewer\\x64\\Debug\\NotoSansCJKkr-Regular.otf

 //   //auto path = QCoreApplication::applicationDirPath()+"NotoSansCJKkr-Regular.otf";
 //   //QByteArray fontPathUtf8 = path.toUtf8(); // UTF-8로 변환


 //   //io.Fonts->AddFontFromFileTTF("C:\\Projects\\sample\\Qt3DViewer\\x64\\Debug\\NotoSansCJKkr-Regular.otf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());


 //   ImFontConfig font_cfg;
 //   font_cfg.OversampleH = 3;
 //   font_cfg.OversampleV = 3;
 //   font_cfg.PixelSnapH = true;

 //   //io.Fonts->AddFontFromFileTTF("C:\\Projects\\sample\\Qt3DViewer\\x64\\Debug\\NotoSansCJKkr-Regular.otf", 20.0f, &font_cfg, io.Fonts->GetGlyphRangesKorean());


 //   //static const ImWchar* ranges = io.Fonts->GetGlyphRangesKorean();
 //   //io.Fonts->AddFontFromFileTTF("C:\\Projects\\sample\\Qt3DViewer\\x64\\Debug\\NotoSansCJKkr-Regular.otf", 20.0f, &font_cfg, ranges);
 //   
 //   static const ImWchar customRange[] = {
 //   0x0020, 0x00FF, // 기본 라틴
 //   0x3131, 0x3163, // 한글 자모
 //   0xAC00, 0xD7A3, // 한글 완성형
 //   0
 //   };
 //   io.Fonts->AddFontFromFileTTF("NotoSansCJKkr-Regular.otf", 30.0f, &font_cfg, customRange);

 //   /*ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
 //   if (font) {
 //       const ImFontGlyph* glyph = font->FindGlyph(u'한');
 //       if (glyph) {
 //           std::cout << "[디버깅] '한' 글리프 있음! Index: " << glyph->Codepoint << std::endl;
 //       }
 //       else {
 //           std::cout << "[디버깅] '한' 글리프 없음! => 폰트에 한글이 포함되지 않았거나 범위 누락" << std::endl;
 //       }

 //       std::cout << "[디버깅] 전체 글리프 개수: " << font->Glyphs.Size << std::endl;
 //   }
 //   else {
 //       std::cout << "[디버깅] 폰트 로딩 실패! Fonts[0]이 nullptr입니다." << std::endl;
 //   }*/

 ///*   std::string name = fileReader->patientName.c_str();
 //   qDebug() << "[디버깅] 환자 이름 원본:"
 //       << QString::fromStdString(fileReader->patientName);*/

 //   QString name = QString::fromLocal8Bit(fileReader->patientName.c_str());
 //   qDebug() << "[디버깅] 환자 이름 (로컬 인코딩 → UTF-8):" << name;

 //  
 //   //// UTF-8 유효성 검사 (간단한 방식)
 //   //bool isUtf8 = true;
 //   //for (size_t i = 0; i < name.size(); ++i) {
 //   //    unsigned char c = name[i];
 //   //    if (c >= 0x80) {
 //   //        if ((c & 0xE0) == 0xC0 && i + 1 < name.size() &&
 //   //            (name[i + 1] & 0xC0) == 0x80) {
 //   //            i += 1;
 //   //        }
 //   //        else if ((c & 0xF0) == 0xE0 && i + 2 < name.size() &&
 //   //            (name[i + 1] & 0xC0) == 0x80 &&
 //   //            (name[i + 2] & 0xC0) == 0x80) {
 //   //            i += 2;
 //   //        }
 //   //        else {
 //   //            isUtf8 = false;
 //   //            break;
 //   //        }
 //   //    }
 //   //}
 //   //qDebug() << "[디버깅] 환자 이름 UTF-8 여부:"
 //   //    << (isUtf8 ? "✅ UTF-8" : "❌ UTF-8 아님");


 //   io.Fonts->Build();



 //   //ImFont* font = io.Fonts->Fonts[0];
 //   //if (font) {
 //   //    const ImFontGlyph* glyph = font->FindGlyph(u'신');
 //   //    qDebug() << "[디버깅] '신' 글리프 있음:" << (glyph ? "✅ 있음" : "❌ 없음");
 //   //}







    // 폰트 등록은 여기서!
    static bool fontLoaded = false;
    if (!fontLoaded) {
        ImFontConfig font_cfg;
        font_cfg.OversampleH = 3;
        font_cfg.OversampleV = 3;
        font_cfg.PixelSnapH = true;

        static const ImWchar customRange[] = {
            0x0020, 0x00FF,
            0x3131, 0x3163,
            0xAC00, 0xD7A3,
            0
        };

        io.Fonts->AddFontFromFileTTF("NotoSansCJKkr-Regular.otf", 18.0f, &font_cfg, customRange);
        io.Fonts->Build();
        fontLoaded = true;
    }



    // ✅ 여기에 ImGui 렌더링 추가!
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();





  
    ImGui::SetNextWindowPos(ImVec2(0, 0)); // 좌측 상단 위치
    ImGui::SetNextWindowSize(ImVec2(130, 150));
    ImGui::Begin((QString::fromLocal8Bit("환자 정보")).toUtf8().constData(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
   // ImGui::Text("Patient Name: %s", fileReader->patientName.c_str());
        QString name = QString::fromLocal8Bit(fileReader->patientName.c_str());

    //ImGui::Text(QString::fromLocal8Bit("이름: ")%s", name.toUtf8().constData());
        QString label = QString::fromLocal8Bit("이름 : ") + name;
        ImGui::Text("%s", label.toUtf8().constData());


    //qDebug()  << "name.toUtf8().constData() : " << name.toUtf8().constData() << endl;

        QString patientMF = QString::fromLocal8Bit(fileReader->patientMF.c_str());
        QString patientMFLabel = QString::fromLocal8Bit("성별 : ") + patientMF;
        ImGui::Text("%s", patientMFLabel.toUtf8().constData());

        QString patientID = QString::fromLocal8Bit(fileReader->patientID.c_str());
        QString patientIDLabel = QString::fromLocal8Bit("아이디 : ") + patientID;
        ImGui::Text("%s", patientIDLabel.toUtf8().constData());

        /*QString patientAge = QString::fromLocal8Bit(fileReader->patientAge.c_str());
        QString patientAgeLabel = QString::fromLocal8Bit("나이 : ") + patientAge;
        ImGui::Text("%s", patientAgeLabel.toUtf8().constData());*/

        QString patientBirth = QString::fromLocal8Bit(fileReader->birthDate.c_str());
        QString patientBirthDate = QString::fromLocal8Bit("생년월일 : ") + patientBirth;
        ImGui::Text("%s", patientBirthDate.toUtf8().constData());

        QString studyDate = QString::fromLocal8Bit(fileReader->studyDate.c_str());
        QString studyDateLabel = QString::fromLocal8Bit("검사일 : ") + studyDate;
        ImGui::Text("%s", studyDateLabel.toUtf8().constData());

    ////ImGui::Text("Birth Date: %s", fileReader->birthDate.c_str());
    //ImGui::Text("성별: %s", fileReader->patientMF.c_str());
    //ImGui::Text("아이디: %s", fileReader->patientID.c_str());
    //ImGui::Text("나이: %s", fileReader->patientAge.c_str());
    //ImGui::Text("검사일: %s", fileReader->studyDate.c_str());

    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());





	//// 4. 스왑체인 Present



	m_pSwapChain->Present(1, 0);




	//m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);


	//DrawFullScreenQuad();


	emit rendered(); // Qt ??볥젃??
}

void QDirect3D11Widget::DrawFullScreenQuad()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pDeviceContext->IASetInputLayout(m_inputLayout);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	m_pDeviceContext->Draw(4, 0); // 4개의 정점으로 quad 출력
}

void QDirect3D11Widget::DrawQuadWithTexture(ID3D11ShaderResourceView* pSRV, const D3D11_VIEWPORT& vp)
{
	m_pDeviceContext->RSSetViewports(1, &vp);
	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);
	m_pDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	m_pDeviceContext->IASetInputLayout(m_inputLayout);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pDeviceContext->Draw(4, 0);
}



void QDirect3D11Widget::onReset()
{
	// 1. 疫꿸퀣???귐딅꺖????곸젫
	for (auto& view : m_RTViews)
		ReleaseObject(view);
	m_RTViews.clear();

	//	ReleaseObject(m_pSwapChainRTV);

	ID3D11Texture2D* pBackBuffer = Q_NULLPTR;

	
	ReleaseObject(pBackBuffer);

	// 4. ??쎈늄??쎄쾿?????쐭 ??野???源??
	//initializeRenderTargets(); // ??????λ땾?癒?퐣 m_RTViews, m_SRViews ??밴쉐
}



void QDirect3D11Widget::resetEnvironment()
{
	// TODO: Add your own custom default environment, i.e:
	// m_pCamera->resetCamera();

	onReset();

	if (!m_bRenderActive) tick();
}

void QDirect3D11Widget::wheelEvent(QWheelEvent* event)
{
	if (event->angleDelta().x() == 0)
	{
		// TODO: Update your camera position based on the delta value.
	}
	else if (event->angleDelta().x() !=
		0) // horizontal scrolling - mice with another side scroller.
	{
		// m_pCamera->MouseWheelH += (float)(event->angleDelta().y() / WHEEL_DELTA);
	}
	else if (event->angleDelta().y() != 0)
	{
		// m_pCamera->MouseWheel += (float)(event->angleDelta().y() / WHEEL_DELTA);
	}

	QWidget::wheelEvent(event);
}

QPaintEngine* QDirect3D11Widget::paintEngine() const
{
	return Q_NULLPTR;
}

void QDirect3D11Widget::paintEvent(QPaintEvent* event) {}

void QDirect3D11Widget::resizeEvent(QResizeEvent* event)
{
	if (m_bDeviceInitialized)
	{
		onReset();
		emit widgetResized();
	}

	QWidget::resizeEvent(event);
}

bool QDirect3D11Widget::event(QEvent* event)
{
	switch (event->type())
	{
		// Workaround for https://bugreports.qt.io/browse/QTBUG-42183 to get key strokes.
		// To make sure that we always have focus on the widget when we enter the rect area.
	case QEvent::Enter:
	case QEvent::FocusIn:
	case QEvent::FocusAboutToChange:
		if (::GetFocus() != m_hWnd)
		{
			QWidget* nativeParent = this;
			while (true)
			{
				if (nativeParent->isWindow()) break;

				QWidget* parent = nativeParent->nativeParentWidget();
				if (!parent) break;

				nativeParent = parent;
			}

			if (nativeParent && nativeParent != this &&
				::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
				::SetFocus(m_hWnd);
		}
		break;
	case QEvent::KeyPress:
		emit keyPressed((QKeyEvent*)event);
		break;
	case QEvent::MouseMove:
		emit mouseMoved((QMouseEvent*)event);
		break;
	case QEvent::MouseButtonPress:
		emit mouseClicked((QMouseEvent*)event);
		break;
	case QEvent::MouseButtonRelease:
		emit mouseReleased((QMouseEvent*)event);
		break;
	}

	return QWidget::event(event);
}

LRESULT QDirect3D11Widget::WndProc(MSG* pMsg)
{
	// Process wheel events using Qt's event-system.
	if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) return false;

	return false;
}

#if QT_VERSION >= 0x050000
bool QDirect3D11Widget::nativeEvent(const QByteArray& eventType,
	void* message,
	long* result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

#    ifdef Q_OS_WIN
	MSG* pMsg = reinterpret_cast<MSG*>(message);
	return WndProc(pMsg);
#    endif

	return QWidget::nativeEvent(eventType, message, result);
}

#else // QT_VERSION < 0x050000
bool QDirect3D11Widget::winEvent(MSG* message, long* result)
{
	Q_UNUSED(result);

#    ifdef Q_OS_WIN
	MSG* pMsg = reinterpret_cast<MSG*>(message);
	return WndProc(pMsg);
#    endif

	return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000
