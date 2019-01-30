#define STRICT
#include "dxstdafx.h"
#include "CSAATutorialApp.h"
#include <shared/GetFilePath.h>
#include <shared/NV_Error.h>

#include <DXUT/SDKmisc.h>

LPDIRECT3DVERTEXBUFFER9			g_pVBuffer;
LPDIRECT3DVERTEXBUFFER9			g_pCBuffer;
LPDIRECT3DINDEXBUFFER9			g_pIBuffer;
LPDIRECT3DVERTEXDECLARATION9	g_pVDecl;

ID3DXEffect*					g_pEffect;

int								g_SampleCount = 0;
int								g_QualityLevel = 0;
bool							g_Warning = false;

float	g_Geometry[] =
{
	0, 1.0f, 0,
	-1.0f, -1.0f, 0,
	1.0f, -1.0f, 0,
};

float	g_Colors[] =
{
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
};

short	g_Indices[] =
{
	0, 1, 2,
};

D3DVERTEXELEMENT9	VDECL[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },

	D3DDECL_END()
};

bool AppInit(IDirect3DDevice9* pd3dDevice, wchar_t* filename)
{
	HRESULT	hr;
	float*	pDataf = NULL;
	short*	pData16 = NULL;

	pd3dDevice->CreateVertexBuffer(2048, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT,
									&g_pVBuffer, NULL);
	pd3dDevice->CreateVertexBuffer(2048, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT,
									&g_pCBuffer, NULL);
	pd3dDevice->CreateIndexBuffer(2048, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
									&g_pIBuffer, NULL);

	V( g_pVBuffer->Lock(0, 0, (VOID**)&pDataf, 0));
	memcpy(pDataf, &g_Geometry, sizeof(g_Geometry));
	V( g_pVBuffer->Unlock());

	V( g_pCBuffer->Lock(0, 0, (VOID**)&pDataf, 0));
	memcpy(pDataf, &g_Colors, sizeof(g_Colors));
	V( g_pCBuffer->Unlock());

	V( g_pIBuffer->Lock(0, 0, (VOID**)&pData16, 0));
	memcpy(pData16, &g_Indices, sizeof(g_Indices));
	V( g_pIBuffer->Unlock());

	pd3dDevice->CreateVertexDeclaration(VDECL, &g_pVDecl);

	V( D3DXCreateEffectFromFile(pd3dDevice,
								filename,
								NULL,
								NULL,
								D3DXSHADER_DEBUG,
								NULL,
								&g_pEffect,
								NULL));

	return true;
};

void AppRelease(void)
{
	g_pVBuffer->Release();
	g_pCBuffer->Release();
	g_pIBuffer->Release();
	g_pEffect->Release();
	g_pVDecl->Release();
}

void AppRender(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX	LWVP;
	UINT		numVertices = sizeof(g_Indices) / sizeof(short);
	UINT		pass, passes;

	D3DXMatrixMultiply(&LWVP, g_Camera.GetWorldMatrix(), g_Camera.GetViewMatrix());
    D3DXMatrixMultiply(&LWVP, &LWVP, g_Camera.GetProjMatrix());
	g_pEffect->SetMatrix("LocalWorldViewProj", &LWVP);

	pd3dDevice->SetStreamSource(0, g_pVBuffer, 0, sizeof(D3DXVECTOR3));
	pd3dDevice->SetStreamSource(1, g_pCBuffer, 0, sizeof(D3DXVECTOR4));
	pd3dDevice->SetIndices(g_pIBuffer);
	pd3dDevice->SetVertexDeclaration(g_pVDecl);

	g_pEffect->SetTechnique("Main");

	g_pEffect->Begin(&passes, 0);
	for (pass = 0; pass < passes; pass++)
	{
		g_pEffect->BeginPass(pass);
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numVertices / 3);
		g_pEffect->EndPass();
	}
	g_pEffect->End();
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Set the callback functions. These functions allow the sample framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"CSAATutorial" );
//	DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 1024, 1024, IsDeviceAcceptable, ModifyDeviceSettings );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
//    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );

	g_HUD.AddRadioButton( IDC_NO_AA, 1, L"No AA (S=0 Q=0)", 5, 106, 200, 22, true);
	g_HUD.AddRadioButton( IDC_4X, 1, L"4X MSAA (S=4 Q=0)", 5, 130, 200, 22, false);
	g_HUD.AddRadioButton( IDC_8X, 1, L"8X CSAA (S=4 Q=2)", 5, 154, 200, 22, false);
	g_HUD.AddRadioButton( IDC_8XQ, 1, L"8XQ CSAA (S=8 Q=0)", 5, 178, 200, 22, false);
	g_HUD.AddRadioButton( IDC_16X, 1, L"16X CSAA (S=4 Q=4)", 5, 202, 200, 22, false);
	g_HUD.AddRadioButton( IDC_16XQ, 1, L"16XQ CSAA (S=8 Q=2)", 5, 226, 200, 22, false);
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    BOOL bCapsAcceptable = TRUE;

    // TODO: Perform checks to see if these display caps are acceptable.

    if (!bCapsAcceptable)
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
	DWORD	qualityLevels;

	// before we create/recreate the device, set up AA parameters
	switch (g_SampleCount)
	{
		case 0:
            // CheckDeviceMultiSampleType used to verify the max quality level.
            // See page 5 of the related white paper
			DXUTGetD3D9Object()->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
															pDeviceSettings->d3d9.DeviceType,
															pDeviceSettings->d3d9.pp.BackBufferFormat,
															pDeviceSettings->d3d9.pp.Windowed,
															D3DMULTISAMPLE_NONE,
															&qualityLevels);
            // These new CSAA parameters will be passed to DX through
			pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_NONE;
			pDeviceSettings->d3d9.pp.MultiSampleQuality = qualityLevels <= g_QualityLevel ? 0 : g_QualityLevel;
		break;

		case 4:
			DXUTGetD3D9Object()->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
															pDeviceSettings->d3d9.DeviceType,
															pDeviceSettings->d3d9.pp.BackBufferFormat,
															pDeviceSettings->d3d9.pp.Windowed,
															D3DMULTISAMPLE_4_SAMPLES,
															&qualityLevels);

			pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
			pDeviceSettings->d3d9.pp.MultiSampleQuality = qualityLevels <= g_QualityLevel ? 0 : g_QualityLevel;
		break;

		case 8:
			DXUTGetD3D9Object()->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
															pDeviceSettings->d3d9.DeviceType,
															pDeviceSettings->d3d9.pp.BackBufferFormat,
															pDeviceSettings->d3d9.pp.Windowed,
															D3DMULTISAMPLE_8_SAMPLES,
															&qualityLevels);

			pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
			pDeviceSettings->d3d9.pp.MultiSampleQuality = qualityLevels <= g_QualityLevel ? 0 : g_QualityLevel;
		break;
	}
	// show a warning if we didn't support requested CSAA mode
	if(qualityLevels <= g_QualityLevel)
	{
			g_QualityLevel = 0;
			g_Warning = true;
	} else	g_Warning = false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Set up our view matrix. A view matrix can be defined given an eye point and
    // a point to lookat. Here, we set the eye five units back along the z-axis and 
	// up three units and look at the origin.
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
//    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	if (AppInit(pd3dDevice, (wchar_t*)GetFilePath::GetFilePath(L"..\\MEDIA\\programs\\csstutorial.cso", true).c_str()) == false)
		return S_FALSE;


    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // TODO: update world
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;

    // Clear the viewport
