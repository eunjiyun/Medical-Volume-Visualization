#include "FileReader.h"
//#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/ofstd/ofcond.h>
#include "dcmtk/ofstd/ofchrenc.h" // 문자셋 변환기

#include <dcmtk/dcmdata/dctypes.h>
//#include <dcmtk/config/osconfig.h>  
#include <filesystem>
#include<iterator>
#include<algorithm>
#include<numeric>

#include<map>


FileReader::FileReader()
	: m_width{ 0 },
	m_height{ 0 },
	m_depth{ 0 },
	m_volumeData()/*,
	m_filePaths()*/
{

	std::cout << "[FileReader] Initialized with empty volume and file list." << std::endl;
}


std::string convertCP949ToUTF8(const std::string& euckr)
{
	int lenW = MultiByteToWideChar(949, 0, euckr.c_str(), -1, NULL, 0);
	std::wstring wstr(lenW, 0);
	MultiByteToWideChar(949, 0, euckr.c_str(), -1, &wstr[0], lenW);

	int lenU8 = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string utf8(lenU8, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8[0], lenU8, NULL, NULL);

	return utf8;
}



bool FileReader::LoadDICOMSeries(std::string folderPath, ID3D11Device* g_pd3dDevice)
{
	cout << "this : " << this << endl;
	d3dDevice = g_pd3dDevice;

	if (!m_filePaths.empty()) {
		//if(0<m_filePaths.size())
		m_filePaths.clear();

		axialTextureCache.clear();
		coronalTextureCache.clear();
		sagittalTextureCache.clear();
	}


	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		//if (entry.path().extension() == ".dcm") 
		if (entry.is_regular_file() && entry.path().extension() == ".dcm")
		{
			m_filePaths.push_back(entry.path().string());

			if (entry.path().string() == m_filePaths.front())
			{


				DcmFileFormat file;
				OFCondition status = file.loadFile(/*folderPath + "0000.dcm"*/entry.path().string().c_str());

				DcmDataset* dataset = file.getDataset();



				//dataset->convertToUTF8();  // DCMTK 3.6.7 이상
				//dataset->putAndInsertString(DCM_SpecificCharacterSet, "ISO_IR 192"); // UTF-8

				OFString widthStr, heightStr;
				dataset->findAndGetOFString(DCM_Columns, widthStr);   // (0028,0011)
				dataset->findAndGetOFString(DCM_Rows, heightStr);      // (0028,0010)


				m_width = std::stoi(widthStr.c_str());
				m_height = std::stoi(heightStr.c_str());

				std::cout << "Width: " << m_width << ", Height: " << m_height << std::endl;

				OFString rawName;
				if (dataset->findAndGetOFString(DCM_PatientName, rawName).good())
				{
					std::string utf8Name = convertCP949ToUTF8(rawName.c_str());
					patientName = utf8Name.c_str(); // ✅ OFString은 std::string에서 바로 대입 가능
				}



				OFString wcStr, wwStr;
				if (dataset->findAndGetOFString(DCM_WindowCenter, wcStr).good() &&
					dataset->findAndGetOFString(DCM_WindowWidth, wwStr).good() &&
					/*dataset->findAndGetOFString(DCM_PatientName, patientName).good() &&*/
					dataset->findAndGetOFString(DCM_PatientBirthDate, birthDate).good() &&
					dataset->findAndGetOFString(DCM_StudyDate, studyDate).good() &&
					dataset->findAndGetOFString(DCM_PatientID, patientID).good() &&
					dataset->findAndGetOFString(DCM_PatientSex, patientMF).good()
					) {

					volWC = windowCenter = std::stof(wcStr.c_str());
					volWW = windowWidth = std::stof(wwStr.c_str());



					/*	std::string utf8Name = convertCP949ToUTF8(rawName.c_str());
						patientName = utf8Name.c_str();*/




						/* patientName = std::stof(wcStr.c_str());
						 birthDate = std::stof(wwStr.c_str());
						 studyDate = std::stof(wcStr.c_str());
						 patientID = std::stof(wwStr.c_str());
						 patientMF = std::stof(wcStr.c_str());*/


					std::cout << "Window Center: " << windowCenter << ", Window Width: " << windowWidth << std::endl;
				}



				OFString pixelSpacingStr, sliceThicknessStr, imagePositionStr, imageOrientationStr;


				// Pixel Spacing (0028,0030)
				if (dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacingStr).good()) {
					std::stringstream ss(pixelSpacingStr.c_str());
					std::string sx, sy;
					std::getline(ss, sx, '\\');
					if (!std::getline(ss, sy, '\\')) {
						sy = sx; // fallback: 둘 다 같은 값으로 설정
					}

					/*views[1].spacing.x = std::stof(sx);
					views[1].spacing.y = std::stof(sy);
					std::cout << "Pixel Spacing: " << views[1].spacing.x << " x " << views[1].spacing.y << std::endl;*/

					views.spacing.x = std::stof(sx);
					views.spacing.y = std::stof(sy);
					std::cout << "Pixel Spacing: " << views.spacing.x << " x " << views.spacing.y << std::endl;
				}

				//// Slice Thickness (0018,0050)
				//if (dataset->findAndGetOFString(DCM_SliceThickness, sliceThicknessStr).good()) {
				//    views[1].spacing.z = std::stof(sliceThicknessStr.c_str());
				//    std::cout << "Slice Thickness: " << views[1].spacing.z << std::endl;
				//}

				 // Slice Thickness (0018,0050)
				if (dataset->findAndGetOFString(DCM_SliceThickness, sliceThicknessStr).good()) {
					views.spacing.z = std::stof(sliceThicknessStr.c_str());
					std::cout << "Slice Thickness: " << views.spacing.z << std::endl;
				}

				//// Image Position (Patient) (0020,0032)
				//if (dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionStr).good()) {
				//    std::stringstream ss(imagePositionStr.c_str());
				//    std::string ox, oy, oz;
				//    std::getline(ss, ox, '\\');
				//    std::getline(ss, oy, '\\');
				//    std::getline(ss, oz, '\\');
				//    views[0].origin.x = std::stof(ox);
				//    views[0].origin.y = std::stof(oy);
				//    views[0].origin.z = std::stof(oz);
				//    std::cout << "Image Origin: (" << views[0].origin.x << ", " << views[0].origin.y << ", " << views[0].origin.z << ")" << std::endl;
				//}
				if (dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionStr).good()) {
					std::stringstream ss(imagePositionStr.c_str());
					std::string ox, oy, oz;

					std::getline(ss, ox, '\\');

					if (!std::getline(ss, oy, '\\')) {
						oy = "0.0"; // fallback 또는 ox와 동일하게 설정해도 됨
						std::cerr << "Warning: Missing Y value in ImagePositionPatient" << std::endl;
					}

					if (!std::getline(ss, oz, '\\')) {
						oz = "0.0"; // fallback 또는 ox와 동일하게 설정해도 됨
						std::cerr << "Warning: Missing Z value in ImagePositionPatient" << std::endl;
					}

					try {
						/* views[1].origin.x = std::stof(ox);
						 views[1].origin.y = std::stof(oy);
						 views[1].origin.z = std::stof(oz);*/

						views.origin.x = std::stof(ox);
						views.origin.y = std::stof(oy);
						views.origin.z = std::stof(oz);
					}
					catch (const std::exception& e) {
						std::cerr << "Error parsing ImagePositionPatient: " << e.what() << std::endl;
					}

					// std::cout << "Image Origin: (" << views[1].origin.x << ", " << views[1].origin.y << ", " << views[1].origin.z << ")" << std::endl;
					std::cout << "Image Origin: (" << views.origin.x << ", " << views.origin.y << ", " << views.origin.z << ")" << std::endl;

				}
				// Image Orientation (Patient) (0020,0037) - 가장 중요!

				if (dataset->findAndGetOFString(DCM_ImageOrientationPatient, imageOrientationStr).good()) {
					std::stringstream ss(imageOrientationStr.c_str());
					std::string vals[6];
					int count = 0;

					// 백슬래시로 구분하여 읽기
					std::string token;
					while (count < 6 && std::getline(ss, token, '\\')) {
						vals[count] = token;
						++count;
					}

					// 값 검증 및 출력
					std::cout << "Parsed " << count << " values:" << std::endl;
					for (int i = 0; i < count; ++i) {
						std::cout << "  vals[" << i << "] = [" << vals[i] << "]" << std::endl;
					}




					// 6개 값이 모두 있는지 확인
					if (count == 6) {
						try {
							views.rowDir.x = std::stof(vals[0]);
							views.rowDir.y = std::stof(vals[1]);
							views.rowDir.z = std::stof(vals[2]);

							views.colDir.x = std::stof(vals[3]);
							views.colDir.y = std::stof(vals[4]);
							views.colDir.z = std::stof(vals[5]);

							std::cout << "Row Dir: (" << views.rowDir.x << ", "
								<< views.rowDir.y << ", " << views.rowDir.z << ")" << std::endl;
							std::cout << "Col Dir: (" << views.colDir.x << ", "
								<< views.colDir.y << ", " << views.colDir.z << ")" << std::endl;
						}
						catch (const std::exception& e) {
							std::cerr << "Error converting to float: " << e.what() << std::endl;
							// 기본값 설정
							views.rowDir = { 1.0f, 0.0f, 0.0f };
							views.colDir = { 0.0f, 1.0f, 0.0f };
						}
					}
					else {
						std::cerr << "Error: Expected 6 values, got " << count << std::endl;
						// 기본값 설정
						views.rowDir = { 1.0f, 0.0f, 0.0f };
						views.colDir = { 0.0f, 1.0f, 0.0f };
					}
				}
				else {
					std::cerr << "ImageOrientationPatient tag not found" << std::endl;
					// 기본값 설정
					views.rowDir = { 1.0f, 0.0f, 0.0f };
					views.colDir = { 0.0f, 1.0f, 0.0f };
				}



				OFString slopeStr, interceptStr;

				// ⚙️ Rescale Slope (0028,1053)
				/*if (dataset->findAndGetOFString(DCM_RescaleSlope, slopeStr).good()) {
					m_rescaleSlope = std::stof(slopeStr.c_str());
				}
				else {*/
					m_rescaleSlope = 1.0f; // 기본값
				//}

				//// ⚙️ Rescale Intercept (0028,1052)
				//if (dataset->findAndGetOFString(DCM_RescaleIntercept, interceptStr).good()) {
				//	m_rescaleIntercept = std::stof(interceptStr.c_str());
				//}
				//else {
				m_rescaleIntercept = -1024.0f; // 기본값
			//}

				std::cout << "Rescale Slope: " << m_rescaleSlope
					<< ", Intercept: " << m_rescaleIntercept << std::endl;


			}
		}
	}

	m_depth = static_cast<int>(m_filePaths.size());
	m_volumeData.resize(m_width * m_height * m_depth);

	//views[1].imageSize = DirectX::XMFLOAT3(m_width, m_height, m_depth);
	//views[1].sliceIndex = 0; // 초기 슬라이스 인덱스 (축상 뷰 기준)

	views.imageSize = DirectX::XMFLOAT3(m_width, m_height, m_depth);
	// views.sliceIndex = 0; // 초기 슬라이스 인덱스 (축상 뷰 기준)


	for (int i{}; i < m_depth; ++i) {
		std::string path = m_filePaths[i];
		if (!ParseSlice(path, i)) {
			std::cerr << "Failed to parse slice " << i << std::endl;
			return false;
		}
	}


	AnalyzeHUDistribution();

	return true;
}

