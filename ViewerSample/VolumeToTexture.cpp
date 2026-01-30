#include "VolumeToTexture.h"
#include <vector>
#include <fstream>
#include<iostream>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")



VolumeToTexture::VolumeToTexture()
	: m_width(0)
	, m_height(0)
	, m_initialized(false)
{
}

VolumeToTexture::~VolumeToTexture()
{
	Shutdown();
}

bool VolumeToTexture::Initialize(ID3D11Device* device, int width, int height)
{
	std::cout << "========================================" << std::endl;
	std::cout << "⭐ VolumeToTexture::Initialize() called" << std::endl;
	std::cout << "   Device: " << device << std::endl;
	std::cout << "   Size: " << width << "x" << height << std::endl;

	if (m_initialized)
		Shutdown();

	//// ⭐ Simple Quad 초기화
	//if (!InitSimpleQuad(device)) {
	//	std::cout << "❌ Failed to initialize Simple Quad" << std::endl;
	//	return false;
	//}


	m_width = width;
	m_height = height;

	// ========== Step 1: CreateRenderTarget ==========
	std::cout << ">> Step 1/5: CreateRenderTarget..." << std::endl;
	if (!CreateRenderTarget(device, width, height)) {
		std::cout << "CreateRenderTarget FAILED!" << std::endl;
		return false;
	}
	std::cout << "CreateRenderTarget succeeded\n" << std::endl;

	// ========== Step 2: CreateFullscreenQuad ==========
	std::cout << ">> Step 2/5: CreateFullscreenQuad..." << std::endl;
	if (!CreateFullscreenQuad(device)) {
		std::cout << "CreateFullscreenQuad FAILED!" << std::endl;
		return false;
	}
	std::cout << "CreateFullscreenQuad succeeded" << std::endl;
	std::cout << "   >> m_quadVertexBuffer: " << m_quadVertexBuffer << std::endl;

	// ⭐ 여기서 NULL 체크!
	if (!m_quadVertexBuffer) {
		std::cout << "CRITICAL: m_quadVertexBuffer is NULL after CreateFullscreenQuad!" << std::endl;
		return false;
	}
	std::cout << "   >> QuadVB verified: NOT NULL\n" << std::endl;

	if (!LoadShaders(device))
		return false;

	if (!CreateConstantBuffer(device))
		return false;

	if (!CreateSamplers(device))
		return false;


	m_initialized = true;
	return true;
}

void VolumeToTexture::Shutdown()
{
	//ReleaseRenderTarget();
	//ReleaseFullscreenQuad();
	//ReleaseShaders();
	//ReleaseConstantBuffer();
	//ReleaseSamplers();

	//m_initialized = false;
}

bool VolumeToTexture::CreateRenderTarget(ID3D11Device* device, int width, int height)
{
	// Texture2D 생성
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;  // HDR
	//texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_resultTexture);
	if (FAILED(hr))
		return false;

	// RTV 생성
	hr = device->CreateRenderTargetView(m_resultTexture, nullptr, &m_resultRTV);
	if (FAILED(hr))
		return false;

	// SRV 생성
	hr = device->CreateShaderResourceView(m_resultTexture, nullptr, &m_resultSRV);
	if (FAILED(hr))
		return false;

	return true;
}

void VolumeToTexture::ReleaseRenderTarget()
{
	//m_resultSRV.Reset();
	//m_resultRTV.Reset();
	//m_resultTexture.Reset();
}

bool VolumeToTexture::CreateFullscreenQuad(ID3D11Device* device)
{
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	// Fullscreen quad (NDC 좌표)
	Vertex vertices[] = {
		{ XMFLOAT3(-1.0f,  1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },  // 좌상
		{ XMFLOAT3(1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },  // 우상
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },  // 좌하
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },  // 우하
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &m_quadVertexBuffer);

	std::cout << "hr : " << hr << std::endl;

	return SUCCEEDED(hr);
}

