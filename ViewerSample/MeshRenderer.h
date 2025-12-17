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

struct ClipSettings {
	DirectX::XMFLOAT4 clipPlane;  // (nx, ny, nz, d)
	int enableClip;
	DirectX::XMFLOAT3 padding;
};


class MeshRenderer
{
public:
	MeshRenderer() {};
	~MeshRenderer(){};
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

public:
	void CreateTwoPassStates(ID3D11Device* device);
	//void RenderMeshTwoPass(ID3D11DeviceContext* context);

	void RenderMesh(ID3D11DeviceContext* context, ID3D11Buffer* m_meshVertexBuffer,
		ID3D11VertexShader* m_meshVS, ID3D11PixelShader* m_meshPS, ID3D11InputLayout* m_meshInputLayout,
		ID3D11Buffer* m_clipSettingsBuffer, ID3D11Buffer* m_meshConstantBuffer, ID3D11ShaderResourceView* m_meshTexture,
		ID3D11SamplerState* m_MeshSamplerState, ID3D11Device* m_pDevice, int m_meshVertexCount,
		float maxMesh, float maxPhysicalVol, float overallSize,
		XMMATRIX w, XMMATRIX v, XMMATRIX p);
	void Cleanup();
};