ID3D11Texture2D* FileReader::getOrCreateAxialTexture(int z) {
	if (axialTextureCache.find(z) != axialTextureCache.end()) return axialTextureCache[z];

	std::vector<uint8_t> slice = GenerateAxialSlice(z);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_width, m_height, d3dDevice);
	axialTextureCache[z] = texture;
	return texture;
}

ID3D11Texture2D* FileReader::getOrCreateCoronalTexture(int y) {
	if (coronalTextureCache.find(y) != coronalTextureCache.end()) return coronalTextureCache[y];

	std::vector<uint8_t> slice = GenerateCoronalSlice(y);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_width, m_depth, d3dDevice);
	coronalTextureCache[y] = texture;
	return texture;
}

ID3D11Texture2D* FileReader::getOrCreateSagittalTexture(int x) {
	if (sagittalTextureCache.find(x) != sagittalTextureCache.end()) return sagittalTextureCache[x];

	std::vector<uint8_t> slice = GenerateSagittalSlice(x);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_height, m_depth, d3dDevice);
	sagittalTextureCache[x] = texture;
	return texture;
}


//bool FileReader::ParseSlice(const std::string path, int sliceIndex) {
//	DcmFileFormat file;
//
//
//	OFCondition status = file.loadFile(path.c_str(), EXS_Unknown, EGL_noChange);
//
//	if (!status.good()) {
//		std::cerr << " Failed to load DICOM file: " << path << std::endl;
//		return false;
//	}
//
//	DcmDataset* dataset = file.getDataset();
//
//
//	const Sint16* pixelData = nullptr;
//	status = dataset->findAndGetSint16Array(DCM_PixelData, pixelData);
//	if (!status.good() || nullptr == pixelData) {
//		//std::cerr << " Failed to get pixel data from: " << path << std::endl;
//		std::cerr << "❌ Failed to get pixel data from: " << path << std::endl;
//		std::cerr << "🔍 Error detail: " << status.text() << std::endl;
//
//		return false;
//	}
//
//
//	int sliceSize = m_width * m_height;
//	int offset = sliceIndex * sliceSize;
//
//	for (int i{}; i < sliceSize; ++i) {
//		m_volumeData[offset + i] = pixelData[i];
//
//
//	}
//
//
//	std::cout << " DICOM Metadata for slice " << sliceIndex << std::endl;
//
//
//	return true;
//}