void VolumeToTexture::ReleaseFullscreenQuad()
{
	//m_quadVertexBuffer->Reset();
	m_inputLayout.Reset();
}

//bool VolumeToTexture::LoadShaders(ID3D11Device* device)
//{
//	// 셰이더 파일 읽기 (또는 컴파일)
//	// 여기서는 이미 컴파일된 .cso 파일을 읽는다고 가정
//
//	// Vertex Shader
//	std::ifstream vsFile("SimpleQuad_VS.cso", std::ios::binary);
//	if (!vsFile.is_open())
//		return false;
//
//	std::vector<char> vsData((std::istreambuf_iterator<char>(vsFile)),
//		std::istreambuf_iterator<char>());
//	vsFile.close();
//
//	HRESULT hr = device->CreateVertexShader(vsData.data(), vsData.size(), nullptr,
//		m_vertexShader.GetAddressOf());
//	if (FAILED(hr))
//		return false;
//
//	// Input Layout
//	D3D11_INPUT_ELEMENT_DESC layout[] = {
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	};
//
//	hr = device->CreateInputLayout(layout, 2, vsData.data(), vsData.size(),
//		m_inputLayout.GetAddressOf());
//	if (FAILED(hr))
//		return false;
//
//	// Pixel Shader
//	std::ifstream psFile("SimpleQuad_PS.cso", std::ios::binary);
//	if (!psFile.is_open())
//		return false;
//
//	std::vector<char> psData((std::istreambuf_iterator<char>(psFile)),
//		std::istreambuf_iterator<char>());
//	psFile.close();
//
//	hr = device->CreatePixelShader(psData.data(), psData.size(), nullptr,
//		m_pixelShader.GetAddressOf());
//	return SUCCEEDED(hr);
//}



bool VolumeToTexture::LoadShaders(ID3D11Device* device)
{
	std::cout << "   >> LoadShaders() called" << std::endl;

	HRESULT hr;

	// ========== Vertex Shader ==========
	std::cout << "      Compiling SimpleQuad_VS.hlsl..." << std::endl;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3DCompileFromFile(
		L"SimpleQuad_VS.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			std::cout << "      ❌ VS compile error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
			errorBlob->Release();
		}
		else {
			std::cout << "      ❌ VS file not found or compile failed! HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
		}
		return false;
	}

	std::cout << "      ✅ VS compiled, size: " << vsBlob->GetBufferSize() << " bytes" << std::endl;

	// Vertex Shader 생성
	hr = device->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		nullptr,
		m_vertexShader.GetAddressOf()
	);

	if (FAILED(hr)) {
		std::cout << "      ❌ CreateVertexShader failed! HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
		vsBlob->Release();
		return false;
	}

	std::cout << "      ✅ VS created: " << m_vertexShader.Get() << std::endl;

	// ========== Input Layout ==========
	std::cout << "      Creating Input Layout..." << std::endl;

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(
		layout,
		2,
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		m_inputLayout.GetAddressOf()
	);

	vsBlob->Release();  // ⭐ 여기서 해제

	if (FAILED(hr)) {
		std::cout << "      ❌ CreateInputLayout failed! HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
		return false;
	}

	std::cout << "      ✅ Input Layout created: " << m_inputLayout.Get() << std::endl;

	// ========== Pixel Shader ==========
	std::cout << "      Compiling SimpleQuad_PS.hlsl..." << std::endl;

	ID3DBlob* psBlob = nullptr;

	hr = D3DCompileFromFile(
		L"SimpleQuad_PS.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			std::cout << "      ❌ PS compile error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
			errorBlob->Release();
		}
		else {
			std::cout << "      ❌ PS file not found or compile failed! HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
		}
		return false;
	}

	std::cout << "      ✅ PS compiled, size: " << psBlob->GetBufferSize() << " bytes" << std::endl;

	// Pixel Shader 생성
	hr = device->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		nullptr,
		m_pixelShader.GetAddressOf()
	);

	psBlob->Release();  // ⭐ 여기서 해제

	if (FAILED(hr)) {
		std::cout << "      ❌ CreatePixelShader failed! HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
		return false;
	}

	std::cout << "      ✅ PS created: " << m_pixelShader.Get() << std::endl;

	return true;
}

