//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\NearClipPlane
// File:  NearClipPlane.cpp
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
// Link to: DX9SDKSampleFramework.lib d3dxof.lib dxguid.lib d3dx9dt.lib d3d9.lib winmm.lib comctl32.lib
//
//-----------------------------------------------------------------------------
// File: NearClipPlane.cpp
// 
// Desc: This sample shows how to use clip planes to implement a planar mirror.
//       The scene is reflected in a mirror and rendered in a 2nd pass. The
//       corners of the mirrors, together with the camera eye point, are used
//       to define a custom set of clip planes so that the reflected geometry
//       appears only within the mirror's boundaries.
//-----------------------------------------------------------------------------
#define STRICT
#include <Windows.h>
#include <commctrl.h>
#include <math.h>
#include <shared/GetFilePath.h>

#include "NearClipPlane.h"


#define TEAPOT_X	L"MEDIA\\models\\Teapot.x"

struct MIRRORVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    DWORD       color;

    static const DWORD FVF;
};
const DWORD MIRRORVERTEX::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;

HINSTANCE g_hInstance = NULL;

//-----------------------------------------------------------------------------
// Name: NearClipPlane()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
NearClipPlane::NearClipPlane()
{
    m_pTeapot           = NULL;
    m_pMirrorVB         = NULL;
    m_bUseClipPlanes    = 0;
    m_bRenderClipPlane  = 1;
    m_dir               = 1;
}


//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::OneTimeSceneInit()
{
    // Initialize the camera's orientation
    m_vEyePt    = D3DXVECTOR3( 0.0f, 2.0f, -6.5f );
    m_vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
    m_vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::FrameMove()
{
    // Update the Earth's rotation angle
    static FLOAT fRotationAngle = 0.0f;
    //if( FALSE == m_ArcBall.IsBeingDragged() )
    //    fRotationAngle += m_fElapsedTime;

    // Setup viewing postion from ArcBall
    D3DXMatrixRotationY( &m_matWorld, -fRotationAngle );
    D3DXMatrixMultiply( &m_matWorld, &m_matWorld, m_ArcBall.GetRotationMatrix() );
    D3DXMatrixMultiply( &m_matWorld, &m_matWorld, m_ArcBall.GetTranslationMatrix() );

    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -3.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtRH( &m_matView, &vEyePt, &vLookatPt, &vUpVec );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Renders all objects in the scene.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::RenderScene(LPDIRECT3DDEVICE9 pd3dDevice)
{
    D3DXMATRIX id;
    D3DXMatrixIdentity(& id);
    pd3dDevice->SetTransform( D3DTS_WORLD, &id);
    pd3dDevice->SetTransform( D3DTS_VIEW, &m_matView );

    // Render the object
    m_pTeapot->Render( pd3dDevice );

    // Output statistics
/*    if (m_bUseClipPlanes)
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), "Clip Planes" );
    else
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), "Projection Matrix" );*/

    return S_OK;
}

void NearClipPlane::DisableClipPlanes(LPDIRECT3DDEVICE9 pd3dDevice)
{
    pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0);
    pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProj);

}





//-----------------------------------------------------------------------------
// Name: RenderMirror()
// Desc: Renders the scene as reflected in a mirror. The corners of the mirror
//       define a plane, which is used to build the reflection matrix. The
//       scene is rendered with the cull-mode reversed, since all normals in
//       the scene are likewise reflected.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::SetClipPlane(LPDIRECT3DDEVICE9 pd3dDevice)
{
    D3DXMATRIXA16 matWorldSaved;
    D3DXMATRIXA16 matReflectInMirror;

    D3DXVECTOR3 a1( 0.0f, -1.5f, 1.5f );
    D3DXVECTOR3 b1( 0.0f,  1.5f, 1.5f );
    D3DXVECTOR3 c1( 0.0f,  -1.5f,-1.5f );
    D3DXVECTOR3 d1( 0.0f,  1.5f,-1.5f );

    D3DXVECTOR3 a,b,c,d;

    D3DXVec3TransformCoord(&a, &a1, &m_matWorld );
    D3DXVec3TransformCoord(&b, &b1, &m_matWorld );
    D3DXVec3TransformCoord(&c, &c1, &m_matWorld );
    D3DXVec3TransformCoord(&d, &d1, &m_matWorld );

    // Set the custom clip planes (so geometry is clipped by mirror edges).
    // This is the heart of this sample. The mirror has 4 edges, so there are
    // 4 clip planes, each defined by two mirror vertices and the eye point.
    
    if (m_dir)
        D3DXPlaneFromPoints( &m_clip_plane, &a, &c, &b );
    else
        D3DXPlaneFromPoints( &m_clip_plane, &a, &b, &c );

    pd3dDevice->SetClipPlane( 0, m_clip_plane);

    if (m_bUseClipPlanes)
    {
        pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
        pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProj);
    }
    else 
    {
        ClipProjectionMatrix(m_matView, m_matProj, m_clip_plane, pd3dDevice);
    }

    return 0;
}


