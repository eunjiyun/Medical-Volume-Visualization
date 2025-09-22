#include "stdafx.h"
#include "d3dclass.h"
#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "QDirect3D11Widget.h"


D3DClass::D3DClass()
	: ui(new Ui::ViewerSampleClass)
{
	//qtD3dWidget = ui->view;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}


bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	// ??륁춦??녿┛???怨밴묶?????館鍮??덈뼄
	m_vsync_enabled = vsync;

	// DirectX 域밸챶????紐낃숲??륁뵠????븍꽅?귐? ??밴쉐??몃빍??
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
	{
		return false;
	}

	// ??븍꽅??揶쏆빘猿쒐몴??????뤿연 筌ｃ꺂苡뀐쭪?域밸챶???燁삳?諭??紐낃숲??륁뵠??????怨? ??밴쉐??몃빍??
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter)))
	{
		return false;
	}

	// ?곗뮆??筌뤴뫀?????????筌ｃ꺂苡뀐쭪?????怨? 筌왖?類λ???덈뼄.
	IDXGIOutput* adapterOutput = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapterOutput)))
	{
		return false;
	}

	// ?곗뮆??(筌뤴뫀?????????DXGI_FORMAT_R8G8B8A8_UNORM ??뽯뻻 ?類ㅻ뻼??筌띿쉶??筌뤴뫀諭???? 揶쎛?紐꾩긿??덈뼄
	unsigned int numModes = 0;
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
	{
		return false;
	}

	// 揶쎛?館釉?筌뤴뫀諭?筌뤴뫀??怨? 域밸챶???뚮춦??鈺곌퀬鍮?????館釉??귐딅뮞?紐? ??밴쉐??몃빍??
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// ??곸젫 ?遺용뮞???쟿??筌뤴뫀諭???????귐딅뮞?紐? 筌?쑴???덈뼄
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
	{
		return false;
	}

	// ??곸젫 筌뤴뫀諭??遺용뮞???쟿??筌뤴뫀諭???????遺얇늺 ??덊돩/?誘れ뵠??筌띿쉶???遺용뮞???쟿??筌뤴뫀諭띄몴?筌≪뼚???덈뼄.
	// ?怨밸???野껉퍔??筌≪뼚?앾쭖?筌뤴뫀??怨쀬벥 ??덉쨮?⑥쥙臾???쑴????브쑬??? ?브쑴??揶쏅??????館鍮??덈뼄.
	unsigned int numerator = 0;
	unsigned int denominator = 0;
	for (unsigned int i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// ??쑬逾??쇰춦??뽰벥 ?닌듼쒙㎗?? ??대뮸??덈뼄
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc)))
	{
		return false;
	}

	// ??쑬逾??쇰춦??筌롫뗀?덄뵳???몄쎗 ??μ맄??筌롫떽?獄쏅뗄?????μ맄嚥????館鍮??덈뼄
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// ??쑬逾??쇰춦??뽰벥 ??已?????館鍮??덈뼄
	size_t stringLength = 0;
	if (wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128) != 0)
	{
		return false;
	}

	// ?遺용뮞???쟿??筌뤴뫀諭??귐딅뮞?紐? ??곸젫??몃빍??
	delete[] displayModeList;
	displayModeList = 0;

	// ?곗뮆??????怨? ??곸젫??몃빍??
	adapterOutput->Release();
	adapterOutput = 0;

	// ????怨? ??곸젫??몃빍??
	adapter->Release();
	adapter = 0;

	// ??븍꽅??揶쏆빘猿쒐몴???곸젫??몃빍??
	factory->Release();
	factory = 0;

	// ??쇱넁筌ｋ똻???닌듼쒙㎗?? ?λ뜃由?酉鍮??덈뼄
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 獄쏄퉭苡??? 1揶쏆뮆彛??????롫즲嚥?筌왖?類λ???덈뼄
	swapChainDesc.BufferCount = 1;

	// 獄쏄퉭苡??깆벥 ?蹂?뵠?? ?誘れ뵠??筌왖?類λ???덈뼄
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// 32bit ??쀫읂??곷뮞????쇱젟??몃빍??
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 獄쏄퉭苡??깆벥 ??덉쨮?⑥쥙臾???쑴?????쇱젟??몃빍??
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// 獄쏄퉭苡??깆벥 ?????몃즲??筌왖?類λ???덈뼄
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// ??뺣쐭筌띻낯肉????????덈즲???紐껊굶??筌왖?類λ???덈뼄
	swapChainDesc.OutputWindow = hwnd;

	// 筌렺?怨쀪묘???춦???類ｋ빍??
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 筌≪럥???or ????쎄쾿??筌뤴뫀諭띄몴???쇱젟??몃빍??
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// ??쇳떔 ??깆뵥 ??뽮퐣 獄???由곁몴?筌왖?類λ릭筌왖 ??놁벉??곗쨮 ??쇱젟??몃빍??
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ?곗뮆?????쇱벉 獄쏄퉭苡??? ??쑴??袁⑥쨯 筌왖?類λ???덈뼄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// ?곕떽? ???????삋域밸챶? ?????? ??녿뮸??덈뼄
	swapChainDesc.Flags = 0;

	// ??깆퓗??덇볼??DirectX 11 嚥???쇱젟??몃빍??
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// ??쇱넁 筌ｋ똻?? Direct3D ?關??獄?Direct3D ?關???뚢뫂???쎈뱜??筌띾슢踰??덈뼄.
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext)))
	{
		return false;
	}




	// 獄쏄퉭苡??????怨? ??대선??щ빍??
	ID3D11Texture2D* backBufferPtr = nullptr;
	if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr)))
	{
		return false;
	}

	// 獄?甕곌쑵??????怨뺤쨮 ???쐭 ??野??됯퀡? ??밴쉐??뺣뼄.
	if (FAILED(m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView)))
	{
		return false;
	}

	// 獄쏄퉭苡??????怨? ??곸젫??몃빍??
	backBufferPtr->Release();
	backBufferPtr = 0;

	// 繹먮봿??甕곌쑵???닌듼쒙㎗?? ?λ뜃由?酉鍮??덈뼄
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// 繹먮봿??甕곌쑵???닌듼쒙㎗?? ?臾믨쉐??몃빍??
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// ??쇱젟??繹먮봿?좄린袁る쓠 ?닌듼쒙㎗?? ?????뤿연 繹먮봿??甕곌쑵????용뮞?얜Ŧ? ??밴쉐??몃빍??
	if (FAILED(m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer)))
	{
		return false;
	}

	// ??쎈???怨밴묶 ?닌듼쒙㎗?? ?λ뜃由?酉鍮??덈뼄
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// ??쎈???怨밴묶 ?닌듼쒙㎗?? ?臾믨쉐??몃빍??
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// ??? ?類ｃ늺????쎈????쇱젟??낅빍??
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// ??? ?猷멥늺????쎈????쇱젟??낅빍??
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 繹먮봿????쎈???怨밴묶????밴쉐??몃빍??
	if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState)))
	{
		return false;
	}

	// 繹먮봿????쎈???怨밴묶????쇱젟??몃빍??
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// 繹먮봿????쎈???됯퀣???닌듼쒙㎗?? ?λ뜃由?酉鍮??덈뼄
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// 繹먮봿????쎈?????닌듼쒙㎗?? ??쇱젟??몃빍??
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// 繹먮봿????쎈???됯퀡? ??밴쉐??몃빍??
	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView)))
	{
		return false;
	}

	// ???쐭筌??????됯퀣? 繹먮봿????쎈??甕곌쑵?곭몴??곗뮆?????쐭 ???뵠????깆뵥??獄쏅뗄???븍???덈뼄
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// 域밸챶??쭪??????곫ⓦ끆??獄쎻뫖苡??野껉퀣?????뤿뮞???닌듼쒙㎗?? ??쇱젟??몃빍??
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	//rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	//rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// 獄쎻뫕???臾믨쉐???닌듼쒙㎗?곷퓠????뤿뮞????깆뵠?? ?怨밴묶??筌띾슢踰??덈뼄
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState)))
	{
		return false;
	}

	// ??곸젫 ??뤿뮞????깆뵠?? ?怨밴묶????쇱젟??몃빍??
	m_deviceContext->RSSetState(m_rasterState);

	// ???쐭筌띻낯???袁る퉸 ?됯퀬猷?紐? ??쇱젟??몃빍??
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// ?됯퀬猷?紐? ??밴쉐??몃빍??
	m_deviceContext->RSSetViewports(1, &viewport);

	// ??????곗졊????쇱젟??몃빍??
	float fieldOfView = XM_PI / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	// 3D ???쐭筌띻낯??袁る립 ??????곗졊??筌띾슢踰??덈뼄
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// ?硫명???곗졊????踰???곗졊嚥??λ뜃由?酉鍮??덈뼄
	m_worldMatrix = XMMatrixIdentity();

	// 2D ???쐭筌띻낯??袁る립 筌욊낫????????곗졊??筌띾슢踰??덈뼄
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);



	//qtD3dWidget->init();

	return true;
}


void D3DClass::Shutdown()
{
	// ?ル굝利?????덈즲??筌뤴뫀諭뜻에???쇱젟??? ??놁몵筌???쇱넁 筌ｋ똻?????곸젫 ??????됱뇚揶쎛 獄쏆뮇源??몃빍??
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	// 甕곌쑵?곭몴?筌왖????깆뱽 ??쇱젟??몃빍??
	float color[4] = { red, green, blue, alpha };

	// 獄쏄퉭苡??? 筌왖?怨룸빍??
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// 繹먮봿??甕곌쑵?곭몴?筌왖?怨룸빍??
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void D3DClass::EndScene()
{
	// ???쐭筌띻낯???袁⑥┷??뤿????嚥??遺얇늺??獄?甕곌쑵?곭몴???뽯뻻??몃빍??
	if (m_vsync_enabled)
	{
		// ?遺얇늺 ??덉쨮 ?⑥쥙臾???쑴????⑥쥙???몃빍??
		m_swapChain->Present(1, 0);
	}
	else
	{
		// 揶쎛?館釉???쥓?ㅵ칰??곗뮆???몃빍??
		m_swapChain->Present(0, 0);
	}
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}


void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
}