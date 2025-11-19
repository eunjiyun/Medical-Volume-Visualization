#include <DirectXMath.h>
#include <Windows.h>
#include "ArcBall.h"

ArcBall::ArcBall(void)
{
	Reset() ;
	m_vDownPt    = XMFLOAT3( 0, 0, 0);
	m_oldPt	     = XMFLOAT3( 0, 0, 0);
	m_vCurrentPt = XMFLOAT3( 0, 0, 0);
	m_Offset.x   = 0 ;
	m_Offset.y   = 0 ;
}

void ArcBall::Init(float nWidth, float nHeight)
{
	SetWindow(nWidth, nHeight);
}

ArcBall::~ArcBall()
{
}

void ArcBall::Reset()
{
	m_qDown      = XMVectorSet(0, 0, 0, 1);
	m_qNow       = XMVectorSet(0, 0, 0, 1);
	m_increament = XMVectorSet(0, 0, 0, 1);
	m_mRotation  = XMMatrixIdentity();
	m_bDrag      = FALSE;
	m_fRadius    = 1.0f;
}

void ArcBall::OnBegin(float nX, float nY)
{
	// enter drag state only if user click the window's client area
	if( nX >= m_Offset.x && 
		nX <= m_Offset.x + m_nWidth &&
		nY >= m_Offset.y &&
		nY < m_Offset.y + m_nHeight)
	{
		m_bDrag = true ; // begin drag state
		m_qDown = m_qNow ;
		m_vDownPt = ScreenToVector(nX, nY);
		m_oldPt = m_vDownPt ;
	}
}

void ArcBall::OnMove(float nX, float nY)
{
	if(m_bDrag)
	{
		m_vCurrentPt = ScreenToVector(nX, nY);
		m_increament = QuatFromBallPoints( m_oldPt, m_vCurrentPt );

		m_qNow = XMQuaternionMultiply(m_qDown, QuatFromBallPoints( m_vDownPt, m_vCurrentPt));
		m_oldPt = m_vCurrentPt ;
	}
}

void ArcBall::OnEnd()
{
	m_bDrag = false ;
}

void ArcBall::SetOffset( INT nX, INT nY )
{
	m_Offset.x = nX ; 
	m_Offset.y = nY ;
}

void ArcBall::SetWindow(float nWidth, float nHeight, float fRadius)
{
	 m_nWidth  = nWidth; 
	 m_nHeight = nHeight; 
	 m_fRadius = fRadius; 
	 m_vCenter = XMFLOAT2(m_nWidth / 2.0f, m_nHeight / 2.0f);
}

XMVECTOR ArcBall::GetRotationQuat()
{
	return m_qNow ;
}

XMMATRIX ArcBall::GetRotationMatrix()
{
	m_mRotation = XMMatrixRotationQuaternion(m_qNow) ;
	return m_mRotation;
}

XMVECTOR ArcBall::GetRotationQuatIncreament()
{
	return m_increament ;
}

XMMATRIX ArcBall::GetRotationMatrixIncreament()
{
	m_mRotation = XMMatrixRotationQuaternion(m_increament);
	return m_mRotation;
}

XMVECTOR ArcBall::QuatFromBallPoints(XMFLOAT3 startPoint, XMFLOAT3 endPoint )
{
	XMVECTOR dotVector = XMVector3Dot(XMLoadFloat3(&startPoint), XMLoadFloat3(&endPoint));
	float fDot = XMVectorGetX(dotVector);

	XMVECTOR vPart;
	vPart = XMVector3Cross(XMLoadFloat3(&startPoint), XMLoadFloat3(&endPoint));

	//XMVECTOR result = XMVectorSet(XMVectorGetX(vPart), XMVectorGetY(vPart), XMVectorGetZ(vPart), fDot);
	//return result;

	// ... 기존 코드 ...
	XMVECTOR result = XMVectorSet(XMVectorGetX(vPart), XMVectorGetY(vPart), XMVectorGetZ(vPart), fDot);
	return XMQuaternionNormalize(result);  // ✅ 추가
}

XMFLOAT3 ArcBall::ScreenToVector(float fScreenPtX, float fScreenPtY)
{
	// Scale to screen
	FLOAT x = -( fScreenPtX - m_Offset.x - m_nWidth / 2 ) / ( m_fRadius * m_nWidth / 2 );
	FLOAT y = ( fScreenPtY - m_Offset.y - m_nHeight / 2 ) / ( m_fRadius * m_nHeight / 2 );

	FLOAT z = 0.0f;
	FLOAT mag = x * x + y * y;

	if( mag > 1.0f )
	{
		FLOAT scale = 1.0f / sqrtf( mag );
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf( 1.0f - mag );

	return XMFLOAT3( x, y, z);
}

// ArcBall.h에 추가
void ArcBall::SetRotation(XMVECTOR quat)
{
	m_qNow = quat;
	m_qDown = quat;
}