void VolumeToTexture::ReleaseShaders()
{
	m_pixelShader.Reset();
	m_vertexShader.Reset();
}

bool VolumeToTexture::CreateConstantBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(CBData);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.GetAddressOf());
	return SUCCEEDED(hr);
}

void VolumeToTexture::ReleaseConstantBuffer()
{
	//m_quadVertexBuffer.Reset();
}

bool VolumeToTexture::CreateSamplers(ID3D11Device* device)
{
	// Linear Sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = device->CreateSamplerState(&sampDesc, m_linearSampler.GetAddressOf());
	if (FAILED(hr))
		return false;

	// Point Clamp Sampler
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	hr = device->CreateSamplerState(&sampDesc, m_pointClampSampler.GetAddressOf());
	return SUCCEEDED(hr);
}

void VolumeToTexture::ReleaseSamplers()
{
	m_linearSampler.Reset();
	m_pointClampSampler.Reset();
}

// QDirect3D11Widget.cpp

//bool VolumeToTexture::InitSimpleQuad(ID3D11Device* device)
//{
//	// ========== 1. Vertex Buffer ==========
//	SimpleVertex vertices[] = {
//		{ XMFLOAT2(-1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },  // 좌상
//		{ XMFLOAT2(1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },  // 우상
//		{ XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },  // 좌하
//		{ XMFLOAT2(1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },  // 우하
//	};
//
//	D3D11_BUFFER_DESC bufferDesc = {};
//	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	bufferDesc.ByteWidth = sizeof(vertices);
//	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//	D3D11_SUBRESOURCE_DATA initData = {};
//	initData.pSysMem = vertices;
//
//
//	HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, m_quadVertexBuffer.GetAddressOf());
//	if (FAILED(hr)) {
//		std::cout << "❌ Failed to create simple quad vertex buffer" << std::endl;
//		return false;
//	}
//
//	// ========== 2. Shaders ==========
//	// Vertex Shader 컴파일
//	ID3DBlob* vsBlob = nullptr;
//	ID3DBlob* errorBlob = nullptr;
//
//	const char* vsCode = R"(
//        struct VSInput
//        {
//            float2 pos : POSITION;
//            float2 uv : TEXCOORD0;
//        };
//        
//        struct VSOutput
//        {
//            float4 pos : SV_POSITION;
//            float2 uv : TEXCOORD0;
//        };
//        
//        VSOutput main(VSInput input)
//        {
//            VSOutput output;
//            output.pos = float4(input.pos, 0, 1);
//            output.uv = input.uv;
//            return output;
//        }
//    )";
//
//	hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
//		"main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
//
//	if (FAILED(hr)) {
//		if (errorBlob) {
//			std::cout << "❌ VS Compile Error:" << (char*)errorBlob->GetBufferPointer() << std::endl;
//			errorBlob->Release();
//		}
//		return false;
//	}
//
//
//
//	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
//		nullptr, m_vertexShader.GetAddressOf());
//	if (FAILED(hr)) {
//		vsBlob->Release();
//		return false;
//	}
//
//	// ========== 3. Input Layout ==========
//	D3D11_INPUT_ELEMENT_DESC layout[] = {
//		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	};
//
//
//
//
//	hr = device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
//		vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
//	vsBlob->Release();
//
//	if (FAILED(hr)) {
//		std::cout << "❌ Failed to create input layout" << std::endl;
//		return false;
//	}
//
//	// ========== 4. Pixel Shader ==========
//	const char* psCode = R"(
//        Texture2D tex : register(t0);
//        SamplerState samp : register(s0);
//        
//        struct PSInput
//        {
//            float4 pos : SV_POSITION;
//            float2 uv : TEXCOORD0;
//        };
//        
//        float4 main(PSInput input) : SV_Target
//        {
//            return tex.Sample(samp, input.uv);
//        }
//    )";
//
//	ID3DBlob* psBlob = nullptr;
//	hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
//		"main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
//
//	if (FAILED(hr)) {
//		if (errorBlob) {
//			std::cout << "❌ PS Compile Error:" << (char*)errorBlob->GetBufferPointer() << std::endl;
//			errorBlob->Release();
//		}
//		return false;
//	}
//
//
//
//
//	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
//		nullptr, m_pixelShader.GetAddressOf());
//	psBlob->Release();
//
//	if (FAILED(hr)) {
//		std::cout << "❌ Failed to create pixel shader" << std::endl;
//		return false;
//	}
//
//	// ========== 5. Sampler State ==========
//	if (!m_linearSampler) {  // 이미 있으면 스킵
//		D3D11_SAMPLER_DESC sampDesc = {};
//		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
//		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
//		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
//		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
//
//		hr = device->CreateSamplerState(&sampDesc, m_linearSampler.GetAddressOf());
//		if (FAILED(hr)) {
//			std::cout << "❌ Failed to create sampler state" << std::endl;
//			return false;
//		}
//	}
//
//	std::cout << "✅ Simple Quad initialized" << std::endl << std::endl;
//	return true;
//}