bool FileReader::DecompressDICOM(DcmDataset* dataset) {
	DcmXfer originalXfer(dataset->getOriginalXfer());

	if (!originalXfer.isEncapsulated()) {
		return true;
	}

	std::cout << "📦 Decompressing from: " << originalXfer.getXferName() << std::endl;

	OFCondition status = dataset->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);

	if (!status.good()) {
		std::cerr << "❌ Decompression failed: " << status.text() << std::endl;
		return false;
	}

	std::cout << "✅ Decompressed successfully" << std::endl;
	return true;
}

const Sint16* FileReader::GetPixelData(DcmDataset* dataset) {
	DcmElement* element = nullptr;
	OFCondition status = dataset->findAndGetElement(DCM_PixelData, element);

	if (!status.good() || element == nullptr) {
		std::cerr << "❌ Pixel Data element not found" << std::endl;
		return nullptr;
	}

	DcmVR vr(element->getVR());
	std::cout << "🔍 VR: " << vr.getVRName()
		<< ", Length: " << element->getLength() << " bytes" << std::endl;

	// OW, US, SS 모두 처리
	if (vr.getEVR() == EVR_OW || vr.getEVR() == EVR_US) {
		Uint16* data = nullptr;
		status = element->getUint16Array(data);

		if (status.good() && data != nullptr) {
			std::cout << "✅ Pixel data loaded (as Uint16)" << std::endl;
			return reinterpret_cast<Sint16*>(data);
		}
	}
	else if (vr.getEVR() == EVR_SS) {
		Sint16* data = nullptr;
		status = element->getSint16Array(data);

		if (status.good() && data != nullptr) {
			std::cout << "✅ Pixel data loaded (as Sint16)" << std::endl;
			return data;
		}
	}

	std::cerr << "❌ Failed to read pixel data: " << status.text() << std::endl;
	return nullptr;
}

