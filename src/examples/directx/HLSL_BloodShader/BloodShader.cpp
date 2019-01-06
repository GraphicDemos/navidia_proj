#include "dxstdafx.h"
#include "BloodMap.h"

#include <DXUT.h>
#include <DXUTMesh.h>
#include <DXUTcamera.h>
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         
ID3DXSprite*            g_pSprite = NULL;   
ID3DXEffect*            g_pEffect = NULL;       
CDXUTSDKMesh            g_Mesh;
D3DXMATRIXA16           g_mView;
CModelViewerCamera      g_Camera;               
CDXUTDialog             g_HUD;                  
CDXUTDialog             g_SampleUI;             
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
IDC_TOGGLEFULLSCREEN,
IDC_TEXLISTBOX,
IDC_COMBOBLOODSPOT,
IDC_BLOODCOLORRED,
IDC_BLOODCOLORGREEN,
IDC_BLOODCOLORBLUE,
IDC_BLOODCOLORRED_STATIC,
IDC_BLOODCOLORGREEN_STATIC,
IDC_BLOODCOLORBLUE_STATIC,
IDC_REDBUTTON,
IDC_GREENBUTTON,
};

	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE] = 
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_TEXCOORD, 0},
		{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_NORMAL, 0},
		{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_TANGENT, 0},
		{0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_BINORMAL, 0},
			D3DDECL_END()
	};


	D3DXMATRIX				m_pWVP;					// WorldViewProjectino
	D3DXMATRIX				m_pWorld;				// World
	D3DXMATRIX				m_pView;				// View
	D3DXMATRIX				m_pProj;				// Projection
	int						m_iTextureSelect;		// Switch to select what texture will be used textures
	int						m_iBloodSelect;			// Switch to select what BloodMap will be loaded
	bool					m_Loaded;				// True if the BloodMap has been initialized
	bool					m_ResetCalled;			// True when the device has been reset

	bool					m_NewTexOrBlood;
	LPD3DXEFFECT			m_pEffect;
	IDirect3DTexture9*		m_pSurfaceMap;
	IDirect3DTexture9*		m_pSurfaceTexture;

	ID3DXMesh*				m_pD3DXMesh;
	BloodMap				m_BloodMap;

	FLOAT fAspect;
	//Render Helper is used to switch techniques and render targets
	void RenderHelper(char* Technique, LPDIRECT3DSURFACE9 Target, LPDIRECT3DSURFACE9 Stencil, IDirect3DDevice9* pd3dDevice);
	void ChangeTex(IDirect3DDevice9* pd3dDevice);

#include <shared/GetFilePath.h>

	//const D3DSURFACE_DESC*	m_d3dsdBackBuffer;
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
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
	DXUTSetCallbackD3D9DeviceCreated(OnCreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnResetDevice);
	DXUTSetCallbackD3D9DeviceLost(OnLostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnDestroyDevice);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(KeyboardProc);
	DXUTSetCallbackD3D9FrameRender(OnFrameRender);
	DXUTSetCallbackD3D9DeviceFrameMove(OnFrameMove);
    DXUTSetCursorSettings( true, true );

    InitApp();

    DXUTInit( true, true,NULL, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"Blood Shader" );
	DXUTCreateDevice(true, 800, 720);

    //DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 640, 480, IsDeviceAcceptable, ModifyDeviceSettings );

    DXUTMainLoop();

    return DXUTGetExitCode();
}

