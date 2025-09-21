#include "stdafx.h"
#include "d3dclass.h"
#include <QtWidgets/QMainWindow>
#include <QCheckBox>


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
	//// ?섏쭅?숆린???곹깭瑜???ν빀?덈떎
	//m_vsync_enabled = vsync;

	//// DirectX 洹몃옒???명꽣?섏씠???⑺넗由щ? ?앹꽦?⑸땲??
	//IDXGIFactory* factory = nullptr;
	//if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
	//{
	//	return false;
	//}

	//// ?⑺넗由?媛앹껜瑜??ъ슜?섏뿬 泥ル쾲吏?洹몃옒??移대뱶 ?명꽣?섏씠???대럞?곕? ?앹꽦?⑸땲??
	//IDXGIAdapter* adapter = nullptr;
	//if (FAILED(factory->EnumAdapters(0, &adapter)))
	//{
	//	return false;
	//}

	//// 異쒕젰(紐⑤땲???????泥ル쾲吏??대럞?곕? 吏?뺥빀?덈떎.
	//IDXGIOutput* adapterOutput = nullptr;
	//if (FAILED(adapter->EnumOutputs(0, &adapterOutput)))
	//{
	//	return false;
	//}

	//// 異쒕젰 (紐⑤땲???????DXGI_FORMAT_R8G8B8A8_UNORM ?쒖떆 ?뺤떇??留욌뒗 紐⑤뱶 ?섎? 媛?몄샃?덈떎
	//unsigned int numModes = 0;
	//if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
	//{
	//	return false;
	//}

	//// 媛?ν븳 紐⑤뱺 紐⑤땲?곗? 洹몃옒?쎌뭅??議고빀????ν븷 由ъ뒪?몃? ?앹꽦?⑸땲??
	//DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	//if (!displayModeList)
	//{
	//	return false;
	//}

	//// ?댁젣 ?붿뒪?뚮젅??紐⑤뱶?????由ъ뒪?몃? 梨꾩썎?덈떎
	//if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
	//{
	//	return false;
	//}

	//// ?댁젣 紐⑤뱺 ?붿뒪?뚮젅??紐⑤뱶??????붾㈃ ?덈퉬/?믪씠??留욌뒗 ?붿뒪?뚮젅??紐⑤뱶瑜?李얠뒿?덈떎.
	//// ?곹빀??寃껋쓣 李얠쑝硫?紐⑤땲?곗쓽 ?덈줈怨좎묠 鍮꾩쑉??遺꾨え? 遺꾩옄 媛믪쓣 ??ν빀?덈떎.
	//unsigned int numerator = 0;
	//unsigned int denominator = 0;
	//for (unsigned int i = 0; i<numModes; i++)
	//{
	//	if (displayModeList[i].Width == (unsigned int)screenWidth)
	//	{
	//		if (displayModeList[i].Height == (unsigned int)screenHeight)
	//		{
	//			numerator = displayModeList[i].RefreshRate.Numerator;
	//			denominator = displayModeList[i].RefreshRate.Denominator;
	//		}
	//	}
	//}

	//// 鍮꾨뵒?ㅼ뭅?쒖쓽 援ъ“泥대? ?살뒿?덈떎
	//DXGI_ADAPTER_DESC adapterDesc;
	//if (FAILED(adapter->GetDesc(&adapterDesc)))
	//{
	//	return false;
	//}

	//// 鍮꾨뵒?ㅼ뭅??硫붾え由??⑸웾 ?⑥쐞瑜?硫붽?諛붿씠???⑥쐞濡???ν빀?덈떎
	//m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//// 鍮꾨뵒?ㅼ뭅?쒖쓽 ?대쫫????ν빀?덈떎
	//size_t stringLength = 0;
	//if (wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128) != 0)
	//{
	//	return false;
	//}

	//// ?붿뒪?뚮젅??紐⑤뱶 由ъ뒪?몃? ?댁젣?⑸땲??
	//delete[] displayModeList;
	//displayModeList = 0;

	//// 異쒕젰 ?대럞?곕? ?댁젣?⑸땲??
	//adapterOutput->Release();
	//adapterOutput = 0;

	//// ?대럞?곕? ?댁젣?⑸땲??
	//adapter->Release();
	//adapter = 0;

	//// ?⑺넗由?媛앹껜瑜??댁젣?⑸땲??
	//factory->Release();
	//factory = 0;

	//// ?ㅼ솑泥댁씤 援ъ“泥대? 珥덇린?뷀빀?덈떎
	//DXGI_SWAP_CHAIN_DESC swapChainDesc;
	//ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//// 諛깅쾭?쇰? 1媛쒕쭔 ?ъ슜?섎룄濡?吏?뺥빀?덈떎
	//swapChainDesc.BufferCount = 1;

	//// 諛깅쾭?쇱쓽 ?볦씠? ?믪씠瑜?吏?뺥빀?덈떎
	//swapChainDesc.BufferDesc.Width = screenWidth;
	//swapChainDesc.BufferDesc.Height = screenHeight;

	//// 32bit ?쒗럹?댁뒪瑜??ㅼ젙?⑸땲??
	//swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//// 諛깅쾭?쇱쓽 ?덈줈怨좎묠 鍮꾩쑉???ㅼ젙?⑸땲??
	//if (m_vsync_enabled)
	//{
	//	swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
	//	swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	//}
	//else
	//{
	//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//}

	//// 諛깅쾭?쇱쓽 ?ъ슜?⑸룄瑜?吏?뺥빀?덈떎
	//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//// ?쒕뜑留곸뿉 ?ъ슜???덈룄???몃뱾??吏?뺥빀?덈떎
	//swapChainDesc.OutputWindow = hwnd;

	//// 硫?곗깦?뚮쭅???뺣땲??
	//swapChainDesc.SampleDesc.Count = 1;
	//swapChainDesc.SampleDesc.Quality = 0;

	//// 李쎈え??or ??ㅽ겕由?紐⑤뱶瑜??ㅼ젙?⑸땲??
	//if (fullscreen)
	//{
	//	swapChainDesc.Windowed = false;
	//}
	//else
	//{
	//	swapChainDesc.Windowed = true;
	//}

	//// ?ㅼ틪 ?쇱씤 ?쒖꽌 諛??ш린瑜?吏?뺥븯吏 ?딆쓬?쇰줈 ?ㅼ젙?⑸땲??
	//swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//// 異쒕젰???ㅼ쓬 諛깅쾭?쇰? 鍮꾩슦?꾨줉 吏?뺥빀?덈떎
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//// 異붽? ?듭뀡 ?뚮옒洹몃? ?ъ슜?섏? ?딆뒿?덈떎
	//swapChainDesc.Flags = 0;

	//// ?쇱쿂?덈꺼??DirectX 11 濡??ㅼ젙?⑸땲??
	//D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	//// ?ㅼ솑 泥댁씤, Direct3D ?μ튂 諛?Direct3D ?μ튂 而⑦뀓?ㅽ듃瑜?留뚮벊?덈떎.
	//if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
	//	D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext)))
	//{
	//	return false;
	//}

	//// 諛깅쾭???ъ씤?곕? ?살뼱?듬땲??
	//ID3D11Texture2D* backBufferPtr = nullptr;
	//if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr)))
	//{
	//	return false;
	//}

	//// 諛?踰꾪띁 ?ъ씤?곕줈 ?뚮뜑 ?寃?酉곕? ?앹꽦?쒕떎.
	//if (FAILED(m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView)))
	//{
	//	return false;
	//}

	//// 諛깅쾭???ъ씤?곕? ?댁젣?⑸땲??
	//backBufferPtr->Release();
	//backBufferPtr = 0;

	//// 源딆씠 踰꾪띁 援ъ“泥대? 珥덇린?뷀빀?덈떎
	//D3D11_TEXTURE2D_DESC depthBufferDesc;
	//ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	//// 源딆씠 踰꾪띁 援ъ“泥대? ?묒꽦?⑸땲??
	//depthBufferDesc.Width = screenWidth;
	//depthBufferDesc.Height = screenHeight;
	//depthBufferDesc.MipLevels = 1;
	//depthBufferDesc.ArraySize = 1;
	//depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthBufferDesc.SampleDesc.Count = 1;
	//depthBufferDesc.SampleDesc.Quality = 0;
	//depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//depthBufferDesc.CPUAccessFlags = 0;
	//depthBufferDesc.MiscFlags = 0;

	//// ?ㅼ젙??源딆씠踰꾪띁 援ъ“泥대? ?ъ슜?섏뿬 源딆씠 踰꾪띁 ?띿뒪爾먮? ?앹꽦?⑸땲??
	//if (FAILED(m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer)))
	//{
	//	return false;
	//}

	//// ?ㅽ뀗???곹깭 援ъ“泥대? 珥덇린?뷀빀?덈떎
	//D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	//ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//// ?ㅽ뀗???곹깭 援ъ“泥대? ?묒꽦?⑸땲??
	//depthStencilDesc.DepthEnable = true;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	//depthStencilDesc.StencilEnable = true;
	//depthStencilDesc.StencilReadMask = 0xFF;
	//depthStencilDesc.StencilWriteMask = 0xFF;

	//// ?쎌? ?뺣㈃???ㅽ뀗???ㅼ젙?낅땲??
	//depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	//depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//// ?쎌? ?룸㈃???ㅽ뀗???ㅼ젙?낅땲??
	//depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	//depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//// 源딆씠 ?ㅽ뀗???곹깭瑜??앹꽦?⑸땲??
	//if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState)))
	//{
	//	return false;
	//}

	//// 源딆씠 ?ㅽ뀗???곹깭瑜??ㅼ젙?⑸땲??
	//m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	//// 源딆씠 ?ㅽ뀗??酉곗쓽 援ъ“泥대? 珥덇린?뷀빀?덈떎
	//D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	//ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//// 源딆씠 ?ㅽ뀗??酉?援ъ“泥대? ?ㅼ젙?⑸땲??
	//depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//depthStencilViewDesc.Texture2D.MipSlice = 0;

	//// 源딆씠 ?ㅽ뀗??酉곕? ?앹꽦?⑸땲??
	//if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView)))
	//{
	//	return false;
	//}

	//// ?뚮뜑留????酉곗? 源딆씠 ?ㅽ뀗??踰꾪띁瑜?異쒕젰 ?뚮뜑 ?뚯씠???쇱씤??諛붿씤?⑺빀?덈떎
	//m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//// 洹몃젮吏???대━怨ㅺ낵 諛⑸쾿??寃곗젙???섏뒪??援ъ“泥대? ?ㅼ젙?⑸땲??
	//D3D11_RASTERIZER_DESC rasterDesc;
	//rasterDesc.AntialiasedLineEnable = false;
	//rasterDesc.CullMode = D3D11_CULL_BACK;
	////rasterDesc.CullMode = D3D11_CULL_NONE;
	//rasterDesc.DepthBias = 0;
	//rasterDesc.DepthBiasClamp = 0.0f;
	//rasterDesc.DepthClipEnable = true;
	//rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.FrontCounterClockwise = false;
	////rasterDesc.FrontCounterClockwise = true;
	//rasterDesc.MultisampleEnable = false;
	//rasterDesc.ScissorEnable = false;
	//rasterDesc.SlopeScaledDepthBias = 0.0f;

	//// 諛⑷툑 ?묒꽦??援ъ“泥댁뿉???섏뒪???쇱씠? ?곹깭瑜?留뚮벊?덈떎
	//if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState)))
	//{
	//	return false;
	//}

	//// ?댁젣 ?섏뒪???쇱씠? ?곹깭瑜??ㅼ젙?⑸땲??
	//m_deviceContext->RSSetState(m_rasterState);

	//// ?뚮뜑留곸쓣 ?꾪빐 酉고룷?몃? ?ㅼ젙?⑸땲??
	//D3D11_VIEWPORT viewport;
	//viewport.Width = (float)screenWidth;
	//viewport.Height = (float)screenHeight;
	//viewport.MinDepth = 0.0f;
	//viewport.MaxDepth = 1.0f;
	//viewport.TopLeftX = 0.0f;
	//viewport.TopLeftY = 0.0f;

	//// 酉고룷?몃? ?앹꽦?⑸땲??
	//m_deviceContext->RSSetViewports(1, &viewport);

	//// ?ъ쁺 ?됰젹???ㅼ젙?⑸땲??
	//float fieldOfView = XM_PI / 4.0f;
	//float screenAspect = (float)screenWidth / (float)screenHeight;

	//// 3D ?뚮뜑留곸쓣?꾪븳 ?ъ쁺 ?됰젹??留뚮벊?덈떎
	//m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	//// ?멸퀎 ?됰젹????벑 ?됰젹濡?珥덇린?뷀빀?덈떎
	//m_worldMatrix = XMMatrixIdentity();

	//// 2D ?뚮뜑留곸쓣?꾪븳 吏곴탳 ?ъ쁺 ?됰젹??留뚮벊?덈떎
	//m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}


