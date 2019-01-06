//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\NearClipPlane
// File:  NearClipPlane.h
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

#ifndef _NEAR_CLIP_PLANE_H_
#define _NEAR_CLIP_PLANE_H_

#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
//-----------------------------------------------------------------------------
// Name: class NearClipPlane
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. NearClipPlane 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class NearClipPlane
{
public:
    CD3DArcBall   m_ArcBall;                // ArcBall used for mouse input

	CDXUTXFileMesh*    m_pTeapot;     // The teapot object
    D3DXMATRIXA16 m_matTeapot;   // Teapot's local matrix

    LPDIRECT3DVERTEXBUFFER9 m_pMirrorVB;

    D3DXMATRIXA16 m_matWorld;
    D3DXMATRIXA16 m_matView;
    D3DXMATRIXA16 m_matProj;

    D3DXVECTOR3   m_vEyePt;      // Vectors defining the camera
    D3DXVECTOR3   m_vLookatPt;
    D3DXVECTOR3   m_vUpVec;

    D3DXPLANE  m_clip_plane;

    bool m_bCanLight;

    // methods:
    HRESULT SetClipPlane(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT RenderMirror(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT RenderScene(LPDIRECT3DDEVICE9 pd3dDevice);
    void    HandleKey(int wParam, int bIsVirtualKey);
    void    ClipProjectionMatrix(D3DXMATRIXA16 & matView, D3DXMATRIXA16 & matProj, D3DXPLANE & clip_plane,
                                 LPDIRECT3DDEVICE9 pd3dDevice);
    void    DisableClipPlanes(LPDIRECT3DDEVICE9 pd3dDevice);
    void    ShowHelp();

public: 
    // runtime settings:
    int m_bUseClipPlanes;
    int m_bRenderClipPlane;
    int m_dir;

    NearClipPlane();
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // implemented virtual functions:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);       // called once @ startup
    HRESULT DeleteDeviceObjects();     // called once @ exit
    HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);    // called at startup & after a resize
    HRESULT InvalidateDeviceObjects(); // called at exit & before a resize
    HRESULT FrameMove();
    HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
};



#endif