void InitApp()
{
    // Initialize dialogs
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

	// Initialize dialogs
	CDXUTListBox* pList;
	g_HUD.SetCallback( OnGUIEvent ); 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, 0, 125, 22 );
	g_HUD.AddListBox(IDC_TEXLISTBOX,0,40,150,43,0,&pList);
	if(pList)
	{
		pList->AddItem(L"Wall", (void*)0);
		pList->AddItem(L"Bowl", (void*)1);
	}
	g_HUD.GetListBox(IDC_TEXLISTBOX)->SelectItem(0);
	int radioStartY = 10;
	g_HUD.AddComboBox(IDC_COMBOBLOODSPOT,100,10,150,24,L'B');
	g_HUD.GetComboBox(IDC_COMBOBLOODSPOT)->AddItem(L"(B)lood Spot: Big",(void*)0);
	g_HUD.GetComboBox(IDC_COMBOBLOODSPOT)->AddItem(L"(B)lood Spot: Circle",(void*)1);
	g_HUD.GetComboBox(IDC_COMBOBLOODSPOT)->AddItem(L"(B)lood Spot: Many",(void*)2);

	g_HUD.AddSlider(IDC_BLOODCOLORRED,250,0,150,22,0,1000,928);
	g_HUD.AddSlider(IDC_BLOODCOLORGREEN,250,22,150,22,0,1000,156);
	g_HUD.AddSlider(IDC_BLOODCOLORBLUE,250,44,150,22,0,1000,156);
	
	TCHAR sz[100];

	_sntprintf( sz, 100, TEXT("Red:   %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()); sz[99] = 0;
	g_HUD.AddStatic(IDC_BLOODCOLORRED_STATIC,sz,400,0,100,22);
	_sntprintf( sz, 100, TEXT("Green: %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()); sz[99] = 0;
	g_HUD.AddStatic(IDC_BLOODCOLORGREEN_STATIC,sz,400,22,100,22);
	_sntprintf( sz, 100, TEXT("Blue:  %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()); sz[99] = 0;
	g_HUD.AddStatic(IDC_BLOODCOLORBLUE_STATIC,sz,400,44,100,22);

	g_HUD.AddButton(IDC_REDBUTTON,L"(R)ed Blood", 400,200,125,22,L'R');
	g_HUD.AddButton(IDC_GREENBUTTON,L"(G)reen Blood", 400,222,125,22,L'G');


	m_iTextureSelect			= 0; 
	m_iBloodSelect				= 0;
	m_pEffect					= NULL;
	m_pSurfaceMap				= NULL;
	m_pSurfaceTexture			= NULL;
	m_NewTexOrBlood				= false;
	m_pD3DXMesh					= NULL;

}

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;
    // check vertex shading support
    if (pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support 
    if (pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
 
    return SUCCEEDED((nErrors > nPrevErrors) ? E_FAIL : S_OK);
}

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

    D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 2.0f, 0.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
    return S_OK;
}

HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

    D3DXCreateSprite( pd3dDevice, &g_pSprite );

	g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	fAspect = ((FLOAT)pBackBufferSurfaceDesc->Width) / pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(D3DX_PI/4, fAspect, 1.0f, 100.0f);
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	m_ResetCalled = true;
    // TODO: setup render states
	m_Loaded = false;

    SAFE_RELEASE(m_pEffect);
	
	hr = D3DXCreateEffectFromFile(pd3dDevice,
								  GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\HLSL_BloodShader\\Blood.fx")).c_str(),
								  NULL,
								  NULL,
								  D3DXSHADER_DEBUG,
								  NULL,
								  &m_pEffect,
								  NULL);
	ChangeTex(pd3dDevice);

	g_HUD.GetControl(IDC_TOGGLEFULLSCREEN)->SetLocation(pBackBufferSurfaceDesc->Width-125, 10);
	g_HUD.GetControl(IDC_TEXLISTBOX)->SetLocation(pBackBufferSurfaceDesc->Width - 150, pBackBufferSurfaceDesc->Height - 125);
	g_HUD.GetControl(IDC_COMBOBLOODSPOT)->SetLocation(pBackBufferSurfaceDesc->Width - 150, pBackBufferSurfaceDesc->Height - 160);

	g_HUD.GetControl(IDC_BLOODCOLORRED)->SetLocation(pBackBufferSurfaceDesc->Width - 250, pBackBufferSurfaceDesc->Height - 76);
	g_HUD.GetControl(IDC_BLOODCOLORGREEN)->SetLocation(pBackBufferSurfaceDesc->Width - 250, pBackBufferSurfaceDesc->Height - 54);
	g_HUD.GetControl(IDC_BLOODCOLORBLUE)->SetLocation(pBackBufferSurfaceDesc->Width - 250, pBackBufferSurfaceDesc->Height - 32);

	g_HUD.GetControl(IDC_BLOODCOLORRED_STATIC)->SetLocation(pBackBufferSurfaceDesc->Width - 100, pBackBufferSurfaceDesc->Height - 76);
	g_HUD.GetControl(IDC_BLOODCOLORGREEN_STATIC)->SetLocation(pBackBufferSurfaceDesc->Width - 100, pBackBufferSurfaceDesc->Height - 54);
	g_HUD.GetControl(IDC_BLOODCOLORBLUE_STATIC)->SetLocation(pBackBufferSurfaceDesc->Width - 100, pBackBufferSurfaceDesc->Height - 32);

	g_HUD.GetControl(IDC_REDBUTTON)->SetLocation(pBackBufferSurfaceDesc->Width - 125, pBackBufferSurfaceDesc->Height - 212);
	g_HUD.GetControl(IDC_GREENBUTTON)->SetLocation(pBackBufferSurfaceDesc->Width - 125, pBackBufferSurfaceDesc->Height - 190);
    return S_OK;
}

void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    g_Camera.FrameMove( fElapsedTime );

	if(m_NewTexOrBlood)	{
		m_NewTexOrBlood = false;
		ChangeTex(pd3dDevice);
	}

	D3DXMATRIX pWV;
	D3DXMatrixMultiply(&pWV, &m_pWorld, &m_pView);
	D3DXMatrixMultiply(&m_pWVP, &pWV, &m_pProj);
}

bool FPS_lock(float FPS, float fTime)
{
	static float lastSecond = fTime;
	static float frameCount = 0.0;

	float correctFrame = (fTime - lastSecond) * FPS;
	if(correctFrame > 1.0f)
	{
		correctFrame -= 1.0f;
		lastSecond = fTime;
		frameCount = 0.0f;
	}
	if(frameCount < correctFrame)
	{
		frameCount += 1.0f;
		return true;
	}

	return false;
}

void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	D3DXMatrixMultiply(&m_pWVP, g_Camera.GetWorldMatrix(), g_Camera.GetViewMatrix() );
	D3DXMatrixMultiply(&m_pWVP, &m_pWVP, g_Camera.GetProjMatrix());
	m_pEffect->SetMatrix("World", g_Camera.GetWorldMatrix());
	m_pEffect->SetMatrix("WorldViewProjection", &m_pWVP);

	IDirect3DSurface9*		pBB;
	IDirect3DSurface9*		pZ;
	
	pd3dDevice->GetRenderTarget(0, &pBB);
	pd3dDevice->GetDepthStencilSurface(&pZ);
	
    // Begin the scene
    if( SUCCEEDED( pd3dDevice->BeginScene()) )
    {
		//Limit bloodshader animation to 50 FPS
		if(FPS_lock(50.0f, (float)fTime))
		{
			m_pEffect->SetTexture("GravityMap", m_BloodMap.pBloodTexture);
			RenderHelper("ComputeGravity", m_BloodMap.pBloodSurface, NULL, pd3dDevice);
			//Swap the gravitymap
			m_BloodMap.Swap();
		}

		//Render to BackBuffer
		RenderHelper("FinalBlend_Tech", pBB, pZ, pd3dDevice);

		// Render stats and help text  
		g_HUD.OnRender( fElapsedTime ); 
		RenderText();
		pd3dDevice->EndScene();

		SAFE_RELEASE(pBB);
		SAFE_RELEASE(pZ);
    }
}

void RenderHelper(char* Technique, LPDIRECT3DSURFACE9 Target, LPDIRECT3DSURFACE9 Stencil, IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	//Helper function used to switch between render targets
	hr = pd3dDevice->SetRenderTarget(0, Target);
	hr = pd3dDevice->SetDepthStencilSurface(Stencil);
	if(Stencil == NULL)
		pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET,
							0x00000000, 1.0f, 0L );
	else
		pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
							0x00000000, 1.0f, 0L );
	hr = m_pEffect->SetTechnique(Technique);

	UINT iPass, iPasses;
	hr = m_pEffect->Begin(&iPasses, 0);
			for(iPass = 0; iPass < iPasses; iPass++)
			{
				hr = m_pEffect->BeginPass(iPass);
				hr = m_pD3DXMesh->DrawSubset(0);
				hr = m_pEffect->EndPass();
			}
	hr = m_pEffect->End();
}