//void QDirect3D11Widget::FullScreenPassSet()
//{
//
//	D3D11_VIEWPORT vp{};
//	vp.TopLeftX = 0;
//	vp.TopLeftY = 0;
//	vp.Width = static_cast<float>(width() / 2);
//	vp.Height = static_cast<float>(height() / 2);
//	vp.MinDepth = 0.0f;
//	vp.MaxDepth = 1.0f;
//	m_pDeviceContext->RSSetViewports(1, &vp);
//
//	// ✅ 1️⃣ 상수 버퍼 준비
//
//	D3D11_BUFFER_DESC cbd{};
//	cbd.ByteWidth = sizeof(CB);
//	cbd.Usage = D3D11_USAGE_DYNAMIC;
//	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	HRESULT hr = m_pDevice->CreateBuffer(&cbd, nullptr, &cbRay);
//	if (FAILED(hr)) {
//		OutputDebugStringA("❌ Failed to create constant buffer\n");
//		return;
//	}
//
//	// ✅ 2️⃣ 풀스크린 사각형 정점 (좌표 + UV)
//	Vtx quad[4] = {
//		{{-1.f, -1.f}, {0.f, 1.f}},
//		{{-1.f,  1.f}, {0.f, 0.f}},
//		{{ 1.f, -1.f}, {1.f, 1.f}},
//		{{ 1.f,  1.f}, {1.f, 0.f}},
//	};
//
//	// ✅ 3️⃣ 정점 버퍼 생성 (한 번만 만들면 좋지만, 지금은 함수 내에서도 OK)
//	if (!m_quadVB) {
//		D3D11_BUFFER_DESC vbd{};
//		vbd.ByteWidth = sizeof(quad);
//		vbd.Usage = D3D11_USAGE_DEFAULT;
//		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//		D3D11_SUBRESOURCE_DATA initVB{};
//		initVB.pSysMem = quad;
//
//		HRESULT hrVB = m_pDevice->CreateBuffer(&vbd, &initVB, &m_quadVB);
//		if (FAILED(hrVB)) {
//			OutputDebugStringA("❌ Failed to create fullscreen quad vertex buffer\n");
//			return;
//		}
//	}
//
//	// ✅ 6️⃣ 상수 버퍼 데이터 채우기
//
//	////cb.View = XMMatrixTranspose(v);
//	////cb.Proj = XMMatrixTranspose(p);
//	//cb.InvView = XMMatrixTranspose(iv);
//	//cb.InvProj = XMMatrixTranspose(ip);
//	////cb.VolumeWorld = XMMatrixTranspose(w);
//	//cb.InvVolumeWorld = XMMatrixTranspose(iw);
//
//
//	m_pDeviceContext->OMSetDepthStencilState(m_VolumeDepthState.Get(), 0);
//
//
//	XMStoreFloat4x4(&cb.InvView, XMMatrixTranspose(invViewMat));
//	XMStoreFloat4x4(&cb.InvProj, XMMatrixTranspose(invProjMat));
//	XMStoreFloat4x4(&cb.InvVolumeWorld, XMMatrixTranspose(invWorldMat));
//
//	//// 디버깅용
//	//XMMATRIX i= XMMatrixMultiply(projMat, invProjMat);
//
//	//XMFLOAT4X4 I;
//	//XMStoreFloat4x4(&I, i);
//	//std::cout <<
//	//	 I._11 << " " << I._12 << " " << I._13 << " " << I._14 << "\n" <<
//	//	I._21 << " " << I._22 << " " << I._23 << " " << I._24 << "\n" <<
//	//	I._31 << " " << I._32 << " " << I._33 << " " << I._34 << "\n" <<
//	//	I._41 << " " << I._42 << " " << I._43 << " " << I._44 << "\n\n";
//
//
//
//
//	//// ✅ 실제 카메라 위치 사용
//	//cb.CameraPosWS = XMFLOAT3(
//	//	XMVectorGetX(eye),
//	//	XMVectorGetY(up),
//	//	XMVectorGetZ(at)
//	//);
//
//	XMStoreFloat4x4(&cb.View, XMMatrixTranspose(viewMat));
//	XMStoreFloat4x4(&cb.Projection, XMMatrixTranspose(projMat));
//
//
//	cb.CameraPosAndAlpha.x = XMVectorGetX(eye);
//	cb.CameraPosAndAlpha.y = XMVectorGetY(eye);
//	cb.CameraPosAndAlpha.z = XMVectorGetZ(eye);
//
//
//	//// ✅ 권장값
//	//cb.MaxSteps = 256;  // 또는 128~512 사이
//	////cb.MaxSteps =1536;  // 또는 128~512 사이
//
//	//cb.Voxel = XMFLOAT3(fileReader->m_width, fileReader->m_height, fileReader->m_depth);
//
//	cb.VoxelAndMaxSteps.x = fileReader->m_width;
//	cb.VoxelAndMaxSteps.y = fileReader->m_height;
//	cb.VoxelAndMaxSteps.z = fileReader->m_depth;
//	cb.VoxelAndMaxSteps.w = 256.f;
//
//	cb.HuParams.x = fileReader->m_rescaleSlope;
//	cb.HuParams.y = fileReader->m_rescaleIntercept;
//	cb.HuParams.z = fileReader->volWC - fileReader->volWW / 2.0;
//	cb.HuParams.w = fileReader->volWC + fileReader->volWW / 2.0;
//
//
//	D3D11_MAPPED_SUBRESOURCE mapped{};
//	m_pDeviceContext->Map(cbRay.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
//	memcpy(mapped.pData, &cb, sizeof(cb));
//	m_pDeviceContext->Unmap(cbRay.Get(), 0);
//
//	// ✅ 7️⃣ 파이프라인 세팅
//	UINT stride = sizeof(Vtx);
//	UINT offset = 0;
//	ID3D11Buffer* vb[] = { m_quadVB.Get() };
//
//	m_pDeviceContext->IASetVertexBuffers(0, 1, vb, &stride, &offset);
//	m_pDeviceContext->IASetInputLayout(layoutQuad);
//	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	m_pDeviceContext->VSSetShader(vsFullscreen, nullptr, 0);
//	m_pDeviceContext->PSSetShader(psRaymarch, nullptr, 0);
//
//	ID3D11Buffer* cbs[] = { cbRay.Get() };
//	m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs);
//	m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);
//
//
//	// ⭐ 텍스처 바인딩
//	ID3D11ShaderResourceView* srvs[2] = {
//		 m_volumeSRV.Get(),                          // t0
//		m_transferFunction->GetSRV()          // t1
//	};
//	m_pDeviceContext->PSSetShaderResources(0, 2, srvs);
//
//
//	// ⭐ Sampler 바인딩
//	ID3D11SamplerState* samplers[2] = {
//		m_volumeSampler.Get(),    // s0
//		m_tfSampler         // s1
//	};
//	m_pDeviceContext->PSSetSamplers(0, 2, samplers);
//
//
//	// ✅ 8️⃣ 드로우
//	m_pDeviceContext->Draw(4, 0);
//}