bool FileReader::ParseSlice(const std::string path, int sliceIndex) {
	DcmFileFormat file;
	OFCondition status = file.loadFile(path.c_str());

	if (!status.good()) {
		std::cerr << "❌ Failed to load: " << path << std::endl;
		return false;
	}

	DcmDataset* dataset = file.getDataset();

	// 압축 해제
	if (!DecompressDICOM(dataset)) {
		return false;
	}

	// Pixel Data 읽기
	const Sint16* pixelData = GetPixelData(dataset);
	if (pixelData == nullptr) {
		std::cerr << "❌ Failed to get pixel data from: " << path << std::endl;
		return false;
	}

	// 데이터 복사
	int sliceSize = m_width * m_height;
	int offset = sliceIndex * sliceSize;

	for (int i = 0; i < sliceSize; ++i) {
		m_volumeData[offset + i] = pixelData[i];
	}

	std::cout << "✅ Slice " << sliceIndex << " loaded" << std::endl;
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
void FileReader::PrintMetadata()
{
	DcmFileFormat file;
	OFCondition status = file.loadFile(m_filePaths[0].c_str());
	if (!status.good()) {
		std::cerr << "PrintMetadata : Failed to load DICOM file: " << m_filePaths[0] << std::endl;
		return;
	}

	DcmDataset* dataset = file.getDataset();
	OFString patientID, patientAge, studyDate;

	if (dataset->findAndGetOFString(DCM_PatientID, patientID).good())
		std::cout << "Patient ID: " << patientID << std::endl;

	if (dataset->findAndGetOFString(DCM_PatientAge, patientAge).good())
		std::cout << "Patient Age: " << patientAge << std::endl;

	if (dataset->findAndGetOFString(DCM_StudyDate, studyDate).good())
		std::cout << "Study Date: " << studyDate << std::endl;
}

std::vector<uint8_t> FileReader::GenerateAxialSlice(int zIndex)
{
	const size_t sliceSize = static_cast<size_t>(m_width) * m_height;

	std::vector<int16_t> rawSlice(sliceSize);
	const size_t offset = static_cast<size_t>(zIndex) * sliceSize;

	if (offset + sliceSize > m_volumeData.size()) {
		std::cerr << "Invalid zIndex: out of bounds." << std::endl;
		std::cout << "zIndex : " << zIndex << endl;
		return {};
	}

	std::copy(
		m_volumeData.begin() + offset,
		m_volumeData.begin() + offset + sliceSize,
		rawSlice.begin()
	);

	std::vector<uint8_t> normalized;
	//NormalizeSlice(rawSlice, normalized, windowCenter, windowWidth);
	NormalizeSlice(rawSlice, normalized, windowCenter, windowWidth);


	std::vector<uint8_t> rgbaSlice(sliceSize * 4);
	for (size_t i = 0; i < sliceSize; ++i) {
		uint8_t gray = normalized[i];
		rgbaSlice[i * 4 + 0] = gray; // R
		rgbaSlice[i * 4 + 1] = gray; // G
		rgbaSlice[i * 4 + 2] = gray; // B
		rgbaSlice[i * 4 + 3] = 255;  // A
	}

	return rgbaSlice;
}


std::vector<uint8_t> FileReader::GenerateCoronalSlice(int yIndex)
{
	const size_t sliceWidth = static_cast<size_t>(m_width);
	const size_t sliceHeight = static_cast<size_t>(m_depth);
	const size_t sliceSize = sliceWidth * sliceHeight;

	std::vector<int16_t> rawSlice(sliceSize);

	for (size_t z = 0; z < m_depth; ++z) {
		for (size_t x = 0; x < m_width; ++x) {
			size_t srcIndex = z * (m_width * m_height) + yIndex * m_width + x;
			size_t dstIndex = z * m_width + x;
			rawSlice[dstIndex] = m_volumeData[srcIndex];
		}
	}

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized, windowCenter, windowWidth);

	std::vector<uint8_t> rgbaSlice(sliceSize * 4);
	for (size_t i = 0; i < sliceSize; ++i) {
		uint8_t gray = normalized[i];
		rgbaSlice[i * 4 + 0] = gray; // R
		rgbaSlice[i * 4 + 1] = gray; // G
		rgbaSlice[i * 4 + 2] = gray; // B
		rgbaSlice[i * 4 + 3] = 255;  // A
	}

	return rgbaSlice;
}

