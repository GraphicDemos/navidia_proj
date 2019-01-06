//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VertexMorph
// File:  VertexMorph.h
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

#ifndef VERTEXMORPH_H
#define VERTEXMORPH_H

#include <windows.h>
#include <d3d9.h>
#include <shared/MouseUI9.h>
#include "special/nvmesh.h"
#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)

class DolphinVertex
{
public:
    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
    D3DXVECTOR2 Texture;
};

typedef DolphinVertex SeaFloorVertex;
#define DOLPHINVERTEX_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define SEAFLOORVERTEX_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

//-----------------------------------------------------------------------------
// Name: class VertexMorph
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. VertexMorph 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class VertexMorph
{
private: 
	
	// data members
    bool    m_bWireframe;

    // our main Effect file, which houses our techniques (shaders)
    LPD3DXEFFECT m_pEffect;

    // timekeeping
    float   m_time;
    float   m_startTime;
    int     m_frame;

    // the dolphin
    IDirect3DVertexBuffer9* m_pDolphinVB[3];
    IDirect3DIndexBuffer9* m_pDolphinIB;
    IDirect3DVertexDeclaration9* m_pDeclaration; 
    DWORD m_dwNumIndices;
    DWORD m_dwNumVertices;

    // the sea floor
    NVMesh* m_pFloorMesh;
    LPDIRECT3DTEXTURE9 m_pDolphinMap;
    std::vector<LPDIRECT3DTEXTURE9> m_vecCausticTextures;
	 
	// view & projection matrices
    D3DXMATRIX m_view, m_proj;

    // added methods
    HRESULT SetTransform(D3DXMATRIX& world);

public: 
	void    ToggleWireframe()	{ m_bWireframe = !m_bWireframe; };
    VertexMorph();
    // implemented virtual functions:
    HRESULT InvalidateDeviceObjects(); // called just before device is Reset
    HRESULT RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice);    // called when device is restored
    HRESULT Render(IDirect3DDevice9* pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
};

#endif