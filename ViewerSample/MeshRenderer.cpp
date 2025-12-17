#include<iostream>
#include "MeshRenderer.h"
#include "PLYLoader.h"

void MeshRenderer::CreateTwoPassStates(ID3D11Device* device)
{
	HRESULT hr;

	// ✅ Description 구조체: 지역 변수로 선언
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	D3D11_BLEND_DESC blendDesc = {};

	// ========== Rasterizer ==========
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.SlopeScaledDepthBias = 0.0f;
	hr=device->CreateRasterizerState(&rastDesc, &rastState);


	// ✅ 디버그 추가!
	if (FAILED(hr)) {
		std::cout << "❌ Failed to create rasterizer state!" << std::endl;
	}
	else {
		std::cout << "✅ Rasterizer state created:" << rastState << std::endl;
	}




	// Depth State: Write ON
	//D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // ZWrite On
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;


	device->CreateDepthStencilState(&depthDesc, &depthWriteState);

	// ✅ 디버그 추가!
	if (FAILED(hr)) {
		std::cout << "❌ Failed to create depthWriteState!" << std::endl;
	}
	else {
		std::cout << "✅ depthWriteState created:" << depthWriteState << std::endl;
	}



	// Blend State: Color Write OFF

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;  // ColorMask 0


	hr=device->CreateBlendState(&blendDesc, &noColorWriteState);


	// ✅ 디버그 추가!
	if (FAILED(hr)) {
		std::cout << "❌ Failed to create noColorWriteState!" << std::endl;
	}
	else {
		std::cout << "✅ noColorWriteState created:" << noColorWriteState << std::endl;
	}



	// ==========================================
	// ======= PASS 2: Transparent Render =======
	// ==========================================

	// Depth State: Write OFF, Test ON
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // ZWrite Off
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;  // ✅ LESS_EQUAL!
	hr=device->CreateDepthStencilState(&depthDesc, &depthReadState);

	// ✅ 디버그 추가!
	if (FAILED(hr)) {
		std::cout << "❌ Failed to create depthReadState!" << std::endl;
	}
	else {
		std::cout << "✅ depthReadState created:" << depthReadState << std::endl;
	}


	// Blend State: Alpha Blending
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;



	hr = device->CreateBlendState(&blendDesc, &alphaBlendState);

	// ✅ 디버그 추가!
	if (FAILED(hr)) {
		std::cout << "❌ Failed to create alphaBlendState!" << std::endl;
	}
	else {
		std::cout << "✅ alphaBlendState created:" << alphaBlendState << std::endl;
	}

	std::cout << "========== All States Created ==========" << std::endl;

	//// ========== Pass 1: Depth Write State ==========
	//D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	//depthDesc.DepthEnable = TRUE;
	//depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // ZWrite On
	//depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//depthDesc.StencilEnable = FALSE;

	//hr = device->CreateDepthStencilState(&depthDesc, &m_depthWriteState);
	//if (FAILED(hr)) {
	//	// 에러 처리
	//}

	//// ========== Pass 2: Depth Read State ==========
	//depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // ZWrite Off
	//depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;       // ✅ LESS_EQUAL 중요!

	//hr = device->CreateDepthStencilState(&depthDesc, &m_depthReadState);
	//if (FAILED(hr)) {
	//	// 에러 처리
	//}

	//// ========== Pass 1: No Color Write State ==========
	//D3D11_BLEND_DESC blendDesc = {};
	//blendDesc.AlphaToCoverageEnable = FALSE;
	//blendDesc.IndependentBlendEnable = FALSE;
	//blendDesc.RenderTarget[0].BlendEnable = FALSE;
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;  // ColorMask 0 (색상 안 씀)

	//hr = device->CreateBlendState(&blendDesc, &m_noColorWriteState);
	//if (FAILED(hr)) {
	//	// 에러 처리
	//}

	//// ========== Pass 2: Alpha Blend State ==========
	//blendDesc.RenderTarget[0].BlendEnable = TRUE;
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	//blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//hr = device->CreateBlendState(&blendDesc, &m_alphaBlendState);
	//if (FAILED(hr)) {
	//	// 에러 처리
	//}
}