std::vector<uint8_t> FileReader::GenerateSagittalSlice(int xIndex)
{
	const size_t sliceWidth = static_cast<size_t>(m_height);
	const size_t sliceHeight = static_cast<size_t>(m_depth);
	const size_t sliceSize = sliceWidth * sliceHeight;

	std::vector<int16_t> rawSlice(sliceSize);

	for (size_t z = 0; z < m_depth; ++z) {
		for (size_t y = 0; y < m_height; ++y) {
			size_t srcIndex = z * (m_width * m_height) + y * m_width + xIndex;
			size_t dstIndex = z * m_height + y;
			rawSlice[dstIndex] = m_volumeData[srcIndex];
		}
	}

	std::vector<uint8_t> normalized;
	NormalizeSlice(rawSlice, normalized, windowCenter, windowWidth);

	std::vector<uint8_t> rgbaSlice(sliceSize * 4);
	for (size_t i = 0; i < sliceSize; ++i) {
		uint8_t gray = normalized[i];
		rgbaSlice[i * 4 + 0] = gray; // R
		rgbaSlice[i * 4 + 1] = gray; // G
		rgbaSlice[i * 4 + 2] = gray; // B
		rgbaSlice[i * 4 + 3] = 255;  // A
	}

	return rgbaSlice;
}


//1202
bool FileReader::NormalizeSlice(const std::vector<int16_t>& rawSlice,
	std::vector<uint8_t>& outSlice,
	float windowCenter,
	float windowWidth)
{
	////// ⭐ 디버그!
	////std::cout << "NormalizeSlice called: WC=" << windowCenter
	////	<< ", WW=" << windowWidth << std::endl;



	//if (rawSlice.empty() || windowWidth <= 1e-5f) return false;

	//const float minHU = windowCenter - windowWidth / 2.0f;//-1000
	//const float maxHU = windowCenter + windowWidth / 2.0f;//3000

	////cout << "NormalizeSlice minHU :" << minHU << endl;
	////cout<< "NormalizeSlice maxHU :" << maxHU << endl;

	//outSlice.resize(rawSlice.size());

	////// ⭐ 첫 10개 raw 값 확인
	////std::cout << "   First 10 raw: ";
	////for (int i = 0; i < 10 && i < rawSlice.size(); ++i) {
	////	std::cout << rawSlice[i] << " ";
	////}
	////std::cout << std::endl;

	////std::cout << "rawSlice size : " << rawSlice.size() << std::endl;

	//for (size_t i = 0; i < rawSlice.size(); ++i) {
	//	// ⭐⭐⭐ 여기 수정!
	//	float val = static_cast<float>(rawSlice[i]) * m_rescaleSlope + m_rescaleIntercept;

	//	//cout << "1202 val : " << val << endl;

	//	if (val < minHU) val = minHU;
	//	if (val > maxHU) val = maxHU;

	//	float normalized = (val - minHU) / (maxHU - minHU);
	//	outSlice[i] = static_cast<uint8_t>(normalized * 255.0f);

	//	//cout << "1202 outSlice : " << outSlice[i] << endl;
	//}

	//return true;






	if (rawSlice.empty() || windowWidth <= 1e-5f) return false;

	const float minHU = windowCenter - windowWidth / 2.0f;
	const float maxHU = windowCenter + windowWidth / 2.0f;

	outSlice.resize(rawSlice.size());

	for (size_t i = 0; i < rawSlice.size(); ++i) {
		// ⭐ 패딩 체크
		if (rawSlice[i] > 60000 || rawSlice[i] < -30000) {
			outSlice[i] = 0;  // 검은색
			continue;
		}

		// ⭐⭐⭐ Rescale 제거! Raw 값이 이미 HU!
		float val = static_cast<float>(rawSlice[i]);  // ← 이것만!

		// Window/Level 적용
		if (val < minHU) val = minHU;
		if (val > maxHU) val = maxHU;

		float normalized = (val - minHU) / (maxHU - minHU);
		outSlice[i] = static_cast<uint8_t>(normalized * 255.0f);
	}

	return true;
}

