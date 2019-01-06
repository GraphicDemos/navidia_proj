#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/SDKmisc.h>

#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include "resource.h"

#include "shader_Aniso.h"

#include <shared/GetFilePath.h>
#include <shared/NVFileDialog.h>

#ifdef _DEBUG
  #define CREATE_EFFECT_FLAGS (D3DXSHADER_DEBUG)
#else
  #define CREATE_EFFECT_FLAGS (0)
#endif

HINSTANCE g_hInstance = NULL;

#include <TCHAR.H>
typedef std::basic_string<TCHAR> tstring; 

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
CDXUTDialog		g_HUD;
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CModelViewerCamera      g_Camera;               

enum
{
IDC_TOGGLEFULLSCREEN,
IDC_WIREFRAME,
IDC_LOAD,
};

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext  );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();


INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceFrameMove( OnFrameMove );

	DXUTSetCursorSettings( true, true );

    InitApp();

    DXUTInit( true, true, NULL, true ); // Parse the command line and handle the default hotkeys
    DXUTCreateWindow( L"Anisotropic Lighting (HLSL/DX9)" );
    DXUTCreateDevice( true, 640, 576 );

    DXUTMainLoop();

	return DXUTGetExitCode();
}

void InitApp()
{
    g_main_menu = 0;
    g_context_menu = 0;

    g_pVertexBuffer = NULL; 
    g_pIndexBuffer = NULL;
    g_pShadeTexture = NULL; 
    g_pAttributes = NULL;
    g_bWireframe = false;

	g_bUseTrilinear = true;
    g_fRadius = 0.0f;
    g_vecCenter = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_strFileName = TEXT("MEDIA\\models\\UFO-01_NoTexture.X");
    g_pEffect = NULL;

    D3DXMatrixIdentity(&g_world);
    D3DXMatrixIdentity(&g_view);
    D3DXMatrixIdentity(&g_proj);

    //--------//

    g_HUD.Init( &g_DialogResourceManager );

	g_HUD.SetFont( 0, L"Arial", 14, 400 );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Full screen", 35, 35, 125, 22 );
	g_HUD.AddButton(IDC_WIREFRAME, L"(Show (W)ireframe",0,0,125,22,L'W');
	g_HUD.AddButton(IDC_LOAD, L"(L)oad .X File Mesh",0,0,125,22,L'L');

}

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    IDirect3D9* pD3D = DXUTGetD3D9Object(); 

    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

	// check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support mipmaps!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureCaps & D3DPTFILTERCAPS_MIPFLINEAR))
    {
        g_bUseTrilinear = false;
    }

    if(pCaps->MaxSimultaneousTextures < 2)
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support two simultaneous textures!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? false : true;
}

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );
	return S_OK;
}

HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

	g_HUD.SetLocation( 0, 0 );
	g_HUD.SetSize(pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height);
	g_HUD.GetControl(IDC_TOGGLEFULLSCREEN)->SetLocation(pBackBufferSurfaceDesc->Width-125, 10);
	g_HUD.GetControl(IDC_WIREFRAME)->SetLocation(pBackBufferSurfaceDesc->Width-125, 32);
	g_HUD.GetControl(IDC_LOAD)->SetLocation(pBackBufferSurfaceDesc->Width-125, 54);

    assert(pd3dDevice);

	if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    //initialize mouse UI
    D3DVIEWPORT9    viewport;
    RECT            rect;

    pd3dDevice->GetViewport(&viewport);
    rect.left   = rect.top = 0;
    rect.bottom = viewport.Height;
    rect.right  = viewport.Width;
