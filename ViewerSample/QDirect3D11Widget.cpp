/*
 *
 */
#pragma comment(lib, "d3d11.lib")

#include "QDirect3D11Widget.h"

#include <QDebug>

//Qt????源???룐뫂遊?癒?퐣 獄쏆뮇源??롫뮉 ??源?紐? ??쀬겱??롫뮉 揶쏆빘猿?
#include <QEvent>

//Qt?癒?퐣 筌띾뜆????醫롮뵠???紐껋삌??ㅻ굡 ??쎄쾿嚥???뽯뮞筌ｌ꼶? 筌ｌ꼶??????????롫뮉 ??삳쐭
#include <QWheelEvent>



#include "stdafx.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshader.h"
#include "graphicsclass.h"





constexpr int FPS_LIMIT = 60.0f;
constexpr int MS_PER_FRAME = (int)((1.0f / FPS_LIMIT) * 1000.0f);

QDirect3D11Widget::QDirect3D11Widget(QWidget * parent)
	: QWidget(parent)
	, m_pDevice(Q_NULLPTR)
	, m_pDeviceContext(Q_NULLPTR)
	, m_pSwapChain(Q_NULLPTR)
	, m_RTViews(4,Q_NULLPTR)
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

	for(auto& view: m_RTViews)
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

void QDirect3D11Widget::showEvent(QShowEvent * event)
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

	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);


	initializeRenderTargets();

	createSwapChainRTV();

	return true;
}

void QDirect3D11Widget::onFrame()
{
	if (m_bRenderActive) tick();

	beginScene();
	render();
	endScene();
}

void QDirect3D11Widget::beginScene()
{
	//???쐭??野껋옕? ??甕곕뜄彛???쇱젟
	//DX11?? 筌ㅼ뮆? 8揶쏆뮇?????쐭??野껋옕????덈뻻??獄쏅뗄???븍막 ????됱벉


	/*m_pDeviceContext->OMSetRenderTargets
	(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), NULL);*/

	for (int i{}; i < m_RTViews.size(); ++i) {


		D3D11_VIEWPORT vp = {};
		vp.Width = width() / 2.0f;
		vp.Height = height() / 2.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;


		//switch (i) {
		//case 0: vp.TopLeftX = 0;           vp.TopLeftY = 0;           break; // 좌상단
		//case 1: vp.TopLeftX = width() / 2; vp.TopLeftY = 0;           break; // 우상단
		//case 2: vp.TopLeftX = 0;           vp.TopLeftY = height() / 2; break; // 좌하단
		//case 3: vp.TopLeftX = width() / 2; vp.TopLeftY = height() / 2; break; // 우하단
		//}


		if (0 == i) {
			vp.TopLeftX = 0;           
			vp.TopLeftY = 0;

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 빨강

			//float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			//m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 개별 설정
			//m_pDeviceContext->RSSetViewports(1, &vp);
			//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);
		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2; 
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 초록

			//float clearColor[4] = { 0.0f,1.0f,  0.0f, 1.0f };
			//m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 개별 설정
			//m_pDeviceContext->RSSetViewports(1, &vp);
			//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);

		}
		else if (2 == i) {
			vp.TopLeftX = 0;           
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // 파랑


			//float clearColor[4] = { 0.0f,0.0f,  1.0f, 1.0f };
			//m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 개별 설정
			//m_pDeviceContext->RSSetViewports(1, &vp);
			//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);
		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2; 
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // 노랑

			//float clearColor[4] = { 1.0f,1.0f,  0.0f,1.0f };
			//m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 개별 설정
			//m_pDeviceContext->RSSetViewports(1, &vp);
			//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);
		}

		//float clearColor[4] = { m_BackColor.r, m_BackColor.g, m_BackColor.b, m_BackColor.a };


		m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 개별 설정

	//	m_pDeviceContext->CopyResource(m_pSwapChain, m_RTViews[0]); // 또는 마지막 타겟

		m_pDeviceContext->RSSetViewports(1, &vp);
		//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);

		//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], (const Float*)m_BackColor);
		m_pDeviceContext->ClearRenderTargetView(m_RTViews[i],
			reinterpret_cast<const float *>(&m_BackColor));
		




		//m_pDeviceContext->RSSetViewports(1, &vp);

		//// 렌더 타겟 설정 및 클리어
		////m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr);
		//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], reinterpret_cast<const float*>(&m_BackColor));

		//////// 원하는 콘텐츠 렌더링
		//////RenderSceneForTarget(i);


		////// 뷰포트 설정 (각 타겟에 맞게)
		////D3D11_VIEWPORT vp = {};
		////vp.TopLeftX = 0;
		////vp.TopLeftY = 0;
		////vp.Width = width();
		////vp.Height = height();
		////vp.MinDepth = 0.0f;
		////vp.MaxDepth = 1.0f;
		////m_pDeviceContext->RSSetViewports(1, &vp);







		////m_pDeviceContext->ClearRenderTargetView(m_RTViews[i],
		////	reinterpret_cast<const float *>(&m_BackColor));



		//////// 렌더링
		//////deviceContext->ClearRenderTargetView(m_RTViews[i], clearColor);
		//////DrawSceneForTarget(i); // 각 타겟에 맞는 콘텐츠 렌더링


	}


	/*m_pDeviceContext->OMSetRenderTargets
	(1, m_RTViews.data(), NULL);

	for (int i{}; i < 1; ++i) {
		m_pDeviceContext->ClearRenderTargetView(m_RTViews[i],
			reinterpret_cast<const float *>(&m_BackColor));
	}*/
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
		// 1. 텍스처 생성
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

		// 2. RenderTargetView 생성
		ID3D11RenderTargetView* pRTV = nullptr;
		DXCall(m_pDevice->CreateRenderTargetView(pTexture, nullptr, &pRTV));
		m_RTViews.push_back(pRTV);

		// 3. ShaderResourceView 생성
		ID3D11ShaderResourceView* pSRV = nullptr;
		DXCall(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pSRV));
		m_SRViews.push_back(pSRV);

		// 4. 텍스처 해제
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

