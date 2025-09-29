#include "FileReader.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/ofstd/ofcond.h>

#include <dcmtk/dcmdata/dctypes.h>
#include <dcmtk/config/osconfig.h>  // 플랫폼별 타입 정의
#include <filesystem>
#include<iterator>
#include<algorithm>
#include<iterator>
#include<numeric>



FileReader::FileReader() 
	: m_width{ 0 },
m_height{ 0 },
m_depth{ 0 },
m_volumeData()/*,
m_filePaths()*/
{
	// ??獄쏅똻????????棺??짆?????獒??縕?猿녿뎨????ㅺ컼????⑥レ툓????좊읈???	
	std::cout << "[FileReader] Initialized with empty volume and file list." << std::endl;
}


bool FileReader::LoadDICOMSeries(std::string folderPath,ID3D11Device* g_pd3dDevice)
{
	cout << "this : " << this << endl;


	if (!m_filePaths.empty())
		//if(0<m_filePaths.size())
		m_filePaths.clear();

	
	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		//if (entry.path().extension() == ".dcm") 
		if (entry.is_regular_file() && entry.path().extension()== ".dcm")
		{
			m_filePaths.push_back(entry.path().string());


			// width, height는 첫 번째 파일에서만 읽기
            if (entry.path().string() == m_filePaths.front())
            {
				
				DcmFileFormat file;
				OFCondition status = file.loadFile(/*folderPath + "0000.dcm"*/entry.path().string());

				DcmDataset* dataset = file.getDataset();

				// 메타데이터 출력 (선택 사항)
				OFString widthStr, heightStr;
				dataset->findAndGetOFString(DCM_Columns, widthStr);   // (0028,0011)
				dataset->findAndGetOFString(DCM_Rows, heightStr);      // (0028,0010)


				m_width = std::stoi(widthStr.c_str());
				m_height = std::stoi(heightStr.c_str());

				std::cout << "Width: " << m_width << ", Height: " << m_height << std::endl;
			}
		}
	}

	m_depth = static_cast<int>(m_filePaths.size());
	m_volumeData.resize(m_width * m_height * m_depth);

	for (int i{}; i < m_depth; ++i) {
		std::string path = m_filePaths[i];
		if (!ParseSlice(path, i)) {
			std::cerr << "Failed to parse slice " << i << std::endl;
			return false;
		}
	}

    ComputeGlobalMinMax(); // 로딩 직후 전체 min/max 계산

    int zIndex = m_depth / 2; // 가운데 슬라이스
    std::vector<uint8_t> axialSlice = GenerateAxialSlice(zIndex);
    axialTexture = CreateTextureFromSlice(axialSlice, m_width, m_height, g_pd3dDevice);

    int zIndexC = m_depth / 2; // 가운데 슬라이스
    std::vector<uint8_t> coronalSlice = GenerateCoronalSlice(zIndexC);
    coronalTexture = CreateTextureFromSlice(coronalSlice, m_width, m_depth, g_pd3dDevice);

    int zIndexS = m_depth / 2; // 가운데 슬라이스
    std::vector<uint8_t> sagittalSlice = GenerateSagittalSlice(zIndexS);
    sagittalTexture = CreateTextureFromSlice(sagittalSlice, m_height, m_depth, g_pd3dDevice);

	return true;
}


bool FileReader::ParseSlice(const std::string path, int sliceIndex) {
	DcmFileFormat file;
	OFCondition status = file.loadFile(path.c_str());

	if (!status.good()) {
		std::cerr << "❌ Failed to load DICOM file: " << path << std::endl;
		return false;
	}

	DcmDataset* dataset = file.getDataset();

	// 픽셀 데이터 가져오기
	const Uint16* pixelData = nullptr;
	status = dataset->findAndGetUint16Array(DCM_PixelData, pixelData);
	if (!status.good() || nullptr==pixelData ) {
		std::cerr << "❌ Failed to get pixel data from: " << path << std::endl;
		return false;
	}

	// 슬라이스 위치 계산
	int sliceSize = m_width * m_height;
	int offset = sliceIndex * sliceSize;

	// 픽셀 복사
	for (int i{}; i < sliceSize; ++i) {
		m_volumeData[offset + i] = pixelData[i];
	}

   


	// 메타데이터 출력 (선택 사항)
	//OFString patientName, birthDate, studyDate, kvp;
	dataset->findAndGetOFString(DCM_PatientName, patientName);
	dataset->findAndGetOFString(DCM_PatientBirthDate, birthDate);
	dataset->findAndGetOFString(DCM_StudyDate, studyDate);
//	dataset->findAndGetOFString(DCM_KVP, kvp);



    // 추가 정보
    dataset->findAndGetOFString(DCM_PatientID, patientID);       // 환자 ID
    dataset->findAndGetOFString(DCM_PatientSex, patientMF);     // 성별 (M/F/O)
   // dataset->findAndGetOFString(DCM_PatientAge, patientAge);     // 나이 (예: "032Y")


	//std::string birthYear = birthDate.substr(0, 4);

	std::cout << " DICOM Metadata for slice " << sliceIndex << std::endl;
	std::cout << " Patient Name: " << patientName << std::endl;
	std::cout << " Birth Date:   " << birthDate << std::endl;
	std::cout << " Study Date:  " << studyDate << std::endl;
	std::cout << " DCM_PatientAge:          " << patientAge <<  std::endl;
    std::cout << " DCM_PatientBirthDate:          " << birthDate << std::endl;

	return true;
}

