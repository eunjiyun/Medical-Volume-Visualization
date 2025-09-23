#pragma once
#include<vector>
#include "stdafx.h"

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
	std::vector<uint16_t> rawSlice; // 반드시 있어야 함!

public:
	bool LoadDICOMSeries(const std::string& folderPath);
	bool ParseSlice(std::string& filePath, int sliceIndex);
	bool BuildVolume();
	void PrintMetadata();
	std::vector<uint8_t> GenerateAxialSlice(int zIndex);
	std::vector<uint8_t> GenerateCoronalSlice(int yIndex);
	std::vector<uint8_t> GenerateSagittalSlice(int xIndex);
	bool NormalizeSlice(const std::vector<uint16_t>& rawSlice, std::vector<uint8_t>& outSlice);
	ID3D11ShaderResourceView* CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice);
};

