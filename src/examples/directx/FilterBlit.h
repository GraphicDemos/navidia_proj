//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_FilterBlit
// File:  FilterBlit.h
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

#ifndef FILTERBLIT_H
#define FILTERBLIT_H

#include <windows.h>
#include <d3d9.h>
#include <shared/MouseUI9.h>

//-----------------------------------------------------------------------------
// Name: class FilterBlit
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. FilterBlit 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class FilterBlit
{
public: 

    // data members
    MouseUI *m_pUI;
    HMENU   m_main_menu;
    HMENU   m_context_menu;
    LPD3DXEFFECT m_pEffect;
    LPD3DXFONT   m_pFont;

    // methods
    HRESULT CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice);
    void    CreateAndWriteUVOffsets(int width, int height);

    typedef enum 
    {
        FIRST_FILTER_OPTION = 0,
        CONE_FILTER     = 0,
        BOX9_FILTER     ,
        BOX16_FILTER    ,
        SHARPEN_FILTER  ,
        LUMINANCE_EDGE  ,
        NUM_FILTER_OPTIONS
    } eFilterOptions;

    typedef struct tagQuadVertex
    {
        D3DXVECTOR3 Position;
        D3DXVECTOR2 Tex;
    } QuadVertex;

    enum 
    {
        kMaxNumPasses = 3,
    };

    eFilterOptions           meDisplayOption;
    eFilterOptions           meInitDisplayOption;
    bool                     mbWireframe;

    LPDIRECT3DVERTEXBUFFER9  m_pVertexBuffer;

    LPDIRECT3DSURFACE9       mpBackbufferColor; 
    LPDIRECT3DSURFACE9       mpBackbufferDepth; 

    LPDIRECT3DTEXTURE9       mpTextureToFilter;
    LPDIRECT3DTEXTURE9       mpTextureFiltered[kMaxNumPasses];       
    LPDIRECT3DSURFACE9       mpFilterTarget   [kMaxNumPasses]; 

    FilterBlit();

    HRESULT InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice); // called just before device is Reset
    HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);    // called when device is restored
    HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
};

#endif