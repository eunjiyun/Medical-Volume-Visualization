#include<iostream>
#include "MeshRenderer.h"
#include "PLYLoader.h"


void MeshRenderer::CreateTwoPassStates(ID3D11Device* device)
{
	HRESULT hr;

	//  Description 구조체: 지역 변수로 선언
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
	hr = device->CreateRasterizerState(&rastDesc, &rastState);


	//  디버그 추가!
	if (FAILED(hr)) {
		std::cout << " Failed to create rasterizer state!" << std::endl;
	}
	else {
		std::cout << " Rasterizer state created:" << rastState << std::endl;
	}


	D3D11_DEPTH_STENCIL_DESC depthWriteDesc = {};
	depthWriteDesc.DepthEnable = TRUE;
	//depthWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 중요
	depthWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  //  반드시 ALL
	//depthWriteDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;      //  핵심
	depthWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthWriteDesc.StencilEnable = FALSE;

	device->CreateDepthStencilState(&depthWriteDesc, &depthWriteState);

	//  디버그 추가!
	if (FAILED(hr)) {
		std::cout << " Failed to create depthWriteState!" << std::endl;
	}
	else {
		std::cout << " depthWriteState created:" << depthWriteState << std::endl;
	}



	//// Blend State: Color Write OFF

	//blendDesc.RenderTarget[0].BlendEnable = FALSE;
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;  // ColorMask 0


	//hr=device->CreateBlendState(&blendDesc, &noColorWriteState);


	//  디버그 추가!
	if (FAILED(hr)) {
		std::cout << " Failed to create noColorWriteState!" << std::endl;
	}
	else {
		std::cout << " noColorWriteState created:" << noColorWriteState << std::endl;
	}

	ID3D11BlendState* noColorWriteState = nullptr;

	D3D11_BLEND_DESC noColorDesc = {};
	noColorDesc.AlphaToCoverageEnable = FALSE;
	noColorDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& rt = noColorDesc.RenderTarget[0];
	rt.BlendEnable = FALSE;                 //  블렌딩 자체도 꺼도 됨
	rt.RenderTargetWriteMask = 0;           //  컬러 출력 완전 차단 (핵심)

	hr = device->CreateBlendState(&noColorDesc, &noColorWriteState);

	if (FAILED(hr))
	{
		std::cout << " Failed to create noColorWriteState!" << std::endl;
	}
	else
	{
		std::cout << " noColorWriteState created: " << noColorWriteState << std::endl;
	}




	// ==========================================
	// ======= PASS 2: Transparent Render =======
	// ==========================================

	// Depth State: Write OFF, Test ON
	D3D11_DEPTH_STENCIL_DESC pass3DepthDesc = {};
	//	pass3DepthDesc.DepthEnable = FALSE;                         //  Test OFF!
	pass3DepthDesc.DepthEnable = TRUE;                      //  ON
	pass3DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //  Write OFF
	pass3DepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; //  핵심
	pass3DepthDesc.StencilEnable = FALSE;
	hr = device->CreateDepthStencilState(&pass3DepthDesc, &depthReadState);

	//depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // ZWrite Off
	//depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;  //  LESS_EQUAL!
	//hr=device->CreateDepthStencilState(&depthDesc, &depthReadState);

	//  디버그 추가!
	if (FAILED(hr)) {
		std::cout << " Failed to create depthReadState!" << std::endl;
	}
	else {
		std::cout << " depthReadState created:" << depthReadState << std::endl;
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


	//  디버그 추가!
	if (FAILED(hr)) {
		std::cout << " Failed to create alphaBlendState!" << std::endl;
	}
	else {
		std::cout << " alphaBlendState created:" << alphaBlendState << std::endl;
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
	//depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;       //  LESS_EQUAL 중요!

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

void MeshRenderer::RenderMeshDepth(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
	ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11RenderTargetView* sceneDepthRTV , ID3D11InputLayout* m_meshInputLayout,
	ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer,  ID3D11Texture2D* m_meshTexture, ID3D11ShaderResourceView* m_meshDepthSRV,
	ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
	float maxMesh, float maxPhysicalVol, float volWidth, float volHeight, float volDepth, float overallSize,
	XMMATRIX userRotMat, XMMATRIX v, XMMATRIX p, float width, float height)
{

	if (!m_meshVertexBuffer || 0==m_meshVertexCount ) {
		//std::cout << "[RenderMeshDepth]  VertexBuffer 없음 또는 VertexCount=0" << std::endl;
		return;
	}

	//std::cout << "[RenderMeshDepth] 시작" << std::endl;


	ID3D11RenderTargetView* curRTV = nullptr;
	ID3D11DepthStencilView* curDSV = nullptr;
	context->OMGetRenderTargets(1, &curRTV, &curDSV);
	//std::cout << "[RenderMeshDepth] OMGetRenderTargets: curRTV=" << curRTV << " curDSV=" << curDSV << std::endl;


	if (curRTV) curRTV->Release();
	if (!curDSV) {
		//std::cout << "[RenderMeshDepth]  curDSV가 nullptr" << std::endl;
		return;
	}
	/*std::cout << "[RenderMeshDepth] BEGIN" << std::endl;
	std::cout << "  VertexCount = " << m_meshVertexCount << std::endl;
	std::cout << "  sceneDepthRTV = " << sceneDepthRTV << std::endl;
	std::cout << "  curDSV(before) = " << curDSV << std::endl;*/




	//  SceneDepth를 RenderTarget으로 설정
	ID3D11RenderTargetView* rtvs[] = { sceneDepthRTV  };
	context->OMSetRenderTargets(1, rtvs, curDSV);
	//std::cout << "[RenderMeshDepth] OMSetRenderTargets 완료" << std::endl;



	ID3D11RenderTargetView* dbgRTV = nullptr;
	ID3D11DepthStencilView* dbgDSV = nullptr;

	context->OMGetRenderTargets(1, &dbgRTV, &dbgDSV);

	//std::cout << "[OMSetRenderTargets]" << std::endl;
	//std::cout << "  RTV = " << dbgRTV << std::endl;
	//std::cout << "  DSV = " << dbgDSV << std::endl;

	if (dbgRTV) dbgRTV->Release();
	if (dbgDSV) dbgDSV->Release();


	//  SceneDepth 클리어
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(sceneDepthRTV, clearColor);

	context->ClearDepthStencilView(curDSV,
		D3D11_CLEAR_DEPTH, 1.0f, 0);

	//if (curDSV) curDSV->Release();






	////  Depth pass는 반드시 full-res viewport
	D3D11_VIEWPORT fullVP = {};
	fullVP.TopLeftX = 0.0f;
	fullVP.TopLeftY = 0.0f;
	fullVP.Width = static_cast<float>(width)/2;   // 전체 화면 width
	fullVP.Height = static_cast<float>(height)/2;  // 전체 화면 height
	fullVP.MinDepth = 0.0f;
	fullVP.MaxDepth = 1.0f;

	D3D11_VIEWPORT vp;
	UINT vpCount = 1;
	context->RSGetViewports(&vpCount, &vp);

	/*std::cout << "[Viewport]" << std::endl;
	std::cout << "  x=" << vp.TopLeftX
		<< " y=" << vp.TopLeftY
		<< " w=" << vp.Width
		<< " h=" << vp.Height << std::endl;*/

	context->RSSetViewports(1, &fullVP);


	////  SceneDepth를 RenderTarget으로 설정
	//ID3D11RenderTargetView* rtvs[] = { rtv };  // ← 추가 필요
	//context->OMSetRenderTargets(1, rtvs, depthStencilView);

	//// SceneDepth 클리어
	//float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//context->ClearRenderTargetView(m_sceneDepthRTV, clearColor);


	context->PSSetSamplers(5, 1, &m_PointClampSampler);  // s5 채우기


	// ========== Shader 바인딩 ==========
	context->VSSetShader(m_meshVS, nullptr, 0);
	context->PSSetShader(m_meshPS, nullptr, 0); //
	context->IASetInputLayout(m_meshInputLayout);



	//s r t v p
	rotation = XMMatrixRotationX(XM_PI);

	rotX = XMQuaternionRotationAxis(
		XMVectorSet(1, 0, 0, 0),  // X축
		-XM_PIDIV2                  // 90도
	);

	rotY = XMQuaternionRotationAxis(
		XMVectorSet(0, 0, 1, 0),
		XM_PI  // Y축 180도
	);

	//// 테스트할 회전들
	//XMMATRIX test1 = XMMatrixRotationX(XM_PIDIV2);        // 90도
	//XMMATRIX test2 = XMMatrixRotationX(-XM_PIDIV2);       // -90도
	//XMMATRIX test3 = XMMatrixRotationX(XM_PI);            // 180도

	//XMMATRIX test4 = XMMatrixRotationY(XM_PI);            // Y축 180도

	//XMMATRIX test5 = XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(XM_PI);
	//XMMATRIX test6 = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationZ(XM_PI);


	//Dx11 좌표계로 메쉬 정렬 안 하면 
	//크기가 변해서 볼륨과의 스케일 정합이 깨지는 이유는
	//Scale 값이 바뀐 게 아니라
	//Scale이 적용되는 축 기준이 바뀌었기 때문
	DirectX::XMMATRIX scale = XMMatrixScaling(meshScale, meshScale, meshScale);

	//볼륨 - 메쉬 기본은 rotx, roty 인데 rotation은 메쉬에만 추가로 곱해줌.

	//initialMeshWorld = coordinateSystemTransform*XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY))*rotation*test3;
	initialMeshWorld = scale* XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY))*rotation;
	meshWorldMat = initialMeshWorld * XMMatrixTranspose(userRotMat)/**coordinateSystemTransform*/;



	//스케일을 볼륨걸 적용한 유저 로테이션을 곱해야지 회전 싱크가 맞음
	//전치 행렬을 안 쓰고 전치 안 한 사용자 회전 행렬을 메쉬에 적용해서 그런걸지도? 

	//Transpose를 뒤에 곱했을 때 축이 안 틀어진 이유는
	//	그게 “로컬 기준 역회전”처럼 동작했기 때문이고,
	//	userRotation을 앞에 곱했을 때 축이 틀어진 이유는
	//	initialMeshWorld가 아직 월드 기준 좌표계가 아니기 때문이다.



	XMVECTOR s, r, t;
	XMMatrixDecompose(&s, &r, &t, initialMeshWorld);

	//std::cout<< "World scale mesh:"
	//	<<" "<< XMVectorGetX(s)
	//	<< " " << XMVectorGetY(s)
	//	<< " " << XMVectorGetZ(s);

	//std::cout << "World translation mesh:"
	//	<< " " << XMVectorGetX(t)
	//	<< " " << XMVectorGetY(t)
	//	<< " " << XMVectorGetZ(t);



	

	ExtractAxes(&volWorldMat, &meshWorldMat);

	// HLSL에서는 mul(vector, matrix) 사용
	  // 실제 적용 순서: S -> R -> T (의도한 대로)

	MeshConstantBufferWithCT cbM;
	cbM.WVP = XMMatrixTranspose(meshWorldMat * v * p);
	cbM.View = XMMatrixTranspose(v);
	cbM.World = XMMatrixTranspose(meshWorldMat);
	//cbM.CTBlendParams = XMFLOAT4(ctBlendStrength, 0.0f, 0.0f, faceBlend);  //  CT 강도



	context->UpdateSubresource(m_meshConstantBuffer, 0, nullptr, &cbM, 0, 0);
	context->VSSetConstantBuffers(0, 1, &m_meshConstantBuffer);

	//// 깊이 스테이트
	//context->OMSetDepthStencilState(depthWriteState, 0);
	//context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	//context->RSSetState(rastState);

	// ========== Clipping Settings ==========
	ClipSettings cs;
	cs.clipPlane = DirectX::XMFLOAT4(0, 0, 1, -0.15f);
	cs.enableClip = 1;

	context->UpdateSubresource(m_clipSettingsBuffer, 0, nullptr, &cs, 0, 0);
	context->PSSetConstantBuffers(1, 1, &m_clipSettingsBuffer);

	//// ========== Texture/Sampler ==========
	//context->PSSetShaderResources(0, 1, &m_meshDepthSRV);
	//context->PSSetSamplers(0, 1, &m_MeshSamplerState);

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
	//context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	//context->OMSetBlendState(noColorWriteState, nullptr, 0xffffffff);
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);



	ID3D11DepthStencilState* dbgDepthState = nullptr;
	UINT stencilRef = 0;
	context->OMGetDepthStencilState(&dbgDepthState, &stencilRef);

	/*std::cout << "[DepthState]" << std::endl;
	std::cout << "  Bound depth state = " << dbgDepthState << std::endl;*/

	if (dbgDepthState) dbgDepthState->Release();



	////  실제로 바인딩되었는지 확인
	//ID3D11DepthStencilState* currentDepthState = nullptr;
	//UINT stencilRef;
	//context->OMGetDepthStencilState(&currentDepthState, &stencilRef);

	//std::cout << "Actually bound depth state:" << currentDepthState << std::endl;

	//if (currentDepthState) {
	//	if (currentDepthState == depthWriteState) {
	//		std::cout << " Correct depth state bound!" << std::endl;
	//	}
	//	else {
	//		std::cout << " Wrong depth state bound!" << std::endl;
	//	}
	//	currentDepthState->Release();
	//}
	//else {
	//	std::cout << " No depth state bound!" << std::endl;
	//}




	//std::cout << "Drawing" << m_meshVertexCount << "vertices..." << std::endl;


	// 렌더링 (Depth만 기록)
	context->Draw(m_meshVertexCount, 0);


	/*ID3D11Resource* depthRes = nullptr;
	curDSV->GetResource(&depthRes);

	ID3D11Texture2D* depthTex = nullptr;
	depthRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&depthTex);

	if (depthTex)
	{
		D3D11_TEXTURE2D_DESC desc;
		depthTex->GetDesc(&desc);

		std::cout << "[DepthBuffer Desc]" << std::endl;
		std::cout << "  Format = " << desc.Format << std::endl;
		std::cout << "  Size = " << desc.Width << " x " << desc.Height << std::endl;
	}
	else
	{
		std::cout << " depthTex is null" << std::endl;
	}

	if (depthTex) depthTex->Release();
	if (depthRes) depthRes->Release();*/




	////  깊이 버퍼를 SceneDepth 텍스처로 복사
	//
	//curDSV->GetResource((ID3D11Resource**)&m_meshTexture);

	//if (curDSV) curDSV->Release();

	//if (m_meshTexture && sceneDepthTexture)
	//{
	//	// Depth Stencil Buffer → SceneDepth 복사
	//	context->CopyResource(sceneDepthTexture, m_meshTexture);
	//}

	//if (m_meshTexture) m_meshTexture->Release();




	/*ID3D11Resource* res = nullptr;
	curDSV->GetResource(&res);

	ID3D11Texture2D* depthTex = nullptr;
	res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&depthTex);

	if (depthTex && sceneDepthTexture)
		context->CopyResource(sceneDepthTexture, depthTex);


	std::cout << "[CopyResource]" << std::endl;
	std::cout << "  depthTex = " << depthTex << std::endl;
	std::cout << "  sceneDepthTexture = " << sceneDepthTexture << std::endl;




	if (depthTex) depthTex->Release();
	if (res) res->Release();*/

}

