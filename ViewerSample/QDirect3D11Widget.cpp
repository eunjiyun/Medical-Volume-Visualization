/*
 *
 */
#pragma comment(lib, "d3d11.lib")

#include "QDirect3D11Widget.h"

#include <QDebug>

 //Qt?????繹???猷먮쳜???????꾩룇裕뉑틦??濡ル츎 ???繹?筌? ???ш껑??濡ル츎 ?띠룇鍮섊뙼?
#include <QEvent>

//Qt?????嶺뚮씭??????ル‘逾???筌뤾퍔????산덧 ???꾩씩????戮?츩嶺뚳퐣瑗? 嶺뚳퐣瑗??????????濡ル츎 ???녹맠
#include <QWheelEvent>



#include "stdafx.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshader.h"
#include "graphicsclass.h"
#include <wrl/client.h>
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





	initializeRenderTargets();

	createSwapChainRTV();
	// ???곗씠??珥덇린??
	InitShaders();

	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);

	return true;
}

void QDirect3D11Widget::onFrame()
{
	if (m_bRenderActive) tick();

	beginScene();
	//render();
	RenderAllQuads();
	endScene();
}

void QDirect3D11Widget::beginScene()
{
	//???????롪퍔?? ???뺢퀡?꾢퐲????깆젧
	//DX11?? 嶺뚣끉裕? 8?띠룇裕?????????롪퍔??????덈뻣???꾩룆????釉띾쭑 ?????깅쾳


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

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 鍮④컯

		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 珥덈줉


		}
		else if (2 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ?뚮옉

		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?몃옉

		}

		
		m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 媛쒕퀎 ?ㅼ젙

	
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

	for (int i = 0; i < 4; ++i) {
		// 1. ?띿뒪泥??앹꽦
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

		// 2. RenderTargetView ?앹꽦
		ID3D11RenderTargetView* pRTV = nullptr;
		DXCall(m_pDevice->CreateRenderTargetView(pTexture, nullptr, &pRTV));
		m_RTViews.push_back(pRTV);

		// 3. ShaderResourceView ?앹꽦
		ID3D11ShaderResourceView* pSRV = nullptr;
		DXCall(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pSRV));
		m_SRViews.push_back(pSRV);

		// 4. ?띿뒪泥??댁젣
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


	// 酉고룷???ㅼ젙 (?꾩껜 ?붾㈃)
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(width());
	vp.Height = static_cast<float>(height());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 媛??뚮뜑 ?寃잛쓣 ShaderResourceView濡??붾㈃??異쒕젰
	for (int i{}; i < m_SRViews.size(); ++i) {
		// ?? DrawQuadWithTexture(m_SRViews[i], viewport[i]);
		// ??遺遺꾩? ?곗씠?붿? ?뺤젏 踰꾪띁濡?援ы쁽?댁빞 ?댁슂


		if (0 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 鍮④컯
		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 珥덈줉
		}
		else if (2 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ?뚮옉
		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?몃옉
		}




		m_pDeviceContext->RSSetViewports(1, &vp); // 酉고룷???ㅼ젙

		DrawQuadWithTexture(m_SRViews[i], vp); // viewport[i]???꾩튂 ?뺣낫


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

	m_pDeviceContext->IASetInputLayout(m_inputLayout);

	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);


	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 6. ConstantBuffer ?곸슜 (?됱긽 ?꾨떖)
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);

	m_pDeviceContext->Draw(4, 0);
}
void QDirect3D11Widget::InitShaders()
{
	using Microsoft::WRL::ComPtr;

	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
	ComPtr<ID3DBlob> errorBlob;

	// 1. Vertex Shader 而댄뙆??
	HRESULT hr = D3DCompileFromFile(
		L"VertexShader.hlsl", nullptr, nullptr,
		"VSMain", "vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&vsBlob, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Vertex Shader 而댄뙆???ㅽ뙣");
	}

	// 2. Pixel Shader 而댄뙆??
	hr = D3DCompileFromFile(
		L"PixelShader.hlsl", nullptr, nullptr,
		"PSMain", "ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&psBlob, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Pixel Shader 而댄뙆???ㅽ뙣");
	}

	// 3. ?곗씠??媛앹껜 ?앹꽦
	DXCall(m_pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		nullptr, &m_vertexShader));
	DXCall(m_pDevice->CreatePixelShader(
		psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		nullptr, &m_pixelShader));

	// 4. ?낅젰 ?덉씠?꾩썐 ?앹꽦
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

	// 5. Constant Buffer ?앹꽦
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

	// 6. ?뺤젏 踰꾪띁 ?앹꽦
	Vertex vertices[] = {
		{ -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }, // 醫뚯긽
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f }, // ?곗긽
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, // 醫뚰븯
		{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f }  // ?고븯
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
	case 0: m_BackColor = { 1.0f, 0.0f, 0.0f, 1.0f }; break; // 鍮④컯
	case 1: m_BackColor = { 0.0f, 1.0f, 0.0f, 1.0f }; break; // 珥덈줉
	case 2: m_BackColor = { 0.0f, 0.0f, 1.0f, 1.0f }; break; // ?뚮옉
	case 3: m_BackColor = { 1.0f, 1.0f, 0.0f, 1.0f }; break; // ?몃옉
	}
}

void QDirect3D11Widget::RenderAllQuads()
{
	// 1. ?뚮뜑 ?寃??ㅼ젙
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);

	// 2. ?꾩껜 ?붾㈃ 珥덇린??(寃??諛곌꼍)
	m_BackColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pSwapChainRTV, reinterpret_cast<float*>(&m_BackColor));

	// 3. ?щ텇?좊줈 ?됱긽 quad 異쒕젰
	for (int i = 0; i < 4; ++i) {
		D3D11_VIEWPORT vp = CreateViewport(i);  // 酉고룷???ㅼ젙
		SetBackgroundColor(i);                  // ?됱긽 ?ㅼ젙
		UpdateColorBuffer();                    // ConstantBuffer???됱긽 ?꾨떖
		DrawColoredQuad(vp);                    // ?됱긽 quad 異쒕젰
	}

	emit rendered(); // Qt ?쒓렇??
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
	// 1. 湲곗〈 由ъ냼???댁젣
	for (auto& view : m_RTViews)
		ReleaseObject(view);
	m_RTViews.clear();

	//	ReleaseObject(m_pSwapChainRTV);

	ID3D11Texture2D* pBackBuffer = Q_NULLPTR;

	
	ReleaseObject(pBackBuffer);

	// 4. ?ㅽ봽?ㅽ겕由??뚮뜑 ?寃??ъ깮??
	initializeRenderTargets(); // ?????⑥닔?먯꽌 m_RTViews, m_SRViews ?앹꽦
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
