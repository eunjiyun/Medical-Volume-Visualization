#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>        // XMMatrix 등
#include <QtCore/QDebug>        // Qt 포함은 나중에
#include<QWidget>
#include<QTimer>
#include<qscrollbar.h>
#include<qlabel.h>
#include <QPainter>
#include <QPen>
#include <QColor>
#include<stdexcept>
#include<unordered_map>
#include<vector>
#include "ArcBall.h"
using namespace std;

using Microsoft::WRL::ComPtr;



#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include <D3Dcompiler.h>
#include <directxmath.h>
using namespace DirectX;



struct CB
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
	DirectX::XMMATRIX InvView;
	DirectX::XMMATRIX InvProj;
	DirectX::XMMATRIX VolumeWorld;     // 볼륨의 월드 변환(스케일/회전/이동)
	DirectX::XMMATRIX InvVolumeWorld;
	DirectX::XMFLOAT3 CameraPosWS;     float Step;      // 샘플 간격 (예: 0.002~0.01)
	int   MaxSteps;                   DirectX::XMFLOAT3 Voxel;

	DirectX::XMFLOAT4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
};


//
//cbuffer CB : register(b0)
//{
//	matrix View;
//	matrix Proj;
//	matrix InvView;
//	matrix InvProj;
//	matrix VolumeWorld;
//	matrix InvVolumeWorld;
//	float3 CameraPosWS;
//	float Step;
//	int   MaxSteps;
//	float3 Voxel;
//	//float Pad0;
//
//	//// 🔽 추가
//	//float  HuSlope;        // RescaleSlope
//	//float  HuIntercept;    // RescaleIntercept
//	//float  HuMin;          // 윈도우/TF용 HU 최소값 (예: -1000)
//	//float  HuMax;          // 윈도우/TF용 HU 최대값 (예: 3000)
//
//	float4 HuParams;  // x=Slope, y=Intercept, z=Min, w=Max
//};

class FileReader;


struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
};

struct SliceSeriesSrv {
	std::vector<ID3D11ShaderResourceView*> slices;
	int flagIndex[4];
	int currentIndex{};
};

struct SliceSeriesRtv {
	std::vector<ID3D11RenderTargetView*> slices;
	int flagIndex[4];
	int currentIndex{};
};

struct ViewInfoCB {
	int viewIndex;
	int padding[3]; // 16바이트 정렬을 맞추기 위해
};

struct ViewGeometry {
	XMFLOAT3 origin;
	XMFLOAT3 rowDir;
	XMFLOAT3 colDir;
	float pixelSpacingX;
	float pixelSpacingY;
	float sliceSpacing;
};


// QDirect3D11Widget.h 또는 .cpp 상단에 선언

struct VolumeConstants {
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;


	DirectX::XMFLOAT4 Voxel;  // 선택사항: 와이어프레임 색상 등
	
	//float Pad0;
	// // 🔽 추가
	//float  HuSlope;        // RescaleSlope
	//float  HuIntercept;    // RescaleIntercept
	//float  HuMin;          // 윈도우/TF용 HU 최소값 (예: -1000)
	//float  HuMax;          // 윈도우/TF용 HU 최대값 (예: 3000)

	DirectX::XMFLOAT4 HuParams;
};

struct SlicePlane {
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11ShaderResourceView* texture;
	DirectX::XMFLOAT4X4 worldMatrix;
	int viewType; // 1: Axial, 2: Coronal, 3: Sagittal
};


class QDirect3D11Widget : public QWidget
{
	Q_OBJECT
public:
	QScrollBar* scrollAxial;
	QScrollBar* scrollCoronal;
	QScrollBar* scrollSagittal;

	//ArcBall m_arcball;

	QLabel* labelVolume;
	QLabel* labelAxial;
	QLabel* labelCoronal;
	QLabel* labelSagittal;


	// 슬라이스 정보 라벨 추가
	QLabel* sliceInfoAxial;
	QLabel* sliceInfoCoronal;
	QLabel* sliceInfoSagittal;

	//DirectX::XMFLOAT3 currentPatientCoord[4];
	DirectX::XMFLOAT2 currentUV[4] = {
	{0.0f, 0.0f},
	{0.5f, 0.5f},
	{0.5f, 0.5f},
	{0.5f, 0.5f}
	};

