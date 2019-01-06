#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/SDKmisc.h>

#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include "resource.h"

#include "NVTriStripTest.h"

#include <shared/GetFilePath.h>
#include <shared/NVFileDialog.h>
#include <NvTriStrip.h>

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
CDXUTDialog		            g_HUD;
CModelViewerCamera          g_Camera;               
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs

enum
{
IDC_WIREFRAME,
IDC_CYCLEOPT
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
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
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

    DXUTInit( true, true,NULL, true ); // Parse the command line and handle the default hotkeys
    DXUTCreateWindow( L"NvTriStrip Test App (HLSL/DX9)" );
    DXUTCreateDevice( true, 640, 576 );

    DXUTMainLoop();

	return DXUTGetExitCode();
}

void InitApp()
{
    g_main_menu = 0;
    g_context_menu = 0;

    g_pVertexBuffer        = NULL; 
    g_pIndexBufferUnOpt    = NULL;
	g_pIndexBufferOptList  = NULL;
	g_pIndexBufferOptStrip = NULL;

	g_pSkinTexture  = NULL; 

	g_pPrimitiveGroupsStrip = NULL;
	g_pPrimitiveGroupsList  = NULL;

    g_bWireframe = false;

	g_dwNumVerts      = 0;
	g_dwNumFacesStrip = 0;
	g_dwNumFacesList  = 0;

	g_bUseTrilinear = true;
    g_fRadius = 0.0f;
    g_vecCenter = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_strFileName = TEXT("MEDIA\\models\\Tiny\\tiny.x");
    g_pEffect = NULL;

    D3DXMatrixIdentity(&g_world);
    D3DXMatrixIdentity(&g_view);
    D3DXMatrixIdentity(&g_proj);

    //--------//
    g_HUD.Init( &g_DialogResourceManager );
	g_HUD.SetFont( 0, L"Arial", 14, 400 );
    g_HUD.SetCallback( OnGUIEvent );
  	g_HUD.AddButton( IDC_CYCLEOPT, L"(C)ycle Optimizations", 35, 35, 125, 22, L'C' );
	g_HUD.AddButton(IDC_WIREFRAME, L"Show (W)ireframe",0,0,125,22,L'W');

	g_renderMode = RM_NOOPT;
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
	g_HUD.GetControl(IDC_CYCLEOPT)->SetLocation(pBackBufferSurfaceDesc->Width-125, 10);
	g_HUD.GetControl(IDC_WIREFRAME)->SetLocation(pBackBufferSurfaceDesc->Width-125, 32);

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
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\NVTriStripTest.cso")).c_str(),
        NULL, NULL, CREATE_EFFECT_FLAGS, NULL, &g_pEffect, NULL);
    if (FAILED(hr))
        return hr;

    // select the only Technique from the Effect now, since we'll stick with it the whole time
    if (FAILED(g_pEffect->SetTechnique("BasicLighting")))
    {
        MessageBox(NULL, _T("Failed to set 'BasicLighting' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    
    // load model
    hr = LoadXFile(GetFilePath::GetFilePath(g_strFileName).data(), VertexType::FVF_Flags,pd3dDevice );
    if(FAILED(hr))
        return hr;
    
    // use this vertex buffer as our source for vertices
    hr = pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(VertexType));
    if (FAILED(hr))
        return hr;
    
    //note: don't need to set render states because they're set in the .fx file
    
	//load texture	
	D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\models\\Tiny\\tiny_skin.bmp")).data(), &g_pSkinTexture);

	g_pEffect->SetTexture("DiffuseTexture", g_pSkinTexture);

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
		D3DXMATRIX worldMat;
		D3DXMatrixIdentity(&worldMat);
        SetVertexShaderMatrices(worldMat);

        pd3dDevice->SetRenderState(D3DRS_FILLMODE, (g_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

        pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 60, 60, 60), 1.0f, 0L);

        pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

        UINT uPasses;
        g_pEffect->Begin(&uPasses, 0);  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.

		float offset = 150.0f;
        for(UINT uPass = 0; uPass < uPasses; uPass++)
        {
            // Set the state for a particular pass in a technique.
            g_pEffect->BeginPass(uPass);

			DWORD numFaces = 0;
			D3DPRIMITIVETYPE primType = D3DPT_TRIANGLELIST;
			switch (g_renderMode)
			{
				case RM_NOOPT:
					pd3dDevice->SetIndices(g_pIndexBufferUnOpt);
					numFaces = g_dwNumFacesList;
					primType = D3DPT_TRIANGLELIST;
					break;
				case RM_OPTLIST:
					pd3dDevice->SetIndices(g_pIndexBufferOptList);
					numFaces = g_dwNumFacesList;
					primType = D3DPT_TRIANGLELIST;
					break;
				case RM_OPTSTRIP:
					pd3dDevice->SetIndices(g_pIndexBufferOptStrip);
					numFaces = g_dwNumFacesStrip;
					primType = D3DPT_TRIANGLESTRIP;
					break;
			}

			//render three times
			for (int i = 0; i < 3; ++i)
			{
				g_pEffect->CommitChanges();
				pd3dDevice->DrawIndexedPrimitive(primType, 0, 0, g_dwNumVerts, 0, numFaces);

				D3DXMATRIX outMat;

				//variation 1
				D3DXMatrixTranslation(&outMat, offset, offset, 0.0f);
				D3DXMatrixMultiply(&outMat, &worldMat, &outMat);
				SetVertexShaderMatrices(outMat);

				g_pEffect->CommitChanges();
				pd3dDevice->DrawIndexedPrimitive(primType, 0, 0, g_dwNumVerts, 0, numFaces);

				//variation 2
				D3DXMatrixTranslation(&outMat, -offset, offset, 0.0f);
				D3DXMatrixMultiply(&outMat, &worldMat, &outMat);
				SetVertexShaderMatrices(outMat);

				g_pEffect->CommitChanges();
				pd3dDevice->DrawIndexedPrimitive(primType, 0, 0, g_dwNumVerts, 0, numFaces);

				//variation 3
				D3DXMatrixTranslation(&outMat, offset, -offset, 0.0f);
				D3DXMatrixMultiply(&outMat, &worldMat, &outMat);
				SetVertexShaderMatrices(outMat);

				g_pEffect->CommitChanges();
				pd3dDevice->DrawIndexedPrimitive(primType, 0, 0, g_dwNumVerts, 0, numFaces);

				//variation 4
				D3DXMatrixTranslation(&outMat, -offset, -offset, 0.0f);
				D3DXMatrixMultiply(&outMat, &worldMat, &outMat);
				SetVertexShaderMatrices(outMat);

				g_pEffect->CommitChanges();
				pd3dDevice->DrawIndexedPrimitive(primType, 0, 0, g_dwNumVerts, 0, numFaces);
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
	switch (g_renderMode)
	{
		case RM_NOOPT:
			txtHelper.DrawTextLine( TEXT("Unoptimized Indices") );
			break;
		case RM_OPTLIST:
			txtHelper.DrawTextLine( TEXT("Optimized List") );
			break;
		case RM_OPTSTRIP:
			txtHelper.DrawTextLine( TEXT("Optimized Strip") );
			break;
	}
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
	case IDC_WIREFRAME:
		{
		    g_bWireframe = !g_bWireframe;
			if(!g_bWireframe)
				g_HUD.GetButton(IDC_WIREFRAME)->SetText(L"Show (W)ireframe");
			else
				g_HUD.GetButton(IDC_WIREFRAME)->SetText(L"Hide (W)ireframe");
			break;
		}
	case IDC_CYCLEOPT:
		{
			switch (g_renderMode)
			{
			case RM_NOOPT:
				g_renderMode = RM_OPTLIST;
				break;
			case RM_OPTLIST:
				g_renderMode = RM_OPTSTRIP;
				break;
			case RM_OPTSTRIP:
				g_renderMode = RM_NOOPT;
				break;
			}
			
			break;
		}
	}
}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();

    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBufferUnOpt);
	SAFE_RELEASE(g_pIndexBufferOptList);
	SAFE_RELEASE(g_pIndexBufferOptStrip);

	SAFE_RELEASE(g_pSkinTexture);

    SAFE_RELEASE(g_pEffect);

	SAFE_DELETE_ARRAY(g_pPrimitiveGroupsStrip);
	SAFE_DELETE_ARRAY(g_pPrimitiveGroupsList);

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

    D3DXVECTOR4 lightDir(0.0f, -0.5f, 1.0f, 0.0f);
    D3DXVec4Normalize(&lightDir, &lightDir);

    g_pEffect->SetValue("LightVec", (void*)(FLOAT*)&lightDir, sizeof(D3DXVECTOR3));
	
	D3DXVECTOR4 lightCol(1.0f, 1.0f, 0.8f, 0.0f);
	g_pEffect->SetValue("LightColor", (void*)(FLOAT*)&lightCol, sizeof(D3DXVECTOR3));

    D3DXVECTOR3 eye, lookAt, up;

    eye.x    = g_vecCenter.x; eye.y    = g_vecCenter.y; eye.z    = -g_fRadius*2.8f;
	//eye.x    = g_vecCenter.x; eye.y    = g_vecCenter.y + g_fRadius * 1.5; eye.z    = g_vecCenter.z;
    lookAt.x = g_vecCenter.x; lookAt.y = g_vecCenter.y; lookAt.z = g_vecCenter.z;
    up.x   = 0.0f;          up.y     = 1.0f;          up.z     = 0.0f;
    
	D3DXMATRIX movEye;
	D3DXMatrixRotationYawPitchRoll(&movEye, D3DXToRadian(0.0f), D3DXToRadian(225.0f), D3DXToRadian(90.0f));
	D3DXVECTOR4 outVec;
	D3DXVec3Transform(&outVec, &eye, &movEye);
	eye.x = outVec.x;
	eye.y = outVec.y;
	eye.z = outVec.z;
    g_pEffect->SetValue("EyePos", (void*)(FLOAT*)&eye, sizeof(D3DXVECTOR3));

    D3DXMatrixIdentity(&g_world);
//	D3DXMatrixRotationYawPitchRoll(&g_world, D3DXToRadian(-90.0f), D3DXToRadian(90.0f), D3DXToRadian(0.0f));
    D3DXMatrixLookAtLH(&g_view, &eye, &lookAt, &up);
    
    D3DXMatrixPerspectiveFovLH(&g_proj,
        D3DXToRadian(60.0f),
        1,
        2.0f,
        800.0f);

	g_Camera.SetViewParams( &eye, &lookAt );
	g_Camera.SetProjParams( D3DXToRadian(60.0f), 1, 5.0f, 1200.0f);
    //matrices
	D3DXMATRIX identMat;
	D3DXMatrixIdentity(&identMat);
    SetVertexShaderMatrices(identMat);

    return S_OK;
}