void RenderText()
{
	/*
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	txtHelper.DrawTextLine( DXUTGetFrameStats() );
	txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
    txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height-15*6 );
    txtHelper.SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"Controls:" );

    txtHelper.SetInsertionPos( 250, pd3dsdBackBuffer->Height-15*5 );
    txtHelper.DrawTextLine( L"Hide help: F1.  PAUSE pauses.\n"
							L"Arrow Keys Move Camera\n"
							L"LMB Rotates Camera.\n"
                            L"Quit: ESC\n" );
    txtHelper.End();
	*/
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
	case IDC_TEXLISTBOX:
		{
			DXUTListBoxItem *pItem = ((CDXUTListBox *)pControl)->GetItem( ((CDXUTListBox *)pControl)->GetSelectedIndex( -1 ) );
			m_iTextureSelect = (int)(size_t)pItem->pData;
			m_NewTexOrBlood = true;
			m_Loaded = false;
			break;
		}
	case IDC_COMBOBLOODSPOT:
		{
			DXUTComboBoxItem *pItem = ((CDXUTComboBox*)pControl)->GetSelectedItem();

			m_iBloodSelect = (int)(size_t)pItem->pData;
			m_Loaded = false;
			
			m_NewTexOrBlood = true;
			break;
		}
	case IDC_BLOODCOLORRED:
	case IDC_BLOODCOLORGREEN:
	case IDC_BLOODCOLORBLUE:
		{
			
			TCHAR sz[100];

			_sntprintf( sz, 100, TEXT("Red:   %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORRED_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Green: %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORGREEN_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Blue:  %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORBLUE_STATIC)->SetText(sz);

			m_pEffect->SetValue("BloodColor", (void*)(FLOAT*)(D3DXVECTOR3(g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()/1000.0f,
				g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()/1000.0f,g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()/1000.0f)),
				sizeof(D3DXVECTOR3));
			break;
		}
	case IDC_REDBUTTON:
		{	
			g_HUD.GetSlider(IDC_BLOODCOLORRED)->SetValue(928);
			g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->SetValue(156);
			g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->SetValue(156);
			TCHAR sz[100];

			_sntprintf( sz, 100, TEXT("Red:   %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORRED_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Green: %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORGREEN_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Blue:  %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORBLUE_STATIC)->SetText(sz);

			m_pEffect->SetValue("BloodColor", (void*)(FLOAT*)(D3DXVECTOR3(g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()/1000.0f,
				g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()/1000.0f,g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()/1000.0f)),
				sizeof(D3DXVECTOR3));

			break;
		}
	case IDC_GREENBUTTON:
		{
			g_HUD.GetSlider(IDC_BLOODCOLORRED)->SetValue(39);
			g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->SetValue(840);
			g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->SetValue(0);
			TCHAR sz[100];

			_sntprintf( sz, 100, TEXT("Red:   %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORRED_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Green: %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORGREEN_STATIC)->SetText(sz);
			_sntprintf( sz, 100, TEXT("Blue:  %d/1000"), g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLOODCOLORBLUE_STATIC)->SetText(sz);

			m_pEffect->SetValue("BloodColor", (void*)(FLOAT*)(D3DXVECTOR3(g_HUD.GetSlider(IDC_BLOODCOLORRED)->GetValue()/1000.0f,
				g_HUD.GetSlider(IDC_BLOODCOLORGREEN)->GetValue()/1000.0f,g_HUD.GetSlider(IDC_BLOODCOLORBLUE)->GetValue()/1000.0f)),
				sizeof(D3DXVECTOR3));

			break;
		}

	}
}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    
    SAFE_RELEASE(g_pSprite);

	SAFE_RELEASE(m_pEffect);

	SAFE_RELEASE(m_pSurfaceMap);

	SAFE_RELEASE(m_pSurfaceTexture);

	SAFE_RELEASE(m_pD3DXMesh);
	m_BloodMap.Release();
}

void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
}

void ChangeTex(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	TCHAR* SurfaceMap;
	TCHAR* SurfaceTexture;
	TCHAR* BloodMap;
	TCHAR* Mesh;

	SAFE_RELEASE(m_pSurfaceMap);
	SAFE_RELEASE(m_pSurfaceTexture);

	SAFE_RELEASE(m_pD3DXMesh);

	switch(m_iTextureSelect)
	{
	case 0:
		SurfaceMap = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\cobblemap.dds";
		SurfaceTexture = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\cobble.dds";
		Mesh = L"MEDIA\\models\\HLSL_BloodShader\\plane.x";
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		break;

	case 1:
		SurfaceMap = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\ugabowl1f_n.dds";
		SurfaceTexture = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\ugabowl1f.dds";
		Mesh = L"MEDIA\\models\\HLSL_BloodShader\\simpleBowl.x";
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		break;
	}

	switch(m_iBloodSelect)
	{
	case 0:
 		BloodMap = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\fluidmap0.dds";
		break;
	case 1:
		BloodMap = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\fluidmap1.dds";
		break;
	case 2:
		BloodMap = L"MEDIA\\textures\\2D\\HLSL_BloodShader\\fluidmap2.dds";
	}

	m_BloodMap.Init(pd3dDevice, GetFilePath::GetFilePath(BloodMap).c_str());
	
	tstring tmp = GetFilePath::GetFilePath(SurfaceMap);
	hr = D3DXCreateTextureFromFile(pd3dDevice, tmp.c_str(), &m_pSurfaceMap);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(SurfaceTexture).c_str(), &m_pSurfaceTexture);
	m_pEffect->SetTexture("SurfaceMap", m_pSurfaceMap);
	m_pEffect->SetTexture("SurfaceTexture", m_pSurfaceTexture);

	hr = D3DXLoadMeshFromX(GetFilePath::GetFilePath(Mesh).c_str(),
						   D3DXMESH_MANAGED,
						   pd3dDevice,
						   NULL,
						   NULL,
						   NULL,
						   NULL,
						   &m_pD3DXMesh);

	ID3DXMesh* pTempMesh = NULL;
	m_pD3DXMesh->CloneMesh(D3DXMESH_MANAGED ,
						   decl,
						   pd3dDevice,
						   &pTempMesh);
	SAFE_RELEASE(m_pD3DXMesh);

	D3DXComputeNormals(pTempMesh, NULL);

	D3DXComputeTangent(pTempMesh,
					   0,
					   0,
					   0,
					   true,
					   NULL);
	m_pD3DXMesh = pTempMesh;
}