void VolumeToTexture::RenderVolumeToTexture(
	ID3D11DeviceContext* context,
	ID3D11ShaderResourceView* volumeSRV,
	ID3D11ShaderResourceView* transferFunctionSRV,
	ID3D11ShaderResourceView* sceneDepthSRV,
	const XMMATRIX& invView,
	const XMMATRIX& invProj,
	const XMMATRIX& invVolumeWorld,
	const XMMATRIX& view,
	const XMMATRIX& projection,
	const XMFLOAT3& cameraPos,
	float renderMode,
	const XMFLOAT3& voxelDim,
	float maxSteps,
	float huMin,
	float huMax
	,
	ID3D11VertexShader* vs, ID3D11PixelShader* ps,
	ID3D11InputLayout* layout, ID3D11Buffer* cb
	, UINT stride, UINT offset, ID3D11Buffer* m_quadVertexBuffer)
{
	if (!m_initialized)
		return;


	////// DrawTextureToScreen() 시작에 추가
	//std::cout << "✅ RenderVolumeToTexture called" << std::endl;
	//////	std::cout << "   SRV: " << srv << std::endl;

	////if (nullptr == m_quadVertexBuffer)
	//	std::cout << "   QuadVB: " << m_quadVertexBuffer<< std::endl;
	////std::cout << "   VS: " << m_vertexShader.Get() << std::endl;
	////std::cout << "   PS: " << m_pixelShader.Get() << std::endl;

	// Constant Buffer 업데이트
	D3D11_MAPPED_SUBRESOURCE mapped;
	//HRESULT hr = context->Map(m_quadVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	HRESULT hr = context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		CBData* cbData = static_cast<CBData*>(mapped.pData);
		cbData->InvView = XMMatrixTranspose(invView);
		cbData->InvProj = XMMatrixTranspose(invProj);
		cbData->InvVolumeWorld = XMMatrixTranspose(invVolumeWorld);
		cbData->View = XMMatrixTranspose(view);
		cbData->Projection = XMMatrixTranspose(projection);
		cbData->CameraPosAndAlpha = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, renderMode);
		cbData->VoxelAndMaxSteps = XMFLOAT4(voxelDim.x, voxelDim.y, voxelDim.z, maxSteps);
		cbData->HuParams = XMFLOAT4(0, 0, huMin, huMax);

		context->Unmap(m_constantBuffer.Get(), 0);
	}

	// 백업 상태
	ComPtr<ID3D11RenderTargetView> oldRTV;
	ComPtr<ID3D11DepthStencilView> oldDSV;
	context->OMGetRenderTargets(1, oldRTV.GetAddressOf(), oldDSV.GetAddressOf());

	D3D11_VIEWPORT oldViewport;
	UINT numViewports = 1;
	context->RSGetViewports(&numViewports, &oldViewport);

	// Render Target 설정
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(m_resultRTV, clearColor);
	context->OMSetRenderTargets(1, &m_resultRTV, nullptr);

	// Viewport 설정
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(m_width);
	viewport.Height = static_cast<float>(m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// 셰이더 설정
	context->IASetInputLayout(layout);
	context->VSSetShader(vs, nullptr, 0);
	context->PSSetShader(ps, nullptr, 0);


	// Constant Buffer
	context->PSSetConstantBuffers(0, 1, &cb);

	// Textures
	ID3D11ShaderResourceView* srvs[] = { volumeSRV, transferFunctionSRV, nullptr, nullptr, nullptr, sceneDepthSRV };
	context->PSSetShaderResources(0, 6, srvs);

	// Samplers
	ID3D11SamplerState* samplers[] = { m_linearSampler.Get(), m_linearSampler.Get(), nullptr, nullptr, nullptr, m_pointClampSampler.Get() };
	context->PSSetSamplers(0, 6, samplers);

	// Draw Fullscreen Quad
	//UINT stride = sizeof(float) * 5;  // pos(3) + uv(2)
	//UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_quadVertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->Draw(4, 0);

	//	// DrawTextureToScreen() 시작에 추가
	//	std::cout << "✅ RenderVolumeToTexture called" << std::endl;
	////	std::cout << "   SRV: " << srv << std::endl;
	//
	//	if(nullptr== m_quadVertexBuffer.Get())
	//		std::cout << "   QuadVB: " << m_quadVertexBuffer.Get() << std::endl;
	//	//std::cout << "   VS: " << m_vertexShader.Get() << std::endl;
	//	//std::cout << "   PS: " << m_pixelShader.Get() << std::endl;


		// 상태 복원
	context->OMSetRenderTargets(1, oldRTV.GetAddressOf(), oldDSV.Get());
	context->RSSetViewports(1, &oldViewport);

	// Unbind SRVs
	ID3D11ShaderResourceView* nullSRVs[6] = { nullptr };
	context->PSSetShaderResources(0, 6, nullSRVs);



	//// RenderVolumeView() 끝에 추가
	//std::cout << "✅ VolumeToTexture completed" << std::endl;
	//std::cout << "   Result SRV: " <<GetResultSRV() << std::endl;


}