void MeshRenderer::RenderMeshViewZ(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
	ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11RenderTargetView* sceneDepthRTV, ID3D11InputLayout* m_meshInputLayout,
	ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11Texture2D* m_meshTexture, ID3D11ShaderResourceView* m_meshDepthSRV,
	ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
	float maxMesh, float maxPhysicalVol, float volWidth, float volheight, float volDepth, float overallSize,
	XMMATRIX userRotMat, XMMATRIX v, XMMATRIX p, float width, float height)
{

	if (!m_meshVertexBuffer || m_meshVertexCount == 0) {
		//std::cout << "[RenderMeshDepth]  VertexBuffer 없음 또는 VertexCount=0" << std::endl;
		return;
	}

	//std::cout << "[RenderMeshDepth] 시작" << std::endl;


	ID3D11RenderTargetView* curRTV = nullptr;
	ID3D11DepthStencilView* curDSV = nullptr;
	context->OMGetRenderTargets(1, &curRTV, &curDSV);
	//std::cout << "[RenderMeshDepth] OMGetRenderTargets: curRTV=" << curRTV << " curDSV=" << curDSV << std::endl;


	//if (curRTV) curRTV->Release();
	//if (!curDSV) {
	//	//std::cout << "[RenderMeshDepth]  curDSV가 nullptr" << std::endl;
	//	return;
	//}
	///*std::cout << "[RenderMeshDepth] BEGIN" << std::endl;
	//std::cout << "  VertexCount = " << m_meshVertexCount << std::endl;
	//std::cout << "  sceneDepthRTV = " << sceneDepthRTV << std::endl;
	//std::cout << "  curDSV(before) = " << curDSV << std::endl;*/




	//  SceneDepth를 RenderTarget으로 설정
	ID3D11RenderTargetView* rtvs[] = { sceneDepthRTV };
	context->OMSetRenderTargets(1, rtvs, nullptr);
	//std::cout << "[RenderMeshDepth] OMSetRenderTargets 완료" << std::endl;



	ID3D11RenderTargetView* dbgRTV = nullptr;
	ID3D11DepthStencilView* dbgDSV = nullptr;

	context->OMGetRenderTargets(1, &dbgRTV, &dbgDSV);

	//std::cout << "[OMSetRenderTargets]" << std::endl;
	//std::cout << "  RTV = " << dbgRTV << std::endl;
	//std::cout << "  DSV = " << dbgDSV << std::endl;

	if (dbgRTV) dbgRTV->Release();
	if (dbgDSV) dbgDSV->Release();


	//  SceneDepth 클리어
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(sceneDepthRTV, clearColor);

	context->ClearDepthStencilView(curDSV,
		D3D11_CLEAR_DEPTH, 1.0f, 0);

	//if (curDSV) curDSV->Release();


	////  Depth pass는 반드시 full-res viewport
	D3D11_VIEWPORT fullVP = {};
	fullVP.TopLeftX = 0.0f;
	fullVP.TopLeftY = 0.0f;
	fullVP.Width = static_cast<float>(width) / 2;   // 전체 화면 width
	fullVP.Height = static_cast<float>(height) / 2;  // 전체 화면 height
	fullVP.MinDepth = 0.0f;
	fullVP.MaxDepth = 1.0f;

	D3D11_VIEWPORT vp;
	UINT vpCount = 1;
	context->RSGetViewports(&vpCount, &vp);

	/*std::cout << "[Viewport]" << std::endl;
	std::cout << "  x=" << vp.TopLeftX
		<< " y=" << vp.TopLeftY
		<< " w=" << vp.Width
		<< " h=" << vp.Height << std::endl;*/

	context->RSSetViewports(1, &fullVP);



	context->PSSetSamplers(5, 1, &m_PointClampSampler);  // s5 채우기


	// ========== Shader 바인딩 ==========
	context->VSSetShader(m_meshVS, nullptr, 0);
	context->PSSetShader(m_meshPS, nullptr, 0); // 
	context->IASetInputLayout(m_meshInputLayout);




	rotation = XMMatrixRotationX(XM_PI);

	//s r t v p



	rotX = XMQuaternionRotationAxis(
		XMVectorSet(1, 0, 0, 0),  // X축
		-XM_PIDIV2                  // 90도
	);

	rotY = XMQuaternionRotationAxis(
		XMVectorSet(0, 0, 1, 0),
		XM_PI  // Y축 180도
	);



	////볼륨 - 메쉬 기본은 rotx, roty 인데 rotation은 메쉬에만 추가로 곱해줌.
	//initialMeshWorld =coordinateSystemTransform/**flipYZ*rotation*/;
	//initialMeshWorld = coordinateSystemTransform*XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY))*rotation*test3;
	initialMeshWorld = meshScale*XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY))*rotation;


	//스케일을 볼륨걸 적용한 유저 로테이션을 곱해야지 회전 싱크가 맞음
	//전치 행렬을 안 쓰고 전치 안 한 사용자 회전 행렬을 메쉬에 적용해서 그런걸지도? 

	//Transpose를 뒤에 곱했을 때 축이 안 틀어진 이유는
	//	그게 “로컬 기준 역회전”처럼 동작했기 때문이고,
	//	userRotation을 앞에 곱했을 때 축이 틀어진 이유는
	//	initialMeshWorld가 아직 월드 기준 좌표계가 아니기 때문이다.


	// HLSL에서는 mul(vector, matrix) 사용
	  // 실제 적용 순서: S -> R -> T (의도한 대로)

	meshWorldMat = initialMeshWorld * XMMatrixTranspose(userRotMat)/**coordinateSystemTransform*/;

	MeshConstantBufferWithCT cbM;
	cbM.WVP = XMMatrixTranspose(meshWorldMat * v * p);
	cbM.View = XMMatrixTranspose(v);
	cbM.World = XMMatrixTranspose(meshWorldMat);




	context->UpdateSubresource(m_meshConstantBuffer, 0, nullptr, &cbM, 0, 0);
	context->VSSetConstantBuffers(0, 1, &m_meshConstantBuffer);


	// ========== Clipping Settings ==========
	ClipSettings cs;
	cs.clipPlane = DirectX::XMFLOAT4(0, 0, 1, -0.15f);
	cs.enableClip = 1;

	context->UpdateSubresource(m_clipSettingsBuffer, 0, nullptr, &cs, 0, 0);
	context->PSSetConstantBuffers(1, 1, &m_clipSettingsBuffer);



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
	context->OMSetDepthStencilState(nullptr, 0);
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);



	ID3D11DepthStencilState* dbgDepthState = nullptr;
	UINT stencilRef = 0;
	context->OMGetDepthStencilState(&dbgDepthState, &stencilRef);



	if (dbgDepthState) dbgDepthState->Release();



	// 렌더링 (Depth만 기록)
	context->Draw(m_meshVertexCount, 0);

	context->OMSetRenderTargets(1, &curRTV, curDSV);
	context->OMSetDepthStencilState(dbgDepthState, stencilRef);

	if (curRTV) curRTV->Release();
	if (!curDSV) {
		//std::cout << "[RenderMeshDepth]  curDSV가 nullptr" << std::endl;
		return;
	}
	/*std::cout << "[RenderMeshDepth] BEGIN" << std::endl;
	std::cout << "  VertexCount = " << m_meshVertexCount << std::endl;
	std::cout << "  sceneDepthRTV = " << sceneDepthRTV << std::endl;
	std::cout << "  curDSV(before) = " << curDSV << std::endl;*/

}

