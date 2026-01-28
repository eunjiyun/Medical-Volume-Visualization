#pragma once
#include "stdafx.h"
#include <DirectXMath.h>
using namespace DirectX;


struct MeshConstantBuffer
{
	DirectX::XMMATRIX WVP;
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX WorldView;  // ✅ 추가
};


struct MeshConstantBufferWithCT
{
	XMMATRIX WVP;
	XMMATRIX View;
	XMMATRIX World;  // ✅ 추가: View 행렬
	XMFLOAT4 CTBlendParams;  // ⭐ x = strength, yzw = unused
};

struct ClipSettings {
	DirectX::XMFLOAT4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	DirectX::XMFLOAT3 padding;
};


struct Axes
{
	XMVECTOR X;
	XMVECTOR Y;
	XMVECTOR Z;
};



class MeshRenderer
{
public:
	MeshRenderer() {};
	~MeshRenderer() {};
	// ========== Two-Pass States ==========
	ID3D11DepthStencilState* m_depthWriteState;   // Pass 1: Depth Write
	ID3D11DepthStencilState* m_depthReadState;    // Pass 2: Depth Read Only
	ID3D11BlendState* m_noColorWriteState;        // Pass 1: Color Off
	ID3D11BlendState* m_alphaBlendState;          // Pass 2: Alpha Blend

	ID3D11RasterizerState* rastState{ nullptr };

	ID3D11DepthStencilState* depthWriteState{ nullptr };
	ID3D11BlendState* noColorWriteState{ nullptr };
	ID3D11DepthStencilState* depthReadState{ nullptr };
	ID3D11BlendState* alphaBlendState{ nullptr };

	ID3D11SamplerState* m_PointClampSampler{ nullptr };

	ID3D11Texture2D* m_faceColorTex;

	ID3D11RenderTargetView* m_faceColorRTV;

	ID3D11ShaderResourceView* m_faceColorSRV;

	XMMATRIX centerTranslate = XMMatrixTranslation(
		1,
		1,
		1
	);
	float centerX;
	float centerY;
	float centerZ;
	// ⭐ 메쉬 셰이더는 여기서 관리
	ID3D11VertexShader* m_meshVS;  // FaceMesh_WithCT_VS
	ID3D11PixelShader* m_meshPS;   // FaceMesh_WithCT_PS

	float meshWidth, meshHeight, meshDepth;
	//float meshScale{ /*0.006755915f*/ };
	//float meshScale{ 13.f };
	float meshScale{ 1.f };

	ID3D11Texture2D*	sceneDepthTexture=nullptr;


	XMVECTOR rotY, rotX;
	DirectX::XMMATRIX rotation;
	XMMATRIX volWorldMat;
public:
	void CreateTwoPassStates(ID3D11Device* device);
	//void RenderMeshTwoPass(ID3D11DeviceContext* context);
	float ComputeHandedness(XMVECTOR X, XMVECTOR Y, XMVECTOR Z);
	//bool ExtractAxes(XMMATRIX* volWorld, XMMATRIX* meshWorld);
	bool ExtractAxes(const XMMATRIX* volWorld, const XMMATRIX* meshWorld);
	void RenderMeshDepth(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
		ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11RenderTargetView* sceneDepthRTV ,ID3D11InputLayout* m_meshInputLayout,
		ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11Texture2D* m_meshTexture, ID3D11ShaderResourceView* m_meshDepthSRV,
		ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
		float maxMesh, float maxPhysicalVol, float volWidth, float volheight, float volDepth, float overallSize,
		XMMATRIX w, XMMATRIX v, XMMATRIX p, float width, float height);


	void RenderMeshViewZ(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
		ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11RenderTargetView* sceneDepthRTV, ID3D11InputLayout* m_meshInputLayout,
		ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11Texture2D* m_meshTexture, ID3D11ShaderResourceView* m_meshDepthSRV,
		ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
		float maxMesh, float maxPhysicalVol, float volWidth, float volheight, float volDepth, float overallSize,
		XMMATRIX w, XMMATRIX v, XMMATRIX p, float width, float height);







	DirectX::XMMATRIX initialMeshWorld, meshWorldMat;
	void RenderMesh(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
		ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11InputLayout* m_meshInputLayout,
		ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11ShaderResourceView* m_meshTexture,
		ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
		float maxMesh, float maxPhysicalVol, float volWidth, float volHeight, float volDepth, float overallSize,
		XMMATRIX w, XMMATRIX v, XMMATRIX p);
	MeshConstantBufferWithCT cbM;
	float faceBlend{ 0.5f };
	void MeshRenderer::RenderMeshWithCT(
		ID3D11DeviceContext* context,
		ID3D11Buffer* m_meshVertexBuffer,
		ID3D11VertexShader* m_meshVS,
		ID3D11PixelShader* m_meshPS,
		ID3D11InputLayout* m_meshInputLayout,
		ID3D11Buffer* m_clipSettingsBuffer,
		ID3D11Buffer* m_meshConstantBuffer,
		ID3D11ShaderResourceView* m_meshTexture,      // 얼굴 텍스처
		ID3D11ShaderResourceView* ctTexture,          // ⭐ CT 텍스처
		ID3D11ShaderResourceView* 
		,
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
		float ctBlendStrength);
	void Cleanup();
};

