/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  MouseUI.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:

* Name conflice with DEMOS\Direct3D8\inc\shared\MouseUI.h.  Resolve by specifying
#include "NV_D3DCommon\MouseUI.h"

8/7/2003 - Now has MouseUI.cpp

This is a modified version of Sim's original MouseUI.h.
This one requires you call LButtonDown() and LButtonUp(), but you no longer
 have to pass in the flags parameter.
It has also been moved to the NV_D3DCommon framework so it can be compiled 
 for either D3D8 or D3D9.
It also provides additional options about the mouse input.

By Sim Dietrich & Cem Cebenoyan
and now Cyril Zeller, Greg J.

// What it does :
//
// This UI object supports simple rotation through dragging with the left mouse button held down.
// It also has virtual methods for left button down & up, which could be overriden by a derived
// GUI object that handled buttons or something.
// Cyril Zeller: Added a mode (m_bYawPitchOnly) where rotation is constrained to yaw and pitch (no roll) (useful for camera motion)
// 
// Usage:
//
// Create an instance of the MouseUI object, and pass in the Rect of the window you want it to refer to.
// I originally had it use the HWND of the window, but it occurred to me that we really want to have
// the ability to do sub-areas, for instance if you had a 3D gui, and you only wanted the smaller viewport
// to respond to mouse messages.
//
// If the window resizes, you should call SetRECT() with the new rect size.
//
// One other nice thing about this class is that you don't have to check if the mouse x & y is in the rect.
// The class will always check for you on all button/mouse move messages.

//@@ do this! =)
Greg J:  Added option to override the check x,y out-of-bounds so that mouse control can still
respond if mouse pointer goes outside the window during a drag operation.

// Just call MouseMove when appropriate and the MouseUI will update its internal RotationMatrix.  It doesn't
// orthonormalize the matrix, so it may go wacky eventually.
//
// Then call GetRotationMatrix().  You can use the matrix to rotate around the object, the view or the world as appropriate.
// Then call GetTranslationMatrix().  You can use the matrix to translate the object, the view or the world as appropriate.
//
// To get the rotation & translation matrices back to identity, call Reset()
//
// You can also get/set the sensitivity of the rotations through the Get/SetSensitivityFactor() calls.
// The SensitivityFactor defaults to 1.0f
//

******************************************************************************/



#ifndef H_D3DCOMMON_MOUSEUI_H
#define H_D3DCOMMON_MOUSEUI_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"

#include "NV_D3DCommon_decl_.h"
//@@@ re-work - spec defaults in initialize
static const float MOUSEUI_TRANSLATION_SCALE = 0.05f;
static const float MOUSEUI_ROTATION_SCALE = 1.0f;


class DECLSPEC_NV_D3D_COMMON_API MouseUI
{
protected :
	
    bool m_bLButtonDown;
	bool m_bYawPitchOnly;
    bool m_bUseCameraControl;
	
	unsigned int m_uStartX;
	unsigned int m_uStartY;
	
	unsigned int m_uWidth;
	unsigned int m_uHeight;
	
	D3DXMATRIX	m_MatRotation;
	D3DXMATRIX	m_MatTranslation;
	float		m_fYaw, m_fPitch;
	
	float m_fRotationSensitivity;
	float m_fTranslationSensitivity;
	

	D3DXMATRIX		m_mViewOrientationMatrix;

	RECT m_WindowCoordRect;
	
	void SetLocation( const unsigned int& x, const unsigned int& y )
	{
		m_uStartX = max( (unsigned int)m_WindowCoordRect.left, x );
		m_uStartX = min( (unsigned int)m_WindowCoordRect.right, m_uStartX );
		
		m_uStartY = max( (unsigned int)m_WindowCoordRect.top, y );
		m_uStartY = min( (unsigned int)m_WindowCoordRect.bottom, m_uStartY );
	}
	
	void SetSize()
	{
		m_uWidth =  ( m_WindowCoordRect.right - m_WindowCoordRect.left );
		m_uHeight = ( m_WindowCoordRect.bottom - m_WindowCoordRect.top );
	}
	
public :
	
	enum ViewMode
	{
		VM_STANDARD_ZAXIS,
		VM_CAMERA_AWARE,
		VM_FORCEDWORD = 0xffffffff	
	};

	ViewMode	m_eViewMode;


	MouseUI::MouseUI( const RECT& theRect, bool bCameraControl = false, bool bYawPitchOnly = false ) 
            : m_WindowCoordRect( theRect )
            , m_bUseCameraControl( bCameraControl )
			, m_bYawPitchOnly(bYawPitchOnly)
			, m_fYaw(0.0f), m_fPitch(0.0f)
	{
		m_eViewMode = VM_STANDARD_ZAXIS;
		Reset();
	}
	

