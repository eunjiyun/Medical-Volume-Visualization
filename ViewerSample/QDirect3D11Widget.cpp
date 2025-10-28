
#include "QDirect3D11Widget.h"
#include <QDebug>
#include <QEvent>
#include <QWheelEvent>


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
	/* , m_RTViews(4, Q_NULLPTR)*/
	, m_hWnd(reinterpret_cast<HWND>(winId()))
	, m_bDeviceInitialized(false)
	, m_bRenderActive(false)
	, m_bStarted(false)
	/*  , m_BackColor{ 0.0f, 0.135f, 0.481f, 1.0f }*/
	, m_BackColor{ 0.0f, 0.0f, 0.0f, 1.0f }
	, m_volumeVS(nullptr)           // ← 추가
	, m_volumePS(nullptr)           // ← 추가
	, m_volumeConstantBuffer(nullptr)  // ← 추가
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

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);



	scrollAxial = new QScrollBar(Qt::Vertical, this);
	scrollCoronal = new QScrollBar(Qt::Vertical, this);
	scrollSagittal = new QScrollBar(Qt::Vertical, this);

	// 라벨 생성
	labelVolume = new QLabel("Volume", this);
	labelAxial = new QLabel("Axial(A)", this);
	labelCoronal = new QLabel("Coronal(C)", this);
	labelSagittal = new QLabel("Sagittal(S)", this);



	// Axial 스크롤바 - 보라/마젠타
	scrollAxial->setStyleSheet(
		"QScrollBar:vertical {"
		"   background: #1a1a1a;"
		"   border: 2px solid #D87FD8;"
		"   border-radius: 4px;"
		"   width: 16px;"
		"   margin: 18px 0px 18px 0px;"  // 위아래 화살표 공간
		"}"
		"QScrollBar::handle:vertical {"
		"   background: #D87FD8;"
		"   border-radius: 3px;"
		"   min-height: 30px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"   background: #E89FE8;"
		"}"
		"QScrollBar::add-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: bottom;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #D87FD8;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: top;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #D87FD8;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::up-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-bottom: 6px solid #D87FD8;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::down-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-top: 6px solid #D87FD8;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"   background: none;"
		"}"
	);

	// Coronal 스크롤바 - 청록색
	scrollCoronal->setStyleSheet(
		"QScrollBar:vertical {"
		"   background: #1a1a1a;"
		"   border: 2px solid #00CED1;"
		"   border-radius: 4px;"
		"   width: 16px;"
		"   margin: 18px 0px 18px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"   background: #00CED1;"
		"   border-radius: 3px;"
		"   min-height: 30px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"   background: #20DEE1;"
		"}"
		"QScrollBar::add-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: bottom;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #00CED1;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: top;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #00CED1;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::up-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-bottom: 6px solid #00CED1;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::down-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-top: 6px solid #00CED1;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"   background: none;"
		"}"
	);

	// Sagittal 스크롤바 - 노란색
	scrollSagittal->setStyleSheet(
		"QScrollBar:vertical {"
		"   background: #1a1a1a;"
		"   border: 2px solid #FFD700;"
		"   border-radius: 4px;"
		"   width: 16px;"
		"   margin: 18px 0px 18px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"   background: #FFD700;"
		"   border-radius: 3px;"
		"   min-height: 30px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"   background: #FFE44D;"
		"}"
		"QScrollBar::add-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: bottom;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #FFD700;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: top;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #FFD700;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::up-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-bottom: 6px solid #FFD700;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::down-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-top: 6px solid #FFD700;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"   background: none;"
		"}"
	);




	// Coronal 스크롤바 - 청록색
	scrollCoronal->setStyleSheet(
		"QScrollBar:vertical {"
		"   background: #1a1a1a;"
		"   border: 2px solid #00CED1;"
		"   border-radius: 4px;"
		"   width: 16px;"
		"   margin: 18px 0px 18px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"   background: #00CED1;"
		"   border-radius: 3px;"
		"   min-height: 30px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"   background: #20DEE1;"
		"}"
		"QScrollBar::add-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: bottom;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #00CED1;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: top;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #00CED1;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::up-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-bottom: 6px solid #00CED1;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::down-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-top: 6px solid #00CED1;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"   background: none;"
		"}"
	);

	// Sagittal 스크롤바 - 노란색
	scrollSagittal->setStyleSheet(
		"QScrollBar:vertical {"
		"   background: #1a1a1a;"
		"   border: 2px solid #FFD700;"
		"   border-radius: 4px;"
		"   width: 16px;"
		"   margin: 18px 0px 18px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"   background: #FFD700;"
		"   border-radius: 3px;"
		"   min-height: 30px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"   background: #FFE44D;"
		"}"
		"QScrollBar::add-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: bottom;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #FFD700;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"   background: #2a2a2a;"
		"   height: 18px;"
		"   subcontrol-position: top;"
		"   subcontrol-origin: margin;"
		"   border: 1px solid #FFD700;"
		"   border-radius: 2px;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"   background: #3a3a3a;"
		"}"
		"QScrollBar::up-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-bottom: 6px solid #FFD700;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::down-arrow:vertical {"
		"   image: none;"
		"   border-left: 4px solid transparent;"
		"   border-right: 4px solid transparent;"
		"   border-top: 6px solid #FFD700;"
		"   width: 0px;"
		"   height: 0px;"
		"}"
		"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
		"   background: none;"
		"}"
	);




	// 옵션 2: 각 뷰마다 다른 색상
// Volume - 회색/흰색 (3D 렌더링)
	labelVolume->setStyleSheet(
		"QLabel { "
		"   color: #CCCCCC; "

		"   background-color: transparent; "
		"   padding: 4px 8px; "
		"   border-radius: 3px; "
		"   font-weight: 900; "
		"   font-size: 14px; "
		"   margin: 0px; "
		"}");

	// Axial - 보라/마젠타 계열 (이미지의 Axial(A) 색상)
	labelAxial->setStyleSheet(
		"QLabel { "
		"   color: #D87FD8; "

		"   background-color: transparent; "
		"   padding: 4px 8px; "
		"   border-radius: 3px; "
		"   font-weight: 900; "
		"   font-size: 14px; "
		"   margin: 0px; "
		"}");


	// Coronal - 청록색 (이미지의 Coronal(C) 색상)
	labelCoronal->setStyleSheet(
		"QLabel { "
		"   color: #00CED1; "

		"   background-color: transparent; "
		"   padding: 4px 8px; "
		"   border-radius: 3px; "
		"   font-weight: 900; "
		"   font-size: 14px; "
		"}");

	// Sagittal - 노란색 (이미지의 Sagittal(S) 색상)
	labelSagittal->setStyleSheet(
		"QLabel { "
		"   color: #FFD700; "

		"   background-color: transparent; "
		"   padding: 4px 8px; "
		"   border-radius: 3px; "
		"   font-weight: 900; "
		"   font-size: 14px; "
		"}");



	// 슬라이스 정보 라벨 생성
	sliceInfoAxial = new QLabel("Image 1/1", this);
	sliceInfoCoronal = new QLabel("Image 1/1", this);
	sliceInfoSagittal = new QLabel("Image 1/1", this);

	// 슬라이스 정보 라벨 스타일 (작고 투명하게)
	QString sliceInfoStyle =
		"QLabel { "
		"   color: #CCCCCC; "
		"   background-color: rgba(40, 40, 40, 180); "
		"   padding: 2px 6px; "
		"   border-radius: 2px; "
		"   font-size: 11px; "
		"}";

	sliceInfoAxial->setStyleSheet(sliceInfoStyle);
	sliceInfoCoronal->setStyleSheet(sliceInfoStyle);
	sliceInfoSagittal->setStyleSheet(sliceInfoStyle);

	sliceInfoAxial->adjustSize();
	sliceInfoCoronal->adjustSize();
	sliceInfoSagittal->adjustSize();

	//// ✅ World Matrix 초기화
	//XMStoreFloat4x4(&m_axialPlane.worldMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_coronalPlane.worldMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_sagittalPlane.worldMatrix, XMMatrixIdentity());


//	//=======================================================
//	XMMATRIX scale = XMMatrixScaling(0.8f, 0.8f, 0.8f); // ← 여기서 크기 조절
//
//// ---- Axial (XY plane, z=0)
//	
//		XMMATRIX worldA = scale * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
//		XMStoreFloat4x4(&m_axialPlane.worldMatrix, XMMatrixTranspose(worldA));
//	
//
//	// ---- Coronal (XZ plane, y=0)
//	
//		XMMATRIX worldC = scale * XMMatrixRotationX(XM_PIDIV2);
//		XMStoreFloat4x4(&m_coronalPlane.worldMatrix, XMMatrixTranspose(worldC));
//
//	
//	
//		XMMATRIX worldS = scale * XMMatrixRotationY(XM_PIDIV2);
//		XMStoreFloat4x4(&m_sagittalPlane.worldMatrix, XMMatrixTranspose(worldS));
//	
//
//
//		//===============================================================












	// 시그널 연결
	connect(scrollAxial, &QScrollBar::valueChanged, this, &QDirect3D11Widget::onAxialScroll);
	connect(scrollCoronal, &QScrollBar::valueChanged, this, &QDirect3D11Widget::onCoronalScroll);
	connect(scrollSagittal, &QScrollBar::valueChanged, this, &QDirect3D11Widget::onSagittalScroll);


}

QDirect3D11Widget::~QDirect3D11Widget()
{
	if (m_volumeVS) m_volumeVS->Release();
	if (m_volumePS) m_volumePS->Release();
	if (m_volumeConstantBuffer) m_volumeConstantBuffer->Release();
}

void QDirect3D11Widget::release()
{
	m_bDeviceInitialized = false;
	disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
	m_qTimer.stop();

	for (auto& view : m_RTViews.slices)
		ReleaseObject(view);
	//// for (auto& view : m_RTViews.slices)
	//     ReleaseObject(m_RTViews.slices);


	for (auto& view : m_SRViews.slices)
		ReleaseObject(view);
	////for (auto& view : m_SRViews.slices)
	//    ReleaseObject(m_SRViews.slices);

	ReleaseObject(m_pSwapChain);
	ReleaseObject(m_pDeviceContext);
	ReleaseObject(m_pDevice);
}

void QDirect3D11Widget::run()
{
	//qt 타이머 객체
	//MS_PER_FRAME 간격마다 timeout() 시그널을 발생시킴
	//timeout() 시그널은 내부적으로 pauseFrames, continueFrames 같은 슬롯에 연결되어 있음
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

	//
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
//	initializeVolumeRenderTargets();

	createSwapChainRTV();

	// ✅ 4. Depth Stencil Buffer 생성 (여기서 호출!)
	CreateDepthStencilBuffer();

	InitShaders();
	InitializeVolumeShaders();    // 셰이더 컴파일
	InitializeSlicePlanes();      // ← 1번
	InitializeBoundingCube();     // ← 2번
	InitializeVolumeCamera();     // 카메라 설정


	scrollAxial->setRange(0, fileReader->m_depth - 1);
	scrollAxial->setValue(fileReader->m_depth / 2); // 중앙으로 초기화
	scrollAxial->setPageStep(1);
	scrollAxial->setSingleStep(1);

	// Coronal 스크롤바 설정 (height 기준)
	scrollCoronal->setRange(0, fileReader->m_height - 1);
	scrollCoronal->setValue(fileReader->m_height / 2); // 중앙으로 초기화
	scrollCoronal->setPageStep(1);
	scrollCoronal->setSingleStep(1);

	// Sagittal 스크롤바 설정 (width 기준)
	scrollSagittal->setRange(0, fileReader->m_width - 1);
	scrollSagittal->setValue(fileReader->m_width / 2); // 중앙으로 초기화
	scrollSagittal->setPageStep(1);
	scrollSagittal->setSingleStep(1);

	// 초기 슬라이스 업데이트
	onAxialScroll(scrollAxial->value());
	onCoronalScroll(scrollCoronal->value());
	onSagittalScroll(scrollSagittal->value());


	// 초기 슬라이스 정보 표시
	sliceInfoAxial->setText(QString("Image %1/%2").arg(fileReader->m_depth / 2 + 1).arg(fileReader->m_depth));
	sliceInfoCoronal->setText(QString("Image %1/%2").arg(fileReader->m_height / 2 + 1).arg(fileReader->m_height));
	sliceInfoSagittal->setText(QString("Image %1/%2").arg(fileReader->m_width / 2 + 1).arg(fileReader->m_width));

	sliceInfoAxial->adjustSize();
	sliceInfoCoronal->adjustSize();
	sliceInfoSagittal->adjustSize();


	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);

	return true;
}

void QDirect3D11Widget::LoadDICOMSeries()
{

	fileReader = new FileReader();

	fileReader->LoadDICOMSeries((std::string)"D:\\Data\\sez\\DICOM", m_pDevice);
	// fileReader->LoadDICOMSeries((std::string)"D:\\Data\\DCM", m_pDevice);

}

void QDirect3D11Widget::onFrame()
{
	if (m_bRenderActive) tick();

	//beginScene();
	//render();
	RenderAllQuads();
	//RenderVolumeView();
	endScene();
}

void QDirect3D11Widget::beginScene()
{

	///*m_pDeviceContext->OMSetRenderTargets
	//(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), NULL);*/

	//for (int i{}; i < m_RTViews.size(); ++i) {


	//    D3D11_VIEWPORT vp = {};
	//    vp.Width = width() / 2.0f;
	//    vp.Height = height() / 2.0f;
	//    vp.MinDepth = 0.0f;
	//    vp.MaxDepth = 1.0f;

	//    if (0 == i) {
	//        vp.TopLeftX = 0;
	//        vp.TopLeftY = 0;

	//        m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ??몿而?

	//    }
	//    else if (1 == i) {
	//        vp.TopLeftX = width() / 2;
	//        vp.TopLeftY = 0;

	//        m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ?λ뜄以?


	//    }
	//    else if (2 == i) {
	//        vp.TopLeftX = 0;
	//        vp.TopLeftY = height() / 2;

	//        m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ???삂

	//    }
	//    else if (3 == i) {
	//        vp.TopLeftX = width() / 2;
	//        vp.TopLeftY = height() / 2;

	//        m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?紐껋삂

	//    }


	//    m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr); // 揶쏆뮆????쇱젟


	//    m_pDeviceContext->RSSetViewports(1, &vp);

	//    m_pDeviceContext->ClearRenderTargetView(m_RTViews[i],
	//        reinterpret_cast<const float*>(&m_BackColor));
	//}
}

void QDirect3D11Widget::endScene()
{
	if (FAILED(m_pSwapChain->Present(1, 0))) { onReset(); }
}


//렌더링 루프 중 씬을 갱신하거나 애니메이션, 카메라, UI 상태 등을 업데이트하는 역할
void QDirect3D11Widget::tick()
{
	// TODO: Update your scene here. For aesthetics reasons, only do it here if it's an
	// important component, otherwise do it in the MainWindow.
	// m_pCamera->Tick();

	emit ticked();
}

//void QDirect3D11Widget::CreateTexture3D()
//{
//	// 1. DICOM 슬라이스를 [depth][height][width] 순으로 메모리에 쌓았다고 가정
////    * 16bit HU를 0~1로 스케일해 R16_UNORM으로 업로드하면 간단합니다.
//
//	ComPtr<ID3D11ShaderResourceView> volumeSRV;
//
//	D3D11_TEXTURE3D_DESC td{};
//
//	td.Width = fileReader->m_width;   // X
//	td.Height = fileReader->m_height;  // Y
//	td.Depth = fileReader->m_depth;   // Z (슬라이스 개수)
//	td.MipLevels = 1;
//
//	//DXGI_FORMAT_R8G8B8A8_UNORM
//	td.Format = DXGI_FORMAT_R16_UNORM;   // 간단/가성비 좋음
//	//td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // 간단/가성비 좋음
//	td.Usage = D3D11_USAGE_DEFAULT;
//	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//
//	fileReader->normalizedU16Data.resize(fileReader->m_width * fileReader->m_height * fileReader->m_depth);
//
//	D3D11_SUBRESOURCE_DATA init{};
//	init.pSysMem = fileReader->normalizedU16Data.data();     // uint16_t* 크기: width*height*depth
//	init.SysMemPitch = fileReader->m_width * sizeof(uint16_t);
//	init.SysMemSlicePitch = fileReader->m_width * fileReader->m_height * sizeof(uint16_t);
//
//
//	// 📌 2. raw DICOM 슬라이스 데이터 → 정규화
//	bool ok = fileReader->NormalizeVolumeU16(
//		fileReader->m_volumeData,                 // 원본 voxel 배열
//		fileReader->normalizedU16Data,             // 결과 저장할 벡터
//		fileReader->m_rescaleSlope,
//		fileReader->m_rescaleIntercept,
//		-1000.0f, 3000.0f              // HU 범위 : -1000(공기) ~ + 3000(뼈)
//	);
//
//
//	ComPtr<ID3D11Texture3D> volumeTex;
//	HRESULT hr = m_pDevice->CreateTexture3D(&td, &init, &volumeTex);
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
//	sd.Format = td.Format;
//	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
//	sd.Texture3D.MipLevels = 1;
//	hr=m_pDevice->CreateShaderResourceView(volumeTex.Get(), &sd, &volumeSRV);
//
//	// 샘플러(보간은 Linear, 경계 Clamp)
//	ComPtr<ID3D11SamplerState> samp;
//	D3D11_SAMPLER_DESC smp{};
//	smp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
//
//	smp.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//	smp.MinLOD = 0;
//	smp.MaxLOD = D3D11_FLOAT32_MAX;
//
//	m_pDevice->CreateSamplerState(&smp, &samp);
//
//}


