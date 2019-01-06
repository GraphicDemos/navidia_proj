/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  MouseUI.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DCommonDX9PCH.h"

void MouseUI::Translate( const float& X, const float& Y, const float& Z )
{
	D3DXMATRIX mov;
	D3DXMatrixTranslation( &mov, X, Y, Z );
	D3DXMatrixMultiply( &m_MatTranslation, &m_MatTranslation, &mov );
}

void MouseUI::Translate( D3DXVECTOR3 const & t)
{
	D3DXMATRIX mov;
	D3DXMatrixTranslation( &mov, t.x, t.y, t.z);
	D3DXMatrixMultiply( &m_MatTranslation, &m_MatTranslation, &mov );
}

void MouseUI::Rotate(float fYaw, float fPitch, float fRoll)
{
	D3DXMATRIX rot;
	D3DXMatrixRotationYawPitchRoll(&rot, fYaw, fPitch, fRoll);
	D3DXMatrixMultiply(&m_MatRotation, &m_MatRotation, &rot);
}


void MouseUI::OnLButtonDown( const unsigned int& x, const unsigned int& y )
{
	if ( IsInWindow( x, y ) )
	{
		SetLocation( x, y );
		m_bLButtonDown = true;
	}
	else
	{
		OnLButtonUp( x, y );
	}
}

void MouseUI::OnLButtonUp( const unsigned int& x, const unsigned int& y )
{
	SetLocation( x, y );
	m_bLButtonDown = false;
}

void MouseUI::OnMouseMove( const unsigned int& x, const unsigned int& y )
{
	D3DXVECTOR3		vTrans;
	D3DXVECTOR4		vT4;

	if ( !IsInWindow( x, y ) )
	{
		OnLButtonUp( x, y );
	}
	else if ( IsMouseDown() )
	{ 
		float deltax = ( (float)x - (float)m_uStartX );
		float deltay = ( (float)y - (float)m_uStartY );
		if ( !( ( deltax == 0.0f ) && ( deltay == 0.0f ) ) )
		{
			bool bShift = ( ( GetAsyncKeyState( VK_LSHIFT )   < 0 ) || ( GetAsyncKeyState( VK_SHIFT    ) < 0 ) || ( GetAsyncKeyState( VK_RSHIFT )  < 0 ) );
			//bool bAlt =   ( ( GetAsyncKeyState( VK_LMENU )    < 0 ) || ( GetAsyncKeyState( VK_RMENU    ) < 0 ) || ( GetAsyncKeyState( VK_MENU )    < 0 ) );
			bool bCtl =   ( ( GetAsyncKeyState( VK_LCONTROL ) < 0 ) || ( GetAsyncKeyState( VK_RCONTROL ) < 0 ) || ( GetAsyncKeyState( VK_CONTROL ) < 0 ) );
			if (bShift || bCtl) { //translation
				if (m_bUseCameraControl)
				{
					D3DXMATRIX matLookRotations;
					D3DXMatrixTranspose(&matLookRotations, &m_MatRotation);
					D3DXVECTOR3 camLookDirection( 0.0f, 0.0f, 1.0f );
					D3DXVec3TransformNormal(&camLookDirection, &camLookDirection, &matLookRotations);

					D3DXVECTOR3 const   yVector(0.0f, 1.0f, 0.0f);
					float       const   h = D3DXVec3Dot(&yVector, &camLookDirection);
					D3DXVECTOR3         xzProjectedLookDirection(camLookDirection - h*yVector);

					D3DXVec3Normalize(&xzProjectedLookDirection, &xzProjectedLookDirection);

					D3DXVECTOR3         rightVector;         
					D3DXVec3Cross(&rightVector, &yVector, &xzProjectedLookDirection);

					D3DXVECTOR3         transVector;         

					if ( bShift )
					{
						transVector  =  deltax * m_fTranslationSensitivity * rightVector;
						transVector += -deltay * m_fTranslationSensitivity * yVector;

						Translate( transVector );
					}
					else
					{
						transVector  =  deltay * m_fTranslationSensitivity * xzProjectedLookDirection;
						Translate( transVector );
					}
				}
				else
				{

					if ( bShift )
					{
						vTrans = D3DXVECTOR3(	 deltax * m_fTranslationSensitivity,
												-deltay * m_fTranslationSensitivity,
												 0.0f );
					}
					else 
					{
						vTrans = D3DXVECTOR3(	 0.0f,
												 0.0f,
												 deltay * m_fTranslationSensitivity );
					}
					switch( m_eViewMode )
					{
					case VM_CAMERA_AWARE:

						// Transform the desired view space movement into world space
						//  so the objects move appropriately given the camera view
						D3DXVec3TransformCoord( &vTrans, &vTrans, &m_mViewOrientationMatrix );
						
					case VM_STANDARD_ZAXIS:
					default:
						// do nothing - use the translation vector set above

						break;
					}

					Translate( vTrans );
				}
			}
			else { //rotation
				if (m_bYawPitchOnly) {
					m_fYaw -= D3DXToRadian(- m_fRotationSensitivity * deltax);
					if (m_fYaw < - 180.0f)
						m_fYaw += 360.0f;
					else if (m_fYaw > 180.0f)
						m_fYaw -= 360.0f;
					m_fPitch -= D3DXToRadian(- m_fRotationSensitivity * deltay);
					if (m_fPitch < - 90.0f)
						m_fPitch += 360.0f;
					else if (m_fPitch > 90.0f)
						m_fPitch -= 360.0f;
					D3DXMATRIX oldRot;
					D3DXMatrixTranspose(&oldRot, &m_MatRotation);
					D3DXMatrixRotationYawPitchRoll(&m_MatRotation, m_fYaw, m_fPitch, 0.0f);
					D3DXMatrixTranspose(&m_MatRotation, &m_MatRotation);
					D3DXVECTOR3 oldTrans(m_MatTranslation._41, m_MatTranslation._42, m_MatTranslation._43);
					D3DXVECTOR3 newTrans;
					D3DXVec3TransformCoord(&newTrans, &oldTrans, &oldRot);
					D3DXVec3TransformCoord(&newTrans, &newTrans, &m_MatRotation);
					D3DXMatrixTranslation(&m_MatTranslation, newTrans.x, newTrans.y, newTrans.z);
				}
				else 
				{
					float mag = (float)sqrt( deltax * deltax + deltay * deltay );
					float dx = ( deltax / mag );
					float dy = ( deltay / mag );
					
					//find a vector in the plane perpendicular to delta vector
					float perpx = dy;
					float perpy = dx;
					
					//rotate around this vector
					D3DXVECTOR3 axis( perpx, perpy, 0.0f );

					if( m_eViewMode == VM_CAMERA_AWARE )
					{
						// transform the rotation axis from view to world space also:
						D3DXVec3TransformCoord( &axis, &axis, &m_mViewOrientationMatrix );
					}
					D3DXMATRIX deltaRot;
					D3DXMatrixRotationAxis( &deltaRot, &axis, D3DXToRadian( -m_fRotationSensitivity * mag ) );
					D3DXMatrixMultiply( &m_MatRotation, &m_MatRotation, &deltaRot );
				}
			}
		}
		SetLocation( x, y );
	}
}