void MeshRenderer::RenderMesh(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer, 
	ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11InputLayout* m_meshInputLayout,
	ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11ShaderResourceView* m_meshTexture,
	ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
	float maxMesh,float maxPhysicalVol,float overallSize,
	XMMATRIX w, XMMATRIX v, XMMATRIX p)
{
	if (!m_meshVertexBuffer || m_meshVertexCount == 0) return;

	// ========== Shader 바인딩 ==========
	context->VSSetShader(m_meshVS, nullptr, 0);
	context->PSSetShader(m_meshPS, nullptr, 0);
	context->IASetInputLayout(m_meshInputLayout);

	// ========== Transform 계산 ==========
	//float meshToVolume = (maxMesh / maxPhysicalVol) * overallSize / maxMesh * (float)(1.5f / overallSize);

	float meshScale = 1.5f / maxPhysicalVol;  // 이게 전부!

	float correctionFactor = maxPhysicalVol / maxMesh; // 0.796
	//float meshScale = correctionFactor * overallSize;

	 // ✅ mm 좌표 → 정규화 좌표
	//float meshScale = overallSize / maxPhysicalVol;

 
	//std::cout << "meshScale:" << meshScale << std::endl; // 0.00521
	//std::cout << "Sample vertex -109mm * scale =" << (-109 * meshScale);  // -0.568
	////qDebug() << "Volume range:" << -scaleX * overallSize * 0.5f << "to"
	////	<< scaleX * overallSize * 0.5f;  // -0.65 ~ 0.65



	DirectX::XMMATRIX scale = XMMatrixScaling(
	/*	meshToVolume,
		meshToVolume,
		meshToVolume*/

		meshScale,
		meshScale,
		meshScale
	);

	MeshConstantBuffer cb;
	DirectX::XMMATRIX rotation = XMMatrixRotationX(XM_PI);
	DirectX::XMMATRIX fullWorld = scale * rotation * w;

	cb.WVP = XMMatrixTranspose(fullWorld * v * p);
	cb.World = XMMatrixTranspose(fullWorld);
	cb.WorldView = XMMatrixTranspose(fullWorld * v);


	//// ✅ 디버그: WVP 출력
	//XMFLOAT4X4 wvpFloat;
	//XMStoreFloat4x4(&wvpFloat, cb.WVP);
	//std::cout << "WVP matrix:" << std::endl;
	//std::cout  << "wvpFloat._11 : "<<wvpFloat._11 << 
	//	"wvpFloat._12 : " << wvpFloat._12 << 
	//	"wvpFloat._13 : " << wvpFloat._13 << 
	//	"wvpFloat._14 : " << wvpFloat._14<< std::endl;

	//std::cout  << "wvpFloat._21 : " << wvpFloat._21 << 
	//	"wvpFloat._22 : " << wvpFloat._22 << 
	//	"wvpFloat._23 : " << wvpFloat._23 << 
	//	"wvpFloat._24 : " << wvpFloat._24<< std::endl;

	//std::cout  << "wvpFloat._31 : " << wvpFloat._31 << 
	//	"wvpFloat._32 : " << wvpFloat._32 << 
	//	"wvpFloat._33 : " << wvpFloat._33 << 
	//	"wvpFloat._34 : " << wvpFloat._34<< std::endl;

	//std::cout  << "wvpFloat._41 : " << wvpFloat._41 << 
	//	"wvpFloat._42 : " << wvpFloat._42 << 
	//	"wvpFloat._43 : " << wvpFloat._43 << 
	//	"wvpFloat._44 : " << wvpFloat._44<< std::endl;



	 // World
	XMFLOAT4X4 worldFloat;
	XMStoreFloat4x4(&worldFloat, fullWorld);
	std::cout << "World Matrix _44:" << worldFloat._44 << std::endl;

	// View
	XMFLOAT4X4 viewFloat;
	XMStoreFloat4x4(&viewFloat, v);
	std::cout << "View Matrix:";
	std::cout << "_43 (z translation):" << viewFloat._43 << std::endl;

	// Projection
	XMFLOAT4X4 projFloat;
	XMStoreFloat4x4(&projFloat, p);
	std::cout <<"Projection Matrix:";
	std::cout <<"_33:" << projFloat._33<< std::endl; // Far / (Far - Near)
	std::cout <<"_34:" << projFloat._34<< std::endl; // -Far * Near / (Far - Near)
	std::cout <<"_43:" << projFloat._43<< std::endl; // -1
	std::cout <<"_44:" << projFloat._44<< std::endl; // 0

	// Near/Far 역산
	if (projFloat._43 == -1.0f) {
		float c = projFloat._33;
		float d = projFloat._34;
		// c = f / (f - n)
		// d = -f * n / (f - n)
		// 해결: n = d / (c - 1), f = d / c
		float nearPlane = d / (c - 1.0f);
		float farPlane = d / c;
		std::cout << "✅ Estimated Near: " << nearPlane << std::endl;
		std::cout << "✅ Estimated Far: " << farPlane << std::endl;
	}

	context->UpdateSubresource(m_meshConstantBuffer, 0, nullptr, &cb, 0, 0);
	context->VSSetConstantBuffers(0, 1, &m_meshConstantBuffer);

	// ========== Clipping Settings ==========
	ClipSettings cs;
	cs.clipPlane = DirectX::XMFLOAT4(0, 0, 1, -0.15f);
	cs.enableClip = 1;

	context->UpdateSubresource(m_clipSettingsBuffer, 0, nullptr, &cs, 0, 0);
	context->PSSetConstantBuffers(1, 1, &m_clipSettingsBuffer);

	// ========== Texture/Sampler ==========
	context->PSSetShaderResources(0, 1, &m_meshTexture);
	context->PSSetSamplers(0, 1, &m_MeshSamplerState);

	//// ========== Rasterizer ==========
	//D3D11_RASTERIZER_DESC rastDesc = {};
	//rastDesc.FillMode = D3D11_FILL_SOLID;
	//rastDesc.CullMode = D3D11_CULL_BACK;
	//rastDesc.FrontCounterClockwise = FALSE;
	//rastDesc.DepthBias = 0;
	//rastDesc.DepthBiasClamp = 0.0f;
	//rastDesc.SlopeScaledDepthBias = 0.0f;

	//ID3D11RasterizerState* rastState = nullptr;
	//m_pDevice->CreateRasterizerState(&rastDesc, &rastState);




	context->RSSetState(rastState);

	// ========== Vertex Buffer ==========
	UINT stride = sizeof(PLY::VertexWithTexture);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_meshVertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// ==========================================
	// ========== PASS 1: Depth Write ===========
	// ==========================================

	

	// 바인딩
	context->OMSetDepthStencilState(depthWriteState, 0);
	//context->OMSetBlendState(noColorWriteState, nullptr, 0xffffffff);
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);



	// ✅ 실제로 바인딩되었는지 확인
	ID3D11DepthStencilState* currentDepthState = nullptr;
	UINT stencilRef;
	context->OMGetDepthStencilState(&currentDepthState, &stencilRef);

	std::cout << "Actually bound depth state:" << currentDepthState << std::endl;

	//if (currentDepthState) {
	//	if (currentDepthState == depthWriteState) {
	//		std::cout << "✅ Correct depth state bound!" << std::endl;
	//	}
	//	else {
	//		std::cout << "❌ Wrong depth state bound!" << std::endl;
	//	}
	//	currentDepthState->Release();
	//}
	//else {
	//	std::cout << "❌ No depth state bound!" << std::endl;
	//}




	std::cout << "Drawing" << m_meshVertexCount << "vertices..." << std::endl;




	// 렌더링 (Depth만 기록)
	context->Draw(m_meshVertexCount, 0);



	// 바인딩
	context->OMSetDepthStencilState(depthReadState, 0);
	context->OMSetBlendState(alphaBlendState, nullptr, 0xffffffff);
	//context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	// 렌더링 (반투명)
	context->Draw(m_meshVertexCount, 0);


	
}



void MeshRenderer::Cleanup()
{
	// ========== Cleanup ==========
	if (rastState) rastState->Release();
	if (depthWriteState) depthWriteState->Release();
	if (noColorWriteState) noColorWriteState->Release();
	if (depthReadState) depthReadState->Release();
	if (alphaBlendState) alphaBlendState->Release();





	// State 해제
	if (m_depthWriteState) m_depthWriteState->Release();
	if (m_depthReadState) m_depthReadState->Release();
	if (m_noColorWriteState) m_noColorWriteState->Release();
	if (m_alphaBlendState) m_alphaBlendState->Release();

	// 나머지 리소스들...
}