//bool FileReader::NormalizeVolumeU16(
//	const std::vector<int16_t>& rawVolume,
//	std::vector<uint16_t>& outVolume,
//	float rescaleSlope,
//	float rescaleIntercept,
//	float windowMinHU,
//	float windowMaxHU)
//{
//	if (rawVolume.empty()) return false;
//	outVolume.resize(rawVolume.size());
//
//	floatData.resize(rawVolume.size());
//
//	for (size_t i = 0; i < rawVolume.size(); ++i)
//	{
//		// ⭐ 패딩 값 체크
//		if (rawVolume[i] > 60000 || rawVolume[i] < -30000) {
//			floatData[i] = -1024.0f;
//			continue;
//		}
//		//else
//		{
//			//floatData[i] = rawVolume[i];  // Raw 값 유지
//			floatData[i] = static_cast<float>(rawVolume[i]);
//		}
//
//		//// 1️⃣ 원본 픽셀을 HU 단위로 변환
//		//float hu = rescaleSlope * static_cast<float>(rawVolume[i]) + rescaleIntercept;
//
//		//// 2️⃣ 윈도우 범위 클램프
//		//if (hu < windowMinHU) hu = windowMinHU;
//		//if (hu > windowMaxHU) hu = windowMaxHU;
//
//		//// 3️⃣ 0~1 정규화 후 0~65535로 스케일
//		//float norm = (hu - windowMinHU) / (windowMaxHU - windowMinHU);
//		//outVolume[i] = static_cast<uint16_t>(norm * 65535.0f);
//	}
//
//	return true;
//}
bool FileReader::NormalizeVolumeU16(
	const std::vector<int16_t>& rawVolume,
	std::vector<uint16_t>& outVolume,
	float rescaleSlope,
	float rescaleIntercept,
	float windowMinHU,
	float windowMaxHU)
{
	if (rawVolume.empty()) return false;

	//outVolume.resize(rawVolume.size());
	floatData.resize(rawVolume.size());

	//int outlierCount = 0;

	for (size_t i = 0; i < rawVolume.size(); ++i)
	{
		//// ⭐ 패딩 체크
		//if (rawVolume[i] > 60000 || rawVolume[i] < -30000) {
		//	floatData[i] = -1024.0f;
		//	outVolume[i] = 0;
		//	continue;
		//}

		//// HU 변환
		float hu = static_cast<float>(rawVolume[i]) * rescaleSlope + rescaleIntercept;

		//// ⭐ Outlier 클램핑
		//if (hu < -1500.0f) {
		//	hu = -1024.0f;  // 공기로 설정
		//	outlierCount++;
		//}
		//else if (hu > 3500.0f) {
		//	hu = 3000.0f;   // 치아 최대로
		//	outlierCount++;
		//}

		floatData[i] = hu ;

		//// ⭐ 윈도우링 적용 (Soft tissue window 예: -100 ~ 300)
		//float normalized = (hu - windowMinHU) / (windowMaxHU - windowMinHU);

		//// 범위 클램핑
		//if (normalized < 0.0f) normalized = 0.0f;
		//if (normalized > 1.0f) normalized = 1.0f;

		//// 0~65535 범위로 매핑
		//outVolume[i] = static_cast<uint16_t>(normalized * 65535.0f);
	}


	//if (outlierCount > 0) {
	//	std::cout << "⚠️ Clamped " << outlierCount << " outlier voxels ("
	//		<< (outlierCount * 100.0f / rawVolume.size()) << "%)" << std::endl;
	//}


	return true;
}

//minHU = minHU > hu ? hu : minHU;
//maxHU = maxHU > hu ? maxHU : hu;