HRESULT NearClipPlane::RenderMirror(LPDIRECT3DDEVICE9 pd3dDevice)
{
    pd3dDevice->SetTransform( D3DTS_WORLD, &m_matWorld );
    pd3dDevice->SetTransform( D3DTS_VIEW, &m_matView );

    pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
    pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    pd3dDevice->SetStreamSource( 0, m_pMirrorVB, 0, sizeof(MIRRORVERTEX) );
    pd3dDevice->SetFVF( MIRRORVERTEX::FVF );
    pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
    // Clear the viewport
    pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0L );

    {
        // Render the scene

        // Render the scene in the mirror
        pd3dDevice->SetRenderState( D3DRS_ZENABLE,   true );
        pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,   true );
        pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

        pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

        SetClipPlane(pd3dDevice);

        RenderScene(pd3dDevice);

        pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

        pd3dDevice->SetRenderState( D3DRS_ZENABLE,   false );
        pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,   false );
        pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS);

        DisableClipPlanes(pd3dDevice);

        if (m_bRenderClipPlane)
            RenderMirror(pd3dDevice);
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    m_pTeapot   = new CDXUTXFileMesh;

    // Set up the geometry objects
    if( FAILED( m_pTeapot->Create( pd3dDevice, GetFilePath::GetFilePath( TEAPOT_X ).c_str() ) ) )
        return E_FAIL;

    // Create a square for rendering the mirror
    if( FAILED( pd3dDevice->CreateVertexBuffer( 4*sizeof(MIRRORVERTEX),
                                                  D3DUSAGE_WRITEONLY,
                                                  MIRRORVERTEX::FVF,
                                                  D3DPOOL_MANAGED, &m_pMirrorVB, NULL ) ) )
        return E_FAIL;

    // Initialize the mirror's vertices
    MIRRORVERTEX* v;
    m_pMirrorVB->Lock( 0, 0, (void**)&v, 0 );
    v[0].p = D3DXVECTOR3( 0.0f, -1.5f, 1.5f );
    v[2].p = D3DXVECTOR3( 0.0f, -1.5f,-1.5f );
    v[1].p = D3DXVECTOR3( 0.0f,  1.5f, 1.5f );
    v[3].p = D3DXVECTOR3( 0.0f,  1.5f,-1.5f );
    v[0].n     = v[1].n     = v[2].n     = v[3].n     = D3DXVECTOR3(-1.0f,0.0f,0.0f);
    v[0].color = v[1].color = v[2].color = v[3].color = 0x80ffffff;
    m_pMirrorVB->Unlock();

    return S_OK;
}