float MeshRenderer::ComputeHandedness(XMVECTOR X, XMVECTOR Y, XMVECTOR Z)
{
	XMVECTOR crossXY = XMVector3Cross(X, Y);
	return XMVectorGetX(XMVector3Dot(crossXY, Z));
}



int cnt{};
bool MeshRenderer::ExtractAxes(const XMMATRIX* volWorld, const XMMATRIX* meshWorld)
{
	Axes volAx, meshAx;

	// =========================
	// 1. 축 추출 (Row-major)
	// =========================
	auto Extract = [](const XMMATRIX& m, int row)
	{
		return XMVector3Normalize(XMVectorSet(
			m.r[row].m128_f32[0],
			m.r[row].m128_f32[1],
			m.r[row].m128_f32[2],
			0.0f));
	};

	volAx.X = Extract(*volWorld, 0);
	volAx.Y = Extract(*volWorld, 1);
	volAx.Z = Extract(*volWorld, 2);

	meshAx.X = Extract(*meshWorld, 0);
	meshAx.Y = Extract(*meshWorld, 1);
	meshAx.Z = Extract(*meshWorld, 2);

	// =========================
	// 2. 축 방향 내적 비교
	// =========================
	float dotX{ XMVectorGetX(XMVector3Dot(volAx.X, meshAx.X)) };
	float dotY{ XMVectorGetX(XMVector3Dot(volAx.Y, meshAx.Y)) };
	float dotZ{ XMVectorGetX(XMVector3Dot(volAx.Z, meshAx.Z)) };

	bool axisAligned{
		fabs(dotX) > 0.99f &&
		fabs(dotY) > 0.99f &&
		fabs(dotZ) > 0.99f };

	// =========================
	// 3. handedness 비교
	// =========================
	float volHand{ ComputeHandedness(volAx.X, volAx.Y, volAx.Z) };
	float meshHand{ ComputeHandedness(meshAx.X, meshAx.Y, meshAx.Z) };

	bool sameHandedness{ (volHand * meshHand) > 0.0f };

	// =========================
	// 4. 좌우 반전 판정 (핵심)
	// =========================
	bool isLeftRightFlipped{ (dotX < 0.0f) };


	if (0 == cnt) {

		// =========================
		// 4. 로그 출력 (검증용)
		// =========================
		std::cout << "[Axis Dot]\n";
		std::cout << "X: " << dotX << " Y: " << dotY << " Z: " << dotZ << "\n";

		std::cout << "[Handedness]\n";
		std::cout << "Volume: " << volHand
			<< " Mesh: " << meshHand << "\n";

		if (!axisAligned)
			std::cout << " Axis direction mismatch\n";
		else
			std::cout << " Axis directions aligned\n";

		if (!sameHandedness)
			std::cout << " Handedness mismatch (mirror)\n";
		else
			std::cout << " Same handedness\n";


		if (isLeftRightFlipped)
			std::cout << " Left/Right flipped (X axis inverted)\n";
		else
			std::cout << " Left/Right direction consistent\n";

		++cnt;
	}

	// =========================
  // 6. 최종 판정
  // =========================
  // 덴탈 기준:
  // - 축 정렬 OK
  // - 좌우 반전 


	return axisAligned && sameHandedness;
}