void QDirect3D11Widget::DrawQuadWithTexture(ID3D11ShaderResourceView* pSRV, const D3D11_VIEWPORT& vp)
{
	// 1. 뷰포트 설정
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 2. 셰이더 바인딩
	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	// 3. 텍스처 바인딩
	m_pDeviceContext->PSSetShaderResources(0, 1, &pSRV);

	// 4. 정점 버퍼 설정
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 5. 드로우 호출
	m_pDeviceContext->Draw(4, 0); // 사각형
}

void QDirect3D11Widget::render()
{
	// TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
	// important component, otherwise do it in the MainWindow.
	// m_pCamera->Apply();




	//m_pDeviceContext->OMSetRenderTargets(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), nullptr);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);


	// 뷰포트 설정 (전체 화면)
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(width());
	vp.Height = static_cast<float>(height());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &vp);

	// 각 렌더 타겟을 ShaderResourceView로 화면에 출력
	for (int i{}; i < m_SRViews.size(); ++i) {
		// 예: DrawQuadWithTexture(m_SRViews[i], viewport[i]);
		// 이 부분은 셰이더와 정점 버퍼로 구현해야 해요


		if (0 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 빨강
		}
		else if (1 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = 0;

			m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // 초록
		}
		else if (2 == i) {
			vp.TopLeftX = 0;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // 파랑
		}
		else if (3 == i) {
			vp.TopLeftX = width() / 2;
			vp.TopLeftY = height() / 2;

			m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // 노랑
		}




		m_pDeviceContext->RSSetViewports(1, &vp); // 뷰포트 설정

		DrawQuadWithTexture(m_SRViews[i], vp); // viewport[i]는 위치 정보


	}


	emit rendered();
}
//
//void QDirect3D11Widget::onReset()
//{
//	ID3D11Texture2D * pBackBuffer = Q_NULLPTR;
//
//
//
//	//??쇱넁筌ｋ똻???甕곌쑵????由곁몴??袁⑹삺 ??덈즲????由??筌띿쉳苡?鈺곌퀣??
//	DXCall(m_pSwapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0));
//
//	//?귐딄텢??곸グ????甕곌쑵?곭몴?揶쎛?紐꾩긾
////	DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));//250919 ??由?雅뚯눘苑?筌ｌ꼶??
//
//	//獄쏄퉭苡????용뮞筌ｌ꼶? 疫꿸퀡而??곗쨮 ???쐭??野껋옓????밴쉐
//	//DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTView));
//
//
//	for (int i{}; i < m_RTViews.size(); ++i) {
//		//ID3D11Texture2D* pTexture = nullptr;
//		ID3D11Texture2D* pBackBuffer = nullptr;
//
//		//??쇱넁筌ｋ똻??癒?퐣 獄쏄퉭苡????곕┛ (癰귣똾????롪돌????쇱넁筌ｋ똻??癒?퐣 獄쏆꼶??怨몄몵嚥???노뮉 
//		//野껋럩????諭????닌듼?????춸)
//		DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
//
//		//???쐭??野껋옓????밴쉐??곴퐣 甕겸돧苑??????		
//		DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_RTViews[i]));
//	//	m_RTViews.push_back(pBackBuffer);
//
//
//		//獄쏄퉭苡????곸젫
//		ReleaseObject(pBackBuffer);
//	}
//
//
//	for (auto& view : m_RTViews)
//		ReleaseObject(view);
//
//	m_RTViews.clear();
//
//	if(m_pSwapChainRTV)
//		ReleaseObject(m_pSwapChainRTV); // 기존 RTV 해제
//
//}