	float m_cameraDistance;

	float m_rotationX = 0.0f;  // X축 회전 (pitch)
	float m_rotationY = 0.0f;  // Y축 회전 (yaw)

	QPoint m_lastMousePos = { 0, 0 };
	bool m_isDragging = false;
	QPoint currentPos;


	XMVECTOR m_rotation;  // 쿼터니언
	XMVECTOR m_initialRotation;

//	QPoint m_lastMousePos;
//	bool m_isDragging;

public:
	//// 마우스 입력 처리
	//void OnMouseDown(int x, int y);
	//void OnMouseUp();
	//void OnMouseMove(int x, int y);

	void UpdateVolumeMatrix();
	void Render();


public:
	QDirect3D11Widget(QWidget * parent);
	~QDirect3D11Widget();

	void release();
	void resetEnvironment();

	void run();
	void pauseFrames();
	void continueFrames();

	bool init();

	void LoadDICOMSeries();


	void mousePressEvent(QMouseEvent* event);

	
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

	// 마우스 입력 처리
	//void OnMouseDown(int x, int y);
	//void OnMouseUp();




	//void OnMouseMove(int x, int y);

	//void onRotationChanged(float x, float y);








	int GetClickedViewIndex(int px, int py, int width, int height);
	ViewGeometry GetAxialGeometry();
	ViewGeometry GetCoronalGeometry();
	ViewGeometry GetSagittalGeometry();
	int ComputeSliceIndexForView(const XMFLOAT3& patientCoord, int viewIndex);
	int px[4], py[4];
	int clickedViewIndex{};


	D3D11_VIEWPORT viewPort;

	//std::vector<std::vector<uint8_t>> sliceData;
	ComPtr<ID3D11ShaderResourceView> texArraySRV;
private:


	void beginScene();
	void endScene();

	void tick();




	void initializeRenderTargets();
	void initializeVolumeRenderTargets();

	void createSwapChainRTV();

	//void DrawQuadWithTexture(ID3D11ShaderResourceView* pSRV, const D3D11_VIEWPORT& vp);
	void render();
	void UpdateColorBuffer();
	void UpdateViewIndexBuffer(int viewIndex);
	void DrawColoredQuad(const D3D11_VIEWPORT& vp);

	void InitShaders();
	D3D11_VIEWPORT CreateViewport(int index);
	void SetBackgroundColor(int index);
	void RenderSceneToTarget(int i);

	void UpdateCrosshairFromPatientCoord(DirectX::XMFLOAT3 patientCoord, int i);
	DirectX::XMFLOAT3 GetDefaultPatientCenter();
	//void InitializeCrosshair();
	void RenderAllQuads();
	void DrawFullScreenQuad();
	void DrawQuadWithTexture(ID3D11ShaderResourceView* pSRV, const D3D11_VIEWPORT& vp, int i);

	UINT BytesPerPixel(DXGI_FORMAT format);
	ID3D11Texture2D* CreateTexture2D(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, const void* initData);
	ID3D11ShaderResourceView* CreateTextureSRV(ID3D11Device* device, ID3D11Texture2D* texture);
	void InitTextures(UINT, UINT);

	void InitSampler();
	void InitializeGraphics();

	ID3D11RenderTargetView* getRTVForTexture(ID3D11Texture2D* texture);
	ID3D11ShaderResourceView* getSRVForTexture(ID3D11Texture2D* texture);

	int ComputeSliceIndexFromPatientCoord(int viewIndex, XMFLOAT3 patientCoord);
	int ComputeSliceIndexFromPatientCoord_Robust(
		const XMFLOAT3& patientCoord,    // world/patient coordinate
		int viewIndex,                   // 1: Axial (Z), 2: Coronal (Y), 3: Sagittal (X)
		const XMFLOAT3& origin,          // ImagePositionPatient of reference slice (slice 0)
		const XMFLOAT3& rowDir,          // ImageOrientationPatient[0..2]
		const XMFLOAT3& colDir,          // ImageOrientationPatient[3..5]
		float pixelSpacingX,             // (mm) usually second value in (0028,0030)
		float pixelSpacingY,             // (mm) usually first value in (0028,0030)
		float sliceSpacing,              // (mm) spacing between slices (0018,0088) or SliceThickness
		const XMUINT3& dims              // width, height, depth (voxels)
	);
	DirectX::XMFLOAT2 GetNormalizedUV(int px, int py, int viewIndex);
	XMFLOAT3 GetPatientCoordFromClick(int viewIndex, XMFLOAT2 uv);
	XMFLOAT2 GetCrossUVFromPatientCoord(int viewIndex, XMFLOAT3 patientCoord);