void MeshRenderer::RenderMesh(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
	ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11InputLayout* m_meshInputLayout,
	ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11ShaderResourceView* m_meshTexture,
	ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
	float maxMesh, float maxPhysicalVol, float volWidth, float volHeight, float volDepth, float overallSize,
	XMMATRIX w, XMMATRIX v, XMMATRIX p)
{
	if (!m_meshVertexBuffer || m_meshVertexCount == 0) return;

	// ========== Shader 바인딩 ==========
	context->VSSetShader(m_meshVS, nullptr, 0);
	context->PSSetShader(m_meshPS, nullptr, 0);
	context->IASetInputLayout(m_meshInputLayout);



	//MeshConstantBuffer cb;
	DirectX::XMMATRIX rotation = XMMatrixRotationX(XM_PI);
	//	DirectX::XMMATRIX fullWorld = /*centerTranslate **/scale * rotation * w;
	//DirectX::XMMATRIX fullWorld = scale * rotation * w;

	//	//s r t v p
 // //  DirectX::XMMATRIX fullWorld = scale * rotation /**centerTranslate*/* w;
	//DirectX::XMMATRIX fullWorld = centerTranslate * scale   * rotation  /** w*/;
	////DirectX::XMMATRIX fullWorld = centerTranslate * rotation *scale* w;
	////DirectX::XMMATRIX fullWorld = /*scale * */rotation /**centerTranslate*/* w;



		//rotx = XMMatrixRotationX(-XM_PIDIV2);  // 90도 회전
	//roty = XMMatrixRotationY(XM_PI);  // 90도 회전

	//initialMeshWorld = centerTranslate * scale
	//	* XMMatrixRotationY(XM_PI)*XMMatrixRotationX(XM_PIDIV2)  /** w*/;


	//initialMeshWorld = centerTranslate /** XMMatrixRotationX(XM_PI)*/ * scale
	//	/** XMMatrixRotationY(XM_PI)*/ /** w*/;

	////initialMeshWorld = scale * XMMatrixRotationX(XM_PI)*centerTranslate;

	///*std::cout << "meshScale : " << meshScale << std::endl;
	//std::cout << "scale x : " << volWidth / meshWidth / maxPhysicalVol << std::endl;
	//std::cout << "scale y : " << volHeight / meshHeight / maxPhysicalVol << std::endl;
	//std::cout << "scale z : " << volDepth / meshDepth / maxPhysicalVol << std::endl << std::endl << std::endl;*/



	//MeshConstantBuffer cb;
	//cb.WVP = XMMatrixTranspose(initialMeshWorld * v * p);
	//cb.World = XMMatrixTranspose(initialMeshWorld);
	//cb.WorldView = XMMatrixTranspose(initialMeshWorld * v);

	//context->UpdateSubresource(m_meshConstantBuffer, 0, nullptr, &cb, 0, 0);
	//context->VSSetConstantBuffers(0, 1, &m_meshConstantBuffer);

	// ========== Clipping Settings ==========
	ClipSettings cs;
	cs.clipPlane = DirectX::XMFLOAT4(0, 0, 1, -0.15f);
	cs.enableClip = 1;

	context->UpdateSubresource(m_clipSettingsBuffer, 0, nullptr, &cs, 0, 0);
	context->PSSetConstantBuffers(1, 1, &m_clipSettingsBuffer);

	// ========== Texture/Sampler ==========
	context->PSSetShaderResources(0, 1, &m_meshTexture);
	context->PSSetSamplers(0, 1, &m_MeshSamplerState);

	// ========== Rasterizer ==========
	context->RSSetState(rastState);

	// ========== Vertex Buffer ==========
	UINT stride = sizeof(PLY::VertexWithTexture);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_meshVertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ========== PASS 3: 반투명 상태 설정 ==========
	context->OMSetDepthStencilState(depthReadState, 0);
	context->OMSetBlendState(alphaBlendState, nullptr, 0xffffffff);

	// 렌더링 (반투명)
	context->Draw(m_meshVertexCount, 0);
}