void QDirect3D11Widget::onReset()
{
	// 1. 기존 리소스 해제
	for (auto& view : m_RTViews)
		ReleaseObject(view);
	m_RTViews.clear();

//	ReleaseObject(m_pSwapChainRTV);

	ID3D11Texture2D * pBackBuffer = Q_NULLPTR;

	//if (m_pSwapChain) {
	//	// 2. 스왑체인 리사이즈
	//	DXCall(m_pSwapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0));

	//	//// 3. 백버퍼 RTV 재생성
	//	//ID3D11Texture2D* pBackBuffer = nullptr;

	//	//HRESULT hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	//	//if (FAILED(hr)) {
	//	//	qDebug() << "GetBuffer 실패! HRESULT:" << QString::number(hr, 16);
	//	//}




	//	DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	//}



	//DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pSwapChainRTV));
	ReleaseObject(pBackBuffer);

	// 4. 오프스크린 렌더 타겟 재생성
	initializeRenderTargets(); // ← 이 함수에서 m_RTViews, m_SRViews 생성
}


//void QDirect3D11Widget::onReset()
//{
//	ID3D11Texture2D * pBackBuffer = Q_NULLPTR;
//	ReleaseObject(m_pRTView);
//	DXCall(m_pSwapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0));
//	DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
//	DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTView));
//	ReleaseObject(pBackBuffer);
//}

void QDirect3D11Widget::resetEnvironment()
{
	// TODO: Add your own custom default environment, i.e:
	// m_pCamera->resetCamera();

	onReset();

	if (!m_bRenderActive) tick();
}

void QDirect3D11Widget::wheelEvent(QWheelEvent * event)
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

QPaintEngine * QDirect3D11Widget::paintEngine() const
{
	return Q_NULLPTR;
}

void QDirect3D11Widget::paintEvent(QPaintEvent * event) {}

void QDirect3D11Widget::resizeEvent(QResizeEvent * event)
{
	if (m_bDeviceInitialized)
	{
		onReset();
		emit widgetResized();
	}

	QWidget::resizeEvent(event);
}

bool QDirect3D11Widget::event(QEvent * event)
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
			QWidget * nativeParent = this;
			while (true)
			{
				if (nativeParent->isWindow()) break;

				QWidget * parent = nativeParent->nativeParentWidget();
				if (!parent) break;

				nativeParent = parent;
			}

			if (nativeParent && nativeParent != this &&
				::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
				::SetFocus(m_hWnd);
		}
		break;
	case QEvent::KeyPress:
		emit keyPressed((QKeyEvent *)event);
		break;
	case QEvent::MouseMove:
		emit mouseMoved((QMouseEvent *)event);
		break;
	case QEvent::MouseButtonPress:
		emit mouseClicked((QMouseEvent *)event);
		break;
	case QEvent::MouseButtonRelease:
		emit mouseReleased((QMouseEvent *)event);
		break;
	}

	return QWidget::event(event);
}

LRESULT QDirect3D11Widget::WndProc(MSG * pMsg)
{
	// Process wheel events using Qt's event-system.
	if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) return false;

	return false;
}

#if QT_VERSION >= 0x050000
bool QDirect3D11Widget::nativeEvent(const QByteArray & eventType,
	void *             message,
	long *             result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

#    ifdef Q_OS_WIN
	MSG * pMsg = reinterpret_cast<MSG *>(message);
	return WndProc(pMsg);
#    endif

	return QWidget::nativeEvent(eventType, message, result);
}

#else // QT_VERSION < 0x050000
bool QDirect3D11Widget::winEvent(MSG * message, long * result)
{
	Q_UNUSED(result);

#    ifdef Q_OS_WIN
	MSG * pMsg = reinterpret_cast<MSG *>(message);
	return WndProc(pMsg);
#    endif

	return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000