void VolumeToTexture::DrawTextureToScreen(ID3D11Device* device, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context
	, ID3D11VertexShader* vs, ID3D11PixelShader* ps)
{
	//if (!m_quadVertexBuffer.Get()) {
	//	std::cout << "✅ DrawTextureToScreen called" << std::endl;
	//	std::cout << "   QuadVB: " << m_quadVertexBuffer.Get() << std::endl;
	//	return;
	//}


	////// DrawTextureToScreen() 시작에 추가
	//std::cout << "✅ DrawTextureToScreen called" << std::endl;
	////std::cout << "   SRV: " << srv << std::endl;
	//std::cout << "   QuadVB: " << m_quadVertexBuffer << std::endl;
	//std::cout << "SRV ptr = " << srv << std::endl;
	////std::cout << "   VS: " << m_vertexShader.Get() << std::endl;
	////std::cout << "   PS: " << m_pixelShader.Get() << std::endl;


					//			//// ✅ 4. SRV 언바인딩 (중요!)
//			//ID3D11ShaderResourceView* nullSRV = nullptr;
//			//m_pDeviceContext->PSSetShaderResources(5, 1, &nullSRV);
//


	//ID3D11RenderTargetView* nullRTV = nullptr;
	//context->OMSetRenderTargets(1, &nullRTV, nullptr);
	//device->CreateShaderResourceView(
	//	m_resultTexture,
	//	nullptr,
	//	&m_resultSRV
	//);


	// Simple fullscreen quad shader (텍스처 그대로 출력)
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(float) * 5;  // pos(3) + uv(2) = 5 floats = 20 bytes
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_quadVertexBuffer, &stride, &offset);



	//// Shaders
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//context->VSSetShader(vs, nullptr, 0);
	//context->PSSetShader(ps, nullptr, 0);

	context->PSSetShaderResources(0, 1, &srv);
	context->PSSetSamplers(0, 1, &m_linearSampler);

	context->Draw(4, 0);

	// Unbind
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(0, 1, &nullSRV);
}

void VolumeToTexture::Resize(ID3D11Device* device, int width, int height)
{
	if (m_width == width && m_height == height)
		return;

	m_width = width;
	m_height = height;

	ReleaseRenderTarget();
	CreateRenderTarget(device, width, height);
}