int print{};
int printRotate{};



void MeshRenderer::RenderMeshWithCT(
	ID3D11DeviceContext* context,
	ID3D11Buffer* m_meshVertexBuffer,
	ID3D11VertexShader* m_meshVS,
	ID3D11PixelShader* m_meshPS,
	ID3D11InputLayout* m_meshInputLayout,
	ID3D11Buffer* m_clipSettingsBuffer,
	ID3D11Buffer* m_meshConstantBuffer,
	ID3D11ShaderResourceView* m_meshTexture,      // 얼굴 텍스처

	ID3D11ShaderResourceView* ctTexture,          //  CT 텍스처
	ID3D11ShaderResourceView* depthTexture,          //  depth 텍스처
	ID3D11SamplerState* m_MeshSamplerState,
	//ID3D11SamplerState* depthSamplerState,
	ID3D11Device* m_pDevice,
	int m_meshVertexCount,
	float maxMesh,
	float maxPhysicalVol,
	float volWidth, float volHeight, float volDepth,
	float overallSize,
	XMMATRIX userRotMat,
	XMMATRIX v,
	XMMATRIX p,
	float ctBlendStrength)                        //  CT 합성 강도
{
	if (!m_meshVertexBuffer || m_meshVertexCount == 0) return;

	// ========== Shader 바인딩 ==========
	context->VSSetShader(m_meshVS, nullptr, 0);
	context->PSSetShader(m_meshPS, nullptr, 0);   //  FaceMesh_WithCT.hlsl 사용
	context->IASetInputLayout(m_meshInputLayout);

	// ========== Transform 계산 ==========
	//float meshScale = 1.5f / maxPhysicalVol;  //  XMFLOAT3 대응
	//float meshScale =1.f;  //  XMFLOAT3 대응

	//DirectX::XMMATRIX scale = XMMatrixScaling(meshScale, meshScale, meshScale);


	//DirectX::XMMATRIX scale = XMMatrixScaling(
	//	/*	meshToVolume,
	//		meshToVolume,
	//		meshToVolume*/

	//		meshScale*overallSize,
	//		meshScale*overallSize,
	//		meshScale*overallSize
	//	//volWidth / meshWidth / maxPhysicalVol * 1.42f,
	//	//volHeight / meshHeight / maxPhysicalVol * 1.42f*1.09f,
	//	//volDepth / meshDepth / maxPhysicalVol * 1.42f

	//);


	//s r t v p

	rotX = XMQuaternionRotationAxis(
		XMVectorSet(1, 0, 0, 0),  // X축
		-XM_PIDIV2                  // 90도
	);

	rotY = XMQuaternionRotationAxis(
		XMVectorSet(0, 0, 1, 0),
		XM_PI  // Y축 180도
	);


	//initialMeshWorld = XMMatrixRotationX(XM_PI)*XMMatrixTranslation(50.0f, 0.0f, 0.0f)
	//	*XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY));

	//rotation = XMMatrixRotationX(XM_PI);

	//s r t v p
	//initialMeshWorld =scale * rotation;                // 그 다음 회전
	//initialMeshWorld =rotation* centerTranslate;  //  스케일 없음
	//initialMeshWorld = rotation/**XMMatrixTranslation(0.0f, 0.0f, 0.0f)*/; //  스케일 없음


	//// 테스트할 회전들
	//XMMATRIX test1 = XMMatrixRotationX(XM_PIDIV2);        // 90도
	//XMMATRIX test2 = XMMatrixRotationX(-XM_PIDIV2);       // -90도
	//XMMATRIX test3 = XMMatrixRotationX(XM_PI);            // 180도

	//XMMATRIX test4 = XMMatrixRotationY(XM_PI);            // Y축 180도

	//XMMATRIX test5 = XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(XM_PI);
	//XMMATRIX test6 = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationZ(XM_PI);

// 개선 (Y-Z 교환 + X축 반전으로 handedness 맞추기)
	XMMATRIX coordinateSystemTransform = XMMatrixSet(
		1.0f, 0.0f, 0.0f, 0.0f,  // X축 반전 (handedness 변경)
		0.0f, -1.0f, 0.0f, 0.0f,  // Y축 -> Z축
		0.0f, 0.0f, -1.0f, 0.0f,  // Z축 -> Y축
		0.0f, 0.0f, 0.0f, 1.0f
	);


	//initialMeshWorld = coordinateSystemTransform/**test4*//**rotation*/;
	

	//initialMeshWorld = coordinateSystemTransform*
	//	XMMatrixRotationQuaternion(XMQuaternionMultiply(rotX, rotY))*rotation;
	
	//meshWorldMat = initialMeshWorld * XMMatrixTranspose(userRotMat);
	ExtractAxes(&volWorldMat, &meshWorldMat);

	//XMVECTOR s, r, t;
	//XMMatrixDecompose(&s, &r, &t, initialMeshWorld);

	//XMVECTOR s2, r2, t2;
	//XMMatrixDecompose(&s2, &r2, &t2, meshWorldMat);

	//if (XMMatrixIsIdentity(userRotMat)){
	//	std::cout << "before rotate mesh World scale: "
	//		<< XMVectorGetX(s) << "  "
	//		<< XMVectorGetY(s) << "  "
	//		<< XMVectorGetZ(s) << std::endl << std::endl;

	//	std::cout << "before rotate mesh World translation: "
	//		<< XMVectorGetX(t) << "  "
	//		<< XMVectorGetY(t) << "  "
	//		<< XMVectorGetZ(t) << std::endl << std::endl;
	//}
	//else {
	//	std::cout << "after rotate mesh World scale: "
	//		<< XMVectorGetX(s2) << "  "
	//		<< XMVectorGetY(s2) << "  "
	//		<< XMVectorGetZ(s2) << std::endl << std::endl;

	//	std::cout << "after rotate mesh World translation: "
	//		<< XMVectorGetX(t2) << "  "
	//		<< XMVectorGetY(t2) << "  "
	//		<< XMVectorGetZ(t2) << std::endl << std::endl;
	//}


	// HLSL에서는 mul(vector, matrix) 사용
  // 실제 적용 순서: S -> R -> T (의도한 대로)
	cbM.WVP = XMMatrixTranspose(meshWorldMat * v * p);

	cbM.View = XMMatrixTranspose(v);
	cbM.World = XMMatrixTranspose(meshWorldMat);

	cbM.CTBlendParams = XMFLOAT4(ctBlendStrength, 0.0f, 0.0f, faceBlend);  //  CT 강도


	//XMMATRIX vp = v*p;

	//XMVECTOR s, r, t;
	//XMMatrixDecompose(&s, &r, &t, initialMeshWorld*XMMatrixTranspose(userRotMat));
	//

	//std::cout
	//<< "Mesh world scale: "
	//<< XMVectorGetX(s) << ", "
	//<< XMVectorGetY(s) << ", "
	//<< XMVectorGetZ(s) << std::endl;


	context->UpdateSubresource(m_meshConstantBuffer, 0, nullptr, &cbM, 0, 0);
	context->VSSetConstantBuffers(0, 1, &m_meshConstantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_meshConstantBuffer);  //  PS에도 전달

	// ========== Clipping Settings ==========
	ClipSettings cs;
	cs.clipPlane = DirectX::XMFLOAT4(0, 0, 1, -0.15f);
	cs.enableClip = 1;

	context->UpdateSubresource(m_clipSettingsBuffer, 0, nullptr, &cs, 0, 0);
	context->PSSetConstantBuffers(1, 1, &m_clipSettingsBuffer);

	// ========== Texture/Sampler 바인딩 ==========
	//  t0 = 얼굴 텍스처, t1 = CT 텍스처
	ID3D11ShaderResourceView* srvs[3] = {
		m_meshTexture,  // t0
		ctTexture ,      // t1  CT 텍스처
		depthTexture
	};
	context->PSSetShaderResources(0, 3, srvs);



	//  s0 = linear sampler (양쪽 다 사용)
	context->PSSetSamplers(0, 1, &m_MeshSamplerState);
	context->PSSetSamplers(1, 1, &m_PointClampSampler);

	// ========== Rasterizer ==========
	context->RSSetState(rastState);

	// ========== Vertex Buffer ==========
	UINT stride = sizeof(PLY::VertexWithTexture);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_meshVertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ========== 렌더 스테이트 설정 ==========
	//  불투명하게 그리기 (CT 합성 후 완전 불투명)
	context->OMSetDepthStencilState(depthReadState, 0);      // Depth test ON, write OFF
	//context->OMSetBlendState(nullptr, nullptr, 0xffffffff);  //  블렌딩 OFF (불투명)
	context->OMSetBlendState(alphaBlendState, nullptr, 0xffffffff);

	// ========== 렌더링 ==========
	context->Draw(m_meshVertexCount, 0);

	// ========== SRV Unbind ==========
	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	context->PSSetShaderResources(0, 2, nullSRVs);
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