bool FileReader::BuildVolume()
{
	for (int i = 0; i < m_filePaths.size(); ++i) {
		std::string path = m_filePaths[i];
		if (!ParseSlice(path, i)) {
			std::cerr << "BuildVolume : Failed to parse slice: " << path << std::endl;
			return false;
		}
	}
	return true;
}
void FileReader::PrintMetadata()// ???쒓낯寃??嶺뚮㉡?€쾮? modality ??
{
	//??ш끽維쀩??	//???쒓낮??	//?濡ろ떟?????


	DcmFileFormat file;
	OFCondition status = file.loadFile(m_filePaths[0].c_str()); // 癲??類???????????れ삀??
	if (!status.good()) {
		std::cerr << "PrintMetadata : Failed to load DICOM file: " << m_filePaths[0] << std::endl;
		return;
	}

	DcmDataset* dataset = file.getDataset();
	OFString patientID, patientAge, studyDate;

	// ???쒓낯寃?ID
	if (dataset->findAndGetOFString(DCM_PatientID, patientID).good())
		std::cout << "Patient ID: " << patientID << std::endl;

	// ???쒓낮??	if (dataset->findAndGetOFString(DCM_PatientAge, patientAge).good())
		std::cout << "Patient Age: " << patientAge << std::endl;

	// ?濡ろ떟?????	if (dataset->findAndGetOFString(DCM_StudyDate, studyDate).good())
		std::cout << "Study Date: " << studyDate << std::endl;
}


std::vector<uint8_t> FileReader::GenerateAxialSlice(int zIndex)
{
	// ??????⑤８痢???????節뚮쳮雅?	int sliceSize = m_width * m_height;

	// ???亦????Β????? ???⑤챶援??類?뺨??щ빝????モ닪??	std::vector<uint16_t> rawSlice(sliceSize);

    rawSlice.resize(m_width * m_height); // 먼저 크기 확보
	std::copy(
		m_volumeData.begin() + zIndex * m_width * m_height,
		m_volumeData.begin() + (zIndex + 1) * m_width * m_height,
		rawSlice.begin()
	);

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized, m_globalMin, m_globalMax);
	return normalized;

}
//std::vector<uint8_t> FileReader::GenerateCoronalSlice(int yIndex)
//{
//
//	/*std::vector<uint16_t> rawSlice(m_width * m_depth);
//	for (int z = 0; z < m_depth; ++z) {
//		for (int x = 0; x < m_width; ++x) {
//			rawSlice[z * m_width + x] = m_volumeData[z * m_width * m_height + yIndex * m_width + x];
//		}
//	}
//
//	std::vector<uint8_t> normalized;
//	NormalizeSlice(rawSlice, normalized);
//	return normalized;*/
//
//
//    std::vector<uint16_t> rawSlice(m_width * m_depth);
//
//   // for (int z = 0; z < m_depth; ++z) {
//        std::copy(
//            m_volumeData.begin() + yIndex * m_width * m_height + yIndex * m_width,
//            m_volumeData.begin() + yIndex * m_width * m_height + (yIndex + 1) * m_width,
//            rawSlice.begin() + yIndex * m_width
//        );
//   // }
//
//    std::vector<uint8_t> normalized;
//    NormalizeSlice(rawSlice, normalized);
//    return normalized;
//
//}