void QDirect3D11Widget::CreateTexture3D()
{
	const UINT w = fileReader->m_width;
	const UINT h = fileReader->m_height;
	const UINT d = fileReader->m_depth;

	// 0) 크기 검증
	if (w == 0 || h == 0 || d == 0) {
		OutputDebugStringA("❌ Volume size is zero\n");
		return;
	}

	// 1) 정규화 (HU -> 0~65535)  ※ 기본 HU 범위 예시: -1000 ~ 3000
	//   C2572 오류(기본 인수 재정의)는 선언부(.h)에만 default 인수 두고
	//   정의부(.cpp)에서는 default 제거하세요.
	fileReader->normalizedU16Data.resize(size_t(w) * h * d);
	const bool ok = fileReader->NormalizeVolumeU16(
		fileReader->m_volumeData,
		fileReader->normalizedU16Data,
		fileReader->m_rescaleSlope,
		fileReader->m_rescaleIntercept,
		-1000.0f, 3000.0f
	);
	if (!ok) {
		OutputDebugStringA("❌ NormalizeVolumeU16 failed\n");
		return;
	}
	if (fileReader->normalizedU16Data.size() < size_t(w) * h * d) {
		OutputDebugStringA("❌ normalizedU16Data size mismatch\n");
		return;
	}

	// 2) 3D 텍스처 desc
	D3D11_TEXTURE3D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.Depth = d;
	td.MipLevels = 1;
	// ✔ 권장: R16_FLOAT (샘플링/필터링/호환성 안전)
	//   R16_UNORM도 가능하지만 드라이버/샘플링측 이슈 줄이려면 FLOAT이 편합니다.
	td.Format = DXGI_FORMAT_R16_FLOAT;
	//td.Format = DXGI_FORMAT_R16_UNORM;
	//td.Format = DXGI_FORMAT_R32_FLOAT;

	//td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 3) 초기 데이터(피치/슬라이스피치) — "바이트" 기준
	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = fileReader->normalizedU16Data.data();
	init.SysMemPitch = w * sizeof(uint16_t);                  // 한 줄(바이트)
	init.SysMemSlicePitch = w * h * sizeof(uint16_t) /** 4*/;       // 한 장(바이트)

	// 4) 생성
	Microsoft::WRL::ComPtr<ID3D11Texture3D> tex;
	HRESULT hr = m_pDevice->CreateTexture3D(&td, &init, &tex);
	if (FAILED(hr)) {
		OutputDebugStringA("❌ CreateTexture3D failed\n");
		return;
	}

	// 5) SRV (desc=nullptr로 두면 포맷 자동 매칭)
	hr = m_pDevice->CreateShaderResourceView(tex.Get(), nullptr, &m_volumeSRV);
	if (FAILED(hr)) {
		OutputDebugStringA("❌ CreateShaderResourceView failed\n");
		return;
	}

	// 6) 샘플러 (멤버로 보관)
	if (!m_volumeSampler) {
		D3D11_SAMPLER_DESC smp{};
		smp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		smp.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		smp.MinLOD = 0;
		smp.MaxLOD = D3D11_FLOAT32_MAX;
		m_pDevice->CreateSamplerState(&smp, &m_volumeSampler);
	}














	// ===== 1. 알파 블렌딩 상태 생성 =====
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC rtBlend = {};
	rtBlend.BlendEnable = TRUE;
	rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtBlend.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0] = rtBlend;

	hr = m_pDevice->CreateBlendState(&blendDesc, &m_alphaBlendState);
	if (FAILED(hr)) {
		qDebug() << "❌ Failed to create alpha blend state";
	}


	// ===== 2. 깊이 테스트 끈 상태 생성 =====
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = FALSE; // 깊이 테스트 끄기
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthDesc.StencilEnable = FALSE;

	hr = m_pDevice->CreateDepthStencilState(&depthDesc, &m_disableDepthState);
	if (FAILED(hr)) {
		qDebug() << "❌ Failed to create disable depth state";
	}
}


struct CB
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
	DirectX::XMMATRIX InvView;
	DirectX::XMMATRIX InvProj;
	DirectX::XMMATRIX VolumeWorld;     // 볼륨의 월드 변환(스케일/회전/이동)
	DirectX::XMMATRIX InvVolumeWorld;
	DirectX::XMFLOAT3 CameraPosWS;     float Step;      // 샘플 간격 (예: 0.002~0.01)
	int   MaxSteps;                    float Opacity;   float _pad0; float _pad1;
};
struct Vtx { XMFLOAT2 pos; XMFLOAT2 uv; }; // NDC용이 아니라 스크린→NDC는 셰이더에서 변환

//void QDirect3D11Widget::FullScreenPassSet()
//{
//	
//	ComPtr<ID3D11Buffer> cbRay;
//
//	D3D11_BUFFER_DESC cbd{};
//	cbd.ByteWidth = sizeof(CB);
//	cbd.Usage = D3D11_USAGE_DYNAMIC;
//	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	m_pDevice->CreateBuffer(&cbd, nullptr, &cbRay);
//
//
//	
//	Vtx quad[4] = {
//		{{-1.f, -1.f}, {0.f, 1.f}},
//		{{-1.f,  1.f}, {0.f, 0.f}},
//		{{ 1.f, -1.f}, {1.f, 1.f}},
//		{{ 1.f,  1.f}, {1.f, 0.f}},
//	};
//
//
//	// 카메라/행렬 준비 (이미 쓰고 계신 View/Proj 사용)
//	XMMATRIX V = view;      // XMMatrixLookAtLH(...)
//	XMMATRIX P = proj;      // XMMatrixPerspectiveFovLH(...) or Ortho
//	XMMATRIX iV = XMMatrixInverse(nullptr, V);
//	XMMATRIX iP = XMMatrixInverse(nullptr, P);
//
//	// 볼륨을 -0.5~+0.5 박스로 쓰기 위해 적당히 스케일/오프셋
//	//XMMATRIX W = volumeWorld;                  // 예: 스케일*회전*이동
//
//	float sx = fileReader->views.spacing.x; // PixelSpacing X
//	float sy = fileReader->views.spacing.y; // PixelSpacing Y
//	float sz = fileReader->views.spacing.z; // SliceThickness
//	//XMMATRIX W = XMMatrixScaling(sx, sy, sz);
//
//
//	XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(10.0f));
//	XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(-5.0f));
//	XMMATRIX trans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
//	XMMATRIX scale = XMMatrixScaling(sx, sy, sz);
//	XMMATRIX W = scale * rotY * rotX * trans;
//
//
//	//XMMATRIX W = XMMatrixIdentity();
//	//XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
//
//	XMMATRIX iW = XMMatrixInverse(nullptr, W);
//
//	CB cb{};
//	cb.View = XMMatrixTranspose(V);
//	cb.Proj = XMMatrixTranspose(P);
//	cb.InvView = XMMatrixTranspose(iV);
//	cb.InvProj = XMMatrixTranspose(iP);
//	cb.VolumeWorld = XMMatrixTranspose(W);
//	cb.InvVolumeWorld = XMMatrixTranspose(iW);
//	cb.CameraPosWS = {/*eye.x,eye.y,eye.z*/ };
//	cb.Step = 0.004f;        // 화질/성능 트레이드오프
//	cb.MaxSteps = 384;           // 데이터 두께에 따라
//	cb.Opacity = 0.08f;         // 투명도 스케일
//
//	D3D11_MAPPED_SUBRESOURCE m{};
//	m_pDeviceContext->Map(cbRay.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
//	memcpy(m.pData, &cb, sizeof(cb));
//	m_pDeviceContext->Unmap(cbRay.Get(), 0);
//
//	// 파이프라인 바인딩
//	m_pDeviceContext->IASetInputLayout(layoutQuad);
//	m_pDeviceContext->VSSetShader(vsFullscreen, nullptr, 0);
//	m_pDeviceContext->PSSetShader(psRaymarch, nullptr, 0);
//	ID3D11Buffer* cbs[] = { cbRay.Get() };
//	m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs);
//	m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);
//	ID3D11ShaderResourceView* srvs[] = { m_volumeSRV.Get() };
//	m_pDeviceContext->PSSetShaderResources(0, 1, srvs);
//	ID3D11SamplerState* samps[] = { m_volumeSampler.Get() };
//	m_pDeviceContext->PSSetSamplers(0, 1, samps);
//
//
//
//	// ✅ 정점 버퍼 생성 (한 번만 해도 됨)
//	D3D11_BUFFER_DESC vbd{};
//	vbd.ByteWidth = sizeof(quad);
//	vbd.Usage = D3D11_USAGE_DEFAULT;
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//	D3D11_SUBRESOURCE_DATA initVB{};
//	initVB.pSysMem = quad;
//
//	HRESULT hrVB = m_pDevice->CreateBuffer(&vbd, &initVB, &m_quadVB);
//	if (FAILED(hrVB)) {
//		OutputDebugStringA("❌ Failed to create fullscreen quad vertex buffer\n");
//	}
//
//
//
//	UINT stride = sizeof(Vtx);
//	UINT offset = 0;
//	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_quadVB, &stride, &offset);
//	m_pDeviceContext->IASetInputLayout(layoutQuad);
//	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//
//
//
//	//// 블렌딩/깊이: 화면 합성만 할 거면 깊이는 끄거나 쓰지 않아도 됩니다.
//	//m_pDeviceContext->DrawIndexed(6, 0, 0);
//
//	// 3️⃣ 셰이더, 텍스처, 상수 버퍼 바인딩 후
//	m_pDeviceContext->Draw(4, 0);
//
//}
void QDirect3D11Widget::FullScreenPassSet()
{

	/*float blendFactor[4] = { 0, 0, 0, 0 };
	m_pDeviceContext->OMSetBlendState(m_alphaBlendState.Get(), blendFactor, 0xffffffff);*/


	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(width()/2);
	vp.Height = static_cast<float>(height() / 2);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &vp);

	// ✅ 1️⃣ 상수 버퍼 준비
	ComPtr<ID3D11Buffer> cbRay;
	D3D11_BUFFER_DESC cbd{};
	cbd.ByteWidth = sizeof(CB);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_pDevice->CreateBuffer(&cbd, nullptr, &cbRay);
	if (FAILED(hr)) {
		OutputDebugStringA("❌ Failed to create constant buffer\n");
		return;
	}

	// ✅ 2️⃣ 풀스크린 사각형 정점 (좌표 + UV)
	Vtx quad[4] = {
		{{-1.f, -1.f}, {0.f, 1.f}},
		{{-1.f,  1.f}, {0.f, 0.f}},
		{{ 1.f, -1.f}, {1.f, 1.f}},
		{{ 1.f,  1.f}, {1.f, 0.f}},
	};
	// 좌상단 사분면
	/*Vtx quad[4] = {
		{{-1.f,  0.f}, {0.f, 1.f}},
		{{-1.f,  1.f}, {0.f, 0.f}},
		{{ 0.f,  0.f}, {1.f, 1.f}},
		{{ 0.f,  1.f}, {1.f, 0.f}},
	};*/


	// ✅ 3️⃣ 정점 버퍼 생성 (한 번만 만들면 좋지만, 지금은 함수 내에서도 OK)
	if (!m_quadVB) {
		D3D11_BUFFER_DESC vbd{};
		vbd.ByteWidth = sizeof(quad);
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initVB{};
		initVB.pSysMem = quad;

		HRESULT hrVB = m_pDevice->CreateBuffer(&vbd, &initVB, &m_quadVB);
		if (FAILED(hrVB)) {
			OutputDebugStringA("❌ Failed to create fullscreen quad vertex buffer\n");
			return;
		}
	}

	//// ✅ 4️⃣ 카메라/행렬 준비 (이미 사용 중인 view/proj 그대로)
	//XMMATRIX V = view; // XMMatrixLookAtLH(...)
	//XMMATRIX P = proj; // XMMatrixPerspectiveFovLH(...)

	//// 🔧 임시 카메라 (볼륨 중앙을 보는 단순 뷰)
	//XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
	//XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
	//XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f),
	//	(float)width() / (float)height(),
	//	0.1f, 10.0f);

	
	//
	//XMMATRIX iV = XMMatrixInverse(nullptr, V);
	//XMMATRIX iP = XMMatrixInverse(nullptr, P);



	//// 🔧 임시 카메라 (볼륨 중앙을 보는 단순 뷰)
	//XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
	//XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR eye = XMVectorSet(0, 0, -2.0f, 1);
	XMVECTOR at = XMVectorSet(0, 0, 0, 1);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);






	XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
	/*XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f),
		(float)width() / (float)height(),
		0.1f, 10.0f);*/
	//XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width()/2 / ((float)height()/2), 0.1f, 100.0f);
	XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)width()  / ((float)height() ), 0.1f, 10.0f);




	XMMATRIX iV = XMMatrixInverse(nullptr, V);
	XMMATRIX iP = XMMatrixInverse(nullptr, P);

	




	// ✅ 5️⃣ 볼륨 월드 변환 구성
	float sx = fileReader->views.spacing.x;
	float sy = fileReader->views.spacing.y;
	float sz = fileReader->views.spacing.z;

	XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(10.0f));
	XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(-5.0f));
//	XMMATRIX trans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	//XMMATRIX scale = XMMatrixScaling(sx, sy, sz);
	//XMMATRIX scale = XMMatrixScaling(100.0f * sx, 100.0f * sy, 100.0f * sz);
	//XMMATRIX scale = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	//XMMATRIX scale = XMMatrixScaling(100, 100, 100);



	//XMMATRIX scale = XMMatrixScaling(
	//	sx * fileReader->m_width,
	//	sy * fileReader->m_height,
	//	sz* fileReader->m_depth
	//);
	XMMATRIX trans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	////XMMATRIX W = XMMatrixTranspose(scale);
	////XMMATRIX iW = XMMatrixInverse(nullptr, W);

	XMMATRIX center = XMMatrixTranslation(-0.5f, -0.5f, -0.5f);

	// ✅ 볼륨 월드 스케일: normalized box (0~1)
	XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	XMMATRIX W = center*scale * trans;
	XMMATRIX iW = XMMatrixInverse(nullptr, W);


	//XMMATRIX W = scale * rotY * rotX * trans;
	//XMMATRIX iW = XMMatrixInverse(nullptr, W);

	//XMMATRIX W = XMMatrixIdentity();
	//XMMATRIX iW = XMMatrixInverse(nullptr, W);

	// ✅ 6️⃣ 상수 버퍼 데이터 채우기
	CB cb{};
	cb.View = XMMatrixTranspose(V);
	cb.Proj = XMMatrixTranspose(P);
	cb.InvView = XMMatrixTranspose(iV);
	cb.InvProj = XMMatrixTranspose(iP);
	cb.VolumeWorld = XMMatrixTranspose(W);
	cb.InvVolumeWorld = XMMatrixTranspose(iW);
	//cb.CameraPosWS = XMFLOAT3(0, 0, -1.5f);  // 카메라 위치 (필요시 수정)
	//cb.CameraPosWS = XMFLOAT3(0.0f, 0.0f, -2.0f);
	//cb.CameraPosWS = XMFLOAT3(0, 0, -0.5f);
	//cb.CameraPosWS = XMFLOAT3(0, 0, -0.3f);
	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -0.5f);
	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -1.0f);
	//cb.CameraPosWS = XMFLOAT3(0, 0, -2.0f); // z축 앞쪽

	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -0.5f); // 살짝 앞쪽

	cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -0.2f);

	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -1.5f);


	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, -2.0f);


	//cb.CameraPosWS = XMFLOAT3(0.5f, 0.5f, 0.2f);




	/*cb.CameraPosWS = XMFLOAT3(0.5f * sx,
		0.5f * fileReader->m_height * sy,
		-fileReader->m_depth * sz * 1.5f);*/
	
	
	////cb.Step = 0.004f;    // 레이 스텝 크기
	////cb.MaxSteps = 384;
	//cb.Step = 0.001f;