	// Qt Events
private:
	bool           event(QEvent * event) override;
	void           showEvent(QShowEvent * event) override;
	QPaintEngine * paintEngine() const override;
	void           paintEvent(QPaintEvent * event) override;
	void           resizeEvent(QResizeEvent * event) override;
	void           wheelEvent(QWheelEvent * event) override;

	LRESULT WINAPI WndProc(MSG * pMsg);

#if QT_VERSION >= 0x050000
	bool nativeEvent(const QByteArray & eventTypeonAxialScroll, void * message, long * result) override;
#else
	bool winEvent(MSG * message, long * result) override;
#endif

signals:
	// ✅ 2. signals: 섹션 추가
	void rotationChanged(float x, float y);  // ✅ 3. 시그널 선언 (구현 X)

	void deviceInitialized(bool success);

	void eventHandled();
	void widgetResized();

	void ticked();
	void rendered();

	void keyPressed(QKeyEvent *);
	void mouseMoved(QMouseEvent *);
	void mouseClicked(QMouseEvent *);
	void mouseReleased(QMouseEvent *);



private slots:
	void onFrame();
	void onReset();
	void onAxialScroll(int value);
	void onCoronalScroll(int value);
	void onSagittalScroll(int value);





	// Getters / Setters
public:
	HWND const & nativeHandle() const { return m_hWnd; }

	ID3D11Device *           device() const { return m_pDevice; }
	ID3D11DeviceContext *    deviceContext() { return m_pDeviceContext; }
	IDXGISwapChain *         swapChain() { return m_pSwapChain; }
	//ID3D11RenderTargetView * TargetView() const { return m_pRTView; }
	/*std::vector<ID3D11RenderTargetView*> TargetView() const {
		return m_RTViews;
	}*/
	bool renderActive() const { return m_bRenderActive; }
	void setRenderActive(bool active) { m_bRenderActive = active; }

	D3DCOLORVALUE * BackColor() { return &m_BackColor; }


	SliceSeriesSrv m_SRViews;// Volume, m_SRViewsAxial, m_SRViewsCoronal, m_SRViewsSagittal;
	ID3D11Buffer* m_colorBuffer = nullptr;//m_viewIndexBuffer
	ID3D11Buffer* m_viewIndexBuffer = nullptr;//m_viewIndexBuffer

	std::vector<ID3D11RenderTargetView*> rtvPool, activeRTVs;
	std::unordered_map<ID3D11Texture2D*, ID3D11RenderTargetView*> rtvCache;
	std::unordered_map<ID3D11Texture2D*, ID3D11ShaderResourceView*> srvCache;


private:

	ID3D11DeviceContext *    m_pDeviceContext;
	IDXGISwapChain *         m_pSwapChain;
	//ID3D11RenderTargetView * m_pRTView;
	SliceSeriesRtv m_RTViews;// m_RTViewsVolume, m_RTViewsAxial, m_RTViewsCoronal, m_RTViewsSagittal;
	ID3D11RenderTargetView* m_pSwapChainRTV = nullptr;
	std::vector<ID3D11ShaderResourceView*> coronalTextureCacheSrv;
	QTimer m_qTimer;

	HWND m_hWnd;
	bool m_bDeviceInitialized;

	bool m_bRenderActive;
	bool m_bStarted;

	D3DCOLORVALUE m_BackColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black, fully opaque
public:

	ComPtr<ID3D11ShaderResourceView> m_volumeSRV;   // 3D 볼륨 텍스처 SRV
	ComPtr<ID3D11SamplerState> m_volumeSampler;     // 3D 볼륨 샘플러

	ID3D11Device* m_pDevice;


	ID3D11VertexShader*       m_vertexShader, *vsFullscreen;
	ID3D11PixelShader*        m_pixelShader, *psRaymarch;
	ID3D11PixelShader*        m_pixelShaderAxial, *m_pixelShaderCoronal, *m_pixelShaderSagittal;


