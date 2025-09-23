//#define __cplusplus 201103L

#include "FileReader.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>
#include <filesystem>
#include<iterator>
#include<algorithm>

using namespace std;

FileReader::FileReader() 
	: m_width{ 0 },
m_height{ 0 },
m_depth{ 0 },
m_volumeData(),
m_filePaths()
{
	// ??諛댁뎽???????β돦裕?????裕??貫?껆뵳???⑤객臾??怨쀫츊???띠럾???	
	std::cout << "[FileReader] Initialized with empty volume and file list." << std::endl;
}


bool FileReader::LoadDICOMSeries(const std::string& folderPath)
{
	m_filePaths.clear();
	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		if (entry.path().extension() == ".dcm") {
			m_filePaths.push_back(entry.path().string());
		}
	}

	m_depth = static_cast<int>(m_filePaths.size());
	m_volumeData.resize(m_width * m_height * m_depth); // ??瑜곥돡???筌먐쇰꼪?????깃꼍???브퀗???
	return BuildVolume();

}
bool FileReader::ParseSlice(std::string& filePath, int sliceIndex)
{
	DcmFileFormat file;

	//D:\Data\?좎씠吏 cr guide ?꾨줈?앺듃\DICOM
	filePath = "D:\\Data\\?좎씠吏 cr guide ?꾨줈?앺듃\\DICOM";
	OFCondition status = file.loadFile(filePath.c_str());
	if (!status.good()) {
		std::cerr << "Failed to load DICOM file: " << filePath << std::endl;
		return false;
	}

	DcmDataset* dataset = file.getDataset();
	const Uint16* pixelData = nullptr;
	status = dataset->findAndGetUint16Array(DCM_PixelData, pixelData);
	if (!status.good() || pixelData == nullptr) {
		std::cerr << "Failed to get pixel data from: " << filePath << std::endl;
		return false;
	}
	dataset->findAndGetUint16(DCM_Rows, m_height);     // ?筌뤾퍔夷?	dataset->findAndGetUint16(DCM_Columns, m_width);   // ?띠럾???
	// ???곕뻣: sliceIndex????⑤벡逾??곌램?????곌랜踰딀쾮?	int sliceSize = m_width * m_height;
	std::copy(pixelData, pixelData + sliceSize, m_volumeData.begin() + sliceIndex * sliceSize);

	return true;
}

bool FileReader::BuildVolume()
{
	for (int i = 0; i < m_filePaths.size(); ++i) {
		std::string path = m_filePaths[i];
		if (!ParseSlice(path, i)) {
			std::cerr << "Failed to parse slice: " << path << std::endl;
			return false;
		}
	}
	return true;
}
void FileReader::PrintMetadata()// ??瑜곸겱 ?筌먲퐢沅? modality ??
{
	//?熬곣뫗逾??	//??瑜곷턄
	//?롪틵?????


	DcmFileFormat file;
	OFCondition status = file.loadFile(m_filePaths[0].c_str()); // 嶺??뺢퀡??????逾??リ옇??
	if (!status.good()) {
		std::cerr << "Failed to load DICOM file: " << m_filePaths[0] << std::endl;
		return;
	}

	DcmDataset* dataset = file.getDataset();
	OFString patientID, patientAge, studyDate;

	// ??瑜곸겱 ID
	if (dataset->findAndGetOFString(DCM_PatientID, patientID).good())
		std::cout << "Patient ID: " << patientID << std::endl;

	// ??瑜곷턄
	if (dataset->findAndGetOFString(DCM_PatientAge, patientAge).good())
		std::cout << "Patient Age: " << patientAge << std::endl;

	// ?롪틵?????	if (dataset->findAndGetOFString(DCM_StudyDate, studyDate).good())
		std::cout << "Study Date: " << studyDate << std::endl;
}


std::vector<uint8_t> FileReader::GenerateAxialSlice(int zIndex)
{
	// ?????怨룸츩 ??????ｌ뫒亦?	int sliceSize = m_width * m_height;

	// ???沅???⑥щ턄??? ??怨몃굵 ?뺢껴?㎬땻???ル∥??	std::vector<uint16_t> rawSlice(sliceSize);


	std::copy(
		m_volumeData.begin() + zIndex * m_width * m_height,
		m_volumeData.begin() + (zIndex + 1) * m_width * m_height,
		rawSlice.begin()
	);

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized);
	return normalized;

}
std::vector<uint8_t> FileReader::GenerateCoronalSlice(int yIndex)
{
	std::vector<uint16_t> rawSlice(m_width * m_depth);
	for (int z = 0; z < m_depth; ++z) {
		for (int x = 0; x < m_width; ++x) {
			rawSlice[z * m_width + x] = m_volumeData[z * m_width * m_height + yIndex * m_width + x];
		}
	}

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized);
	return normalized;

}
std::vector<uint8_t> FileReader::GenerateSagittalSlice(int xIndex)
{
	std::vector<uint16_t> rawSlice(m_height * m_depth);
	for (int z = 0; z < m_depth; ++z) {
		for (int y = 0; y < m_height; ++y) {
			rawSlice[z * m_height + y] = m_volumeData[z * m_width * m_height + y * m_width + xIndex];
		}
	}

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized);
	return normalized;

}

bool FileReader::NormalizeSlice(const std::vector<uint16_t>& rawSlice, std::vector<uint8_t>& outSlice)
{
	if (rawSlice.empty()) return false;

	uint16_t minVal = *std::min_element(rawSlice.begin(), rawSlice.end());
	uint16_t maxVal = *std::max_element(rawSlice.begin(), rawSlice.end());

	outSlice.resize(rawSlice.size());

	for (size_t i = 0; i < rawSlice.size(); ++i) {
		outSlice[i] = static_cast<uint8_t>(
			255.0 * (rawSlice[i] - minVal) / (maxVal - minVal + 1e-5)
			);
	}

	return true;
}

ID3D11ShaderResourceView* FileReader::CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice)
{
	if (slice.empty()) return nullptr;

	// Direct3D 디바이스가 필요합니다. 외부에서 전달받거나 클래스 멤버로 있어야 합니다.
	//extern ID3D11Device* g_pd3dDevice; // 또는 this->m_device 등으로 처리

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8_UNORM; // 8비트 grayscale
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = slice.data();
	initData.SysMemPitch = width * sizeof(uint8_t);

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, &initData, &texture);
	if (FAILED(hr)) {
		std::cerr << "Failed to create texture from slice." << std::endl;
		return nullptr;
	}

	ID3D11ShaderResourceView* textureView = nullptr;
	hr = g_pd3dDevice->CreateShaderResourceView(texture, nullptr, &textureView);
	texture->Release(); // 뷰가 참조하므로 원본은 해제

	if (FAILED(hr)) {
		std::cerr << "Failed to create shader resource view." << std::endl;
		return nullptr;
	}

	return textureView;
}


