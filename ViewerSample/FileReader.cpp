#include "FileReader.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/ofstd/ofcond.h>

#include <dcmtk/dcmdata/dctypes.h>
//#include <dcmtk/config/osconfig.h>  
#include <filesystem>
#include<iterator>
#include<algorithm>
#include<numeric>



FileReader::FileReader()
    : m_width{ 0 },
    m_height{ 0 },
    m_depth{ 0 },
    m_volumeData()/*,
    m_filePaths()*/
{

    std::cout << "[FileReader] Initialized with empty volume and file list." << std::endl;
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
                OFCondition status = file.loadFile(/*folderPath + "0000.dcm"*/entry.path().string());

                DcmDataset* dataset = file.getDataset();

                OFString widthStr, heightStr;
                dataset->findAndGetOFString(DCM_Columns, widthStr);   // (0028,0011)
                dataset->findAndGetOFString(DCM_Rows, heightStr);      // (0028,0010)


                m_width = std::stoi(widthStr.c_str());
                m_height = std::stoi(heightStr.c_str());

                std::cout << "Width: " << m_width << ", Height: " << m_height << std::endl;


                OFString wcStr, wwStr;
                if (dataset->findAndGetOFString(DCM_WindowCenter, wcStr).good() &&
                    dataset->findAndGetOFString(DCM_WindowWidth, wwStr).good() &&

                    dataset->findAndGetOFString(DCM_PatientName, patientName).good() &&
                    dataset->findAndGetOFString(DCM_PatientBirthDate, birthDate).good() &&
                    dataset->findAndGetOFString(DCM_StudyDate, studyDate).good() &&
                    dataset->findAndGetOFString(DCM_PatientID, patientID).good() &&
                    dataset->findAndGetOFString(DCM_PatientSex, patientMF).good()
                    ) {

                    windowCenter = std::stof(wcStr.c_str());
                    windowWidth = std::stof(wwStr.c_str());


                    /* patientName = std::stof(wcStr.c_str());
                     birthDate = std::stof(wwStr.c_str());
                     studyDate = std::stof(wcStr.c_str());
                     patientID = std::stof(wwStr.c_str());
                     patientMF = std::stof(wcStr.c_str());*/


                    std::cout << "Window Center: " << windowCenter << ", Window Width: " << windowWidth << std::endl;
                }

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


bool FileReader::ParseSlice(const std::string path, int sliceIndex) {
    DcmFileFormat file;
    OFCondition status = file.loadFile(path.c_str(), EXS_Unknown, EGL_noChange);

    if (!status.good()) {
        std::cerr << " Failed to load DICOM file: " << path << std::endl;
        return false;
    }

    DcmDataset* dataset = file.getDataset();


    const Uint16* pixelData = nullptr;
    status = dataset->findAndGetUint16Array(DCM_PixelData, pixelData);
    if (!status.good() || nullptr == pixelData) {
        std::cerr << " Failed to get pixel data from: " << path << std::endl;
        return false;
    }


    int sliceSize = m_width * m_height;
    int offset = sliceIndex * sliceSize;


    for (int i{}; i < sliceSize; ++i) {
        m_volumeData[offset + i] = pixelData[i];
    }


    std::cout << " DICOM Metadata for slice " << sliceIndex << std::endl;


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

    std::vector<uint16_t> rawSlice(sliceSize);
    const size_t offset = static_cast<size_t>(zIndex) * sliceSize;

    if (offset + sliceSize > m_volumeData.size()) {
        std::cerr << "Invalid zIndex: out of bounds." << std::endl;
        return {};
    }

    std::copy(
        m_volumeData.begin() + offset,
        m_volumeData.begin() + offset + sliceSize,
        rawSlice.begin()
    );

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


std::vector<uint8_t> FileReader::GenerateCoronalSlice(int yIndex)
{
    const size_t sliceWidth = static_cast<size_t>(m_width);
    const size_t sliceHeight = static_cast<size_t>(m_depth);
    const size_t sliceSize = sliceWidth * sliceHeight;

    std::vector<uint16_t> rawSlice(sliceSize);

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

    std::vector<uint16_t> rawSlice(sliceSize);

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


bool FileReader::NormalizeSlice(const std::vector<uint16_t>& rawSlice,
    std::vector<uint8_t>& outSlice,
    float windowCenter,
    float windowWidth)
{
    if (rawSlice.empty() || windowWidth <= 1e-5f) return false;

    const float minHU = windowCenter - windowWidth / 2.0f;
    const float maxHU = windowCenter + windowWidth / 2.0f;

    outSlice.resize(rawSlice.size());

    for (size_t i = 0; i < rawSlice.size(); ++i) {
        //float val = static_cast<float>(rawSlice[i]);
        float val = static_cast<float>(static_cast<int16_t>(rawSlice[i]));

        if (val < minHU) val = minHU;
        if (val > maxHU) val = maxHU;

        float normalized = (val - minHU) / (maxHU - minHU);
        outSlice[i] = static_cast<uint8_t>(normalized * 255.0f);
    }

    return true;
}

void FileReader::SliceIdxManage()
{
    currentIndex[1] = m_depth / 2;//a
    currentIndex[2] = m_height / 2;//c
    currentIndex[3] = m_width / 2;//s
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


