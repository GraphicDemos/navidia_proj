//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_PaletteSkin
// File:  shader_PaletteSkin.h
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

#ifndef SHADER_PALETTESKIN_H
#define SHADER_PALETTESKIN_H

#include <windows.h>
#include <d3d9.h>
#include <shared/MouseUI9.h>
#include "NVBScene_Skin.h"

//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class ShaderPaletteSkin
{
public: 
    
    // data members
    MouseUI *m_pUI;
    float   m_animTime;
    bool    m_bWireframe;
    bool    m_bAnimateLight;
    bool    m_bPaused;

    // the effect
    LPD3DXEFFECT m_pEffect;

    // geometry
    NVBScene_Skin* m_pScene;
    std::vector<DWORD> m_Declaration;
    IDirect3DVertexDeclaration9* m_pDeclaration;
    
    // our matrices, etc.
    D3DXMATRIX m_world;
    D3DXMATRIX m_view;
    D3DXMATRIX m_proj;
    D3DXVECTOR4 m_lightDirection;
    
    // number of sections in the mesh
    DWORD m_dwNumSections;
    
    // methods
    HRESULT SetVertexShaderMatrices();
    HRESULT CreateBasisVectors(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer, LPDIRECT3DINDEXBUFFER9 pIndexBuffer);
    void    ToggleWireframe();
    void    ToggleAnimateLight();
    void    TogglePause();
    void    HandleKey(DWORD wParam, int bIsVirtualKey);
    void    ShowHelp();

    ShaderPaletteSkin();
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // implemented virtual functions:
    HRESULT InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice); // called just before device is Reset
    HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);    // called when device is restored
    HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice, float frameTime);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
};

#endif