HRESULT LoadXFile(const TCHAR* fileName, const DWORD dwFVF, IDirect3DDevice9* pd3dDevice)
{
    ID3DXMesh *tempMesh, *tempMeshFVF;
    HRESULT hr = D3DXLoadMeshFromX(const_cast<TCHAR*>(fileName), D3DXMESH_SYSTEMMEM , pd3dDevice, NULL, 
        NULL, NULL, NULL, &tempMesh);
    if (FAILED(hr))
        return hr;
      
    VertexType* pBuff = NULL;
    hr = tempMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pBuff);
    if(FAILED(hr))
        return hr;
    D3DXComputeBoundingSphere((LPD3DXVECTOR3)pBuff, tempMesh->GetNumVertices(), D3DXGetFVFVertexSize(tempMesh->GetFVF()), &g_vecCenter, &g_fRadius);
    tempMesh->UnlockVertexBuffer();

	g_dwNumVerts = tempMesh->GetNumVertices();
	g_dwNumFacesList = tempMesh->GetNumFaces();

	/////////////////////////////////////////////////////////////////////////
	// NVTriStrip setup code
	SetCacheSize(CACHESIZE_GEFORCE3);
	SetStitchStrips(true);
	SetMinStripSize(0);

	//First, we create the optimized list indices
	SetListsOnly(true);

	unsigned short* pIndices = NULL;
	hr = tempMesh->LockIndexBuffer(D3DLOCK_READONLY, (void**)&pIndices);
	if (FAILED(hr))
		return hr;

	unsigned short numSections;
	GenerateStrips(pIndices, g_dwNumFacesList * 3, &g_pPrimitiveGroupsList, &numSections);
	tempMesh->UnlockIndexBuffer();

	//copy optimized data to index buffer
	pd3dDevice->CreateIndexBuffer(g_pPrimitiveGroupsList[0].numIndices * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
		D3DPOOL_DEFAULT, &g_pIndexBufferOptList, NULL);
	unsigned short* pDest;
	g_pIndexBufferOptList->Lock(0, g_pPrimitiveGroupsList[0].numIndices * sizeof(unsigned short), (void**)&pDest, 0);
	memcpy(pDest, g_pPrimitiveGroupsList[0].indices, g_pPrimitiveGroupsList[0].numIndices * sizeof(unsigned short));
	g_pIndexBufferOptList->Unlock();

	//Next, we create the optimized strip indices
	SetListsOnly(false);

	pIndices = NULL;
	hr = tempMesh->LockIndexBuffer(D3DLOCK_READONLY, (void**)&pIndices);
	if (FAILED(hr))
		return hr;

	GenerateStrips(pIndices, g_dwNumFacesList * 3, &g_pPrimitiveGroupsStrip, &numSections);
	tempMesh->UnlockIndexBuffer();

	//copy optimized data to index buffer
	pd3dDevice->CreateIndexBuffer(g_pPrimitiveGroupsStrip[0].numIndices * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
		D3DPOOL_DEFAULT, &g_pIndexBufferOptStrip, NULL);
	g_pIndexBufferOptStrip->Lock(0, g_pPrimitiveGroupsStrip[0].numIndices * sizeof(unsigned short), (void**)&pDest, 0);
	memcpy(pDest, g_pPrimitiveGroupsStrip[0].indices, g_pPrimitiveGroupsStrip[0].numIndices * sizeof(unsigned short));
	g_pIndexBufferOptStrip->Unlock();

	g_dwNumFacesStrip = g_pPrimitiveGroupsStrip[0].numIndices - 2;

	//convert to our format
	hr = tempMesh->CloneMeshFVF(D3DXMESH_WRITEONLY, dwFVF, pd3dDevice, &tempMeshFVF);

	tempMeshFVF->GetVertexBuffer(&g_pVertexBuffer);
	tempMeshFVF->GetIndexBuffer(&g_pIndexBufferUnOpt);

    SAFE_RELEASE(tempMesh);
    SAFE_RELEASE(tempMeshFVF);
    
    hr = pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof( VertexType ) );
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT SetVertexShaderMatrices(const D3DXMATRIX& worldMat)
{
    D3DXMATRIX mvpMat, worldITMat, worldMatTemp;
    
    D3DXMatrixMultiply( &worldMatTemp, &worldMat, g_Camera.GetWorldMatrix());

    D3DXMatrixMultiply(&mvpMat, &worldMatTemp, g_Camera.GetViewMatrix());
    D3DXMatrixMultiply(&mvpMat, &mvpMat, g_Camera.GetProjMatrix());
    
    D3DXMatrixInverse(&worldITMat, NULL, &worldMatTemp);
    D3DXMatrixTranspose(&worldITMat, &worldITMat);    

    g_pEffect->SetValue("WorldViewProj", (void*)(FLOAT*)&mvpMat, sizeof(float)*4*4);
    g_pEffect->SetValue("WorldIT"      , (void*)(FLOAT*)&worldITMat, sizeof(float)*4*4);
    g_pEffect->SetValue("World"        , (void*)(FLOAT*)&worldMatTemp, sizeof(float)*4*4);

    return S_OK;
}