//	cb.MaxSteps = 1024;

	//// 더 부드러운 적분을 위해
	//cb.Step = 0.0012f;
	//cb.MaxSteps = 4096;


	//cb.Step = 0.0005f;

	cb.Step = 0.001f;
	//cb.MaxSteps = 512;
	// ✅ 권장값
	cb.MaxSteps = 256;  // 또는 128~512 사이



	cb.Opacity = 0.08f;
	//cb.Opacity = 0.12f;  // 약간만 높여보세요 (0.08 → 0.12)


	D3D11_MAPPED_SUBRESOURCE mapped{};
	m_pDeviceContext->Map(cbRay.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &cb, sizeof(cb));
	m_pDeviceContext->Unmap(cbRay.Get(), 0);

	// ✅ 7️⃣ 파이프라인 세팅
	UINT stride = sizeof(Vtx);
	UINT offset = 0;
	ID3D11Buffer* vb[] = { m_quadVB.Get() };

	m_pDeviceContext->IASetVertexBuffers(0, 1, vb, &stride, &offset);
	m_pDeviceContext->IASetInputLayout(layoutQuad);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pDeviceContext->VSSetShader(vsFullscreen, nullptr, 0);
	m_pDeviceContext->PSSetShader(psRaymarch, nullptr, 0);

	ID3D11Buffer* cbs[] = { cbRay.Get() };
	m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);

	ID3D11ShaderResourceView* srvs[] = { m_volumeSRV.Get() };
	m_pDeviceContext->PSSetShaderResources(0, 1, srvs);

	ID3D11SamplerState* samps[] = { m_volumeSampler.Get() };
	m_pDeviceContext->PSSetSamplers(0, 1, samps);

	// ✅ 8️⃣ 드로우
	m_pDeviceContext->Draw(4, 0);

}


void QDirect3D11Widget::initializeRenderTargets()
{
	
	m_RTViews.slices.clear();
	m_SRViews.slices.clear();
	m_samplerState.clear();
	//coronalTextureCacheSrv.clear();
	//coronalTextureCacheSrv.resize(fileReader->m_height);

	fileReader->SliceIdxManage();


	for (int i{}; i < 4; ++i) {
		if (0 == i) {



		///*	D3D11_TEXTURE2D_DESC texDesc = {};
		//	texDesc.Width = fileReader->m_width;
		//	texDesc.Height = fileReader->m_depth;
		//	texDesc.MipLevels = 1;
		//	texDesc.ArraySize = fileReader->m_height;
		//	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//	texDesc.SampleDesc.Count = 1;
		//	texDesc.Usage = D3D11_USAGE_DEFAULT;
		//	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;*/

		////	std::vector<D3D11_SUBRESOURCE_DATA> initData(fileReader->m_height);
		//	std::vector<std::vector<uint8_t>> sliceData(fileReader->m_height);

		//	for (int i = 0; i < fileReader->m_height; ++i)
		//	{
		//		/*sliceData[i] = fileReader->GenerateCoronalSlice(i);
		//		initData[i].pSysMem = sliceData[i].data();
		//		initData[i].SysMemPitch = fileReader->m_width * 4 * sizeof(uint8_t);*/



		//		sliceData[i] = fileReader->GenerateCoronalSlice(i);

		//		D3D11_TEXTURE2D_DESC sliceDesc = {};
		//		/*sliceDesc.Width = fileReader->m_width;
		//		sliceDesc.Height = fileReader->m_depth;*/

		//		sliceDesc.Width = fileReader->m_width;
		//		sliceDesc.Height = fileReader->m_depth;
		//		sliceDesc.MipLevels = 1;
		//		sliceDesc.ArraySize = 1;
		//		sliceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//		sliceDesc.SampleDesc.Count = 1;
		//		sliceDesc.Usage = D3D11_USAGE_DEFAULT;
		//		sliceDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		//		D3D11_SUBRESOURCE_DATA initData = {};
		//		initData.pSysMem = sliceData[i].data();
		//		initData.SysMemPitch = fileReader->m_width * 4;

		//		ID3D11Texture2D* sliceTex = nullptr;
		//		HRESULT hr = m_pDevice->CreateTexture2D(&sliceDesc, &initData, &sliceTex);
		//		if (FAILED(hr)) continue;

		//		// 슬라이스 개별 SRV
		//		ID3D11ShaderResourceView* sliceSRV = nullptr;
		//		hr = m_pDevice->CreateShaderResourceView(sliceTex, nullptr, &sliceSRV);
		//		if (SUCCEEDED(hr))
		//		{
		//			coronalTextureCacheSrv[i] = sliceSRV; // ✅ 저장
		//		}
		//
		//	//	*sizeof(uint8_t);
		//	}

		//	//ComPtr<ID3D11Texture2D> texArray;
		//	//HRESULT hr = m_pDevice->CreateTexture2D(&texDesc, initData.data(), &texArray);

		//	//if (SUCCEEDED(hr))
		//	//{
		//	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		//	//	srvDesc.Format = texDesc.Format;
		//	//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		//	//	srvDesc.Texture2DArray.MostDetailedMip = 0;
		//	//	srvDesc.Texture2DArray.MipLevels = 1;
		//	//	srvDesc.Texture2DArray.FirstArraySlice = 0;
		//	//	srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;

		//	//	ComPtr<ID3D11ShaderResourceView> texArraySRV;
		//	//	hr = m_pDevice->CreateShaderResourceView(texArray.Get(), &srvDesc, &texArraySRV);

		//	//	if (SUCCEEDED(hr))
		//	//	{
		//	//		// ✅ 볼륨 전체용 SRV 저장 (원하면 m_SRViews.slices[0] 등에)
		//	//		
		//	//		coronalTextureCacheSrv = texArraySRV.Get();
		//	//	}
		//	//}



			//// ===== 1. 알파 블렌딩 상태 생성 =====
			//D3D11_BLEND_DESC blendDesc = {};
			//blendDesc.AlphaToCoverageEnable = FALSE;
			//blendDesc.IndependentBlendEnable = FALSE;

			//D3D11_RENDER_TARGET_BLEND_DESC rtBlend = {};
			//rtBlend.BlendEnable = TRUE;
			//rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			//rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			//rtBlend.BlendOp = D3D11_BLEND_OP_ADD;
			//rtBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
			//rtBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
			//rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			//rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			//blendDesc.RenderTarget[0] = rtBlend;

			//HRESULT hr = m_pDevice->CreateBlendState(&blendDesc, &m_alphaBlendState);
			//if (FAILED(hr)) {
			//	qDebug() << "❌ Failed to create alpha blend state";
			//}


			//// ===== 2. 깊이 테스트 끈 상태 생성 =====
			//D3D11_DEPTH_STENCIL_DESC depthDesc = {};
			//depthDesc.DepthEnable = FALSE; // 깊이 테스트 끄기
			//depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			//depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			//depthDesc.StencilEnable = FALSE;

			//hr = m_pDevice->CreateDepthStencilState(&depthDesc, &m_disableDepthState);
			//if (FAILED(hr)) {
			//	qDebug() << "❌ Failed to create disable depth state";
			//}


CreateTexture3D();




		}
		else if (1 == i) {

			for (int i{}; i < 2; ++i) {

				ID3D11Texture2D* axialTex = fileReader->getOrCreateAxialTexture(fileReader->currentIndex[1]);
				ID3D11RenderTargetView* axialRTV = getRTVForTexture(axialTex);
				m_RTViews.slices.push_back(axialRTV);
				ID3D11ShaderResourceView* axialSRV = getSRVForTexture(axialTex);
				m_SRViews.slices.push_back(axialSRV);
			}


			m_SRViews.flagIndex[1] = m_SRViews.flagIndex[0] + fileReader->m_depth - 1;

		}
		else if (2 == i) {

			ID3D11Texture2D* coronalTex = fileReader->getOrCreateCoronalTexture(fileReader->currentIndex[2]);
			ID3D11RenderTargetView* coronalRTV = getRTVForTexture(coronalTex);
			m_RTViews.slices.push_back(coronalRTV);
			ID3D11ShaderResourceView* coronalSRV = getSRVForTexture(coronalTex);
			m_SRViews.slices.push_back(coronalSRV);

			m_SRViews.flagIndex[2] = m_SRViews.flagIndex[1] + fileReader->m_height - 1;

		}
		else if (3 == i) {

			ID3D11Texture2D* sagittalTex = fileReader->getOrCreateSagittalTexture(fileReader->currentIndex[3]);
			ID3D11RenderTargetView* sagittalRTV = getRTVForTexture(sagittalTex);
			m_RTViews.slices.push_back(sagittalRTV);
			ID3D11ShaderResourceView* sagittalSRV = getSRVForTexture(sagittalTex);
			m_SRViews.slices.push_back(sagittalSRV);

			m_SRViews.flagIndex[3] = m_SRViews.flagIndex[2] + fileReader->m_width - 1;

		}



		// 4. ??용뮞筌???곸젫
	  //  pTexture->Release();
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
}

void QDirect3D11Widget::initializeVolumeRenderTargets()
{
	////m_RTViews.slices.clear();
	////m_SRViews.slices.clear();
	////m_samplerState.clear();

	//////fileReader->currentIndex[0] = fileReader->m_depth / 2;   // Axial (Z 방향)
	//////fileReader->currentIndex[1] = fileReader->m_height / 2;  // Coronal (Y 방향)
	//////fileReader->currentIndex[2] = fileReader->m_width / 2;   // Sagittal (X 방향)

	////fileReader->SliceIdxManage();


	//for (int i{}; i < 4; ++i) {


	//	if (0 == i) {

	//		D3D11_TEXTURE2D_DESC texDesc = {};

	//		//fileReader->axialTexture
	//		/*texDesc.Width = width() / 2;
	//		texDesc.Height = height() / 2;*/

	//		D3D11_TEXTURE2D_DESC desc;
	//		ID3D11Texture2D* axialTex = fileReader->getOrCreateAxialTexture(fileReader->currentIndex[1]);
	//		axialTex->GetDesc(&desc);
	//		texDesc.Width = desc.Width;
	//		texDesc.Height = desc.Height;

	//		texDesc.MipLevels = 1;
	//		texDesc.ArraySize = m_depth;
	//		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//		texDesc.SampleDesc.Count = 1;
	//		texDesc.Usage = D3D11_USAGE_DEFAULT;
	//		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;



	//		D3D11_TEXTURE2D_DESC texDesc = {};
	//		texDesc.Width = width;
	//		texDesc.Height = height;
	//		texDesc.MipLevels = 1;
	//		texDesc.ArraySize = m_depth;
	//		texDesc.Format = DXGI_FORMAT_R8_UNORM;
	//		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	//		ID3D11Texture2D* pTexture = nullptr;

	//		//250922  texture
	//		DXCall(m_pDevice->CreateTexture2D(&texDesc, nullptr, &pTexture));



	//		// 2. RenderTargetView 
	//		ID3D11RenderTargetView* pRTV = nullptr;
	//		DXCall(m_pDevice->CreateRenderTargetView(pTexture, nullptr, &pRTV));
	//		m_RTViews.slices.push_back(pRTV);


	//		// 3. ShaderResourceView 
	//		ID3D11ShaderResourceView* pSRV = nullptr;
	//		DXCall(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pSRV));
	//		m_SRViews.slices.push_back(pSRV);

	//		m_SRViews.flagIndex[0] = m_SRViews.slices.size() - 1;

	//	}
	//	else if (1 == i) {


	//		ID3D11Texture2D* axialTex = fileReader->getOrCreateAxialTexture(fileReader->currentIndex[1]);
	//		ID3D11RenderTargetView* axialRTV = getRTVForTexture(axialTex);
	//		m_RTViews.slices.push_back(axialRTV);
	//		ID3D11ShaderResourceView* axialSRV = getSRVForTexture(axialTex);
	//		m_SRViews.slices.push_back(axialSRV);


	//		m_SRViews.flagIndex[1] = m_SRViews.flagIndex[0] + fileReader->m_depth - 1;

	//	}
	//	else if (2 == i) {

	//		ID3D11Texture2D* coronalTex = fileReader->getOrCreateCoronalTexture(fileReader->currentIndex[2]);
	//		ID3D11RenderTargetView* coronalRTV = getRTVForTexture(coronalTex);
	//		m_RTViews.slices.push_back(coronalRTV);
	//		ID3D11ShaderResourceView* coronalSRV = getSRVForTexture(coronalTex);
	//		m_SRViews.slices.push_back(coronalSRV);

	//		m_SRViews.flagIndex[2] = m_SRViews.flagIndex[1] + fileReader->m_height - 1;

	//	}
	//	else if (3 == i) {

	//		ID3D11Texture2D* sagittalTex = fileReader->getOrCreateSagittalTexture(fileReader->currentIndex[3]);
	//		ID3D11RenderTargetView* sagittalRTV = getRTVForTexture(sagittalTex);
	//		m_RTViews.slices.push_back(sagittalRTV);
	//		ID3D11ShaderResourceView* sagittalSRV = getSRVForTexture(sagittalTex);
	//		m_SRViews.slices.push_back(sagittalSRV);

	//		m_SRViews.flagIndex[3] = m_SRViews.flagIndex[2] + fileReader->m_width - 1;

	//	}



	//	// 4. ??용뮞筌???곸젫
	//  //  pTexture->Release();
	//}


	//// 4. 샘플러 상태 생성
	//D3D11_SAMPLER_DESC sampDesc = {};
	//sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	//sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	//sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	//sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//sampDesc.MinLOD = 0;
	//sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//ID3D11SamplerState* pSampler = nullptr;
	//DXCall(m_pDevice->CreateSamplerState(&sampDesc, &pSampler));
	//m_samplerState.push_back(pSampler);
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
	//// TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
	//// important component, otherwise do it in the MainWindow.
	//// m_pCamera->Apply();




	////m_pDeviceContext->OMSetRenderTargets(static_cast<UINT>(m_RTViews.size()), m_RTViews.data(), nullptr);
	//m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);


	//// ?됯퀬猷????쇱젟 (?袁⑷퍥 ?遺얇늺)
	//D3D11_VIEWPORT vp = {};
	//vp.TopLeftX = 0;
	//vp.TopLeftY = 0;
	//vp.Width = static_cast<float>(width());
	//vp.Height = static_cast<float>(height());
	//vp.MinDepth = 0.0f;
	//vp.MaxDepth = 1.0f;
	//m_pDeviceContext->RSSetViewports(1, &vp);

	//// 揶????쐭 ??野껋옕??ShaderResourceView嚥??遺얇늺???곗뮆??
	//for (int i{}; i < m_SRViews.size(); ++i) {
	//    // ?? DrawQuadWithTexture(m_SRViews[i], viewport[i]);
	//    // ???봔?브쑴? ?怨쀬뵠?遺? ?類ㅼ젎 甕곌쑵?곫에??닌뗭겱??곷튊 ??곸뒄


	//    if (0 == i) {
	//        vp.TopLeftX = 0;
	//        vp.TopLeftY = 0;

	//        m_BackColor.r = 1.0f; m_BackColor.g = 0.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ??몿而?
	//    }
	//    else if (1 == i) {
	//        vp.TopLeftX = width() / 2;
	//        vp.TopLeftY = 0;

	//        m_BackColor.r = 0.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f;  // ?λ뜄以?
	//    }
	//    else if (2 == i) {
	//        vp.TopLeftX = 0;
	//        vp.TopLeftY = height() / 2;

	//        m_BackColor.r = 0.0f; m_BackColor.g = 0.0f; m_BackColor.b = 1.0f; m_BackColor.a = 1.0f; // ???삂
	//    }
	//    else if (3 == i) {
	//        vp.TopLeftX = width() / 2;
	//        vp.TopLeftY = height() / 2;

	//        m_BackColor.r = 1.0f; m_BackColor.g = 1.0f; m_BackColor.b = 0.0f; m_BackColor.a = 1.0f; // ?紐껋삂
	//    }




	//    m_pDeviceContext->RSSetViewports(1, &vp); // ?됯퀬猷????쇱젟

	//    DrawQuadWithTexture(m_SRViews[i], vp); // viewport[i]???袁⑺뒄 ?類ｋ궖


	//}


	//emit rendered();
}
void QDirect3D11Widget::UpdateColorBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	m_pDeviceContext->Map(m_colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &m_BackColor, sizeof(XMFLOAT4));
	m_pDeviceContext->Unmap(m_colorBuffer, 0);

	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);
}

void QDirect3D11Widget::UpdateViewIndexBuffer(int viewIndex)
{
	ViewInfoCB data = {};
	data.viewIndex = viewIndex;

	D3D11_MAPPED_SUBRESOURCE mapped;
	m_pDeviceContext->Map(m_viewIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &data, sizeof(ViewInfoCB));
	m_pDeviceContext->Unmap(m_viewIndexBuffer, 0);

	// b1 슬롯에 바인딩
	m_pDeviceContext->PSSetConstantBuffers(1, 1, &m_viewIndexBuffer);
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
	//InitShaders();
	//InitTextures(width() / 2, height() / 2);
	//InitSampler();       // ← 여기서 샘플러 생성

	//initializeRenderTargets();
}



