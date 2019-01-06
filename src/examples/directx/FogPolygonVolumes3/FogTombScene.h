/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombScene.h

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

#ifndef H_FOGTOMBSCENE_H
#define H_FOGTOMBSCENE_H

#include <NV_D3DCommon/NV_D3DCommonTypes.h>
#pragma warning ( disable : 4005 )		// macro redefinition
#include "dxstdafx.h"
#include <MeshVB.h>
#include <Mesh.h>
#include <DXUT/DXUTcamera.h>
class ThicknessRenderProperties;

//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class FogTombScene
{
public:
	LoadXFile *					m_pLoadXFile;		// NV_D3DCommon utility class, holds the .X file mesh objects
    CFirstPersonCamera			m_Camera;
	IDirect3DDevice9 *			m_pd3dDevice;
	ThicknessRenderProperties * m_pProperties;

	MeshVB *	m_pFogMeshVB;
	Mesh *		m_pFogMesh;
	Mesh *		m_pFogMeshAnimated;

	float		m_fThicknessToColorScaleFactor;

	IDirect3DTexture9 **	m_ppFogColorRamp;

	TextureFactory *		m_pTextureFactory;		// use the m_ppTextureFactory
	TextureFactory **		m_ppTextureFactory;

	D3DXMATRIX				m_matViewProj;					// combined view*projection matrix
	D3DXMATRIX				m_matWorldViewProjTranspose;	// for vshader vertex transform with 4 DP4 calls

	HRESULT CreateFogMesh();
	HRESULT CreateFogMesh2();
	HRESULT FindBoxCorners( Mesh * pMesh, UINT * tc00, UINT * tc10, UINT * tc01, UINT * tc11,
										UINT * bc00, UINT * bc10, UINT * bc01, UINT * bc11 );
	HRESULT	FreeFogMesh();
	HRESULT AnimateFogMesh( float fTimeStepInSeconds );

	// The Calculate..() functions set either m_matViewProj or m_matWorldViewProjTranspose
	D3DXMATRIX * CalculateViewProjMatrix();
	D3DXMATRIX * GetViewProjMatrix()					{ return( & m_matViewProj ); };
	D3DXMATRIX * ApplyWorldToViewProjMatrixAndTranspose( const D3DXMATRIX * pWorld );
	D3DXMATRIX * GetWVPTransMatrix()					{ return( & m_matWorldViewProjTranspose ); };

public:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects( IDirect3DDevice9 * pDev, TextureFactory ** ppTextureFactory = NULL );
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove( float fElapsedTime, bool bAnimateFogVolume = true );

    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    FogTombScene();
    virtual ~FogTombScene();
	void SetAllNull()
	{
		m_pFogMeshVB			= NULL;
		m_pFogMesh				= NULL;
		m_pFogMeshAnimated		= NULL;
		m_pLoadXFile			= NULL;
		m_pd3dDevice			= NULL;
		m_ppFogColorRamp		= NULL;
		m_pTextureFactory		= NULL;
		m_ppTextureFactory		= NULL;

		m_pProperties	= NULL;
		m_fThicknessToColorScaleFactor = 204.0f;
	}
};

#endif