std::vector<uint8_t> FileReader::GenerateCoronalSlice(int yIndex) {
    int sliceSize = m_width * m_depth;
    std::vector<uint16_t> rawSlice(sliceSize);

    for (int z{}; z < m_depth; ++z) {
        for (int x{}; x < m_width; ++x) {
            size_t srcIndex = z * (m_width * m_height) + yIndex * m_width + x;
            size_t dstIndex = z * m_width + x;
            rawSlice[dstIndex] = m_volumeData[srcIndex];
        }
    }

    std::vector<uint8_t> normalized;
    NormalizeSlice(rawSlice, normalized, m_globalMin, m_globalMax);
    return normalized;
}

//std::vector<uint8_t> FileReader::GenerateSagittalSlice(int xIndex)
//{
//	//std::vector<uint16_t> rawSlice(m_height * m_depth);
//	//for (int z = 0; z < m_depth; ++z) {
//	//	for (int y = 0; y < m_height; ++y) {
//	//		rawSlice[z * m_height + y] = m_volumeData[z * m_width * m_height + y * m_width + xIndex];
//	//	}
//	//}
//
//	//std::vector<uint8_t> normalized;
//	//NormalizeSlice(rawSlice, normalized);
//	//return normalized;
//
//
//
//    std::vector<uint16_t> rawSlice(m_height * m_depth);
//
//   // for (int z = 0; z < m_depth; ++z) {
//        for (int y = 0; y < m_height; ++y) {
//            rawSlice[z * m_height + y] = m_volumeData[z * m_width * m_height + y * m_width + xIndex];
//        }
//    //}
//
//    std::vector<uint8_t> normalized;
//    NormalizeSlice(rawSlice, normalized);
//    return normalized;
//}

std::vector<uint8_t> FileReader::GenerateSagittalSlice(int xIndex) {
    int sliceSize = m_height * m_depth;
    std::vector<uint16_t> rawSlice(sliceSize);

    for (int z{}; z < m_depth; ++z) {
        for (int y{}; y < m_height; ++y) {
            size_t srcIndex = z * (m_width * m_height) + y * m_width + xIndex;
            size_t dstIndex = z * m_height + y;
            rawSlice[dstIndex] = m_volumeData[srcIndex];
        }
    }

    std::vector<uint8_t> normalized;
    NormalizeSlice(rawSlice, normalized, m_globalMin, m_globalMax);
    return normalized;
}


bool FileReader::NormalizeSlice(const std::vector<uint16_t>& rawSlice, std::vector<uint8_t>& outSlice, uint16_t globalMin, uint16_t globalMax)
{
	if (rawSlice.empty()) return false;


	/*uint16_t minVal = *std::min_element(rawSlice.begin(), rawSlice.end());
	uint16_t maxVal = *std::max_element(rawSlice.begin(), rawSlice.end());*/

	outSlice.resize(rawSlice.size());

	for (size_t i = 0; i < rawSlice.size(); ++i) {
		outSlice[i] = static_cast<uint8_t>(
			255.0 * (rawSlice[i] - globalMin) / (globalMax - globalMin + 1e-5)
			);
	}

	return true;
}

ID3D11Texture2D* FileReader::CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice)
{
	if (slice.empty()) return nullptr;
    
	// Direct3D ?붾컮?댁뒪媛 ?꾩슂?⑸땲?? ?몃??먯꽌 ?꾨떖諛쏄굅???대옒??硫ㅻ쾭濡??덉뼱???⑸땲??
	//extern ID3D11Device* g_pd3dDevice; // ?먮뒗 this->m_device ?깆쑝濡?泥섎━

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	//texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //8鍮꾪듃 grayscale
    texDesc.Format = DXGI_FORMAT_R8_UNORM; //8鍮꾪듃 grayscale

	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	//texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;


	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = slice.data();
	//initData.SysMemPitch = 4*width * sizeof(uint8_t);
    initData.SysMemPitch = width * sizeof(uint8_t);

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, &initData, &texture);
	if (FAILED(hr)) {
		std::cerr << "Failed to create texture from slice." << std::endl;
		return nullptr;
	}



	//ID3D11ShaderResourceView* textureView = nullptr;
	//hr = g_pd3dDevice->CreateShaderResourceView(texture, nullptr, &textureView);
	//texture->Release(); // 酉곌? 李몄“?섎?濡??먮낯? ?댁젣

	//if (FAILED(hr)) {
	//	std::cerr << "Failed to create shader resource view." << std::endl;
	//	return nullptr;
	//}

	return texture;
}

void FileReader::ComputeGlobalMinMax() {
    if (m_volumeData.empty()) return;

    auto[minIt, maxIt] = std::minmax_element(m_volumeData.begin(), m_volumeData.end());
    m_globalMin = *minIt;
    m_globalMax = *maxIt;
}