//1202
//void FileReader::AnalyzeHUDistribution()
//{
//	if (m_volumeData.empty()) return;
//
//	std::map<int, int> histogram;
//	float minHU = FLT_MAX;
//	float maxHU = -FLT_MAX;
//
//	// ⭐ 패딩 값 확인
//	int paddingCount = 0;
//
//	for (const auto& raw : m_volumeData) {
//		// ⭐ 패딩 값 제외 (65535, 63488 등)
//		if (raw > 60000) {
//			paddingCount++;
//			continue;  // 분석에서 제외
//		}
//
//		float hu = m_rescaleSlope * static_cast<float>(raw) + m_rescaleIntercept;
//
//		minHU = minHU > hu ? hu : minHU;
//		maxHU = maxHU > hu ? maxHU : hu;
//
//		int bucket = static_cast<int>(hu / 100) * 100;
//		histogram[bucket]++;
//	}
//
//	std::cout << "=== HU Distribution Analysis ===" << std::endl;
//	std::cout << "Padding voxels excluded: " << paddingCount
//		<< " (" << (paddingCount * 100.0f / m_volumeData.size()) << "%)" << std::endl;
//	std::cout << "Min HU: " << minHU << std::endl;
//	std::cout << "Max HU: " << maxHU << std::endl;
//	std::cout << "Valid voxels: " << (m_volumeData.size() - paddingCount) << std::endl;
//	std::cout << "\nHU Range | Count | Percentage" << std::endl;
//
//	for (const auto&[bucket, count] : histogram) {
//		float percentage = (count * 100.0f) / m_volumeData.size();
//		if (percentage > 0.1) {  // 0.1% 이상만 출력
//			std::cout << bucket << "~" << (bucket + 100)
//				<< " | " << count
//				<< " | " << std::fixed << std::setprecision(2) << percentage << "%"
//				<< std::endl;
//		}
//	}
//
//	// 조직별 분포
//	int air = 0, soft = 0, bone = 0, teeth = 0, other = 0;
//	int validVoxels = 0;
//
//	for (const auto& raw : m_volumeData) {
//		// ⭐ 패딩 제외
//		if (raw > 60000) continue;
//
//		validVoxels++;
//		float hu = m_rescaleSlope * static_cast<float>(raw) + m_rescaleIntercept;
//
//		if (hu < -400) air++;
//		else if (hu < 200) soft++;
//		else if (hu < 1500) bone++;
//		else if (hu < 3000) teeth++;
//		else other++;
//	}
//
//	std::cout << "\n=== Tissue Distribution (Valid Voxels Only) ===" << std::endl;
//	std::cout << "Air (<-400): " << (air * 100.0f / validVoxels) << "%" << std::endl;
//	std::cout << "Soft Tissue (-400~200): " << (soft * 100.0f / validVoxels) << "%" << std::endl;
//	std::cout << "Bone (200~1500): " << (bone * 100.0f / validVoxels) << "%" << std::endl;
//	std::cout << "Teeth (1500~3000): " << (teeth * 100.0f / validVoxels) << "%" << std::endl;
//	std::cout << "Other (3000+): " << (other * 100.0f / validVoxels) << "%" << std::endl;
//}
void FileReader::AnalyzeHUDistribution()
{
	if (m_volumeData.empty()) return;

	// ⭐ Raw 값 직접 확인!
	std::cout << "\n=== Raw Value Samples ===" << std::endl;
	std::cout << "First 100 raw values:" << std::endl;
	for (int i = 0; i < 100 && i < m_volumeData.size(); ++i) {
		std::cout << m_volumeData[i] << " ";
		if ((i + 1) % 20 == 0) std::cout << std::endl;
	}
	std::cout << "\n" << std::endl;

	std::map<int, int> histogram;
	float minRaw = FLT_MAX;
	float maxRaw = -FLT_MAX;
	float minHU = FLT_MAX;
	float maxHU = -FLT_MAX;
	int paddingCount = 0;
	int outlierCount = 0;  // ⭐ Outlier 카운트

	for (const auto& raw : m_volumeData) {
		// ⭐ 패딩 체크
		if (raw > 60000 || raw < -30000) {
			paddingCount++;
		//	continue;
		}

		// ⭐⭐⭐ Outlier 체크 (정상 HU 범위 밖)
		// 정상 HU 범위: -1024 ~ 3000
		// 여유있게: -1500 ~ 3500
		if (raw < -1500 || raw > 3500) {
			outlierCount++;
			//continue;  // ⭐ 분석에서 제외!
		}

		// Raw 값 범위 확인
		if (raw < minRaw) minRaw = raw;
		if (raw > maxRaw) maxRaw = raw;

		// Raw 값 = HU
		float hu = static_cast<float>(raw) /** 1 + (-1024.f)*/;


		if (hu < minHU) minHU = hu;
		if (hu > maxHU) maxHU = hu;

		int bucket = static_cast<int>(hu / 100) * 100;
		histogram[bucket]++;
	}

	int validVoxels = m_volumeData.size() - paddingCount - outlierCount;

	std::cout << "=== Raw Value Range ===" << std::endl;
	std::cout << "Min Raw: " << minRaw << std::endl;
	std::cout << "Max Raw: " << maxRaw << std::endl;
	std::cout << "Padding: " << paddingCount
		<< " (" << (paddingCount * 100.0f / m_volumeData.size()) << "%)" << std::endl;
	std::cout << "Outliers: " << outlierCount
		<< " (" << (outlierCount * 100.0f / m_volumeData.size()) << "%)" << std::endl;

	std::cout << "\n=== HU Distribution Analysis (Outliers Excluded) ===" << std::endl;
	std::cout << "Min HU: " << minHU << std::endl;
	std::cout << "Max HU: " << maxHU << std::endl;
	std::cout << "Valid voxels: " << validVoxels << "\n" << std::endl;

	std::cout << "HU Range | Count | Percentage" << std::endl;
	for (const auto&[bucket, count] : histogram) {
		float percentage = (count * 100.0f) / validVoxels;
		if (percentage > 0.5f) {
			std::cout << bucket << "~" << (bucket + 100)
				<< " | " << count
				<< " | " << std::fixed << std::setprecision(2)
				<< percentage << "%" << std::endl;
		}
	}

	// 조직별 분포 (Outlier 제외)
	int air = 0, soft = 0, bone = 0, teeth = 0;

	for (const auto& raw : m_volumeData) {
		if (raw > 60000 || raw < -30000) continue;
		if (raw < -1500 || raw > 3500) continue;  // ⭐ Outlier 제외

		float hu = static_cast<float>(raw);

		if (hu < -400) air++;
		else if (hu < 200) soft++;
		else if (hu < 1500) bone++;
		else teeth++;
	}

	std::cout << "\n=== Tissue Distribution (Outliers Excluded) ===" << std::endl;
	std::cout << "Air (<-400): " << (air * 100.0f / validVoxels) << "%" << std::endl;
	std::cout << "Soft Tissue (-400~200): " << (soft * 100.0f / validVoxels) << "%" << std::endl;
	std::cout << "Bone (200~1500): " << (bone * 100.0f / validVoxels) << "%" << std::endl;
	std::cout << "Teeth (1500~3500): " << (teeth * 100.0f / validVoxels) << "%" << std::endl;






	// ⭐ 추가: 특정 범위 샘플 확인
	std::cout << "\n=== Sample Analysis ===" << std::endl;

	// -1100~-1000 (공기) 샘플
	std::cout << "Air range (-1100~-1000) samples: ";
	int airSampleCount = 0;
	for (const auto& raw : m_volumeData) {
		if (raw >= -1100 && raw <= -1000) {
			if (airSampleCount < 20) {
				std::cout << raw << " ";
			}
			airSampleCount++;
		}
	}
	std::cout << "\nTotal: " << airSampleCount << std::endl;

	// 0~100 샘플
	std::cout << "\n0~100 range samples: ";
	int zeroSampleCount = 0;
	for (const auto& raw : m_volumeData) {
		if (raw >= 0 && raw <= 100) {
			if (zeroSampleCount < 20) {
				std::cout << raw << " ";
			}
			zeroSampleCount++;
		}
	}
	std::cout << "\nTotal: " << zeroSampleCount << std::endl;

	// ⭐ 중앙값(Median) 확인
	std::vector<int16_t> sortedData;
	for (const auto& raw : m_volumeData) {
		if (raw > 60000 || raw < -30000) continue;
		if (raw < -1500 || raw > 3500) continue;
		sortedData.push_back(raw);
	}
	std::sort(sortedData.begin(), sortedData.end());

	size_t p25 = sortedData.size() / 4;
	size_t p50 = sortedData.size() / 2;
	size_t p75 = sortedData.size() * 3 / 4;

	std::cout << "\n=== Percentiles ===" << std::endl;
	std::cout << "25th percentile: " << sortedData[p25] << std::endl;
	std::cout << "50th percentile (Median): " << sortedData[p50] << std::endl;
	std::cout << "75th percentile: " << sortedData[p75] << std::endl;
}