//    g_pUI   = new MouseUI(rect, true, true);
//    g_pUI->SetRotationalSensitivityFactor(1.0f);
	g_Camera.SetWindow(viewport.Width, viewport.Height);

	// create our main Effect from our main .fx file
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\anisoHLSL.cso")).c_str(),
        NULL, NULL, CREATE_EFFECT_FLAGS, NULL, &g_pEffect, NULL);
    if (FAILED(hr))
        return hr;

    // select the only Technique from the Effect now, since we'll stick with it the whole time
    if (FAILED(g_pEffect->SetTechnique("AnisotropicLighting")))
    {
        MessageBox(NULL, _T("Failed to set 'AnisotropicLighting' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    
    // load model
    hr = LoadXFile(GetFilePath::GetFilePath(g_strFileName).data(), AnisoVertex::FVF_Flags,pd3dDevice );
    if(FAILED(hr))
        return hr;
    
    // use this vertex buffer as our source for vertices
    hr = pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(AnisoVertex));
    if (FAILED(hr))
        return hr;
    
    //note: don't need to set render states because they're set in the .fx file
    
    //load textures
    D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\textures\\2d\\Aniso2.tga")).data(), &g_pShadeTexture);
    
    g_pEffect->SetTexture("tLookup", g_pShadeTexture);

    pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, g_bUseTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
    
    hr = ResetLightsAndCamera(pd3dDevice);
    if (FAILED(hr))
        return hr;
    
    return S_OK;}

void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove(fElapsedTime);
}

void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr = S_OK;

    if (!FAILED(pd3dDevice->BeginScene()))
    {
        SetVertexShaderMatrices();

        pd3dDevice->SetRenderState(D3DRS_FILLMODE, (g_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

        pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 60, 60, 60), 1.0f, 0L);

        pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);

        UINT uPasses;
        g_pEffect->Begin(&uPasses, 0);  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.

        for(UINT uPass = 0; uPass < uPasses; uPass++)
        {
            // Set the state for a particular pass in a technique.
            g_pEffect->BeginPass(uPass);

            for(unsigned int i = 0; i < g_dwNumSections; i++)
            {
                // note: last call to SetStreamSource determines where the primitive verts come from.
                pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 
                              g_pAttributes[i].VertexStart,
                              g_pAttributes[i].VertexCount, 
                              g_pAttributes[i].FaceStart * 3, 
                              g_pAttributes[i].FaceCount );
            }
			g_pEffect->EndPass();
        }

        g_pEffect->End();

		g_HUD.OnRender( fElapsedTime );

		RenderText();

        pd3dDevice->EndScene();
    }
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 15 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	txtHelper.DrawTextLine( DXUTGetFrameStats() );
	txtHelper.DrawTextLine( DXUTGetDeviceStats() );
	txtHelper.End();

}
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all windows messages to camera & dialogs so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}

void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch(nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		{
			DXUTToggleFullScreen();
			break;
		}
	case IDC_WIREFRAME:
		{
		    g_bWireframe = !g_bWireframe;
			if(!g_bWireframe)
				g_HUD.GetButton(IDC_WIREFRAME)->SetText(L"Show (W)ireframe");
			else
				g_HUD.GetButton(IDC_WIREFRAME)->SetText(L"Hide (W)ireframe");
			break;
		}
	case IDC_LOAD:
		{
                  // Now do file dialog
                  NVXFileDialog aDialog;

                  tstring theResult;
                  bool bWritten = ( aDialog.Open( theResult ) );
                  
                  if ( bWritten )
                  {
                      g_strFileName = theResult;
				      IDirect3DDevice9* pD3Dev = DXUTGetD3D9Device();
                      bWritten = ( LoadXFile( g_strFileName.c_str(), AnisoVertex::FVF_Flags, pD3Dev ) == S_OK );
                      ResetLightsAndCamera(pD3Dev); // redistance camera according to radius of new object
                  }
			break;
		}
	}
}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();

    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBuffer);

    SAFE_RELEASE(g_pShadeTexture);

    SAFE_RELEASE(g_pEffect);

    SAFE_DELETE_ARRAY(g_pAttributes);
    if( g_pFont )
        g_pFont->OnLostDevice();

	SAFE_RELEASE(g_pTextSprite);

}

