#pragma once
// TransferFunction.h

#include<iostream>
#include <vector>
#include "stdafx.h"
class TransferFunction {
private:
	struct TFPoint {
		float value;      // HU 값 (0.0~1.0로 정규화)
		float r, g, b, a; // RGBA
	};

	std::vector<TFPoint> m_controlPoints;
	ID3D11Texture1D* m_tfTexture;
	ID3D11ShaderResourceView* m_tfSRV;
	static const int TF_SIZE = 256;
public:

	TransferFunction();
	~TransferFunction();

	bool Initialize(float center, float width, ID3D11Device* device);


	void SetHUWindow(float center, float width, ID3D11Device* g_pd3dDevice);

	void UpdateTexture(ID3D11Device* g_pd3dDevice);

	ID3D11ShaderResourceView* GetSRV() const { return m_tfSRV; }
	void SetBonePreset(ID3D11Device* g_pd3dDevice);
};