void QDirect3D11Widget::RenderVolumeView()
{



	// ===== ✅ 볼륨 렌더링 (왼쪽 뷰처럼 축소 및 회전) =====
	{
		//		m_pDeviceContext->IASetInputLayout(m_prevVolumeInputLayout);
		//		m_pDeviceContext->VSSetShader(m_volumeQuadVS, nullptr, 0);
		//		m_pDeviceContext->PSSetShader(m_volumeQuadPS, nullptr, 0);
		//		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_volumePrevConstantBuffer);
		//		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_volumePrevConstantBuffer);
		//
		//		m_pDeviceContext->OMSetDepthStencilState(m_disableDepthState.Get(), 0);
		//
		//
		//
		//
		//		//// === 기존 카메라 설정 ===
		//		//XMMATRIX view = XMMatrixLookAtLH(
		//		//	XMVectorSet(-0.3f, 0.3f, -1.2f, 0.0f),  // 카메라 위치
		//		//	XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),    // 바라보는 지점
		//		//	XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)     // 업 벡터
		//		//);
		//
		//		//XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f, 0.1f, 100.0f);
		//
		//
		//
		//
		//		// === 볼륨 회전에 카메라 방향 반영 ===
		//
		////// 1️⃣ view 행렬의 역행렬 계산
		////		XMMATRIX invView = XMMatrixInverse(nullptr, view);
		////
		////		// 2️⃣ 볼륨 스케일 적용
		////		XMMATRIX volumeScale = XMMatrixScaling(0.55f, 0.55f, 0.55f);
		////
		////		// 3️⃣ 볼륨 중심 약간 이동 (턱이 화면 중앙으로)
		////		XMMATRIX volumeOffset = XMMatrixTranslation(0.0f, -0.08f, 0.0f);
		////
		////		// 4️⃣ 최종 월드 행렬 구성
		////		//    🔹 invView를 곱하면 카메라의 회전을 그대로 따라가게 됨
		////		XMMATRIX volumeWorld = volumeScale * invView * volumeOffset;
		//
		//
		//
		//		float blendFactor[4] = { 0,0,0,0 };
		//		m_pDeviceContext->OMSetBlendState(m_alphaBlendState.Get(), blendFactor, 0xffffffff);
		//
		//		float centerY = (fileReader->m_height - 1) * 0.5f;
		//
		//		//// 🟠 볼륨 크기 더 줄임
		//		//XMMATRIX volScale = XMMatrixScaling(0.52f, 0.52f, 0.52f);
		//
		//		////// 🟠 볼륨 공통 회전 (왼쪽 뷰 기준)
		//		////XMMATRIX volRotation =
		//		////	XMMatrixRotationY(-XMConvertToRadians(10.0f)) *
		//		////	XMMatrixRotationX(XMConvertToRadians(6.0f));
		//
		//		//// 🟠 중심 살짝 위로
		//		//XMMATRIX centerOffset = XMMatrixTranslation(0.0f, -0.1f, 0.0f);
		//
		//		for (int y = fileReader->m_height - 1; y >= 0; --y)
		//		{
		//		/*	float offsetY = ((y - centerY) / centerY) * 0.4f;
		//			offsetY *= fileReader->views.spacing.y * 0.7f;*/
		//
		//			float alpha = 1.0f / fileReader->m_height * 0.2f;
		//
		//			//XMMATRIX translation = XMMatrixTranslation(0.0f, offsetY, 0.0f);
		//
		//
		//			////m_CoronalPlane.worldMatrix; 
		//			//XMMATRIX world = volScale * translation * centerOffset;
		//
		//			//XMMATRIX world = volScale * translation * centerOffset;
		//
		//			//XMStoreFloat4x4(&constantsPrev.World, XMMatrixTranspose(world));
		//
		//
		//			XMMATRIX scale = XMMatrixScaling(0.9f, 0.9f, 0.9f); // ← 여기서 크기 조절
		//
		//				// 2️⃣ 회전 — 플레인과 동일한 카메라 시점 정합
		//			XMMATRIX volRotation =
		//				XMMatrixRotationY(XMConvertToRadians(-10.0f)) *   // 오른쪽으로 살짝 회전
		//				XMMatrixRotationX(XMConvertToRadians(6.0f));      // 위에서 약간 내려다봄
		//
		//			   // 3️⃣ 볼륨 중심 약간 이동 (턱 기준으로 정렬)
		//			XMMATRIX volOffset = XMMatrixTranslation(0.0f, -0.08f, 0.0f);
		//
		//			// 4️⃣ 슬라이스 간 세로 offset (적층 높이)
		//			float offsetY = ((y - centerY) / centerY) * 0.4f;
		//			offsetY *= fileReader->views.spacing.y * 0.7f;
		//
		//
		//			XMMATRIX translation = XMMatrixTranslation(0.0f, offsetY, 0.0f);
		//
		//			//XMMATRIX worldC = scale * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
		//
		//
		//
		//			XMMATRIX invView = XMMatrixInverse(nullptr, view);
		//			invView.r[3] = XMVectorSet(0, 0, 0, 1); // 위치 영향 제거, 회전만 적용
		//
		//		//	XMMATRIX volumeScale = XMMatrixScaling(0.55f, 0.55f, 0.55f);
		//		//	XMMATRIX volumeOffset = XMMatrixTranslation(0.0f, -0.08f, 0.0f);
		//		//	XMMATRIX volumeWorld = /*volumeScale **/ invView * volumeOffset;
		//
		//			  // 6️⃣ 최종 World 구성: 스케일 → 회전 → 슬라이스 위치 → 카메라 정합 → 오프셋
		//			XMMATRIX volumeWorld =
		//				scale *
		//				volRotation *
		//				translation *
		//				invView *
		//				volOffset;
		//
		//
		//			XMStoreFloat4x4(&constantsPrev.View, XMMatrixTranspose(view));
		//			XMStoreFloat4x4(&constantsPrev.Projection, XMMatrixTranspose(proj));
		//			XMStoreFloat4x4(&constantsPrev.World, XMMatrixTranspose(volumeWorld));
		//
		//
		//			constantsPrev.Color = XMFLOAT4(1, 1, 1, alpha);
		//	
		//			m_pDeviceContext->UpdateSubresource(m_volumePrevConstantBuffer, 0, nullptr, &constantsPrev, 0, 0);
		//
		//			ID3D11ShaderResourceView* srv = coronalTextureCacheSrv[y];
		//			m_pDeviceContext->PSSetShaderResources(0, 1, &srv);
		//
		//			DrawSliceQuad();
		//		}
		//
		//
		//		m_pDeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
		//		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		//		m_pDeviceContext->VSSetShader(m_volumeVS, nullptr, 0);
		//		m_pDeviceContext->PSSetShader(m_volumePS, nullptr, 0);


		//// 레이마칭 전에 현재 상태 백업
		//ID3D11VertexShader* oldVS = nullptr;
		//ID3D11PixelShader*  oldPS = nullptr;
		//m_pDeviceContext->VSGetShader(&oldVS, nullptr, nullptr);
		//m_pDeviceContext->PSGetShader(&oldPS, nullptr, nullptr);
		//
		//
		//// ✅ (1) 기존 상태 백업
		//ComPtr<ID3D11DepthStencilState> prevDS;
		//UINT prevStencilRef = 0;
		//m_pDeviceContext->OMGetDepthStencilState(&prevDS, &prevStencilRef);

		ComPtr<ID3D11BlendState> prevBS;
		FLOAT prevBlendFactor[4] = { 0, 0, 0, 0 };
		UINT prevSampleMask = 0xffffffff;
		m_pDeviceContext->OMGetBlendState(&prevBS, prevBlendFactor, &prevSampleMask);

		// ✅ (2) 볼륨 렌더링용 상태 설정
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

		// ✅ (3) 볼륨 렌더링 수행
		FullScreenPassSet();

		//// ✅ (4) 원래 상태 복원
		//m_pDeviceContext->OMSetDepthStencilState(prevDS.Get(), prevStencilRef);
		//m_pDeviceContext->OMSetBlendState(prevBS.Get(), prevBlendFactor, prevSampleMask);
		//
		//// 이전 상태 복원
		//m_pDeviceContext->VSSetShader(oldVS, nullptr, 0);
		//m_pDeviceContext->PSSetShader(oldPS, nullptr, 0);
		////SAFE_RELEASE(oldVS);
		////SAFE_RELEASE(oldPS);



	}




	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, m_pDepthStencilView);

	m_pDeviceContext->VSSetShader(m_volumeVS, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_volumePS, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_volumeConstantBuffer);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_volumeConstantBuffer);


	//XMMATRIX view = XMMatrixLookAtLH(
	//	XMVectorSet(-0.3f, 0.3f, -1.2f, 0.0f),  // ← 거의 정면에 가까운 위치 // 카메라 위치 (eye)
	//	XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),    // 원점 바라봄                // 바라보는 대상 (target)
	//	XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)     // 업 벡터                    // 위쪽 방향 (up)
	//);

	// 🎯 기존보다 살짝 사선 시점으로
	//XMVECTOR eye = XMVectorSet(0.6f, 0.5f, -1.0f, 0.0f);     // 오른쪽 위 뒤에서
	//XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);   // 원점(볼륨 중심)
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);       // Y축 기준 위쪽

// ✅ 카메라 위치 조정 (조금 더 정면 + 가까이)
//	XMVECTOR eye = XMVectorSet(-0.45f, 0.25f, -0.9f, 0.0f);  // ← 왼쪽·위로 살짝, 거리 짧게

	XMVECTOR eye = XMVectorSet(-0.2f, 0.2f, -1.0f, 0.0f);  // 더 정면, 더 낮게

	XMVECTOR target = XMVectorZero();                        // 원점(볼륨 중심)
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, -0.05f, 0.0f);




	view = XMMatrixLookAtLH(eye, target, up);
	//XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f, 0.1f, 100.0f);


	//// Orthographic Projection으로 변경
	//float viewWidth = 2.0f;   // 화면에 보일 가로 범위
	//float viewHeight = 2.0f;  // 화면에 보일 세로 범위
	float nearZ = 0.01f;
	float farZ = 100.0f;


	float aspect = (float)(fileReader->m_width*1.1) / (float)(fileReader->m_depth); // Coronal 기준
	float viewHeight = 2.0f;
	float viewWidth = viewHeight * aspect;


	proj = XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);



	XMStoreFloat4x4(&constants.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&constants.Projection, XMMatrixTranspose(proj));

	// ⚙️ 공통 스케일 (크기 조정)
	//XMMATRIX worldScale = XMMatrixScaling(0.55f, 0.55f, 0.55f);

	// ---- Axial (XY plane, z=0)
	{
		/*XMMATRIX world = scale * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
		XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));*/

		constants.World = m_CoronalPlane.worldMatrix; // ✅ 저장된 World Matrix 사용
		constants.Color = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f); // 청록
		m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		DrawPlane(m_CoronalPlane);
	}

	// ---- Coronal (XZ plane, y=0)
	{
		/*XMMATRIX world = scale * XMMatrixRotationX(XM_PIDIV2);
		XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));*/


		constants.World = m_AxialPlane.worldMatrix;  // ✅ 저장된 World Matrix 사용
		constants.Color = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 자홍
		m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		DrawPlane(m_AxialPlane);
	}

	// ---- Sagittal (YZ plane, x=0)
	{
		/*XMMATRIX world = scale * XMMatrixRotationY(XM_PIDIV2);
		XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));*/

		constants.World = m_SagittalPlane.worldMatrix; // ✅ 저장된 World Matrix 사용
		constants.Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 노랑
		m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		DrawPlane(m_SagittalPlane);
	}

	


	
}

void QDirect3D11Widget::InitializeVolumeCamera() {
	using namespace DirectX;

	// View 행렬 (카메라 위치 설정)
	XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, -500.0f, 1.0f);  // 카메라 위치
	XMVECTOR focusPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);   // 바라보는 점
	XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      // 위쪽 방향

	XMMATRIX view = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	XMStoreFloat4x4(&m_volumeViewMatrix, view);

	// Projection 행렬 (원근 투영)
	float aspectRatio = static_cast<float>(width()) / static_cast<float>(height());
	XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,      // 45도 시야각
		aspectRatio,
		1.0f,           // Near plane
		1000.0f         // Far plane
	);
	XMStoreFloat4x4(&m_volumeProjectionMatrix, projection);
}

void QDirect3D11Widget::InitializeVolumeShaders()
{
	// Vertex Shader 컴파일
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;


	// Vertex Shader 컴파일
	ID3DBlob* vsPrevBlob = nullptr;
	ID3DBlob* psPrevBlob = nullptr;
	ID3DBlob* errorPrevBlob = nullptr;



	// Vertex Shader 컴파일
	ID3DBlob* vsRaymarchBlob = nullptr;
	ID3DBlob* psRaymarchBlob = nullptr;
	ID3DBlob* errorRaymarchBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(L"VolumeVS.hlsl", nullptr, nullptr,
		"VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);


	if (FAILED(hr)) {
		if (errorBlob) {
			qDebug() << "VS Compile Error:" << (char*)errorBlob->GetBufferPointer();
			errorBlob->Release();
		}
		qDebug() << "Failed to compile volume vertex shader!";
		return;
	}


	hr = m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		nullptr, &m_volumeVS);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume vertex shader!";
		return;
	}




	// Pixel Shader 컴파일
	hr = D3DCompileFromFile(L"VolumePS.hlsl", nullptr, nullptr,
		"PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);

	if (FAILED(hr)) {
		if (errorBlob) {
			qDebug() << "PS Compile Error:" << (char*)errorBlob->GetBufferPointer();
			errorBlob->Release();
		}
		qDebug() << "Failed to compile volume pixel shader!";
		return;
	}

	hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		nullptr, &m_volumePS);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume pixel shader!";
		return;
	}

	qDebug() << "✅ Volume shaders compiled successfully!";

	// ✅ Constant Buffer 생성
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(VolumeConstants);  // ← 구조체 크기
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;

	hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_volumeConstantBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create volume constant buffer!";
	}





	hr = D3DCompileFromFile(L"prevVolumeVS.hlsl", nullptr, nullptr,
		"VSVolume", "vs_5_0", 0, 0, &vsPrevBlob, nullptr);


	if (FAILED(hr)) {
		if (errorPrevBlob) {
			qDebug() << "VS Compile Error:" << (char*)errorPrevBlob->GetBufferPointer();
			errorPrevBlob->Release();
		}
		qDebug() << "Failed to compile volume vertex shader!";
		return;
	}


	hr = m_pDevice->CreateVertexShader(vsPrevBlob->GetBufferPointer(),
		vsPrevBlob->GetBufferSize(),
		nullptr, &m_volumeQuadVS);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume vertex shader!";
		return;
	}




	// Pixel Shader 컴파일
	hr = D3DCompileFromFile(L"prevVolumePS.hlsl", nullptr, nullptr,
		"PSVolume", "ps_5_0", 0, 0, &psPrevBlob, nullptr);

	if (FAILED(hr)) {
		if (errorPrevBlob) {
			qDebug() << "PS Compile Error:" << (char*)errorPrevBlob->GetBufferPointer();
			errorPrevBlob->Release();
		}
		qDebug() << "Failed to compile volume pixel shader!";
		return;
	}

	hr = m_pDevice->CreatePixelShader(psPrevBlob->GetBufferPointer(),
		psPrevBlob->GetBufferSize(),
		nullptr, &m_volumeQuadPS);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume pixel shader!";
		return;
	}

	qDebug() << "✅ Volume shaders compiled successfully!";








	hr = D3DCompileFromFile(L"VolumeRaymarchVS.hlsl", nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &vsRaymarchBlob, nullptr);


	if (FAILED(hr)) {
		if (errorRaymarchBlob) {
			qDebug() << "VS Compile Error:" << (char*)errorRaymarchBlob->GetBufferPointer();
			errorRaymarchBlob->Release();
		}
		qDebug() << "Failed to compile volume vertex shader!";
		return;
	}

	//	ID3D11VertexShader*       m_vertexShader, *vsFullscree
	//ID3D11PixelShader*        m_pixelShader, *psRaymarch;
	hr = m_pDevice->CreateVertexShader(vsRaymarchBlob->GetBufferPointer(),
		vsRaymarchBlob->GetBufferSize(),
		nullptr, &vsFullscreen);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume vertex shader!";
		return;
	}




	// Pixel Shader 컴파일
	hr = D3DCompileFromFile(L"VolumeRaymarchPS.hlsl", nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &psRaymarchBlob, nullptr);

	if (FAILED(hr)) {
		if (errorRaymarchBlob) {
			qDebug() << "PS Compile Error:" << (char*)errorRaymarchBlob->GetBufferPointer();
			errorRaymarchBlob->Release();
		}
		qDebug() << "Failed to compile volume pixel shader!";
		return;
	}

	hr = m_pDevice->CreatePixelShader(psRaymarchBlob->GetBufferPointer(),
		psRaymarchBlob->GetBufferSize(),
		nullptr, &psRaymarch);

	if (FAILED(hr)) {
		qDebug() << "Failed to create volume pixel shader!";
		return;
	}

	qDebug() << "✅ Volume shaders compiled successfully!";











	// ✅ Constant Buffer 생성
	D3D11_BUFFER_DESC cbDescPrev = {};
	cbDescPrev.Usage = D3D11_USAGE_DEFAULT;
	cbDescPrev.ByteWidth = sizeof(VolumeConstants);  // ← 구조체 크기
	cbDescPrev.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDescPrev.CPUAccessFlags = 0;
	cbDescPrev.MiscFlags = 0;

	hr = m_pDevice->CreateBuffer(&cbDescPrev, nullptr, &m_volumePrevConstantBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create volume constant buffer!";
	}



	// ✅ 1. 평면용 Input Layout (Position + Texcoord)
	D3D11_INPUT_ELEMENT_DESC planeLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
	  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_pDevice->CreateInputLayout(planeLayout, ARRAYSIZE(planeLayout),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&m_volumeInputLayout);


	// ✅ 1. 평면용 Input Layout (Position + Texcoord)
	D3D11_INPUT_ELEMENT_DESC planePrevLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
	  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_pDevice->CreateInputLayout(planePrevLayout, ARRAYSIZE(planePrevLayout),
		vsPrevBlob->GetBufferPointer(),
		vsPrevBlob->GetBufferSize(),
		&m_prevVolumeInputLayout);




	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,                               D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = m_pDevice->CreateInputLayout(
		layoutDesc,
		ARRAYSIZE(layoutDesc),
		vsRaymarchBlob->GetBufferPointer(),
		vsRaymarchBlob->GetBufferSize(),
		&layoutQuad
	);

	if (FAILED(hr))
		OutputDebugStringA("❌ Failed to create raymarch input layout\n");













	// ✅ 2. 큐브용 Input Layout (Position만)
	D3D11_INPUT_ELEMENT_DESC cubeLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_pDevice->CreateInputLayout(cubeLayout, 1,
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&m_cubeInputLayout);

	vsBlob->Release();
	psBlob->Release();






	qDebug() << "✅ Volume constant buffer created!";

	// ✅ 카메라 행렬 초기화
	InitializeVolumeCamera();
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


	// 3. ?怨쀬뵠??揶쏆빘猿???밴쉐
	DXCall(m_pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		nullptr, &m_vertexShader));

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


	D3D11_BUFFER_DESC cbDescIdx = {};
	cbDescIdx.ByteWidth = sizeof(ViewInfoCB);
	cbDescIdx.Usage = D3D11_USAGE_DYNAMIC;
	cbDescIdx.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDescIdx.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DXCall(m_pDevice->CreateBuffer(&cbDescIdx, nullptr, &m_viewIndexBuffer));
}