void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    SAFE_RELEASE(g_pFont);
}
HRESULT ResetLightsAndCamera(IDirect3DDevice9* pd3dDevice)
{
    // write the constant constants to constant memory now

    D3DXVECTOR4 lightDir(1.0f, 0.0f, -1.0f, 0.0f);
    D3DXVec4Normalize(&lightDir, &lightDir);

    g_pEffect->SetValue("LightVec", (void*)(FLOAT*)&lightDir, sizeof(D3DXVECTOR3));

    D3DXVECTOR3 eye, lookAt, up;

    eye.x    = g_vecCenter.x; eye.y    = g_vecCenter.y; eye.z    = -g_fRadius*2.3f;
    lookAt.x = g_vecCenter.x; lookAt.y = g_vecCenter.y; lookAt.z = g_vecCenter.z;
    up.x   = 0.0f;          up.y     = 1.0f;          up.z     = 0.0f;
    
    g_pEffect->SetValue("EyePos", (void*)(FLOAT*)&eye, sizeof(D3DXVECTOR3));

    D3DXMatrixIdentity(&g_world);
    D3DXMatrixLookAtLH(&g_view, &eye, &lookAt, &up);
    
    D3DXMatrixPerspectiveFovLH(&g_proj,
        D3DXToRadian(60.0f),
        1,
        2.0f,
        800.0f);
    //pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&g_proj);
    
	g_Camera.SetViewParams( &eye, &lookAt );
	g_Camera.SetProjParams( D3DXToRadian(60.0f),1,2.0f,800.0f);
    //matrices
    SetVertexShaderMatrices();

    return S_OK;
}
HRESULT LoadXFile(const TCHAR* fileName, const DWORD dwFVF, IDirect3DDevice9* pd3dDevice)
{
    ID3DXMesh *tempMesh, *tempMeshFVF, *tempMeshOpt;
      
    HRESULT hr = D3DXLoadMeshFromX(const_cast<TCHAR*>(fileName), D3DXMESH_SYSTEMMEM , pd3dDevice, NULL, 
        NULL, NULL, &g_dwNumSections, &tempMesh);
    if (FAILED(hr))
        return hr;
      
    AnisoVertex* pBuff = NULL;
    hr = tempMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pBuff);
    if(FAILED(hr))
        return hr;
    D3DXComputeBoundingSphere((LPD3DXVECTOR3)pBuff, tempMesh->GetNumVertices(), D3DXGetFVFVertexSize(tempMesh->GetFVF()), &g_vecCenter, &g_fRadius);
    tempMesh->UnlockVertexBuffer();

    tempMesh->Optimize(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL, &tempMeshOpt);
    DWORD attribSize = g_dwNumSections;
    hr = tempMeshOpt->GetAttributeTable(NULL, &attribSize);

    SAFE_DELETE_ARRAY( g_pAttributes );

    g_pAttributes = new D3DXATTRIBUTERANGE[attribSize];
    
    hr = tempMeshOpt->GetAttributeTable(g_pAttributes, &attribSize);

    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );

    //convert to our format
    hr = tempMeshOpt->CloneMeshFVF(D3DXMESH_WRITEONLY, dwFVF, pd3dDevice, &tempMeshFVF);
    
    tempMeshFVF->GetVertexBuffer(&g_pVertexBuffer);
    tempMeshFVF->GetIndexBuffer(&g_pIndexBuffer);

    SAFE_RELEASE(tempMesh);
    SAFE_RELEASE(tempMeshFVF);
    SAFE_RELEASE(tempMeshOpt);
      
    //set index buffer
    pd3dDevice->SetIndices( g_pIndexBuffer );

    hr = pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof( AnisoVertex ) );
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT SetVertexShaderMatrices()
{
    D3DXMATRIX mvpMat, worldITMat, worldMat;
    
	D3DXMatrixIdentity(&worldMat);

    D3DXMatrixMultiply( &worldMat, &worldMat, g_Camera.GetWorldMatrix());

    D3DXMatrixMultiply(&mvpMat, &worldMat, g_Camera.GetViewMatrix());
    D3DXMatrixMultiply(&mvpMat, &mvpMat, g_Camera.GetProjMatrix());
    
    D3DXMatrixInverse(&worldITMat, NULL, &worldMat);
    D3DXMatrixTranspose(&worldITMat, &worldITMat);    

    g_pEffect->SetValue("WorldViewProj", (void*)(FLOAT*)&mvpMat, sizeof(float)*4*4);
    g_pEffect->SetValue("WorldIT"      , (void*)(FLOAT*)&worldITMat, sizeof(float)*4*4);
    g_pEffect->SetValue("World"        , (void*)(FLOAT*)&worldMat, sizeof(float)*4*4);

    return S_OK;
}