void FileReader::SliceIdxManage()
{
	currentIndex[1] = m_depth / 2;//a
	currentIndex[2] = m_height / 2;//c
	currentIndex[3] = m_width / 2;//s

	sliceIndex[1] = m_depth;
	sliceIndex[2] = m_height;
	sliceIndex[3] = m_width;

	cout << "1st idx a : " << currentIndex[1] << endl;
	cout << "1st idx c : " << currentIndex[2] << endl;
	cout << "1st idx s : " << currentIndex[3] << endl;
}

void FileReader::SetAxialSlice(int index)
{
	currentIndex[1] = std::clamp(index, 0, m_depth - 1);
	UpdateAxialTexture(currentIndex[1]);
}
void FileReader::SetCoronalSlice(int index)
{
	currentIndex[2] = std::clamp(index, 0, m_height - 1);
	UpdateCoronalTexture(currentIndex[2]);
}
void FileReader::SetSagittalSlice(int index)
{
	currentIndex[3] = std::clamp(index, 0, m_width - 1);
	UpdateSagittalTexture(currentIndex[3]);
}


void FileReader::UpdateAxialTexture(int z)
{
	std::vector<uint8_t> slice = GenerateAxialSlice(z);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_width, m_height, d3dDevice);

	// 기존 텍스처가 있으면 Release
	auto it = axialTextureCache.find(z);
	if (it != axialTextureCache.end()) {
		if (it->second) it->second->Release();
	}

	axialTextureCache[z] = texture;
}

void FileReader::UpdateCoronalTexture(int y)
{
	std::vector<uint8_t> slice = GenerateCoronalSlice(y);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_width, m_depth, d3dDevice);

	// 기존 텍스처가 있으면 Release
	auto it = coronalTextureCache.find(y);
	if (it != coronalTextureCache.end()) {
		if (it->second) it->second->Release();
	}

	coronalTextureCache[y] = texture;
}

void FileReader::UpdateSagittalTexture(int x)
{

	std::vector<uint8_t> slice = GenerateSagittalSlice(x);
	ID3D11Texture2D* texture = CreateTextureFromSlice(slice, m_height, m_depth, d3dDevice);

	// 기존 텍스처가 있으면 Release
	auto it = sagittalTextureCache.find(x);
	if (it != sagittalTextureCache.end()) {
		if (it->second) it->second->Release();
	}

	sagittalTextureCache[x] = texture;
}


ID3D11Texture2D* FileReader::CreateTextureFromSlice(const std::vector<uint8_t>& slice, int width, int height, ID3D11Device* g_pd3dDevice)
{
	if (slice.empty()) return nullptr;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//texDesc.Format = DXGI_FORMAT_R8_UNORM; 

	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	//texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;


	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = slice.data();
	initData.SysMemPitch = 4 * width * sizeof(uint8_t);
	//initData.SysMemPitch = width * sizeof(uint8_t);
	//initData.SysMemPitch = 4 * width * height;

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, &initData, &texture);
	if (FAILED(hr)) {
		std::cerr << "Failed to create texture from slice." << std::endl;
		return nullptr;
	}


	D3D11_BUFFER_DESC cbDesc = {};
	//cbDesc.ByteWidth = sizeof(CrosshairData);
	cbDesc.ByteWidth = ((sizeof(CrosshairData) + 15) / 16) * 16;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;

	hr = g_pd3dDevice->CreateBuffer(&cbDesc, nullptr, &m_crosshairBuffer);
	if (FAILED(hr)) {
		cerr << "[?먮윭] Crosshair ConstantBuffer ?앹꽦 ?ㅽ뙣!";
	}

	return texture;
}