D3D11_VIEWPORT QDirect3D11Widget::CreateViewport(int index)
{

	D3D11_VIEWPORT vp = {};

	float screenWidth = static_cast<float>(width());
	float screenHeight = static_cast<float>(height());

	// 기본 4분할 영역
	float quadWidth = screenWidth / 2.0f;
	float quadHeight = screenHeight / 2.0f;

	// ✅ 각 뷰의 실제 데이터 aspect ratio 계산
	float dataAspect = 1.0f;


	if (0==index ) {
		//// 좌상단 사분할 영역 기준
		//float quadAspect = quadWidth / quadHeight;
		//float dataAspect = (float)(fileReader->m_width) / (float)(fileReader->m_depth);

		//float renderWidth = quadWidth;
		//float renderHeight = quadHeight;
		//float offsetX = 0.0f;
		//float offsetY = 0.0f;

		//if (quadAspect > dataAspect) {
		//	// 세로 기준으로 맞춤
		//	renderWidth = quadHeight * dataAspect;
		//	offsetX = (quadWidth - renderWidth) / 2.0f;
		//}
		//else {
		//	// 가로 기준으로 맞춤
		//	renderHeight = quadWidth / dataAspect;
		//	offsetY = (quadHeight - renderHeight) / 2.0f;
		//}

		//vp.TopLeftX = offsetX;
		//vp.TopLeftY = offsetY;
		//vp.Width = renderWidth;
		//vp.Height = renderHeight;
		//vp.MinDepth = 0.0f;
		//vp.MaxDepth = 1.0f;

		vp.Width = quadWidth;
		vp.Height = quadHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		return vp;

		return vp;
	}


	switch (index) {

	case 1: // Axial (Z축 슬라이싱)
		dataAspect = (float)(fileReader->m_width) / (float)(fileReader->m_height);
		break;

	case 2: // Coronal (Y축 슬라이싱)
		// X × Z 평면
		dataAspect = (float)(fileReader->m_width) / (float)(fileReader->m_depth);
		break;

	case 3: // Sagittal (X축 슬라이싱)
		// Y × Z 평면
		dataAspect = (float)(fileReader->m_height) / (float)(fileReader->m_depth);
		break;

	default: // Volume (3D)
		//dataAspect = 1.0f;
		dataAspect = (float)(fileReader->m_width) / (float)(fileReader->m_depth);
		break;
	}

	// ✅ Aspect ratio 유지하며 최대 크기로 맞춤
	float renderWidth = quadWidth;
	float renderHeight = quadHeight;
	float offsetX = 0.0f;
	float offsetY = 0.0f;

	float quadAspect = quadWidth / quadHeight;

	if (quadAspect > dataAspect) {
		// 4분할 영역이 더 넓음 → 세로에 맞추고 가로 중앙 정렬
		renderWidth = quadHeight * dataAspect;
		offsetX = (quadWidth - renderWidth) / 2.0f;
	}
	else {
		// 4분할 영역이 더 높음 → 가로에 맞추고 세로 중앙 정렬
		renderHeight = quadWidth / dataAspect;
		offsetY = (quadHeight - renderHeight) / 2.0f;
	}

	// ✅ 4분할 위치 설정
	switch (index) {
	case 1: // Axial - 우상단
		vp.TopLeftX = quadWidth + offsetX;
		vp.TopLeftY = offsetY;
		break;

	case 2: // Coronal - 좌하단
		vp.TopLeftX = offsetX;
		vp.TopLeftY = quadHeight + offsetY;
		break;

	case 3: // Sagittal - 우하단
		vp.TopLeftX = quadWidth + offsetX;
		vp.TopLeftY = quadHeight + offsetY;
		break;

	default: // Volume - 좌상단
		vp.TopLeftX = offsetX;
		vp.TopLeftY = offsetY;
		break;
	}

	vp.Width = renderWidth;
	vp.Height = renderHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	return vp;
}




void QDirect3D11Widget::SetBackgroundColor(int index)
{
	switch (index) {
	case 0: m_BackColor = { 1.0f, 0.0f, 0.0f, 1.0f }; break;
	case 1: m_BackColor = { 0.0f, 1.0f, 0.0f, 1.0f }; break;
	case 2: m_BackColor = { 0.0f, 0.0f, 1.0f, 1.0f }; break;
	case 3: m_BackColor = { 1.0f, 1.0f, 0.0f, 1.0f }; break;
	}
}


void QDirect3D11Widget::RenderSceneToTarget(int i)
{
	//m_pDeviceContext->OMSetRenderTargets(1, &m_RTViews[i], nullptr);

	////float clearColor[4] = GetClearColorForIndex(i);


	////for (int i{}; i < m_RTViews.size(); ++i)
	//SetBackgroundColor(i);

	//m_pDeviceContext->ClearRenderTargetView(m_RTViews[i], reinterpret_cast<const float*>(&m_BackColor));

	////SetViewportForTarget(i);
	////BindShadersForTarget(i);
	////BindResourcesForTarget(i);

	////m_pDeviceContext->Draw(...); // quad 또는 모델 출력



	// // 3. 해당 뷰포트 설정 (사분할)
	//D3D11_VIEWPORT vp = CreateViewport(i);
	//m_pDeviceContext->RSSetViewports(1, &vp);

	//// 4. 셰이더 바인딩
	//m_pDeviceContext->IASetInputLayout(m_inputLayout);
	//m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	//m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	//// 5. 버텍스 버퍼 설정
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	//m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//// 6. 텍스처/샘플러 바인딩 (MPR 볼륨 or 현재 더미 텍스처)
	///*if (m_textureSRV) {
	//    m_pDeviceContext->PSSetShaderResources(0, 1, &m_textureSRV);
	//}*/
	//if (m_samplerState[0]) {
	//    m_pDeviceContext->PSSetSamplers(0, 1, &m_samplerState[0]);
	//}

	//// 7. 드로우콜 (quad 출력)
	//m_pDeviceContext->Draw(4, 0);
}











//251017
// Axial 뷰 (Z축 슬라이스)
ViewGeometry QDirect3D11Widget::GetAxialGeometry() {
	ViewGeometry geom;
	geom.origin = fileReader->views.origin;  // 원본 DICOM origin
	geom.rowDir = fileReader->views.rowDir;
	geom.colDir = fileReader->views.colDir;
	geom.pixelSpacingX = fileReader->views.spacing.x;  // X spacing
	geom.pixelSpacingY = fileReader->views.spacing.y;   // Y spacing
	geom.sliceSpacing = fileReader->views.spacing.z;     // Z spacing
	return geom;
}

// Coronal 뷰 (Y축 슬라이스)
ViewGeometry QDirect3D11Widget::GetCoronalGeometry() {
	ViewGeometry geom;

	// Origin: 볼륨의 전방 하단 좌측 (front-bottom-left)
	geom.origin = fileReader->views.origin;

	//// Row direction: X축 (좌→우)
	//geom.rowDir = XMFLOAT3(1.0f, 0.0f, 0.0f);

	//// Column direction: Z축 (하→상)
	//geom.colDir = XMFLOAT3(0.0f, 0.0f, 1.0f);

	geom.rowDir = fileReader->views.rowDir;
	geom.colDir = fileReader->views.colDir;

	//	fileReader->views.imageSize = DirectX::XMFLOAT3(fileReader->m_width, fileReader->m_depth, fileReader->m_height);

		// Spacing
	geom.pixelSpacingX = fileReader->views.spacing.x;     // X spacing
	geom.pixelSpacingY = fileReader->views.spacing.z;      // Z spacing (세로)
	geom.sliceSpacing = fileReader->views.spacing.y; ;      // Y spacing (슬라이스 방향)

	return geom;
}

// Sagittal 뷰 (X축 슬라이스)
ViewGeometry QDirect3D11Widget::GetSagittalGeometry() {
	ViewGeometry geom;

	// Origin: 볼륨의 좌측 하단 전방 (left-bottom-front)
	geom.origin = fileReader->views.origin;

	//// Row direction: Y축 (전→후)
	//geom.rowDir = XMFLOAT3(0.0f, 1.0f, 0.0f);

	//// Column direction: Z축 (하→상)
	//geom.colDir = XMFLOAT3(0.0f, 0.0f, 1.0f);

	geom.rowDir = fileReader->views.rowDir;
	geom.colDir = fileReader->views.colDir;

	//	fileReader->views.imageSize = DirectX::XMFLOAT3(fileReader->m_height, fileReader->m_depth, fileReader->m_width);

		// Spacing
	geom.pixelSpacingX = fileReader->views.spacing.y;     // X spacing
	geom.pixelSpacingY = fileReader->views.spacing.z;      // Z spacing (세로)
	geom.sliceSpacing = fileReader->views.spacing.x; ;      // Y spacing (슬라이스 방향)

	return geom;
}

int QDirect3D11Widget::ComputeSliceIndexForView(const XMFLOAT3& patientCoord, int viewIndex)
{
	ViewGeometry geom;
	XMUINT3 dims;

	switch (viewIndex) {
	case 1: // Axial
		geom = GetAxialGeometry();
		dims = XMUINT3(
			fileReader->m_width,
			fileReader->m_height,
			fileReader->m_depth    // ✅ 총 슬라이스 개수 (Z축)
		);
		break;

	case 2: // Coronal
		geom = GetCoronalGeometry();
		dims = XMUINT3(
			fileReader->m_width,
			fileReader->m_height,
			fileReader->m_depth    // ✅ 총 슬라이스 개수 (Z축)
		);
		break;

	case 3: // Sagittal
		geom = GetSagittalGeometry();
		dims = XMUINT3(
			fileReader->m_width,
			fileReader->m_height,
			fileReader->m_depth    // ✅ 총 슬라이스 개수 (Z축)
		);
		break;
	}

	return ComputeSliceIndexFromPatientCoord_Robust(
		patientCoord,
		viewIndex,
		geom.origin,
		geom.rowDir,
		geom.colDir,
		geom.pixelSpacingX,
		geom.pixelSpacingY,
		geom.sliceSpacing,
		dims
	);
}



////디버깅 방법
//qDebug() << "=== View" << viewIndex << "===";
//qDebug() << "Patient Coord:" << patientCoord.x << patientCoord.y << patientCoord.z;
//qDebug() << "Origin:" << origin.x << origin.y << origin.z;
//qDebug() << "RowDir:" << rowDir.x << rowDir.y << rowDir.z;
//qDebug() << "ColDir:" << colDir.x << colDir.y << colDir.z;
//qDebug() << "Spacing:" << pixelSpacingX << pixelSpacingY << sliceSpacing;
//qDebug() << "Dims:" << dims.x << dims.y << dims.z;
//qDebug() << "Computed indices: i=" << ii << "j=" << jj << "k=" << kk;






