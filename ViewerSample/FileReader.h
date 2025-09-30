#pragma once
#include<vector>
#include<string>
#include "stdafx.h"

#include <dcmtk/ofstd/ofstring.h>
using namespace std;


struct CrosshairData
{
    DirectX::XMFLOAT2 cross0;       // tex0용 십자선 위치 (정규화된 UV 좌표)
    DirectX::XMFLOAT2 cross1;       // tex1용
    DirectX::XMFLOAT2 cross2;       // tex2용
    DirectX::XMFLOAT2 cross3;       // tex3용

    float crossThickness;           // 십자선 두께 (예: 0.002f)
    DirectX::XMFLOAT4 crossColor;   // 십자선 색상 (예: 빨강 float4(1,0,0,1))
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
    //Uint16* m_pixelData = nullptr;

    uint16_t m_globalMin = 0;
    uint16_t m_globalMax = 0;



    uint16_t sliceSize = m_width * m_height;
    std::vector<uint16_t> rawSlice; // ?꾩룇瑗띈キ?????곗꽑????


    OFString patientName, birthDate, studyDate, kvp;
    OFString patientID, patientMF, patientAge;
    std::vector<uint8_t> axialSlice;
    ID3D11Texture2D* axialTexture, *coronalTexture, *sagittalTexture;
    int windowCenter, windowWidth;

    ID3D11Buffer* m_crosshairBuffer = nullptr;

public:
    //bool LoadDICOMSeries(const std::string& folderPath);
    bool LoadDICOMSeries(std::string folderPath, ID3D11Device* g_pd3dDevice);

    bool ParseSlice(std::string filePath, int sliceIndex);
    bool BuildVolume();
    void PrintMetadata();
    std::vector<uint8_t> GenerateAxialSlice(int zIndex);
    std::vector<uint8_t> GenerateCoronalSlice(int yIndex);
    std::vector<uint8_t> GenerateSagittalSlice(int xIndex);


    //bool NormalizeSlice(const std::vector<uint16_t>& rawSlice, std::vector<uint8_t>& outSlice, uint16_t globalMin, uint16_t globalMax);



    bool NormalizeSlice(const std::vector<uint16_t>& rawSlice,
        std::vector<uint8_t>& outSlice,
        float windowCenter,
        float windowWidth);




    ID3D11Texture2D* CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice);

    void ComputeGlobalMinMax();


};

