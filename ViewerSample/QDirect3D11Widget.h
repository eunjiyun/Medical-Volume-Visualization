#pragma once

#include<stdexcept>

#include<QWidget>


#include<QTimer>
#include<unordered_map>
#include<qscrollbar.h>
#include<qlabel.h>
#include <QPainter>
#include <QPen>
#include <QColor>



#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>

#include <D3Dcompiler.h>
#include <directxmath.h>
using namespace DirectX;

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
	DirectX::XMFLOAT4 Color;  // 선택사항: 와이어프레임 색상 등
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
	int GetClickedViewIndex(int px, int py, int width, int height);
	ViewGeometry GetAxialGeometry();
	ViewGeometry GetCoronalGeometry();
	ViewGeometry GetSagittalGeometry();
	int ComputeSliceIndexForView(const XMFLOAT3& patientCoord, int viewIndex);
	int px[4], py[4];
	int clickedViewIndex{};


	D3D11_VIEWPORT viewPort;
private:


	void beginScene();
	void endScene();

	void tick();
	void initializeRenderTargets();

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


public:

	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);


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

	QTimer m_qTimer;

	HWND m_hWnd;
	bool m_bDeviceInitialized;

	bool m_bRenderActive;
	bool m_bStarted;

	D3DCOLORVALUE m_BackColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black, fully opaque
public:
	ID3D11Device* m_pDevice;


	ID3D11VertexShader*       m_vertexShader = nullptr;
	ID3D11PixelShader*        m_pixelShader = nullptr;
	ID3D11PixelShader*        m_pixelShaderAxial, *m_pixelShaderCoronal, *m_pixelShaderSagittal;


	ID3D11VertexShader*       m_volumeVS = nullptr;
	ID3D11PixelShader*        m_volumePS = nullptr;

	ID3D11Buffer*             m_vertexBuffer = nullptr;

	ID3D11InputLayout*        m_inputLayout = nullptr;

	ID3D11InputLayout*        m_volumeInputLayout = nullptr;
	ID3D11InputLayout* m_cubeInputLayout;        // ✅ 큐브용 (Position만)

	//ID3D11Buffer* m_vertexBuffer = nullptr;


	ID3D11Buffer* m_volumeConstantBuffer;  // ← 여기 추가!

		// ✅ 큐브 관련
	ID3D11Buffer* m_cubeVertexBuffer;
	ID3D11Buffer* m_cubeIndexBuffer;

	// ✅ 3D 평면들
	SlicePlane m_axialPlane;
	SlicePlane m_coronalPlane;
	SlicePlane m_sagittalPlane;

	// ✅ 올바른 선언
	DirectX::XMFLOAT4X4 m_volumeViewMatrix;
	DirectX::XMFLOAT4X4 m_volumeProjectionMatrix;

	ID3D11DepthStencilView* m_pDepthStencilView;  // ← 이게 있는지 확인
	VolumeConstants constants{};

public:

	void RenderVolumeView(/*const D3D11_VIEWPORT& vp*/);
	void InitializeVolumeCamera();
	void InitializeVolumeShaders();

	void InitializeSlicePlanes();
	void InitializeBoundingCube();
	void UpdateSlicePlanePositions();
	void RenderBoundingCube(const VolumeConstants& constants);
	void DrawPlane();
	void CreateDepthStencilBuffer();


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