void QDirect3D11Widget::mousePressEvent(QMouseEvent* event)
{
	px[0] = event->pos().x(); // 클릭된 x 좌표
	py[0] = event->pos().y(); // 클릭된 y 좌표


	qDebug() << "px:" << px << "py:" << py;


	//float scale = this->devicePixelRatioF();
	//float px = event->pos().x() * scale;
	//float py = event->pos().y() * scale;


	//ImGuiIO& io = ImGui::GetIO();
	//if (event->button() == Qt::LeftButton)
	//    io.MouseDown[0] = true;

	// 예: 클릭된 뷰가 i번째 뷰라고 가정
	//int clickedViewIndex = i; // 0: Axial, 1: Coronal, 2: Sagittal, 3: Volume
	clickedViewIndex = GetClickedViewIndex(px[0], py[0], this->width(), this->height());
	px[clickedViewIndex] = px[0];
	py[clickedViewIndex] = py[0];

	

	viewPort = CreateViewport(clickedViewIndex); // i = 0~3
	viewX = viewPort.TopLeftX;
	viewY = viewPort.TopLeftY;
	viewWidth = viewPort.Width;
	viewHeight = viewPort.Height;


	// 마우스 클릭 좌표 정규화
	float normX = static_cast<float>(px[clickedViewIndex] - viewX) / viewWidth;
	float normY = static_cast<float>(py[clickedViewIndex] - viewY) / viewHeight;


	// 모든 뷰에 동일한 십자선 위치 적용
	currentUV[clickedViewIndex] = { normX, normY };


	// ✅ Aspect ratio 고려한 정규화 좌표
	//XMFLOAT2 normUV = GetNormalizedUV(px[clickedViewIndex], py[clickedViewIndex], clickedViewIndex);

	//currentUV[clickedViewIndex] = crossUV;
	// 클릭된 위치 → 환자 좌표
	patientCoord = GetPatientCoordFromClick(clickedViewIndex, currentUV[clickedViewIndex]);



	for (int i{ 1 }; i <= 3; ++i) {
		fileReader->views.centerPatientCoord[i] = patientCoord;


		//ComputeSliceIndexFromPatientCoord_Robust
	   // fileReader->currentIndex[i] = ComputeSliceIndexFromPatientCoord(i, patientCoord);
	   /* fileReader->currentIndex[i] = ComputeSliceIndexFromPatientCoord_Robust(patientCoord, i,
			fileReader->views.origin, XMFLOAT3(1, 0, 0), XMFLOAT3(0, 1, 0),
			fileReader->views.spacing.x, fileReader->views.spacing.y, 0.15f, XMUINT3(632, 794, 794));*/
		fileReader->currentIndex[i] = ComputeSliceIndexForView(patientCoord, i);


		ID3D11RenderTargetView* rtvA, *rtvC, *rtvS;
		ID3D11ShaderResourceView* srvA, *srvC, *srvS;
		ID3D11Texture2D* texA, *texC, *texS;

		// int viewIndex = GetClickedViewIndex(px, py, this->width(), this->height()); // 현재 뷰 인덱스 (0: Axial, 1: Coronal, 2: Sagittal, 3: 기타)
		if (clickedViewIndex != i) {

			switch (i) {
			case 1:
				fileReader->UpdateAxialTexture(fileReader->currentIndex[1]);
				texA = fileReader->axialTextureCache[fileReader->currentIndex[1]];
				srvA = getSRVForTexture(texA);
				m_SRViews.slices[1] = srvA;

				rtvA = getRTVForTexture(texA);
				m_RTViews.slices[1] = rtvA;


				sliceInfoAxial->hide();
				sliceInfoAxial->setText(QString("Image %1/%2").arg(fileReader->m_depth - fileReader->currentIndex[1] + 1).arg(fileReader->m_depth));
				//sliceInfoAxial->adjustSize();
			  //  sliceInfoAxial->repaint();  // 강제로 다시 그리기
				sliceInfoAxial->show();


				//onAxialScroll(fileReader->currentIndex[1]);
				break;
			case 2:
				fileReader->UpdateCoronalTexture(fileReader->currentIndex[2]);
				texC = fileReader->coronalTextureCache[fileReader->currentIndex[2]];
				srvC = getSRVForTexture(texC);
				m_SRViews.slices[2] = srvC;

				rtvC = getRTVForTexture(texC);
				m_RTViews.slices[2] = rtvC;


				sliceInfoCoronal->hide();
				sliceInfoCoronal->setText(QString("Image %1/%2").arg(fileReader->currentIndex[2] + 1).arg(fileReader->m_height));
				//sliceInfoCoronal->adjustSize();
			  //  sliceInfoCoronal->repaint();  // 강제로 다시 그리기
				sliceInfoCoronal->show();



				//onCoronalScroll(fileReader->currentIndex[2]);
				break;
			case 3:
				fileReader->UpdateSagittalTexture(fileReader->currentIndex[3]);
				texS = fileReader->sagittalTextureCache[fileReader->currentIndex[3]];
				srvS = getSRVForTexture(texS);
				m_SRViews.slices[3] = srvS;

				rtvS = getRTVForTexture(texS);
				m_RTViews.slices[3] = rtvS;

				sliceInfoSagittal->hide();
				sliceInfoSagittal->setText(QString("Image %1/%2").arg(fileReader->currentIndex[3] + 1).arg(fileReader->m_width));
				//sliceInfoSagittal->adjustSize();
				//sliceInfoSagittal->repaint();  // 강제로 다시 그리기
				sliceInfoSagittal->show();

				//onSagittalScroll(fileReader->currentIndex[3]);
				break;
			}
		}

	}

	UpdateSlicePlanePositions();

	//RenderVolumeView(); // 강제 호출로 확인


	// 렌더링 업데이트
	update();

	RenderVolumeView(); // 강제 호출로 확인

	qDebug() << "a cur slice : " << 631 - fileReader->currentIndex[1] << endl;
	qDebug() << "c cur slice : " << fileReader->currentIndex[2] << endl;
	qDebug() << "s cur slice : " << fileReader->currentIndex[3] << endl;
}

int QDirect3D11Widget::GetClickedViewIndex(int px, int py, int width, int height)
{
	if (px < width / 2 && py < height / 2)
		return 0; // 왼쪽 위 → Axial
	else if (px >= width / 2 && py < height / 2)
		return 1; // 오른쪽 위 → Coronal
	else if (px < width / 2 && py >= height / 2)
		return 2; // 왼쪽 아래 → Sagittal
	else
		return 3; // 오른쪽 아래 → Volume
}

DirectX::XMFLOAT3 QDirect3D11Widget::GetDefaultPatientCenter()
{
	//// Axial 뷰 기준으로 중앙 좌표 계산
	//const ViewInfo& axialView = fileReader->views[1];

	// Axial 뷰 기준으로 중앙 좌표 계산
	const ViewInfo& axialView = fileReader->views;

	float centerX = axialView.origin.x + (axialView.imageSize.x * axialView.spacing.x) / 2.0f;
	float centerY = axialView.origin.y + (axialView.imageSize.y * axialView.spacing.y) / 2.0f;
	float centerZ = axialView.origin.z + (axialView.imageSize.z * axialView.spacing.z) / 2.0f;

	/*for (int i{1}; i < 4; ++i)
		currentPatientCoord[i] = DirectX::XMFLOAT3(centerX, centerY, centerZ);*/


	for (int i{ 1 }; i < 4; ++i)
		currentUV[i] = DirectX::XMFLOAT2(0.5f, 0.5f);

	return DirectX::XMFLOAT3(centerX, centerY, centerZ);
}


//void QDirect3D11Widget::InitializeCrosshair()
//{
//    //// 기본 중심점: 환자 좌표계의 중앙 또는 첫 슬라이스 기준
//    //DirectX::XMFLOAT3 patientCoord = GetDefaultPatientCenter(); // 예: 영상 중앙 좌표
//
//    //// 십자선 데이터 구조 초기화
//    //CrosshairData crosshair = {};
//
//    //for (int i = 0; i < 4; ++i)
//    //{
//    //    DirectX::XMFLOAT2 uv = GetCrossUVFromPatientCoord(i, patientCoord);
//
//    //    switch (i)
//    //    {
//    //    case 0: crosshair.cross0 = uv; break;
//    //    case 1: crosshair.cross1 = uv; break;
//    //    case 2: crosshair.cross2 = uv; break;
//    //    case 3: crosshair.cross3 = uv; break;
//    //    }
//    //}
//
//    //crosshair.crossThickness = 0.002f;
//    //crosshair.crossColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // 빨강
//
//    //// GPU에 전달
//    //m_pDeviceContext->UpdateSubresource(fileReader->m_crosshairBuffer, 0, nullptr, &crosshair, 0, 0);
//
//
//    int viewIndex = GetClickedViewIndex(px, py, this->width(), this->height()); // 현재 뷰 인덱스 (0: Axial, 1: Coronal, 2: Sagittal, 3: 기타)
//    UpdateViewIndexBuffer(viewIndex); // 반드시 렌더링 전에 호출
//    m_pDeviceContext->PSSetConstantBuffers(1, 1, &m_viewIndexBuffer); // b1 슬롯
//
//    DirectX::XMFLOAT3 patientCoord = GetDefaultPatientCenter(); // 환자 좌표계 기준 중심점
//
//    CrosshairData crosshair = {};
//    crosshair.crossUV = GetCrossUVFromPatientCoord(viewIndex, patientCoord); // 현재 뷰에 맞는 UV 좌표
//    crosshair.crossThickness = 0.002f;
//    crosshair.crossColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // 빨강
//
//    m_pDeviceContext->UpdateSubresource(fileReader->m_crosshairBuffer, 0, nullptr, &crosshair, 0, 0);
//
//}

void QDirect3D11Widget::UpdateCrosshairFromPatientCoord(DirectX::XMFLOAT3 patientCoord, int i)
{
	/*CrosshairData crosshair = {};

	for (int i = 0; i < 4; ++i)
	{
		DirectX::XMFLOAT2 uv = GetCrossUVFromPatientCoord(i, patientCoord);

		switch (i)
		{
		case 0: crosshair.cross0 = uv; break;
		case 1: crosshair.cross1 = uv; break;
		case 2: crosshair.cross2 = uv; break;
		case 3: crosshair.cross3 = uv; break;
		}
	}

	crosshair.crossThickness = 0.002f;
	crosshair.crossColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	m_pDeviceContext->UpdateSubresource(fileReader->m_crosshairBuffer, 0, nullptr, &crosshair, 0, 0);*/

	CrosshairData crosshair = {};

	// int viewIndex = GetClickedViewIndex(px, py, this->width(), this->height()); // 현재 뷰 인덱스 (0: Axial, 1: Coronal, 2: Sagittal, 3: 기타)

	 // 현재 뷰에 맞는 십자선 위치 계산
	crosshair.crossUV = GetCrossUVFromPatientCoord(i, patientCoord);

	// ✅ Aspect ratio 고려한 정규화 좌표
//	XMFLOAT2 normUV = GetNormalizedUV(px[clickedViewIndex], px[clickedViewIndex], clickedViewIndex);

	// 십자선 스타일 설정
	if (0 == i)
		crosshair.crossThickness = 0.0f;
	else
		crosshair.crossThickness = 0.002f;


	crosshair.crossColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // 빨강

	// GPU에 전달
	m_pDeviceContext->UpdateSubresource(fileReader->m_crosshairBuffer, 0, nullptr, &crosshair, 0, 0);

}


void QDirect3D11Widget::CreateDepthStencilBuffer()
{
	// Depth Stencil Texture 생성
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width();
	depthDesc.Height = height();
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	m_pDevice->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);

	// Depth Stencil View 생성
	m_pDevice->CreateDepthStencilView(depthStencilBuffer, nullptr, &m_pDepthStencilView);
	depthStencilBuffer->Release();
}

//m_pDeviceContext->OMSetRenderTargets(4, m_RTViews.slices.data(), nullptr);

	////// 3. 셰이더에 바인딩
	////m_pDeviceContext->PSSetConstantBuffers(0, 1, &fileReader->m_crosshairBuffer);
	//m_pDeviceContext->PSSetShaderResources(0, 4, m_SRViews.slices.data());     // tex0~tex3
	//m_pDeviceContext->PSSetSamplers(0, 1, m_samplerState.data());       // samp0~samp3


	////rtv 너무 많이 생성해서 생기는 오류//251001
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;

	//m_pDeviceContext->IASetInputLayout(m_inputLayout);
	//m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	//m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	//m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	//m_pDeviceContext->Draw(4, 0); // 4개의 정점으로 quad 출력


void QDirect3D11Widget::RenderAllQuads()
{
	
	// 클릭된 위치 → 환자 좌표
	patientCoord = GetPatientCoordFromClick(clickedViewIndex, currentUV[clickedViewIndex]);

	 //// 2. 백버퍼에 출력할 준비
	m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, m_pDepthStencilView);
	m_pDeviceContext->ClearRenderTargetView(m_pSwapChainRTV, reinterpret_cast<float*>(&m_BackColor));

	// ✅ 깊이 버퍼 클리어 (3D 렌더링에 필요)
	if (m_pDepthStencilView) {
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView,
			D3D11_CLEAR_DEPTH,
			1.0f, 0);
	}

	// 3. 각 렌더 타겟 텍스처를 quad로 출력
	for (int i{}; i < 4; ++i)
	{
		D3D11_VIEWPORT vp = CreateViewport(i); // ← 4분할 뷰포트 계산

		m_pDeviceContext->RSSetViewports(1, &vp); // ✅ 모든 뷰에 설정

		//qDebug() << "Viewport" << i << ":" << vp.Width << "x" << vp.Height; // ✅ 로그

		if (0 == i) {

			RenderVolumeView();
		}
		else {

			// ✅ Depth Buffer 해제 (2D는 필요 없음)
			m_pDeviceContext->OMSetRenderTargets(1, &m_pSwapChainRTV, nullptr);
			DrawQuadWithTexture(m_SRViews.slices[i], vp, i);      // ← 여기서 호출!
		}
	}


	//if (!m_volumeVS || !m_volumePS) {
	//	qDebug() << "❌ Shaders are null!";
	//	return;
	//}


	ImGuiIO& io = ImGui::GetIO();



	// ✅ 여기에 ImGui 렌더링 추가!
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();




	// ✅ Begin/End 없이 바로 그리기
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	float cx = screenSize.x * 0.5f;
	float cy = screenSize.y * 0.5f;

	// 수직선 (연한 회색)
	drawList->AddLine(ImVec2(cx, 0), ImVec2(cx, screenSize.y), IM_COL32(211, 211, 211, 255), 2.0f);

	// 수평선 (연한 회색)
	drawList->AddLine(ImVec2(0, cy), ImVec2(screenSize.x, cy), IM_COL32(211, 211, 211, 255), 2.0f);


	// ImGui 렌더링 마무리
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_pSwapChain->Present(1, 0);

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

void QDirect3D11Widget::DrawQuadWithTexture(ID3D11ShaderResourceView* pSRV, const D3D11_VIEWPORT& vp, int i)
{
	//m_pDeviceContext->RSSetViewports(1, &vp);
	m_pDeviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pixelShader, nullptr, 0);



	UpdateCrosshairFromPatientCoord(patientCoord, i);

	// 3. 셰이더에 바인딩
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &fileReader->m_crosshairBuffer);


	m_pDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	m_pDeviceContext->IASetInputLayout(m_inputLayout);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pDeviceContext->Draw(4, 0);
}

ID3D11RenderTargetView* QDirect3D11Widget::getRTVForTexture(ID3D11Texture2D* texture)
{
	// 이미 캐싱된 RTV가 있으면 반환
	auto it = rtvCache.find(texture);
	if (it != rtvCache.end())
		return it->second;

	// 없으면 새로 생성하고 캐시에 저장
	ID3D11RenderTargetView* rtv = nullptr;
	DXCall(device()->CreateRenderTargetView(texture, nullptr, &rtv));
	rtvCache[texture] = rtv;
	return rtv;

}

ID3D11ShaderResourceView* QDirect3D11Widget::getSRVForTexture(ID3D11Texture2D* texture)
{
	// 이미 캐싱된 SRV가 있으면 반환
	auto it = srvCache.find(texture);
	if (it != srvCache.end())
		return it->second;

	// 새로 생성
	ID3D11ShaderResourceView* srv = nullptr;
	DXCall(m_pDevice->CreateShaderResourceView(texture, nullptr, &srv));

	// 캐시에 저장
	srvCache[texture] = srv;
	return srv;
}



// helper: dot, cross, subtract convert XMFLOAT3 -> XMVECTOR
static XMVECTOR V(const XMFLOAT3& a) { return XMLoadFloat3(&a); }
static XMFLOAT3 ToXMF3(XMVECTOR v) { XMFLOAT3 r; XMStoreFloat3(&r, v); return r; }

