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
	// Direct3D ?띠룇鍮섊뙼???諛댁뎽
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D ?띠룇鍮섊뙼??貫?껆뵳??
	//250922
	if(!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, (LPCSTR)L"Could not initialize Direct3D.", (LPCSTR)L"Error", MB_OK);
		return false;
	}
	/*if (!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}*/
	//11111111111111111
	//m_Direct3D->qtD3dWidget->deviceInitialized();

	// m_Camera ?띠룇鍮섊뙼???諛댁뎽
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// ?곸궠?筌??????????깆젧
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// m_Model ?띠룇鍮섊뙼???諛댁뎽
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//// m_Model ?띠룇鍮섊뙼??貫?껆뵳??
	//if (!m_Model->Initialize(m_Direct3D->qtD3dWidget->m_pDevice))
	//{
	//	//250922
	//	//MessageBox(hwnd, (LPCSTR)L"Could not initialize the model object.", (LPCSTR)L"Error", MB_OK);
	//	MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
	//	return false;
	//}

	// m_ColorShader ?띠룇鍮섊뙼???諛댁뎽
	m_ColorShader = new ColorShader;
	if (!m_ColorShader)
	{
		return false;
	}

	// m_ColorShader ?띠룇鍮섊뙼??貫?껆뵳??
	//250922
	if (!m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, (LPCSTR)L"Could not initialize the color shader object.", (LPCSTR)L"Error", MB_OK);
		return false;
	}
	/*if (!m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}*/

	return true;
}


void GraphicsClass::Shutdown()
{
	// m_ColorShader ?띠룇鍮섊뙼??꾩룇瑗??
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	// m_Model ?띠룇鍮섊뙼??꾩룇瑗??
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// m_Camera ?띠룇鍮섊뙼??꾩룇瑗??
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Direct3D ?띠룇鍮섊뙼??꾩룇瑗??
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}


bool GraphicsClass::Frame()
{
	// ?잙갭梨?????類ｌ맠嶺?嶺뚳퐣瑗??
	return Render();
}


bool GraphicsClass::Render()
{
	// ?????잙갭梨?怨ルЬ??熬곥굥???뺢퀗??怨?ご?嶺뚯솘???⑤８鍮??
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// ?곸궠?筌??源녿꺄 ?熬곣뫚?????⑤벡逾?????怨쀬죯????諛댁뎽??紐껊퉵??
	m_Camera->Render();

	// ?곸궠?筌????d3d ?띠룇鍮섊뙼???????븐뼔援? ??????????怨쀬죯???띠럾??筌뤾쑴湲???덈펲
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 嶺뚮ㅄ維???뺢퀗????? ?筌뤾퍓????뺢퀗??怨?ご??잙갭梨??????逾????源녿데???꾩룄????琉우뿰 ??類ㅼŦ??源녿굵 繞벿뮻???들뜮????덈펲.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// ??源껊쭜 ???逾??? ?????琉우뿰 嶺뚮ㅄ維????????춯?삳궞?????덈펲.
	if (!m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
	{
		return false;
	}

	// ?뺢퀗??????怨몃뮔????븐뻼????怨쀫츊???紐껊퉵??
	m_Direct3D->EndScene();

	return true;
}