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
	// Direct3D ?¶ì†ë¹˜çŒ¿???ë°´ì‰
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D ?¶ì†ë¹˜çŒ¿??Î»?ƒç”±??
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

	// m_Camera ?¶ì†ë¹˜çŒ¿???ë°´ì‰
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// ?ì‚³?ï§??????????±ì Ÿ
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// m_Model ?¶ì†ë¹˜çŒ¿???ë°´ì‰
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//// m_Model ?¶ì†ë¹˜çŒ¿??Î»?ƒç”±??
	//if (!m_Model->Initialize(m_Direct3D->qtD3dWidget->m_pDevice))
	//{
	//	//250922
	//	//MessageBox(hwnd, (LPCSTR)L"Could not initialize the model object.", (LPCSTR)L"Error", MB_OK);
	//	MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
	//	return false;
	//}

	// m_ColorShader ?¶ì†ë¹˜çŒ¿???ë°´ì‰
	m_ColorShader = new ColorShader;
	if (!m_ColorShader)
	{
		return false;
	}

	// m_ColorShader ?¶ì†ë¹˜çŒ¿??Î»?ƒç”±??
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
	// m_ColorShader ?¶ì†ë¹˜çŒ¿??„ì†ê¼??
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	// m_Model ?¶ì†ë¹˜çŒ¿??„ì†ê¼??
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// m_Camera ?¶ì†ë¹˜çŒ¿??„ì†ê¼??
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Direct3D ?¶ì†ë¹˜çŒ¿??„ì†ê¼??
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}


bool GraphicsClass::Frame()
{
	// ?Ÿë°¸ì±?????ëº£ì­ç­?ç­Œï½Œê¼??
	return Render();
}


bool GraphicsClass::Render()
{
	// ?????Ÿë°¸ì±?ê³«ë¬¾??è¢ã‚‹???•ê³Œ??ê³?ª´?ç­Œì™–???¨ë£¸ë¹??
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// ?ì‚³?ï§??ê¹†ë²¥ ?è¢â‘º?????¨ëº¤ëµ?????ê³—ì¡Š????ë°´ì‰??ëªƒë¹??
	m_Camera->Render();

	// ?ì‚³?ï§????d3d ?¶ì†ë¹˜çŒ¿???????ºì–œêµ? ??????????ê³—ì¡Š???¶ì›??ï§ê¾©ê¸???ˆë¼„
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// ç­Œë¤´ë«€???•ê³Œ????? ?ï§ê»Š????•ê³Œ??ê³?ª´??Ÿë°¸ì±??????ëµ????ê¹†ëµ¥???„ì„????ë¤¿ì—° ??ëº¤ì¨®??ê¹†ë±½ é¤“Î’Â€???µé®????ˆë¼„.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// ??ê¹ƒë§’ ???ëµ??? ?????ë¤¿ì—° ç­Œë¤´ë«€????????­Œ?»ë‚±??€???ˆë¼„.
	if (!m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
	{
		return false;
	}

	// ?•ê³Œ??????ê³¸ë’ ????ºì–‡????ê³—ë®†???ëªƒë¹??
	m_Direct3D->EndScene();

	return true;
}