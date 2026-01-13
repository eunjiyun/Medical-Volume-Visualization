#pragma once
#include<vector>
#include<string>
#include<unordered_map>
#include "stdafx.h"
#include <dcmtk/ofstd/ofstring.h>
// DCMTK Core
#include <dcmtk/dcmdata/dcfilefo.h>   // DcmFileFormat, DcmDataset

using namespace std;


struct CrosshairData
{
	DirectX::XMFLOAT2 crossUV;
	float crossThickness;
	float sharpness;
	DirectX::XMFLOAT4 crossColor;
};

struct ViewInfo {
	DirectX::XMFLOAT3 origin;
	DirectX::XMFLOAT3 spacing;
	DirectX::XMFLOAT3 imageSize;

	DirectX::XMFLOAT3 centerPatientCoord[4];
	DirectX::XMFLOAT3 rowDir, colDir;
};



class FileReader {
public:
	FileReader();
public:
	std::vector<std::string> m_filePaths;
	std::vector<int16_t> m_volumeData;
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

	ViewInfo views; // viewIndex로 접근

public:
	bool LoadDICOMSeries(std::string folderPath, ID3D11Device* g_pd3dDevice);


	// Helper 함수
	bool DecompressDICOM(DcmDataset* dataset);
	const Sint16* GetPixelData(DcmDataset* dataset);
	bool ParseSlice(std::string filePath, int sliceIndex);
	ID3D11Texture2D* getOrCreateAxialTexture(int z);
	ID3D11Texture2D* getOrCreateCoronalTexture(int y);
	ID3D11Texture2D* getOrCreateSagittalTexture(int x);

	bool BuildVolume();
	void PrintMetadata();
	std::vector<uint8_t> GenerateAxialSlice(int zIndex);
	std::vector<uint8_t> GenerateCoronalSlice(int yIndex);
	std::vector<uint8_t> GenerateSagittalSlice(int xIndex);

	void SliceIdxManage();
	void SetAxialSlice(int index);
	void SetCoronalSlice(int index);
	void SetSagittalSlice(int index);

	void UpdateAxialTexture(int z);
	void UpdateCoronalTexture(int y);
	void UpdateSagittalTexture(int x);

	bool NormalizeSlice(const std::vector<int16_t>& rawSlice,
		std::vector<uint8_t>& outSlice,
		float windowCenter,
		float windowWidth);

	//std::vector<uint16_t> normalizedU16Data;
	std::vector<float> floatData;

	bool NormalizeVolumeFloat(
		const std::vector<int16_t>& rawVolume,

		float rescaleSlope,
		float rescaleIntercept,
		float windowMinHU = -1000.0f,
		float windowMaxHU = 3000.0f);

	void AnalyzeHUDistribution();


	ID3D11Texture2D* CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice);
};