	void SetControlOrientationMatrix( D3DXMATRIX matrix )
	{
		// Send in your view matrix so the controls respond with
		//  the correct translation & rotation relative to it
		// Used with VM_CAMERA_AWARE view mode.

		// zero translation and invert matrix so the vieworientation
		//  takes things from view space to world space
		matrix._41 = 0.0f;
		matrix._42 = 0.0f;
		matrix._43 = 0.0f;

		matrix._14 = 0.0f;
		matrix._24 = 0.0f;
		matrix._34 = 0.0f;

		matrix._44 = 1.0f;

		D3DXMatrixInverse( &m_mViewOrientationMatrix, NULL, &matrix );
	}

	void SetViewMode( ViewMode mode ) { m_eViewMode = mode; }


	RECT GetRECT() const { return m_WindowCoordRect; }
	
	void SetRECT( const RECT& rhs ) 
	{
		m_WindowCoordRect = rhs; 
		SetSize();
	}
	
	void GetRotationMatrix( D3DXMATRIX* pMatrix ) const { (*pMatrix) = m_MatRotation; }
	
	D3DXMATRIX GetRotationMatrix() const { return m_MatRotation; }
	D3DXMATRIX * GetRotationMatrixPtr() { return( & m_MatRotation ); }
	
	void GetTranslationMatrix( D3DXMATRIX* pMatrix ) const { (*pMatrix) = m_MatTranslation; }
	
	D3DXMATRIX GetTranslationMatrix() const { return m_MatTranslation; }
	D3DXMATRIX * GetTranslationMatrixPtr() { return( & m_MatTranslation ); }
	
	void SetTranslationMatrix( const D3DXMATRIX & mat )
	{
		m_MatTranslation = mat;
	}

	void SetRotationMatrix( const D3DXMATRIX & mat )
	{
		m_MatRotation = mat;
		m_fYaw = atan2f(mat._13, mat._11);
		m_fPitch = asinf(mat._32);
	}

	void SetMatrix( const D3DXMATRIX & mat )
	{
		D3DXMATRIX rot = mat;
		rot._41 = rot._42 = rot._43 = 0.0f;
		SetRotationMatrix(rot);
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, mat._41, mat._42, mat._43);
		SetTranslationMatrix(trans);
	}

	bool IsMouseDown() const { return ( m_bLButtonDown ); }
	
	void Reset()
	{
		SetSize();
		m_fRotationSensitivity = 1.0f * MOUSEUI_ROTATION_SCALE;
		m_fTranslationSensitivity = 1.0f * MOUSEUI_TRANSLATION_SCALE;
		m_bLButtonDown = false;
		SetLocation( 0, 0 );
		D3DXMatrixIdentity( &m_MatRotation );
		D3DXMatrixIdentity( &m_MatTranslation );
	}
	
	bool IsInWindow( const unsigned int& x, const unsigned int& y )
	{
		SetSize();

		bool res;
		res =			( x >= (unsigned int) m_WindowCoordRect.left );
		res = res &&	( x <  (unsigned int) m_WindowCoordRect.right );
		res = res &&	( y >= (unsigned int) m_WindowCoordRect.top  );
		res = res &&	( y <  (unsigned int) m_WindowCoordRect.bottom );

		return( res );
	}
	

	float GetTranslationalSensitivityFactor() const
	{ 
		return m_fTranslationSensitivity / MOUSEUI_TRANSLATION_SCALE;
	}	
	void  SetTranslationalSensitivityFactor( const float& rhs )
	{ 
		m_fTranslationSensitivity = rhs * MOUSEUI_TRANSLATION_SCALE;
	}

	float GetRotationalSensitivityFactor() const 
	{ 
		return m_fRotationSensitivity / MOUSEUI_ROTATION_SCALE;
	}	
	void  SetRotationalSensitivityFactor( const float & rhs )
	{ 
		m_fRotationSensitivity = rhs * MOUSEUI_ROTATION_SCALE;
	}
	
		// accepts window coordinates
	virtual void OnLButtonDown( const unsigned int& x, const unsigned int& y );	
	virtual void OnLButtonUp( const unsigned int& x, const unsigned int& y );

	virtual void Translate( const float& X, const float& Y, const float& Z );	
	virtual void Translate( D3DXVECTOR3 const & t);




	virtual void Rotate( float fYaw, float fPitch, float fRoll);

    virtual void OnMouseMove( const unsigned int & x, const unsigned int & y );
};



#endif	H_D3DCOMMON_MOUSEUI_H