//    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
  //                      0x000000ff, 1.0f, 0L);
	pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x00000000, 1.0f, 0L);

    // Set the world matrix
    pd3dDevice->SetTransform(D3DTS_WORLD, g_Camera.GetWorldMatrix());

    // Set the projection matrix
    pd3dDevice->SetTransform(D3DTS_PROJECTION, g_Camera.GetProjMatrix());

	// Set the view matrix
	pd3dDevice->SetTransform(D3DTS_VIEW, g_Camera.GetViewMatrix());

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        // TODO: render world
		AppRender(pd3dDevice);

        // Render stats and help text  
        RenderText();

		V( g_HUD.OnRender( fElapsedTime ) );

        // End the scene.
        V( pd3dDevice->EndScene() );
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
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 15 );
	if( g_bShowUI )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( DXUTGetFrameStats() );
		txtHelper.DrawTextLine( DXUTGetDeviceStats() );
        static TCHAR s_CSAATxt[256];
        wsprintf(s_CSAATxt,TEXT("Sample Count = %d"), g_SampleCount);
		txtHelper.DrawTextLine( s_CSAATxt );
		wsprintf(s_CSAATxt,TEXT("Quality Level = %d %s"), g_QualityLevel, g_Warning ? L"(Warning : couldn't get the requested level from the device)" : L"");
		txtHelper.DrawTextLine( s_CSAATxt );

		// Display any additional information text here

		if( !g_bShowHelp )
		{
			txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
			txtHelper.DrawTextLine( TEXT("F1      - Toggle help text") );
		}
	}

	if( g_bShowHelp )
	{
		// Display help text here
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawTextLine( TEXT("F1      - Toggle help text") );
		txtHelper.DrawTextLine( TEXT("H       - Toggle UI") );
		txtHelper.DrawTextLine( TEXT("ESC  - Quit") );
	}
	txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, the sample framework inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_F1:
			g_bShowHelp = !g_bShowHelp;
			break;

		case 'H':
		case 'h':
			g_bShowUI = !g_bShowUI;
			for( int i = 0; i < IDC_LAST; i++ )
				g_HUD.GetControl(i)->SetVisible( g_bShowUI );
			break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events. Especially for CSAA choices
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	HRESULT					hr;
	D3DPRESENT_PARAMETERS	presentParams;
	bool					bWindowed = false;

	switch( nControlID )
    {
//        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;

        // CSAA modes
        // ----------
        // Here are the various combinations that allows us to switch to CSAA modes
        // check ModifyDeviceSettings() to see how are these 2 variables used
        // See table 2 page 5 of the related white paper
		// no AA
		case IDC_NO_AA:
			g_SampleCount = 0;
			g_QualityLevel = 0;
		break;

		// 4X MSAA
		case IDC_4X:
			g_SampleCount = 4;
			g_QualityLevel = 0;
		break;

		// 8X CSAA
		case IDC_8X:
			g_SampleCount = 4;
			g_QualityLevel = 2;
		break;

		// 8XQ CSAA (Quality)
		case IDC_8XQ:
			g_SampleCount = 8;
			g_QualityLevel = 0;
		break;

		// 16X CSAA
		case IDC_16X:
			g_SampleCount = 4;
			g_QualityLevel = 4;
		break;

		// 16XQ CSAA (Quality)
		case IDC_16XQ:
			g_SampleCount = 8;
			g_QualityLevel = 2;
		break;
    }

	presentParams = DXUTGetD3D9PresentParameters();

    // DXUTCreateDevice will call ModifyDeviceSettings() callback to change the settings
    // according to our CSAA choice
	V(DXUTCreateDevice(presentParams.Windowed, 0, 0 ));
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

    if( g_pFont )
        g_pFont->OnLostDevice();

	SAFE_RELEASE(g_pTextSprite);

	AppRelease();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

    SAFE_RELEASE(g_pFont);
}