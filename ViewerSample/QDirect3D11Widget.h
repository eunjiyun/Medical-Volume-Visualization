#pragma once

//C++ ??? ??깆뵠?됰슢??뵳?肉????됱뇚 筌ｌ꼶??exception handling)???袁る립 ??삳쐭
//????삳쐭???袁⑥쨮域밸챶????쎈뻬 餓?獄쏆뮇源??????덈뮉 ??겸봺 ??살첒???怨?????살첒??
//??쀬겱??롫뮉 ??됱뇚 ?????삳굶????볥궗
#include<stdexcept>

#include<QWidget>

//Qt????源???룐뫂遊?? ?怨뚭퍙??뤿선 ?臾먮짗??롫뮉 ?⑥쥙?붶빳? ???????????
//?諭????볦퍢 揶쏄쑨爰쏙쭕?덈뼄 timeout() ??볥젃?癒?뱽 獄쏆뮇源??뽱룖??
//域???볥젃?癒?퓠 ?怨뚭퍙????λ땾(???????癒?짗??곗쨮 ?紐꾪뀱??곸㉡.
#include<QTimer>



//????삳쐭??Microsoft??Direct3D 11 API???類ㅼ벥?????뵬嚥?
//??쇰펶??域밸챶????귐딅꺖??? ?怨쀬뵠?? ???쐭筌????뵠?袁⑥뵬?紐꾩뱽 ??뽯선??????덈뮉 COM ?紐낃숲??륁뵠??삳굶????볥궗

//API (Application Programming Interface)
//?袁⑥쨮域밸챶?믥솒硫? ??뽯뮞??疫꿸퀡????????????뉗쓺 ??곻폒???紐낃숲??륁뵠??
//ex) DirectX, OpenGL, Vulkan

//?怨쀬뵠??(Shader)
//GPU?癒?퐣 ??쎈뻬??롫뮉 ?臾? ?袁⑥쨮域밸챶??
//域밸챶????怨쀬뵠?怨? 筌ｌ꼶???랁??遺얇늺????堉멨칰?癰귣똻?わ쭪? 野껉퀣???
//?ル굝履?: 
//?類ㅼ젎 ?怨쀬뵠??(Vertex Shader) : ?袁⑺뒄, ???읈, ??由???筌ｌ꼶??
//??? ?怨쀬뵠??(Pixel Shader) : ??깃맒, 鈺곌퀡梨? ??용뮞筌?筌ｌ꼶??
//HLSL, GLSL 揶쏆늿? ?怨쀬뵠???紐꾨선嚥??臾믨쉐??
//?怨쀬뵠?遺얜뮉 GPU???④쑴沅??貫?????뽰뒠??곴퐣 ??쇰뻻揶?域밸챶???뚯뱽 ??쀫립??롫뮉 ???뼎

//???쐭筌?(Rendering)
//?怨쀬뵠?怨? ?遺얇늺??域밸챶????⑥눘??
//3D筌뤴뫀?? ??용뮞筌? 鈺곌퀡梨? 燁삳?李???源놁뱽 ?④쑴沅??곴퐣 
//筌ㅼ뮇伊?怨몄몵嚥????嚥?癰궰??묐퉸 ?遺얇늺???곗뮆???롫뮉 野?
//???쐭筌?野껉퀗?드첎? ?怨뺚봺揶쎛 癰귣???野껊슣?? UI, ?怨멸맒 ??

//???쐭筌????뵠?袁⑥뵬??(Rendering Pipeline)
//1. ??낆젾 鈺곌퀡??(Input Assembly) : ?類ㅼ젎 ?怨쀬뵠?怨? GPU???袁⑤뼎
//2. ?類ㅼ젎 ?怨쀬뵠??: ?袁⑺뒄 ?④쑴沅?
//3. ??뤿뮞?怨뺤뵬??? : ??⑥퍟?類ㅼ뱽 ???嚥?癰궰??
//4. ??? ?怨쀬뵠??: ??깃맒, 鈺곌퀡梨??④쑴沅?
//5. ?곗뮆??癰귣쵑鍮 (Output Merger) : 筌ㅼ뮇伊????筌왖 ??밴쉐

//COM ?紐낃숲??륁뵠??(Component Object Model)
//Windows?癒?퐣 揶쏆빘猿쒐몴???삼펷??獄쎻뫗??
//DirectX??COM 疫꿸퀡而??곗쨮 ??블??
//ex) ID3D11Device, ID3D11Buffer 揶쏆늿? 揶쏆빘猿??COM ?紐낃숲??륁뵠??
//?諭彛?: QueryInterface, AddRef, Release 揶쏆늿? 筌롫뗄苑??뺤쨮 ?온??
//揶쏆빘猿?揶????뻿??筌롫뗀?덄뵳??온?귐? ????酉釉?
//COM?? Directx?癒?퐣 GPU ?귐딅꺖??? ??삼폁 ???袁⑸땾?怨몄뵥 ?닌듼?

//QueryInterface : 揶쏆빘猿쒎첎? 筌왖?癒곕릭????삘뀲 ?紐낃숲??륁뵠??? ?遺욧퍕??롫뮉 筌롫뗄苑??
//COM 揶쏆빘猿??????疫꿸퀡????紐낃숲??륁뵠??살쨮 ??롫떊????볥궗
//QueryInterface ?紐꾪뀱 ???諭??疫꿸퀡???筌왖?癒곕릭筌??紐낃숲??륁뵠??????怨? 獄쏆꼹??
//筌왖?癒곕릭筌왖 ??놁몵筌?E_NOINTERFACE ??살첒 獄쏆꼹??

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>

#include <D3Dcompiler.h>
#include <directxmath.h>
using namespace DirectX;
#include "AlignedAllocationPolicy.h"


class D3DClass;
class CameraClass;
class ModelClass;
class ColorShader;


class QDirect3D11Widget : public QWidget
{
	Q_OBJECT

public:
	QDirect3D11Widget(QWidget * parent);
	~QDirect3D11Widget();

	void release();
	void resetEnvironment();

	void run();
	void pauseFrames();
	void continueFrames();

	bool init();
private:
	

	void beginScene();
	void endScene();

	void tick();
	void render();

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
	bool nativeEvent(const QByteArray & eventType, void * message, long * result) override;
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

	// Getters / Setters
public:
	HWND const & nativeHandle() const { return m_hWnd; }

	ID3D11Device *           device() const { return m_pDevice; }
	ID3D11DeviceContext *    deviceContext() { return m_pDeviceContext; }
	IDXGISwapChain *         swapChain() { return m_pSwapChain; }
	//ID3D11RenderTargetView * TargetView() const { return m_pRTView; }
	std::vector<ID3D11RenderTargetView*> TargetView() const{
		return m_RTViews;
	}
	bool renderActive() const { return m_bRenderActive; }
	void setRenderActive(bool active) { m_bRenderActive = active; }

	D3DCOLORVALUE * BackColor() { return &m_BackColor; }

private:
	
	ID3D11DeviceContext *    m_pDeviceContext;
	IDXGISwapChain *         m_pSwapChain;
	//ID3D11RenderTargetView * m_pRTView;
	std::vector<ID3D11RenderTargetView*> m_RTViews;

	QTimer m_qTimer;

	HWND m_hWnd;
	bool m_bDeviceInitialized;

	bool m_bRenderActive;
	bool m_bStarted;

	D3DCOLORVALUE m_BackColor;
public:
	ID3D11Device* m_pDevice;
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