int QDirect3D11Widget::ComputeSliceIndexFromPatientCoord_Robust(
	const XMFLOAT3& patientCoord,    // world/patient coordinate
	int viewIndex,                   // 1: Axial (Z), 2: Coronal (Y), 3: Sagittal (X)
	const XMFLOAT3& origin,          // ImagePositionPatient of reference slice (slice 0)
	const XMFLOAT3& rowDir,          // ImageOrientationPatient[0..2]
	const XMFLOAT3& colDir,          // ImageOrientationPatient[3..5]
	float pixelSpacingX,             // (mm) usually second value in (0028,0030)
	float pixelSpacingY,             // (mm) usually first value in (0028,0030)
	float sliceSpacing,              // (mm) spacing between slices (0018,0088) or SliceThickness
	const XMUINT3& dims              // width, height, depth (voxels)
)
{
	// Build orthonormal-ish basis (rowDir, colDir, normal)
	XMVECTOR vr = XMVector3Normalize(V(rowDir));//x축
	XMVECTOR vc = XMVector3Normalize(V(colDir));//y축
	XMVECTOR vn = XMVector3Normalize(XMVector3Cross(vr, vc)); // slice normal//z축

	// form scaled basis vectors in mm-per-index
	XMVECTOR basisX = vr * pixelSpacingX;  // 1픽셀 이동 = X mm
	XMVECTOR basisY = vc * pixelSpacingY;  // 1픽셀 이동 = Y mm
	XMVECTOR basisZ = vn * sliceSpacing;   // 1슬라이스 이동 = Z mm

	// vector from origin to patientCoord
	XMVECTOR vOrigin = V(origin);
	XMVECTOR vPt = V(patientCoord);
	XMVECTOR d = vPt - vOrigin;// 원점에서 클릭 지점까지의 벡터

	// Solve linear system [basisX basisY basisZ] * [i j k]^T = d
	// Use linear algebra: invert 3x3 matrix or solve via Cramer's rule.
	// We'll compute inverse of 3x3 matrix M = [bx by bz]
	XMFLOAT3 bx = ToXMF3(basisX);
	XMFLOAT3 by = ToXMF3(basisY);
	XMFLOAT3 bz = ToXMF3(basisZ);

	// Build matrix M (column-major)
	// M = [ bx.x by.x bz.x
	//       bx.y by.y bz.y
	//       bx.z by.z bz.z ]
	float m00 = bx.x, m01 = by.x, m02 = bz.x;
	float m10 = bx.y, m11 = by.y, m12 = bz.y;
	float m20 = bx.z, m21 = by.z, m22 = bz.z;

	// determinant
	float det = m00 * (m11*m22 - m12 * m21)
		- m01 * (m10*m22 - m12 * m20)
		+ m02 * (m10*m21 - m11 * m20);

	if (fabs(det) < 1e-8f) {
		// degenerate basis; fallback to axis-aligned approximate
		// Choose axis based on viewIndex
		int idx = 0;
		switch (viewIndex) {
		case 1: // Axial -> use Z
			idx = static_cast<int>(round((patientCoord.z - origin.z) / sliceSpacing));
			idx = std::clamp(idx, 0, static_cast<int>(dims.z) - 1);
			return idx;
		case 2: // Coronal -> Y
			idx = static_cast<int>(round((patientCoord.y - origin.y) / pixelSpacingY));
			idx = std::clamp(idx, 0, static_cast<int>(dims.y) - 1);
			return idx;
		case 3: // Sagittal -> X
			idx = static_cast<int>(round((patientCoord.x - origin.x) / pixelSpacingX));
			idx = std::clamp(idx, 0, static_cast<int>(dims.x) - 1);
			return idx;
		default:
			return 0;
		}
	}

	// inverse matrix M^-1 (compute adjugate / det)
	float invDet = 1.0f / det;
	float i00 = (m11*m22 - m12 * m21) * invDet;
	float i01 = -(m01*m22 - m02 * m21) * invDet;
	float i02 = (m01*m12 - m02 * m11) * invDet;
	float i10 = -(m10*m22 - m12 * m20) * invDet;
	float i11 = (m00*m22 - m02 * m20) * invDet;
	float i12 = -(m00*m12 - m02 * m10) * invDet;
	float i20 = (m10*m21 - m11 * m20) * invDet;
	float i21 = -(m00*m21 - m01 * m20) * invDet;
	float i22 = (m00*m11 - m01 * m10) * invDet;


	// M = [basisX basisY basisZ] 행렬 구성
	// det(M) 계산 후 역행렬 M⁻¹ 계산
	// (i, j, k) = M⁻¹ * d
	XMFLOAT3 dv; XMStoreFloat3(&dv, d);
	// multiply M^-1 * d to get (i, j, k) in floating
	float fi = i00 * dv.x + i01 * dv.y + i02 * dv.z;
	float fj = i10 * dv.x + i11 * dv.y + i12 * dv.z;
	float fk = i20 * dv.x + i21 * dv.y + i22 * dv.z;

	// Round to nearest integer voxel indices
	int ii = static_cast<int>(std::lround(fi));// 정수 복셀 인덱스로 변환
	int jj = static_cast<int>(std::lround(fj));
	int kk = static_cast<int>(std::lround(fk));

	// clamp to valid range
	ii = std::clamp(ii, 0, static_cast<int>(dims.x) - 1); // 범위 제한
	jj = std::clamp(jj, 0, static_cast<int>(dims.y) - 1);
	kk = std::clamp(kk, 0, static_cast<int>(dims.z) - 1);

	qDebug() << "ComputeSliceIndexFromPatientCoord_Robust\n";


	//// depending on requested viewIndex, return correct slice index:
	//switch (viewIndex)
	//{
	//case 1:// Axial → Z축 슬라이스
	//    kk = std::clamp(kk, 0, static_cast<int>(fileReader->sliceIndex[viewIndex]) - 1);
	//    return kk; // axial -> k (slice along normal)
	//case 2:// Coronal → Y축 슬라이스
	//    jj = std::clamp(jj, 0, static_cast<int>(fileReader->sliceIndex[viewIndex]) - 1);
	//    return jj; // coronal -> j
	//case 3:// Sagittal → X축 슬라이스
	//    ii = std::clamp(ii, 0, static_cast<int>(fileReader->sliceIndex[viewIndex]) - 1);
	//    return ii; // sagittal -> i
	//default:
	//    kk = std::clamp(kk, 0, static_cast<int>(fileReader->sliceIndex[viewIndex]) - 1);
	//    return kk;
	//}


	qDebug() << "🔍 Debug Info:";
	qDebug() << "  ViewIndex:" << viewIndex;
	qDebug() << "  Computed indices (i, j, k):" << ii << jj << kk;
	qDebug() << "  dims:" << dims.x << dims.y << dims.z;
	qDebug() << "  fileReader->sliceIndex[viewIndex]:" << fileReader->sliceIndex[viewIndex];

	switch (viewIndex)
	{
	case 1: return kk; // axial
	case 2: return jj; // coronal
	case 3: return ii; // sagittal
	default: return kk;
	}

}


int QDirect3D11Widget::ComputeSliceIndexFromPatientCoord(int viewIndex, XMFLOAT3 patientCoord)
{
	XMFLOAT3 origin = fileReader->views.origin;
	XMFLOAT3 spacing = fileReader->views.spacing;
	int imageSize = fileReader->sliceIndex[viewIndex]; // 각 축의 슬라이스 개수

	//if (imageSize <= 0 || spacing.x <= 0 || spacing.y <= 0 || spacing.z <= 0)
	//    return 0; // 또는 -1로 에러 표시


	int index = 0;
	float dz;

	switch (viewIndex)
	{
	case 1: // Axial (Z축 기준)
		/*index = round((patientCoord.z - origin.z) / spacing.z);

		break;*/

		dz = patientCoord.z - origin.z;
		if (spacing.z < 0)  // Z축 반전되어 있으면
			dz = -dz;

		index = round(dz / abs(spacing.z));


		//   index = round((origin.z - patientCoord.z) / spacing.z);
		//  index = round((patientCoord.z - origin.z) / (-spacing.z));


		break;

	case 2: // Coronal (Y축 기준)
		index = round((patientCoord.y - origin.y) / spacing.y);

		break;

	case 3: // Sagittal (X축 기준)
		index = round((patientCoord.x - origin.x) / spacing.x);

		break;

	default:
		index = 0;
		break;
	}

	index = std::clamp(index, 0, static_cast<int>(imageSize) - 1);

	return index;
}


DirectX::XMFLOAT2 QDirect3D11Widget::GetNormalizedUV(int px, int py, int viewIndex)
{
	D3D11_VIEWPORT vp = CreateViewport(viewIndex);

	// 클릭 좌표 → 뷰포트 내 좌표
	float localX = px - vp.TopLeftX;
	float localY = py - vp.TopLeftY;

	// ✅ 정규화 (0~1 범위)
	float normX = localX / vp.Width;
	float normY = localY / vp.Height;

	// ✅ Aspect Ratio 보정
	float viewportAspect = vp.Width / vp.Height;

	// 각 뷰의 실제 데이터 aspect ratio
	float dataAspect = 1.0f;
	switch (viewIndex) {
	case 1: // Axial
		dataAspect = (fileReader->m_width * fileReader->views.spacing.x) /
			(fileReader->m_height * fileReader->views.spacing.y);
		break;
	case 2: // Coronal
		dataAspect = (fileReader->m_width * fileReader->views.spacing.x) /
			(fileReader->m_depth * fileReader->views.spacing.z);
		break;
	case 3: // Sagittal
		dataAspect = (fileReader->m_height * fileReader->views.spacing.y) /
			(fileReader->m_depth * fileReader->views.spacing.z);
		break;
	}

	// ✅ Aspect ratio 차이 보정
	if (viewportAspect > dataAspect) {
		// 뷰포트가 더 넓음 → X 좌표 보정
		float scale = dataAspect / viewportAspect;
		normX = (normX - 0.5f) * scale + 0.5f;
	}
	else {
		// 뷰포트가 더 높음 → Y 좌표 보정
		float scale = viewportAspect / dataAspect;
		normY = (normY - 0.5f) * scale + 0.5f;
	}

	return XMFLOAT2(normX, normY);
}


XMFLOAT3 QDirect3D11Widget::GetPatientCoordFromClick(int viewIndex, XMFLOAT2 uv)
{
	/*if (2 == viewIndex)
	{
		qDebug() << "a" << endl;
	}*/
	//// 영상 정보
	//XMFLOAT3 origin = fileReader->views[viewIndex].origin;     // 환자 좌표계 시작점
	//XMFLOAT3 spacing = fileReader->views[viewIndex].spacing;   // 픽셀 간격
	//XMFLOAT3 imageSize = fileReader->views[viewIndex].imageSize; // 영상 크기 (픽셀 단위)
	//int sliceIndex = fileReader->views[viewIndex].sliceIndex;  // 현재 슬라이스 인덱스


	// 영상 정보
	XMFLOAT3 origin = fileReader->views.origin;     // 환자 좌표계 시작점
	XMFLOAT3 spacing = fileReader->views.spacing;   // 픽셀 간격
	XMFLOAT3 imageSize = fileReader->views.imageSize; // 영상 크기 (픽셀 단위)
	int sliceIndex = fileReader->currentIndex[viewIndex];  // 현재 슬라이스 인덱스

	//// 텍스처 좌표 → 픽셀 좌표
	float px = uv.x * imageSize.x;
	float py = uv.y * imageSize.y;

	currentUV[viewIndex] = uv;

	XMFLOAT3 patientCoord;

	switch (viewIndex)
	{
	case 1: // 축상 (XY 평면, Z 고정)

		px = uv.x * fileReader->m_width;
		py = uv.y * fileReader->m_height;


		patientCoord.x = origin.x + px * spacing.x;
		patientCoord.y = origin.y + py * spacing.y;
		patientCoord.z = origin.z + sliceIndex * spacing.z;


		break;

	case 2: // 관상 (XZ 평면, Y 고정)
		px = uv.x * fileReader->m_width;   // X 방향
		py = uv.y * fileReader->m_depth;   // ✅ Z 방향 (depth 사용!)


		patientCoord.x = origin.x + px * spacing.x;
		patientCoord.y = origin.y + sliceIndex * spacing.y;
		patientCoord.z = origin.z + py * spacing.z;
		//	patientCoord.z = origin.z + (imageSize.y - py) * spacing.z;

			//currentPatientCoord[2] = patientCoord;

		break;

	case 3: // 시상 (YZ 평면, X 고정)

		px = uv.x * fileReader->m_height;  // Y 방향
		py = uv.y * fileReader->m_depth;   // ✅ Z 방향 (depth 사용!)


		patientCoord.x = origin.x + sliceIndex * spacing.x;
		patientCoord.y = origin.y + px * spacing.y;
		patientCoord.z = origin.z + py * spacing.z;

		//	patientCoord.z = origin.z + (imageSize.y - py) * spacing.z;

		//	currentPatientCoord[3] = patientCoord;
		break;

	default:
		patientCoord = GetDefaultPatientCenter(); // 또는 적절한 계산
		break;

	}

	//qDebug() << "view index: (" << viewIndex<< ")";
 //   qDebug() << "Origin: (" << origin.x << ", " << origin.y << ", " << origin.z << ")";
 //   qDebug() << "Spacing: (" << spacing.x << ", " << spacing.y << ", " << spacing.z << ")";
 //   qDebug() << "SliceIndex: " << sliceIndex;
 //   qDebug() << "PatientCoord: (" << patientCoord.x << ", " << patientCoord.y << ", " << patientCoord.z << ")";

	return patientCoord;
}


XMFLOAT2 QDirect3D11Widget::GetCrossUVFromPatientCoord(int viewIndex, XMFLOAT3 patientCoord)
{
	XMFLOAT3 origin = fileReader->views.origin;
	XMFLOAT3 spacing = fileReader->views.spacing;
	XMFLOAT3 imageSize = fileReader->views.imageSize;

	if (spacing.x <= 0 || spacing.y <= 0 || spacing.z <= 0 ||
		imageSize.x <= 0 || imageSize.y <= 0 || imageSize.z <= 0) {
		return XMFLOAT2(0.5f, 0.5f); // fallback 중앙값
	}

	float px = 0.0f, py = 0.0f;
	float sizeX = 0.0f, sizeY = 0.0f;
	float spacingX = 1.0f, spacingY = 1.0f;

	switch (viewIndex)
	{
	case 1: // Axial (XY 평면)
		px = (patientCoord.x - origin.x) / spacing.x;
		py = (patientCoord.y - origin.y) / spacing.y;
		sizeX = imageSize.x;
		sizeY = imageSize.y;
		spacingX = spacing.x;
		spacingY = spacing.y;
		break;

	case 2: // Coronal (XZ 평면)
		px = (patientCoord.x - origin.x) / spacing.x;
		py = (patientCoord.z - origin.z) / spacing.z;
		sizeX = imageSize.x;
		sizeY = imageSize.z;
		spacingX = spacing.x;
		spacingY = spacing.z;
		break;

	case 3: // Sagittal (YZ 평면)
		px = (patientCoord.y - origin.y) / spacing.y;
		py = (patientCoord.z - origin.z) / spacing.z;
		sizeX = imageSize.y;
		sizeY = imageSize.z;
		spacingX = spacing.y;
		spacingY = spacing.z;
		break;

	default:
		return XMFLOAT2(0.5f, 0.5f);
	}

	// 정규화
	XMFLOAT2 uv;
	uv.x = px / sizeX;
	uv.y = py / sizeY;

	// ✅ Aspect ratio 보정
	float dataAspect = (sizeX * spacingX) / (sizeY * spacingY);
	D3D11_VIEWPORT vp = CreateViewport(viewIndex);
	float viewportAspect = vp.Width / vp.Height;

	if (viewportAspect > dataAspect) {
		float scale = dataAspect / viewportAspect;
		uv.x = (uv.x - 0.5f) * scale + 0.5f;
	}
	else {
		float scale = viewportAspect / dataAspect;
		uv.y = (uv.y - 0.5f) * scale + 0.5f;
	}

	// 범위 제한
	uv.x = std::clamp(uv.x, 0.0f, 1.0f);
	uv.y = std::clamp(uv.y, 0.0f, 1.0f);

	return uv;
}


// ========================================
// 1. 슬라이스 평면 초기화
// ========================================
void QDirect3D11Widget::InitializeSlicePlanes() {
	// Quad 정점 데이터 (위치 + 텍스처 좌표)
	struct PlaneVertex {
		XMFLOAT3 position;
		XMFLOAT2 texcoord;
	};

	PlaneVertex vertices[] = {
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2(0.0f, 1.0f) }, // 좌하
		{ XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, // 좌상
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 1.0f) }, // 우하
		{ XMFLOAT3(0.5f,  0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f) }  // 우상
	};

	//UINT indices[] = { 0, 1, 2, 2, 1, 3 };
	UINT lineIndices[] = {
		0, 1,  // 좌측
		1, 3,  // 상단
		3, 2,  // 우측
		2, 0   // 하단
	};


	// Vertex Buffer 생성
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;

	HRESULT hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_CoronalPlane.vertexBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create plane vertex buffer!";
		return;
	}

	// 3개 평면 모두 같은 버퍼 공유
	m_AxialPlane.vertexBuffer = m_CoronalPlane.vertexBuffer;
	m_SagittalPlane.vertexBuffer = m_CoronalPlane.vertexBuffer;

	// Index Buffer 생성
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(lineIndices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = lineIndices;

	hr = m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_CoronalPlane.indexBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create plane index buffer!";
		return;
	}

	m_AxialPlane.indexBuffer = m_CoronalPlane.indexBuffer;
	m_SagittalPlane.indexBuffer = m_CoronalPlane.indexBuffer;

	// World Matrix 초기화
	//XMStoreFloat4x4(&m_axialPlane.worldMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_coronalPlane.worldMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_sagittalPlane.worldMatrix, XMMatrixIdentity());



	//=======================================================
	//XMMATRIX scale = XMMatrixScaling(0.8f, 0.8f, 0.8f); // ← 여기서 크기 조절

// ---- Axial (XY plane, z=0)

	
	XMMATRIX worldA = /*scale **/ XMMatrixRotationX(XM_PIDIV2);
	XMStoreFloat4x4(&m_AxialPlane.worldMatrix, XMMatrixTranspose(worldA));
	//constants.World = m_axialPlane.worldMatrix; // ✅ 저장된 World Matrix 사용

	// ---- Coronal (XZ plane, y=0)

	XMMATRIX worldC = /*scale **/ XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&m_CoronalPlane.worldMatrix, XMMatrixTranspose(worldC));
	//constants.World = m_coronalPlane.worldMatrix; // ✅ 저장된 World Matrix 사용


	XMMATRIX worldS = /*scale **/ XMMatrixRotationY(XM_PIDIV2);
	XMStoreFloat4x4(&m_SagittalPlane.worldMatrix, XMMatrixTranspose(worldS));
	//constants.World = m_sagittalPlane.worldMatrix; // ✅ 저장된 World Matrix 사용


	//===============================================================

	qDebug() << "Slice planes initialized successfully!";
}

