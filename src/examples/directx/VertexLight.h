//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VertexLight
// File:  VertexLight.h
// 
// Copyright NVIDIA Corporation 2002-2003
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Note: This code uses the D3D Framework helper library.
//
//-----------------------------------------------------------------------------

#ifndef VERTEXLIGHT_H
#define VERTEXLIGHT_H

#include <windows.h>

#include "dxstdafx.h"
#include <DXUT/SDKmisc.h>
#include "special/nvmesh.h"

#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)

typedef enum tagLightingType
{
    LIGHTTYPE_POINT = 0,
    LIGHTTYPE_DIRECTIONAL,
    LIGHTTYPE_MANYPOINT,
    LIGHTTYPE_TWOSIDE,
      LIGHTTYPE_NUM_VALUES
} LightType;

// A special vertex with a face normal
class SphereVertex
{
public:
    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
    D3DXVECTOR3 FaceNormal;
};

//-----------------------------------------------------------------------------
// Name: class VertexLight
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. VertexLight 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class VertexLight
{
private: 

    // data members
    bool    m_bWireframe;
    bool    m_bPause;
    float   m_fAngle;

    // timekeeping
    float   m_time;
    float   m_animTime;
    float   m_startTime;
    int     m_frame;

    // The light we are currently drawing
    DWORD m_dwCurrentLightDraw;    

    // our main Effect file, which houses our techniques (shaders)
    LPD3DXEFFECT m_pEffect;
    char m_szCurrentTechnique[256];

    // Light colors
    std::vector<D3DXCOLOR> m_LightColorDiffuse;
    std::vector<D3DXCOLOR> m_LightColorSpecular;
    std::vector<D3DXCOLOR> m_LightColorAmbient;

    // Geometry
    IDirect3DVertexBuffer9* m_pSphereBuffer;
	NVMesh * m_pShapeMesh;
	NVMesh*    m_pLightMesh;

    DWORD m_dwSphereFaces;
    DWORD m_dwCurrentShader;
    DWORD m_dwFixedColorShader;
    DWORD m_dwNumLights;

    // added methods
    HRESULT SetTransform(D3DXMATRIX& world);
    HRESULT OnSetMaterial(D3DMATERIAL9* pMat);
    HRESULT GenerateSphere(D3DXVECTOR3& vCenter, FLOAT fRadius, WORD wNumRings, WORD wNumSections, FLOAT sx, FLOAT sy, FLOAT sz, IDirect3DDevice9* pd3dDevice);

public: 
    void		ToggleWireframe();
    void		TogglePause();
    D3DXMATRIX	m_matWorld;
    LightType	m_LightType;

	// view & projection matrices
	D3DXMATRIX m_view, m_proj;

    VertexLight();
	// implemented virtual functions:
    HRESULT InvalidateDeviceObjects(); // called just before device is Reset
    HRESULT RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice);    // called when device is restored
    HRESULT Render(IDirect3DDevice9* pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
	HRESULT OnChangeLightType(IDirect3DDevice9* pd3dDevice);
};

#endif