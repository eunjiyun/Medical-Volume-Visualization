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
	// Direct3D 媛앹껜 ?앹꽦
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D 媛앹껜 珥덇린??
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

	// m_Camera 媛앹껜 ?앹꽦
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// 移대찓???ъ????ㅼ젙
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// m_Model 媛앹껜 ?앹꽦
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//// m_Model 媛앹껜 珥덇린??
	//if (!m_Model->Initialize(m_Direct3D->qtD3dWidget->m_pDevice))
	//{
	//	//250922
	//	//MessageBox(hwnd, (LPCSTR)L"Could not initialize the model object.", (LPCSTR)L"Error", MB_OK);
	//	MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
	//	return false;
	//}

	// m_ColorShader 媛앹껜 ?앹꽦
	m_ColorShader = new ColorShader;
	if (!m_ColorShader)
	{
		return false;
	}

	// m_ColorShader 媛앹껜 珥덇린??
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
	// m_ColorShader 媛앹껜 諛섑솚
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	// m_Model 媛앹껜 諛섑솚
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// m_Camera 媛앹껜 諛섑솚
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Direct3D 媛앹껜 諛섑솚
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}


bool GraphicsClass::Frame()
{
	// 洹몃옒???쒕뜑留?泥섎━
	return Render();
}


bool GraphicsClass::Render()
{
	// ?ъ쓣 洹몃━湲??꾪빐 踰꾪띁瑜?吏?곷땲??
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 移대찓?쇱쓽 ?꾩튂???곕씪 酉??됰젹???앹꽦?⑸땲??
	m_Camera->Render();

	// 移대찓??諛?d3d 媛앹껜?먯꽌 ?붾뱶, 酉?諛??ъ쁺 ?됰젹??媛?몄샃?덈떎
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 紐⑤뜽 踰꾪뀓?ㅼ? ?몃뜳??踰꾪띁瑜?洹몃옒???뚯씠???쇱씤??諛곗튂?섏뿬 ?쒕줈?됱쓣 以鍮꾪빀?덈떎.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// ?됱긽 ?먯씠?붾? ?ъ슜?섏뿬 紐⑤뜽???뚮뜑留곹빀?덈떎.
	if (!m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
	{
		return false;
	}

	// 踰꾪띁???댁슜???붾㈃??異쒕젰?⑸땲??
	m_Direct3D->EndScene();

	return true;
}