void D3DClass::Shutdown()
{
	//// 醫낅즺 ???덈룄??紐⑤뱶濡??ㅼ젙?섏? ?딆쑝硫??ㅼ솑 泥댁씤???댁젣 ?????덉쇅媛 諛쒖깮?⑸땲??
	//if (m_swapChain)
	//{
	//	m_swapChain->SetFullscreenState(false, NULL);
	//}

	//if (m_rasterState)
	//{
	//	m_rasterState->Release();
	//	m_rasterState = 0;
	//}

	//if (m_depthStencilView)
	//{
	//	m_depthStencilView->Release();
	//	m_depthStencilView = 0;
	//}

	//if (m_depthStencilState)
	//{
	//	m_depthStencilState->Release();
	//	m_depthStencilState = 0;
	//}

	//if (m_depthStencilBuffer)
	//{
	//	m_depthStencilBuffer->Release();
	//	m_depthStencilBuffer = 0;
	//}

	//if (m_renderTargetView)
	//{
	//	m_renderTargetView->Release();
	//	m_renderTargetView = 0;
	//}

	//if (m_deviceContext)
	//{
	//	m_deviceContext->Release();
	//	m_deviceContext = 0;
	//}

	//if (m_device)
	//{
	//	m_device->Release();
	//	m_device = 0;
	//}

	//if (m_swapChain)
	//{
	//	m_swapChain->Release();
	//	m_swapChain = 0;
	//}
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	//// 踰꾪띁瑜?吏???됱쓣 ?ㅼ젙?⑸땲??
	//float color[4] = { red, green, blue, alpha };

	//// 諛깅쾭?쇰? 吏?곷땲??
	//m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	//// 源딆씠 踰꾪띁瑜?吏?곷땲??
	//m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void D3DClass::EndScene()
{
	//// ?뚮뜑留곸씠 ?꾨즺?섏뿀?쇰?濡??붾㈃??諛?踰꾪띁瑜??쒖떆?⑸땲??
	//if (m_vsync_enabled)
	//{
	//	// ?붾㈃ ?덈줈 怨좎묠 鍮꾩쑉??怨좎젙?⑸땲??
	//	m_swapChain->Present(1, 0);
	//}
	//else
	//{
	//	// 媛?ν븳 鍮좊Ⅴ寃?異쒕젰?⑸땲??
	//	m_swapChain->Present(0, 0);
	//}
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
	//projectionMatrix = m_projectionMatrix;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	//worldMatrix = m_worldMatrix;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	//orthoMatrix = m_orthoMatrix;
}


void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	/*strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;*/
}