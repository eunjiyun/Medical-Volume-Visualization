#include "stdafx.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshader.h"
#include "graphicsclass.h"
#include "QDirect3D11Widget.h"


GraphicsClass::GraphicsClass()
{
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	// Direct3D ??좊즵??꼯????獄쏅똻??
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D ??좊즵??꼯???縕?猿녿뎨??
	//250922
	/*if(!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, (LPCSTR)L"Could not initialize Direct3D.", (LPCSTR)L"Error", MB_OK);
		return false;
	}*/
	if (!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	//11111111111111111
	//m_Direct3D->qtD3dWidget->deviceInitialized();

	// m_Camera ??좊즵??꼯????獄쏅똻??
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// ?怨멸텭?嶺??????????源놁젳
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// m_Model ??좊즵??꼯????獄쏅똻??
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//// m_Model ??좊즵??꼯???縕?猿녿뎨??
	//if (!m_Model->Initialize(m_Direct3D->qtD3dWidget->m_pDevice))
	//{
	//	//250922
	//	//MessageBox(hwnd, (LPCSTR)L"Could not initialize the model object.", (LPCSTR)L"Error", MB_OK);
	//	MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
	//	return false;
	//}

	// m_ColorShader ??좊즵??꼯????獄쏅똻??
	m_ColorShader = new ColorShader;
	if (!m_ColorShader)
	{
		return false;
	}

	// m_ColorShader ??좊즵??꼯???縕?猿녿뎨??
	//250922
	/*if (!m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, (LPCSTR)L"Could not initialize the color shader object.", (LPCSTR)L"Error", MB_OK);
		return false;
	}*/
	if (!m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// m_ColorShader ??좊즵??꼯???袁⑸즵???
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	// m_Model ??좊즵??꼯???袁⑸즵???
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// m_Camera ??좊즵??꼯???袁⑸즵???
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Direct3D ??좊즵??꼯???袁⑸즵???
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}


bool GraphicsClass::Frame()
{
	// ??숆강筌?????筌먲퐣留좑┼?癲ル슪?ｇ몭??
	return Render();
}


bool GraphicsClass::Render()
{
	// ??????숆강筌??ⓦ꺂糾???ш낄援???類???????癲ル슣?????ㅿ폍???
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// ?怨멸텭?嶺??繹먮끏爰???ш끽維??????ㅻ깹???????⑥ъ／????獄쏅똻???筌뤾퍓???
	m_Camera->Render();

	// ?怨멸텭?嶺????d3d ??좊즵??꼯????????釉먮폇?? ???????????⑥ъ／????좊읈??嶺뚮ㅎ?닸묾????덊렡
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 癲ル슢?꾤땟????類?????? ?嶺뚮ㅎ?????類?????????숆강筌???????????繹먮끏????袁⑸즲????筌뚯슦肉???筌먦끉큔??繹먮끏援?濚욌꼬裕뼘????ㅻ쑏?????덊렡.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// ??繹먭퍓彛???????? ?????筌뚯슦肉?癲ル슢?꾤땟?????????異??녠텪???????덊렡.
	if (!m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
	{
		return false;
	}

	// ?類????????⑤챶裕????釉먮뻤?????⑥レ툓???筌뤾퍓???
	m_Direct3D->EndScene();

	return true;
}