// ========================================
// 2. 바운딩 큐브 초기화
// ========================================
void QDirect3D11Widget::InitializeBoundingCube() {
	// 큐브의 8개 꼭짓점
	XMFLOAT3 cubeVertices[] = {
		// 앞면 (Z = -0.5)
		XMFLOAT3(-0.5f, -0.5f, -0.5f), // 0
		XMFLOAT3(0.5f, -0.5f, -0.5f), // 1
		XMFLOAT3(0.5f,  0.5f, -0.5f), // 2
		XMFLOAT3(-0.5f,  0.5f, -0.5f), // 3
		// 뒷면 (Z = 0.5)
		XMFLOAT3(-0.5f, -0.5f,  0.5f), // 4
		XMFLOAT3(0.5f, -0.5f,  0.5f), // 5
		XMFLOAT3(0.5f,  0.5f,  0.5f), // 6
		XMFLOAT3(-0.5f,  0.5f,  0.5f)  // 7
	};

	// 12개 모서리를 선으로 그리기 위한 인덱스 (24개 = 12선 * 2정점)
	UINT cubeIndices[] = {
		// 앞면 4개 모서리
		0, 1,  1, 2,  2, 3,  3, 0,
		// 뒷면 4개 모서리
		4, 5,  5, 6,  6, 7,  7, 4,
		// 앞뒤 연결 4개 모서리
		0, 4,  1, 5,  2, 6,  3, 7
	};

	// Vertex Buffer 생성
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(cubeVertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = cubeVertices;

	HRESULT hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_cubeVertexBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create cube vertex buffer!";
		return;
	}

	// Index Buffer 생성
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(cubeIndices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = cubeIndices;

	hr = m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_cubeIndexBuffer);
	if (FAILED(hr)) {
		qDebug() << "Failed to create cube index buffer!";
		return;
	}

	qDebug() << "Bounding cube initialized successfully!";
}

// ========================================
// 3. 슬라이스 평면 위치 업데이트
// ========================================
void QDirect3D11Widget::UpdateSlicePlanePositions() {
	if (!fileReader) return;

	
	XMFLOAT3 origin = fileReader->views.origin;
	XMFLOAT3 spacing = fileReader->views.spacing;

	// ⚙️ 공통 스케일
	//XMMATRIX scale = XMMatrixScaling(0.9f, 0.9f, 0.9f);

	{
		// ===== Axial 평면 (XY 평면, Z축 이동) =====
		float totalDepth = fileReader->m_depth * spacing.z;
		float axialZ = origin.z + fileReader->currentIndex[1] * spacing.z;
		float normalizedZ = -(axialZ - origin.z - totalDepth * 0.5f) / totalDepth;

		// 🔹 평면 이동 범위 확장 (예: 1.5배 정도)
		normalizedZ *= 1.9f;

		XMMATRIX axialWorld = /*scale **/
			XMMatrixRotationX(XM_PIDIV2) *
			XMMatrixTranslation(0.0f, normalizedZ, 0.0f);
		//XMStoreFloat4x4(&m_axialPlane.worldMatrix, axialWorld);
		XMStoreFloat4x4(&m_AxialPlane.worldMatrix, XMMatrixTranspose(axialWorld));

		//XMStoreFloat4x4(&constants.World, axialWorld);
		//m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		qDebug() << "Axial Z:" << normalizedZ << "(slice" << fileReader->currentIndex[1] << "/" << fileReader->m_depth << ")";

		//DrawPlane(m_axialPlane);
	}
	{
		// ===== Coronal 평면 (XZ 평면, Y축 이동) =====
		float totalHeight = fileReader->m_height * spacing.y;
		float coronalY = origin.y + fileReader->currentIndex[2] * spacing.y;
		float normalizedY = (coronalY - origin.y - totalHeight * 0.5f) / totalHeight;
		normalizedY *= 1.9f;

		XMMATRIX coronalWorld = /*scale **/ XMMatrixTranslation(0.0f, 0.0f, normalizedY);
		
		//XMStoreFloat4x4(&m_coronalPlane.worldMatrix, coronalWorld);
		XMStoreFloat4x4(&m_CoronalPlane.worldMatrix, XMMatrixTranspose(coronalWorld));


		//XMStoreFloat4x4(&constants.World,  coronalWorld);
		//m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		qDebug() << "Coronal Y:" << normalizedY << "(slice" << fileReader->currentIndex[2] << "/" << fileReader->m_height << ")";

		//DrawPlane(m_coronalPlane);
	}
	{
		// ===== Sagittal 평면 (YZ 평면, X축 이동) =====
		float totalWidth = fileReader->m_width * spacing.x;
		float sagittalX = origin.x + fileReader->currentIndex[3] * spacing.x;
		float normalizedX = (sagittalX - origin.x - totalWidth * 0.5f) / totalWidth;

		// 🔹 평면 이동 범위 확장 (예: 1.5배 정도)
		normalizedX *= 1.9f;

		XMMATRIX sagittalWorld =/* scale **/
			XMMatrixRotationY(XM_PIDIV2) *
			XMMatrixTranslation(normalizedX, 0.0f, 0.0f);
		//XMStoreFloat4x4(&m_sagittalPlane.worldMatrix, sagittalWorld);
		XMStoreFloat4x4(&m_SagittalPlane.worldMatrix, XMMatrixTranspose(sagittalWorld));

		//XMStoreFloat4x4(&constants.World, sagittalWorld);
		//m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);
		qDebug() << "Sagittal X:" << normalizedX << "(slice" << fileReader->currentIndex[3] << "/" << fileReader->m_width << ")";

		//DrawPlane(m_sagittalPlane);
	}
}

// ========================================
// 4. 바운딩 큐브 렌더링
// ========================================
void QDirect3D11Widget::RenderBoundingCube(const VolumeConstants& constants) {
	if (!fileReader) return;

	float width = fileReader->m_width * fileReader->views.spacing.x;
	float height = fileReader->m_height * fileReader->views.spacing.y;
	float depth = fileReader->m_depth * fileReader->views.spacing.z;

	XMFLOAT3 origin = fileReader->views.origin;

	VolumeConstants cubeConstants = constants;
	XMMATRIX cubeWorld = XMMatrixScaling(width, height, depth) *
		XMMatrixTranslation(origin.x + width * 0.5f,
			origin.y + height * 0.5f,
			origin.z + depth * 0.5f);
	XMStoreFloat4x4(&cubeConstants.World, cubeWorld);
	cubeConstants.Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	// Constant Buffer 업데이트
	m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr,
		&cubeConstants, 0, 0);

	// ✅ 큐브 전용 Input Layout 사용
	m_pDeviceContext->IASetInputLayout(m_cubeInputLayout);

	// 버퍼 바인딩
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_cubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 큐브 그리기
	m_pDeviceContext->DrawIndexed(24, 0, 0);
}

// ========================================
// 5. 평면 그리기
// ========================================
void QDirect3D11Widget::DrawPlane(const SlicePlane& plane)
{
	m_pDeviceContext->IASetInputLayout(m_volumeInputLayout);

	UINT stride = sizeof(float) * 5;
	UINT offset = 0;

	m_pDeviceContext->IASetVertexBuffers(0, 1, &plane.vertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(plane.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


	m_pDeviceContext->DrawIndexed(8, 0, 0); // 2 triangles = 6 indices

}
void QDirect3D11Widget::DrawSliceQuad()
{
	// ✅ 정점 레이아웃: (x, y, z, u, v)
	struct Vertex {
		float x, y, z;
		float u, v;
	};

	Vertex vertices[] =
	{
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, // Bottom-left
		{ -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }, // Top-left
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f }, // Top-right
		{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f }  // Bottom-right
	};

	UINT indices[] = { 0, 1, 2, 0, 2, 3 };

	// --- 버퍼 설정 ---
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vinitData = {};
	vinitData.pSysMem = vertices;

	ComPtr<ID3D11Buffer> vertexBuffer;
	m_pDevice->CreateBuffer(&vbd, &vinitData, &vertexBuffer);

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinitData = {};
	iinitData.pSysMem = indices;

	ComPtr<ID3D11Buffer> indexBuffer;
	m_pDevice->CreateBuffer(&ibd, &iinitData, &indexBuffer);

	// --- 렌더링 ---
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pDeviceContext->IASetInputLayout(m_prevVolumeInputLayout); // 이미 있는 InputLayout
	m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// --- 드로우 ---
	m_pDeviceContext->DrawIndexed(6, 0, 0);
}







void QDirect3D11Widget::mouseMoveEvent(QMouseEvent* event) {
	/*ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(event->pos().x(), event->pos().y());*/
}



void QDirect3D11Widget::mouseReleaseEvent(QMouseEvent* event) {
	/*ImGuiIO& io = ImGui::GetIO();
	if (event->button() == Qt::LeftButton)
		io.MouseDown[0] = false;*/
}




void QDirect3D11Widget::onReset()
{
	// 1. 疫꿸퀣???귐딅꺖????곸젫
   /* for (auto& view : m_SRViews.slices)
		ReleaseObject(view);
	m_SRViews.slices.clear();*/

	//	ReleaseObject(m_pSwapChainRTV);

	ID3D11Texture2D* pBackBuffer = Q_NULLPTR;


	ReleaseObject(pBackBuffer);

}



//float z = originZ + sliceIndex * sliceSpacing;
//
//XMMATRIX scale = XMMatrixScaling(2.5f, 2.5f, 2.5f);
//XMMATRIX world = scale * XMMatrixTranslation(0.0f, 0.0f, z);
//XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));
//
//m_pDeviceContext->UpdateSubresource(m_volumeConstantBuffer, 0, nullptr, &constants, 0, 0);



//fileReader->currentIndex[0]
void QDirect3D11Widget::onAxialScroll(int value) {
	// Axial 뷰의 슬라이스 변경
	if (!fileReader) return;

	
		clickedViewIndex = 1;

	// ImGui 로직과 동일: 스크롤 값을 슬라이스 인덱스로 변환
	int newIndex = value;
	newIndex = std::clamp(newIndex, 0, fileReader->m_depth - 1);

	if (newIndex != fileReader->currentIndex[1]) {
		fileReader->currentIndex[1] = newIndex;
		qDebug() << "[Axial] slice index:" << newIndex;

		// 텍스처 업데이트
		fileReader->UpdateAxialTexture(newIndex);

		// SRV 및 RTV 업데이트
		ID3D11Texture2D* tex = fileReader->axialTextureCache[newIndex];
		ID3D11ShaderResourceView* srv = getSRVForTexture(tex);
		m_SRViews.slices[1] = srv;

		ID3D11RenderTargetView* rtv = getRTVForTexture(tex);
		m_RTViews.slices[1] = rtv;


		UpdateSlicePlanePositions();

		//RenderVolumeView(); // 강제 호출로 확인


		// 렌더링 업데이트
		update();

		RenderVolumeView(); // 강제 호출로 확인
	}



	sliceInfoAxial->hide();
	// 슬라이스 정보 업데이트
	sliceInfoAxial->setText(QString("Image %1/%2").arg(newIndex + 1).arg(fileReader->m_depth));
	//sliceInfoAxial->adjustSize();
	//sliceInfoAxial->repaint();  // 강제로 다시 그리기
	sliceInfoAxial->show();
}

void QDirect3D11Widget::onCoronalScroll(int value) {
	if (!fileReader) return;

	clickedViewIndex = 2;

	// ImGui 로직: Coronal은 역방향으로 계산
	int newIndex = fileReader->m_height - 1 - value;
	// 또는 정방향으로 하려면:
	//int newIndex = value;
	newIndex = std::clamp(newIndex, 0, fileReader->m_height - 1);

	if (newIndex != fileReader->currentIndex[2]) {
		fileReader->currentIndex[2] = newIndex;
		qDebug() << "[Coronal] slice index:" << newIndex;

		// 텍스처 업데이트
		fileReader->UpdateCoronalTexture(newIndex);

		// SRV 및 RTV 업데이트
		ID3D11Texture2D* tex = fileReader->coronalTextureCache[newIndex];
		ID3D11ShaderResourceView* srv = getSRVForTexture(tex);
		m_SRViews.slices[2] = srv;

		ID3D11RenderTargetView* rtv = getRTVForTexture(tex);
		m_RTViews.slices[2] = rtv;

		//auto p = GetPatientCoordFromClick(2, currentUV[2]);

		//for (int i{ 1 }; i <= 3; ++i) {
		//	if (2 != i) {
		//		UpdateCrosshairFromPatientCoord(p, i);

		//		// 3. 셰이더에 바인딩
		//		m_pDeviceContext->PSSetConstantBuffers(0, 1, &fileReader->m_crosshairBuffer);
		//	}
		//}

		UpdateSlicePlanePositions();

		// 렌더링 업데이트
		update();

		RenderVolumeView(); // 강제 호출로 확인
	}

	sliceInfoCoronal->hide();
	sliceInfoCoronal->setText(QString("Image %1/%2").arg(newIndex + 1).arg(fileReader->m_height));
	// sliceInfoCoronal->adjustSize();
	 //sliceInfoCoronal->repaint();  // 강제로 다시 그리기
	sliceInfoCoronal->show();
}

void QDirect3D11Widget::onSagittalScroll(int value) {
	if (!fileReader) return;

	clickedViewIndex = 3;

	// ImGui 로직과 동일
	int newIndex = fileReader->m_width - 1 - value;;
	newIndex = std::clamp(newIndex, 0, fileReader->m_width - 1);

	if (newIndex != fileReader->currentIndex[3]) {
		fileReader->currentIndex[3] = newIndex;
		qDebug() << "[Sagittal] slice index:" << newIndex;

		// 텍스처 업데이트
		fileReader->UpdateSagittalTexture(newIndex);

		// SRV 및 RTV 업데이트
		ID3D11Texture2D* tex = fileReader->sagittalTextureCache[newIndex];
		ID3D11ShaderResourceView* srv = getSRVForTexture(tex);
		m_SRViews.slices[3] = srv;

		ID3D11RenderTargetView* rtv = getRTVForTexture(tex);
		m_RTViews.slices[3] = rtv;

		//auto p = GetPatientCoordFromClick(3, currentUV[3]);


		//for (int i{ 1 }; i <= 2; ++i) {
		//	
		//		UpdateCrosshairFromPatientCoord(p, i);

		//		// 3. 셰이더에 바인딩
		//		m_pDeviceContext->PSSetConstantBuffers(0, 1, &fileReader->m_crosshairBuffer);
		//	
		//}

		//RenderVolumeView();

		UpdateSlicePlanePositions();

		// 렌더링 업데이트
		update();

		RenderVolumeView(); // 강제 호출로 확인
	}

	sliceInfoSagittal->hide();
	sliceInfoSagittal->setText(QString("Image %1/%2").arg(newIndex + 1).arg(fileReader->m_width));
	//sliceInfoSagittal->adjustSize();
   // sliceInfoSagittal->repaint();  // 강제로 다시 그리기
	sliceInfoSagittal->show();
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

void QDirect3D11Widget::paintEvent(QPaintEvent* event)
{
	//// D3D11 렌더링
	//render();

	//// Qt로 십자선 그리기
	//QPainter painter(this);
	//painter.setRenderHint(QPainter::Antialiasing);

	//int w = width();
	//int h = height();
	//int halfW = w / 2;
	//int halfH = h / 2;

	//// 그림자 (어두운 선)
	//QPen shadowPen(QColor(0, 0, 0, 150), 2);
	//painter.setPen(shadowPen);
	//painter.drawLine(halfW + 1, 1, halfW + 1, h + 1);
	//painter.drawLine(1, halfH + 1, w + 1, halfH + 1);

	//// 실제 선 (밝은 빨간색)
	//QPen linePen(QColor(255, 80, 80), 2);
	//painter.setPen(linePen);
	//painter.drawLine(halfW, 0, halfW, h);
	//painter.drawLine(0, halfH, w, halfH);




}

void QDirect3D11Widget::resizeEvent(QResizeEvent* event)
{
	if (m_bDeviceInitialized)
	{
		onReset();
		emit widgetResized();
	}

	int w = width() / 2;
	int h = height() / 2;
	int scrollBarWidth = 16;
	int gap = 4;
	int labelMargin = 6;

	// 스크롤바 위치 (각 뷰의 오른쪽)
	scrollAxial->setGeometry(width() - scrollBarWidth - gap, gap,
		scrollBarWidth, h - gap * 2);
	scrollCoronal->setGeometry(w - scrollBarWidth - gap, h + gap,
		scrollBarWidth, h - gap * 2);
	scrollSagittal->setGeometry(width() - scrollBarWidth - gap, h + gap,
		scrollBarWidth, h - gap * 2);

	// 라벨 위치 (좌측 상단 모서리)
	labelVolume->move(0, labelMargin);
	labelAxial->move(w + 2, labelMargin);
	labelCoronal->move(0, h + labelMargin);
	labelSagittal->move(w + 2, h + labelMargin);

	// 슬라이스 정보 라벨 위치 (뷰 이름 라벨 바로 아래)
	int sliceInfoOffset = 28; // 뷰 이름 라벨 높이 + 간격
	sliceInfoAxial->move(w + labelMargin, labelMargin + sliceInfoOffset);
	sliceInfoCoronal->move(labelMargin, h + labelMargin + sliceInfoOffset);
	sliceInfoSagittal->move(w + labelMargin, h + labelMargin + sliceInfoOffset);

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
