#pragma once
class ToothMaskRenderer
{
public:
	bool Initialize(ID3D11Device* device, int width, int height);
	void Render(
		ID3D11DeviceContext* ctx,
		ID3D11ShaderResourceView* volumeSRV,
		ID3D11ShaderResourceView* sceneDepthSRV
	);

	ID3D11ShaderResourceView* GetMaskSRV() const;

private:
	ComPtr<ID3D11Texture2D>        m_maskTex;
	ComPtr<ID3D11RenderTargetView> m_maskRTV;
	ComPtr<ID3D11ShaderResourceView> m_maskSRV;

	ComPtr<ID3D11VertexShader> m_vs;
	ComPtr<ID3D11PixelShader>  m_ps;
};
