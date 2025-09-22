#include "stdafx.h"
#include "inputclass.h"
#include "graphicsclass.h"
#include "systemclass.h"
#include "QDirect3D11Widget.h"
#include "D3DClass.h"



SystemClass::SystemClass()
{
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	// ??덈즲??筌?揶쎛嚥? ?紐껋쨮 ?蹂?뵠 癰궰???λ뜃由??
	int screenWidth = 0;
	int screenHeight = 0;

	// ??덈즲????밴쉐 ?λ뜃由??
	InitializeWindows(screenWidth, screenHeight);

	// m_Input 揶쏆빘猿???밴쉐. ???????삳뮉 ?곕???????癒?벥 ??삳궖????낆젾 筌ｌ꼶????????몃빍??
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}

	// m_Input 揶쏆빘猿??λ뜃由??
	m_Input->Initialize();

	// m_Graphics 揶쏆빘猿???밴쉐.  域밸챶?????뺣쐭筌띻낯??筌ｌ꼶???띾┛ ?袁る립 揶쏆빘猿??낅빍??
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	// m_Graphics 揶쏆빘猿??λ뜃由??
	return m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
}


void SystemClass::Shutdown()
{
	// m_Graphics 揶쏆빘猿?獄쏆꼹??
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// m_Input 揶쏆빘猿?獄쏆꼹??
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// Window ?ル굝利?筌ｌ꼶??
	ShutdownWindows();
}


void SystemClass::Run()
{
	// 筌롫뗄?놅쭪? ?닌듼쒙㎗???밴쉐 獄??λ뜃由??
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// ????癒?쨮?봔???ル굝利?筌롫뗄?놅쭪???獄쏆룇????돱筌왖 筌롫뗄?놅쭪??룐뫂遊썹몴??類ｋ빍??
	while (true)
	{
		// ??덈즲??筌롫뗄?놅쭪???筌ｌ꼶???몃빍??
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// ?ル굝利?筌롫뗄?놅쭪???獄쏆룇??野껋럩??筌롫뗄?놅쭪? ?룐뫂遊썹몴???됲뀱??몃빍??
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// 域??紐꾨퓠??Frame ??λ땾??筌ｌ꼶???몃빍??
			if (!Frame())
				break;
		}
	}
}


bool SystemClass::Frame()
{
	// ESC ??揶쏅Ŋ? 獄??ル굝利??????筌ｌ꼶???몃빍??
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// 域밸챶???揶쏆빘猿??Frame??筌ｌ꼶???몃빍??
	return m_Graphics->Frame();
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// ??삳궖??? ???쑎鈺곕슢?쀥첎? 筌ｌ꼶??
	case WM_KEYDOWN:
	{
		// ?????뵝 flag??m_Input 揶쏆빘猿??筌ｌ꼶???롫즲嚥???몃빍??
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}

	// ??삳궖??? ??λ선鈺곕슢?쀥첎? 筌ｌ꼶??
	case WM_KEYUP:
	{
		// ????곸젫 flag??m_Input 揶쏆빘猿??筌ｌ꼶???롫즲嚥???몃빍??
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	// 域??紐꾩벥 筌뤴뫀諭?筌롫뗄?놅쭪???? 疫꿸퀡??筌롫뗄?놅쭪? 筌ｌ꼶?곫에???랁돥??덈뼄.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	
	// ?紐? ????怨? ??揶쏆빘猿쒏에?筌왖?類λ???덈뼄
	ApplicationHandle = this;

	// ???袁⑥쨮域밸챶????紐꾨뮞??곷뮞??揶쎛?紐꾩긿??덈뼄
	m_hinstance = GetModuleHandle(NULL);

	// ?袁⑥쨮域밸챶????已??筌왖?類λ???덈뼄
	m_applicationName = L"Dx11Demo_04";

	// windows ?????? ?袁⑥삋?? 揶쏆늿????쇱젟??몃빍??
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	//250922
	//wc.lpszClassName = (LPCSTR)m_applicationName;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// windows class???源낆쨯??몃빍??
	RegisterClassEx(&wc);

	// 筌뤴뫀????遺얇늺????곴맒?袁? ??뚮선??щ빍??
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = 0;
	int posY = 0;

	// FULL_SCREEN 癰궰??揶쏅?肉??怨뺤뵬 ?遺얇늺????쇱젟??몃빍??
	if (FULL_SCREEN)
	{
		// ????쎄쾿??筌뤴뫀諭뜻에?筌왖?類λ뻥??삠늺 筌뤴뫀????遺얇늺 ??곴맒?袁? ?怨쀫뮞??????곴맒?袁⑥쨮 筌왖?類λ릭????깃맒??32bit嚥?筌왖?類λ???덈뼄.
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// ????쎄쾿?깃퀣?앮에??遺용뮞???쟿????쇱젟??癰궰野껋?鍮??덈뼄.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}
	else
	{
		// ??덈즲??筌뤴뫀諭??野껋럩??800 * 600 ??由곁몴?筌왖?類λ???덈뼄.
		screenWidth = 800;
		screenHeight = 600;

		// ??덈즲??筌≪럩??揶쎛嚥? ?紐껋쨮????揶쎛??????삳즲嚥???몃빍??
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	////250922
	//// ??덈즲?怨? ??밴쉐??랁??紐껊굶???닌뗫???덈뼄.
	///*m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, (LPCSTR)m_applicationName, (LPCSTR)m_applicationName,
	//	WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
	//	posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);*/
	//m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName,m_applicationName,
	//	WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
	//	posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);


//	qtD3dWidget->showEvent();
	//m_hwnd = reinterpret_cast<HWND>(m_Graphics->m_Direct3D->qtD3dWidget->nativeHandle());




	//m_hwnd = reinterpret_cast<HWND>(qtD3dWidget->nativeHandle());





	//m_hwnd = reinterpret_cast<HWND>(m_pScene->winId());
	//m_hwnd = (HWND)(m_Graphics->m_Direct3D->qtD3dWidget->winId());

	// ??덈즲?怨? ?遺얇늺????뽯뻻??랁???鍮??? 筌왖?類λ???덈뼄
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
}


void SystemClass::ShutdownWindows()
{
	// ????쎄쾿??筌뤴뫀諭????삠늺 ?遺용뮞???쟿????쇱젟???λ뜃由?酉鍮??덈뼄.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// 筌≪럩????볤탢??몃빍??
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// ?袁⑥쨮域밸챶???紐꾨뮞??곷뮞????볤탢??몃빍??
	//250922
	//UnregisterClass((LPCSTR)m_applicationName, m_hinstance);
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// ?紐??????筌〓챷?쒐몴??λ뜃由?酉鍮??덈뼄
	ApplicationHandle = NULL;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// ??덈즲???ル굝利븀몴??類ㅼ뵥??몃빍??
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// ??덈즲?怨? ???뿳?遺? ?類ㅼ뵥??몃빍??
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// 域??紐꾩벥 筌뤴뫀諭?筌롫뗄?놅쭪???? ??뽯뮞???????쇱벥 筌롫뗄?놅쭪? 筌ｌ꼶?곫에???랁돥??덈뼄.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}