	ID3D11VertexShader*       m_volumeVS = nullptr;
	ID3D11PixelShader*        m_volumePS = nullptr;


	//m_volumeQuadVS

	ID3D11VertexShader*       m_volumeQuadVS = nullptr;
	ID3D11PixelShader*        m_volumeQuadPS = nullptr;

	ID3D11Buffer*             m_vertexBuffer = nullptr;

	ID3D11InputLayout*        m_inputLayout = nullptr;//layoutQuad
	ID3D11InputLayout*layoutQuad{ nullptr };

	ID3D11InputLayout*        m_volumeInputLayout = nullptr;

	ID3D11InputLayout*        m_prevVolumeInputLayout = nullptr;
	ID3D11InputLayout* m_cubeInputLayout;        // ✅ 큐브용 (Position만)

	//ID3D11Buffer* m_vertexBuffer = nullptr;


	ID3D11Buffer* m_volumeConstantBuffer;  // ← 여기 추가!
	ID3D11Buffer* m_volumePrevConstantBuffer;  // ← 여기 추가!

	// D3D11 상태 객체들
	Microsoft::WRL::ComPtr<ID3D11BlendState>        m_alphaBlendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_disableDepthState;


		// ✅ 큐브 관련
	ID3D11Buffer* m_cubeVertexBuffer;
	ID3D11Buffer* m_cubeIndexBuffer;

	// ✅ 3D 평면들
	SlicePlane m_CoronalPlane;
	SlicePlane m_AxialPlane;
	SlicePlane m_SagittalPlane;

	// ✅ 올바른 선언
	DirectX::XMFLOAT4X4 m_volumeViewMatrix;
	DirectX::XMFLOAT4X4 m_volumeProjectionMatrix;

	ID3D11DepthStencilView* m_pDepthStencilView;  // ← 이게 있는지 확인

	// // ✅ 각 평면의 World Matrix를 저장
	//XMFLOAT4X4 m_axialPlaneWorld;
	//XMFLOAT4X4 m_coronalPlaneWorld;
	//XMFLOAT4X4 m_sagittalPlaneWorld;
	VolumeConstants constants{};
	VolumeConstants constantsPrev{};


	XMMATRIX view, proj;
	CB cb{};

	XMVECTOR eye /*= XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f)*/;  // 조금 더 뒤로
	XMVECTOR at /*= XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)*/;
	XMVECTOR up /*= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)*/;
	XMMATRIX v, iv, p, ip, rotx,roty, trans, /*scale,*/ w, iw,scale;
public:
	bool isPlaster{ false };
	void plasterVolumeShow();
	void RenderVolumeView(/*const D3D11_VIEWPORT& vp*/);
	void InitializeVolumeCamera();
	void InitializeVolumeShaders();

	void InitializeSlicePlanes();
	void InitializeBoundingCube();
	void UpdateSlicePlanePositions();
	void RenderBoundingCube(const VolumeConstants& constants);
	void DrawPlane(const SlicePlane& plane);
	void DrawSliceQuad();

	void CreateDepthStencilBuffer();


	ComPtr<ID3D11Buffer> m_quadVB;
	void CreateTexture3D();
	void FullScreenPassSet();


	ID3D11Texture2D* m_texture = nullptr;
	std::vector<ID3D11ShaderResourceView*> m_textureSRV;
	std::vector < ID3D11SamplerState*> m_samplerState;


	//ID3D11Texture2D* m_texture = nullptr;
	//ID3D11ShaderResourceView* m_textureSRV = nullptr;

	FileReader* fileReader = nullptr;

	ID3D11ShaderResourceView* axialTextureSRV = nullptr;

	float viewX;
	float viewY;
	float viewWidth;
	float viewHeight;
	DirectX::XMFLOAT3 patientCoord;
	
};



// ############################################################################
// ############################## Utils #######################################
// ############################################################################
#define ReleaseObject(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        object->Release();                                                                    \
        object = Q_NULLPTR;                                                                   \
    }
#define ReleaseHandle(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        CloseHandle(object);                                                                  \
        object = Q_NULLPTR;                                                                   \
    }

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr)
		: std::runtime_error(HrToString(hr))
		, m_hr(hr)
	{
	}
	HRESULT Error() const { return m_hr; }

private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) { throw HrException(hr); }
}

#define DXCall(func) ThrowIfFailed(func)


