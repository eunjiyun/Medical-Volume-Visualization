#ifndef __ARCBALL_H__
#define __ARCBALL_H__

using namespace DirectX;

class ArcBall
{
public:
	ArcBall(void);
	~ArcBall(void);

	void    Init(float nWidth, float nHeight);
	void	Reset();
	void	OnBegin(float nX, float nY);
	void	OnMove(float nX, float nY);
	void	OnEnd();

	XMVECTOR QuatFromBallPoints(XMFLOAT3 startPoint, XMFLOAT3 endPoint);
	XMMATRIX GetRotationMatrix();
	XMMATRIX GetRotationMatrixIncreament();
	XMVECTOR GetRotationQuat();
	XMVECTOR GetRotationQuatIncreament();
	void SetOffset(INT nX, INT nY);
	void SetWindow(float nWidth, float nHeight, float fRadius = 0.9f);
	void SetRotation(XMVECTOR quat);

private:
	POINT	m_Offset;	// window offset
	float	m_nWidth;	// arc ball's window width
	float	m_nHeight; // arc ball's window height
	float	m_fRadius;	// arc ball's radius in screen coordinates
	bool	m_bDrag;	// whether the arc ball is dragged

	XMFLOAT2	m_vCenter;		// center of arc ball
	XMVECTOR	m_qDown;		// quaternion before mouse down
	XMVECTOR	m_qNow;		// current quaternion
	XMVECTOR	m_increament;	// rotation increment 
	XMFLOAT3	m_vDownPt;		// starting point of arc ball rotate
	XMFLOAT3	m_oldPt;		// old point 
	XMFLOAT3	m_vCurrentPt;	// current point of arc ball rotate
	XMMATRIX	m_mRotation;

	// Convert scree point to arcball point(vector)
	XMFLOAT3    ScreenToVector(float fScreenPtX, float fScreenPtY);

};

#endif // end __ARCBALL_H__