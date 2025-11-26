#pragma once
#include<vector>
#include<string>
#include<unordered_map>
#include "stdafx.h"
#include <dcmtk/ofstd/ofstring.h>

using namespace std;


struct CrosshairData
{
    //DirectX::XMFLOAT2 cross0;       // tex0용 십자선 위치 (정규화된 UV 좌표)
    //DirectX::XMFLOAT2 cross1;       // tex1용
    //DirectX::XMFLOAT2 cross2;       // tex2용
    //DirectX::XMFLOAT2 cross3;       // tex3용

    //float crossThickness;           // 십자선 두께 (예: 0.002f)
    //DirectX::XMFLOAT4 crossColor;   // 십자선 색상 (예: 빨강 float4(1,0,0,1))

    DirectX::XMFLOAT2 crossUV;
    float crossThickness;
    DirectX::XMFLOAT4 crossColor;

};

struct ViewInfo {
    DirectX::XMFLOAT3 origin;
    DirectX::XMFLOAT3 spacing;
    DirectX::XMFLOAT3 imageSize;
    /*int sliceIndex;*/

    DirectX::XMFLOAT3 centerPatientCoord[4];
	DirectX::XMFLOAT3 rowDir, colDir;
};



class FileReader {
public:
    FileReader();
public:
    std::vector<std::string> m_filePaths;
    std::vector<uint16_t> m_volumeData;
    UINT16 m_width;
    UINT16 m_height;
    int m_depth = 0;

    uint16_t sliceSize = m_width * m_height;
    std::vector<uint16_t> rawSlice;

	OFString rawName;   // DCMTK에서 사용하는 문자열 타입 (내부적으로 std::string 기반)

    OFString patientName, birthDate, studyDate, kvp;
    OFString patientID, patientMF, patientAge;
    std::vector<uint8_t> axialSlice;
    std::vector < ID3D11Texture2D*> axialTexture, coronalTexture, sagittalTexture;
    int windowCenter, windowWidth;
	int volWC, volWW;

	float m_rescaleSlope, m_rescaleIntercept;

    ID3D11Buffer* m_crosshairBuffer = nullptr;

    int sliceIndex[4], currentIndex[4];

    ID3D11Device* d3dDevice = nullptr;
    std::unordered_map<int, ID3D11Texture2D*> axialTextureCache, coronalTextureCache, sagittalTextureCache;


    //ViewInfo views[4]; // viewIndex로 접근
    ViewInfo views; // viewIndex로 접근


	//Sint16 m_minPixelValue{ 32767 };// 또는 Uint16
	//Sint16 m_maxPixelValue{ -32768 };  // 또는 Uint16
	//float minHU, maxHU;
public:
    //bool LoadDICOMSeries(const std::string& folderPath);
    bool LoadDICOMSeries(std::string folderPath, ID3D11Device* g_pd3dDevice);

    bool ParseSlice(std::string filePath, int sliceIndex);
    ID3D11Texture2D* getOrCreateAxialTexture(int z);
    ID3D11Texture2D* getOrCreateCoronalTexture(int y);
    ID3D11Texture2D* getOrCreateSagittalTexture(int x);

    bool BuildVolume();
    void PrintMetadata();
    std::vector<uint8_t> GenerateAxialSlice(int zIndex);
    std::vector<uint8_t> GenerateCoronalSlice(int yIndex);
    std::vector<uint8_t> GenerateSagittalSlice(int xIndex);


    //bool NormalizeSlice(const std::vector<uint16_t>& rawSlice, std::vector<uint8_t>& outSlice, uint16_t globalMin, uint16_t globalMax);

    void SliceIdxManage();
    void SetAxialSlice(int index);
    void SetCoronalSlice(int index);
    void SetSagittalSlice(int index);

    void UpdateAxialTexture(int z);
    void UpdateCoronalTexture(int y);
    void UpdateSagittalTexture(int x);

    bool NormalizeSlice(const std::vector<uint16_t>& rawSlice,
        std::vector<uint8_t>& outSlice,
        float windowCenter,
        float windowWidth);

	// 📌 1. 볼륨 크기만큼 16비트 배열 확보
	std::vector<uint16_t> normalizedU16Data/*(m_width * m_height * m_depth)*/;

	bool NormalizeVolumeU16(
		const std::vector<uint16_t>& rawVolume,
		std::vector<uint16_t>& outVolume,
		float rescaleSlope,
		float rescaleIntercept,
		float windowMinHU = -1000.0f,
		float windowMaxHU = 3000.0f);


    ID3D11Texture2D* CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice);
};

