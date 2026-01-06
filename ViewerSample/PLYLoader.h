

// PLYLoader.h
#pragma once
#include <vector>
#include <string>
#include <fstream>

namespace PLY {
	struct Vertex {
		float x, y, z;
		float nx, ny, nz;
	};

	struct Face {
		int indices[3];
		float texCoords[6];  // u0,v0, u1,v1, u2,v2
	};

	struct VertexWithTexture {
		float x, y, z;
		float nx, ny, nz;
		float u, v;
	};
}

class PLYLoader
{
public:
	PLYLoader();
	~PLYLoader();

	// PLY 파일 로드
	bool Load(const std::string& filename);

	// 렌더링용 정점 데이터 가져오기
	const std::vector < PLY::VertexWithTexture > & GetRenderVertices() const;

	// 통계 정보
	int GetVertexCount() const { return m_vertexCount; }
	int GetFaceCount() const { return m_faceCount; }
	int GetRenderVertexCount() const { return m_renderVertices.size(); }

private:
	bool ParseHeader(std::ifstream& file);
	bool LoadVertices(std::ifstream& file);
	bool LoadFaces(std::ifstream& file);
	void CreateRenderVertices();

private:
	int m_vertexCount;
	int m_faceCount;
	size_t m_headerEndPos;

	std::vector<PLY::Vertex> m_vertices;
	std::vector<PLY::Face> m_faces;
	std::vector<PLY::VertexWithTexture> m_renderVertices;
};

