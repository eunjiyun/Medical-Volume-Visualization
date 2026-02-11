//#include "stdafx.h"
#include "PLYLoader.h"
#include <sstream>
#include <iostream>

// PLYLoader.cpp
#include <sstream>

PLYLoader::PLYLoader()
	: m_vertexCount(0)
	, m_faceCount(0)
	, m_headerEndPos(0)
{
}

PLYLoader::~PLYLoader() 
{
}

bool PLYLoader::Load(const std::string& filename) {

	//  1. 어떤 경로로 열려고 하는지 확인
	std::cout << "Trying to load PLY from :" << filename << std::endl;


	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	// 1. 헤더 파싱
	if (!ParseHeader(file)) {
		return false;
	}

	file.close();
	file.open(filename, std::ios::binary);
	file.seekg(m_headerEndPos);

	// 2. 정점 데이터 로드
	if (!LoadVertices(file)) {
		return false;
	}

	// 3. Face 데이터 로드
	if (!LoadFaces(file)) {
		return false;
	}

	file.close();

	// 4. 렌더링용 정점 생성
	CreateRenderVertices();

	return true;
}

bool PLYLoader::ParseHeader(std::ifstream& file) {
	std::string line;

	while (std::getline(file, line)) {
		if (line.find("element vertex") != std::string::npos) {
			sscanf(line.c_str(), "element vertex %d", &m_vertexCount);
		}
		else if (line.find("element face") != std::string::npos) {
			sscanf(line.c_str(), "element face %d", &m_faceCount);
		}
		else if (line == "end_header") {
			m_headerEndPos = file.tellg();
			break;
		}
	}

	return m_vertexCount > 0 && m_faceCount > 0;
}

bool PLYLoader::LoadVertices(std::ifstream& file) {
	m_vertices.resize(m_vertexCount);

	for (int i{}; i < m_vertexCount; ++i) {
		file.read(reinterpret_cast<char*>(&m_vertices[i].x), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_vertices[i].y), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_vertices[i].z), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_vertices[i].nx), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_vertices[i].ny), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_vertices[i].nz), sizeof(float));
	}

	return true;
}

bool PLYLoader::LoadFaces(std::ifstream& file) {
	m_faces.resize(m_faceCount);

	for (int i{}; i < m_faceCount; ++i) {
		// vertex_indices 읽기
		uint8_t indexCount;
		file.read(reinterpret_cast<char*>(&indexCount), sizeof(uint8_t));

		for (int j = 0; j < indexCount; j++) {
			int32_t idx;
			file.read(reinterpret_cast<char*>(&idx), sizeof(int32_t));
			if (j < 3) {
				m_faces[i].indices[j] = idx;
			}
		}

		// texcoord 읽기
		uint8_t texCoordCount;
		file.read(reinterpret_cast<char*>(&texCoordCount), sizeof(uint8_t));

		for (int j = 0; j < texCoordCount; j++) {
			float coord;
			file.read(reinterpret_cast<char*>(&coord), sizeof(float));
			if (j < 6) {
				m_faces[i].texCoords[j] = coord;
			}
		}
	}

	return true;
}

void PLYLoader::CreateRenderVertices()
{
	m_renderVertices.clear();
	m_renderVertices.reserve(m_faceCount * 3);

	for (const auto& face : m_faces) {
		for (int i{}; i < 3; ++i) {
			PLY::VertexWithTexture v;

			int idx{ face.indices[i] };


			/*	v.x = m_vertices[idx].x;
				v.y = m_vertices[idx].y;
				v.z = m_vertices[idx].z;*/

				// === 좌표축 변환 (PLY -> CT/DX) ===
			float px{ m_vertices[idx].x };
			float py{ m_vertices[idx].y };
			float pz{ m_vertices[idx].z };

			////v.x = px;
			////v.y = py;
			////v.z = pz;   // 여기 부호가 중요


			//// Case A
			//v.x = px;  v.y = pz;  v.z = -py;

			//////// Case B
			////v.x = px;  v.y = -pz;  v.z = py;

			////// Case C
			////v.x = px;  v.y = py;  v.z = pz; // (원본)

			////// Case D
			////v.x = px;  v.y = -py;  v.z = -pz;


			//// Case A (좌표계 정렬)
			//float x = px;
			//float y = pz;
			//float z = -py;

			// Forward 보정 (X축 -90°)
			v.x = px;
			v.y = py;
			v.z = pz;




			///*v.nx = m_vertices[idx].nx;
			//v.ny = m_vertices[idx].ny;
			//v.nz = m_vertices[idx].nz;*/
			//// 노멀 (반전)
			//v.nx = -m_vertices[idx].nx;
			//v.ny = -m_vertices[idx].ny;
			//v.nz = -m_vertices[idx].nz;




			float nx{ m_vertices[idx].nx };
			float ny{ m_vertices[idx].ny };
			float nz{ m_vertices[idx].nz };

			// PLY → CT 축 변환
		/*	v.nx = nx;
			v.ny = nz;
			v.nz = -ny;*/
			v.nx = nx;
			v.ny = ny;
			v.nz = nz;

			v.u = face.texCoords[i * 2];
			v.v = face.texCoords[i * 2 + 1];

			m_renderVertices.push_back(v);
		}
	}
}


const std::vector<PLY::VertexWithTexture>& PLYLoader::GetRenderVertices() const 
{
	return m_renderVertices;
}