void InitLight(D3DLIGHT9 &light, D3DLIGHTTYPE ltType, float x, float y, float z)
{
    D3DXVECTOR3 vecLightDirUnnormalized(x, y, z);
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type        = ltType;
    light.Diffuse.r   = 1.0f;
    light.Diffuse.g   = 1.0f;
    light.Diffuse.b   = 1.0f;

    D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecLightDirUnnormalized);

    light.Position.x   = x;
    light.Position.y   = y;
    light.Position.z   = z;
    light.Range        = 1000.0f;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    // Set up the geometry objects
    m_pTeapot->RestoreDeviceObjects( pd3dDevice );

    // Set up the textures
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    // Set miscellaneous render states
    pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
    pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );

    // Set up the matrices
    //D3DXMATRIXA16 matWorld, matView, matProj;

    D3DXMatrixIdentity( &m_matWorld );
    D3DXMatrixLookAtRH( &m_matView, &m_vEyePt, &m_vLookatPt, &m_vUpVec );

    D3DSURFACE_DESC d3dsdBackBuffer = *DXUTGetD3D9BackBufferSurfaceDesc();
    FLOAT fAspect = d3dsdBackBuffer.Width / (FLOAT)d3dsdBackBuffer.Height;

    D3DXMatrixPerspectiveFovRH( &m_matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );

    pd3dDevice->SetTransform( D3DTS_WORLD,      &m_matWorld );
    pd3dDevice->SetTransform( D3DTS_VIEW,       &m_matView );
    pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProj );

    // Set up a light
    if( m_bCanLight )
    {
        D3DLIGHT9 light;
        InitLight( light, D3DLIGHT_DIRECTIONAL, 0.2f, -1.0f, -0.2f );
        pd3dDevice->SetLight( 0, &light );
        pd3dDevice->LightEnable( 0, TRUE );
    }
    pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xff555555 );


    // Set the ArcBall parameters
    m_ArcBall.SetWindow( d3dsdBackBuffer.Width, d3dsdBackBuffer.Height, 3.0f );
    m_ArcBall.SetTranslationRadius( 1.0f );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::InvalidateDeviceObjects()
{
    m_pTeapot->InvalidateDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::DeleteDeviceObjects()
{
    m_pTeapot->Destroy();

    SAFE_RELEASE( m_pMirrorVB );

    SAFE_DELETE( m_pTeapot );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT NearClipPlane::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                      D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    if( ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->MaxUserClipPlanes < 4 )
            if (!nErrors++) 
                MessageBox(NULL, _T("Device does not support 4 user clip planes"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }

    m_bCanLight = ( pCaps->VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS ) ||
                 !( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING );

    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}


void NearClipPlane::HandleKey(int wParam, int bIsVirtualKey)
{
    if (!bIsVirtualKey)
    {
        // regular alphanumeric keys; case is preserved, too
        switch(wParam)
        {
        case 'X':
        case 'x':
            m_bUseClipPlanes ^= 1;
            break;

        case 'M':
        case 'm':
            m_bRenderClipPlane ^= 1;
            break;

        case 'D':
        case 'd':
            m_dir ^= 1;
            break;
        }
    }
    else
    {
        // non-alphanumeric keys; use virtual-key codes only.
        switch(wParam)
        {
        case VK_HOME :          // reset scene/settings
            {
                m_bUseClipPlanes = 1;
                m_bRenderClipPlane = 1;
                m_dir = 1;
            }
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT NearClipPlane::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Pass mouse messages to the ArcBall so it can build internal matrices
    m_ArcBall.HandleMessages( hWnd, uMsg, wParam, lParam );

    // Trap context menu
    //if( WM_CONTEXTMENU == uMsg )
    //    return 0;

    switch( uMsg )
    {
    case WM_COMMAND:
        break;

    case WM_CHAR:
        HandleKey((DWORD)wParam, 0);
        break;

    case WM_KEYDOWN:
        HandleKey((DWORD)wParam, 1);
        break;

    default:
        break;
    }

    return 0;
}

/*
void NearClipPlane::ShowHelp()
{
    ::MessageBoxEx( NULL, 
      " F1 - Help \n\n Home - Reset To Defaults \n\n M - toggle rendering of clip plane \n\n D - toggle direction of clip plane \n\n X - toggle use clip plane \n\n",
      "Help", MB_ICONINFORMATION|MB_TASKMODAL|MB_SETFOREGROUND|MB_TOPMOST, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ) );
}
*/



/*
 Input 
    matView - world to eye matrix
    matProj - projection matrix (right handed)
    clip_plane - clip plane definition (in world space)
 Output
    set new proj matrix via SetTransform

   
*/


void NearClipPlane::ClipProjectionMatrix(D3DXMATRIXA16 & matView, D3DXMATRIXA16 & matProj,
                                             D3DXPLANE & clip_plane, LPDIRECT3DDEVICE9 pd3dDevice)
{
  D3DXMATRIXA16 matClipProj;


  D3DXMATRIXA16 WorldToProjection;


  WorldToProjection = matView * matProj;

  // m_clip_plane is plane definition (world space)
  D3DXPlaneNormalize(&clip_plane, &clip_plane);

  D3DXMatrixInverse(&WorldToProjection, NULL, &WorldToProjection);
  D3DXMatrixTranspose(&WorldToProjection, &WorldToProjection);


  D3DXVECTOR4 clipPlane(clip_plane.a, clip_plane.b, clip_plane.c, clip_plane.d);
  D3DXVECTOR4 projClipPlane;

  // transform clip plane into projection space
  D3DXVec4Transform(&projClipPlane, &clipPlane, &WorldToProjection);
  D3DXMatrixIdentity(&matClipProj);


  if (projClipPlane.w == 0)  // or less than a really small value
  {
    // plane is perpendicular to the near plane
    pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj);
    return;
  }
  D3DXVec4Normalize(&projClipPlane, &projClipPlane);  //  bug fix by David Whatley

  if (projClipPlane.w > 0)
  {
      projClipPlane = -projClipPlane;
      projClipPlane.w += 1;
  } 


  // put projection space clip plane in Z column
  matClipProj(0, 2) = projClipPlane.x;
  matClipProj(1, 2) = projClipPlane.y;
  matClipProj(2, 2) = projClipPlane.z;
  matClipProj(3, 2) = projClipPlane.w;

  

  // multiply into projection matrix
  D3DXMATRIXA16 projClipMatrix = matProj * matClipProj;


  pd3dDevice->SetTransform( D3DTS_PROJECTION, &projClipMatrix);

}