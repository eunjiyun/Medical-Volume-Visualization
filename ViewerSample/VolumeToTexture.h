#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class VolumeToTexture
{
public:
	VolumeToTexture();
	~VolumeToTexture();

	// 초기화
	bool Initialize(ID3D11Device* device, int width, int height);
	void Shutdown();

	// 렌더링
	void RenderVolumeToTexture(
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
		float huMax,
		ID3D11VertexShader* vs, ID3D11PixelShader* ps,
		ID3D11InputLayout* layout, ID3D11Buffer* cb
		, UINT stride, UINT offset, ID3D11Buffer* m_quadVertexBuffer

	);

	void DrawTextureToScreen(ID3D11Device* device, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context);

	// 결과 텍스처 가져오기
	//ID3D11ShaderResourceView* GetResultSRV() const { return m_resultSRV.Get(); }
	//ID3D11Texture2D* GetResultTexture() const { return m_resultTexture.Get(); }

	// 크기 조정
	void Resize(ID3D11Device* device, int width, int height);

private:
	// 리소스 생성/해제
	bool CreateRenderTarget(ID3D11Device* device, int width, int height);
	void ReleaseRenderTarget();

	bool CreateFullscreenQuad(ID3D11Device* device);
	void ReleaseFullscreenQuad();

	bool LoadShaders(ID3D11Device* device);
	void ReleaseShaders();

	bool CreateConstantBuffer(ID3D11Device* device);
	void ReleaseConstantBuffer();

	bool CreateSamplers(ID3D11Device* device);
	void ReleaseSamplers();


	//bool InitSimpleQuad(ID3D11Device* device);

public:
	// Render Target
	ID3D11Texture2D* m_resultTexture{ nullptr };
	ID3D11RenderTargetView* m_resultRTV{ nullptr };
	ID3D11ShaderResourceView* m_resultSRV{ nullptr };


	// Fullscreen Quad
	ID3D11Buffer* m_quadVertexBuffer{ nullptr };
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// Shaders
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;

	// Constant Buffer
	struct CBData
	{
		XMMATRIX InvView;
		XMMATRIX InvProj;
		XMMATRIX InvVolumeWorld;
		XMMATRIX View;
		XMMATRIX Projection;
		XMFLOAT4 CameraPosAndAlpha;  // xyz = pos, w = mode
		XMFLOAT4 VoxelAndMaxSteps;   // xyz = voxel, w = maxSteps
		XMFLOAT4 HuParams;            // z = min, w = max
	};
	ComPtr<ID3D11Buffer> m_constantBuffer;

	// Samplers
	ComPtr<ID3D11SamplerState> m_linearSampler;
	ComPtr<ID3D11SamplerState> m_pointClampSampler;

	// ⭐ Simple Quad 리소스 (텍스처를 화면에 그리기용)
	struct SimpleVertex
	{
		XMFLOAT2 pos;
		XMFLOAT2 uv;
	};

	//ComPtr<ID3D11Buffer> m_simpleQuadVB;
	//ComPtr<ID3D11InputLayout> m_simpleQuadLayout;
	//ComPtr<ID3D11VertexShader> m_simpleQuadVS;
	//ComPtr<ID3D11PixelShader> m_simpleQuadPS;


	// Viewport
	int m_width;
	int m_height;

	// State
	bool m_initialized;

public:
	float ctBlendStrength{ 0.3f };//0.15~0.3
};