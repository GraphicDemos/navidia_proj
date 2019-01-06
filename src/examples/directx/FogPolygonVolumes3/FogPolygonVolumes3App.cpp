#define STRICT
#include "dxstdafx.h"

#include "FogPolygonVolumes3App.h"
#include "FogTombDemo.h"
#include "FogTombScene.h"
#include "FogPolygonVolumes3GUI.h"

#include <DXUT\DXUTgui.h>
#include <DXUT\SDKmisc.h>
#define BB_WIDTH	512
#define BB_HEIGHT	512

bool	gbCLPS3				= true;
bool	gbCLPerfHUDOptIn	= false;
bool	gbCLFullscreen		= false;
bool	gbCLNoGUI			= false;

#define CONSOLE_OUT( s )			\
{	OutputDebugStringA( s );		\
	printf( s );					\
}

void ShowGUI( bool bShowUI );

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR strCmdLine, int nCmdShow )
{
	printf( "why???" );
	fprintf( stdout, "ehllo" );
	if( strlen( strCmdLine ) < 1 )
	{
		CONSOLE_OUT( "No command line options detected.\n" );
		CONSOLE_OUT( "Options are:\n" );
		CONSOLE_OUT( "   -ps2   : start in ps.2.0 mode\n");
		CONSOLE_OUT( "   -phopt : opt-in to nvPerfHUD\n");	
		CONSOLE_OUT( "   -fs    : start in fullscreen\n" );
		CONSOLE_OUT( "   -nogui : hide GUI at startup\n" );
	}
	else
	{
		CONSOLE_OUT( "cmd line: ");
		CONSOLE_OUT( strCmdLine );
		CONSOLE_OUT( "\n");
	}
	char * pc;
	pc = strstr( strCmdLine, "-ps2" );
	if( pc != NULL )
		gbCLPS3 =	false;
	pc = strstr( strCmdLine, "-phopt" );
	if( pc != NULL )
		gbCLPerfHUDOptIn = true;
	pc = strstr( strCmdLine, "-fs" );
	if( pc != NULL )
		gbCLFullscreen = true;
	pc = strstr( strCmdLine, "-nogui" );
	if( pc != NULL )
		gbCLNoGUI = true;

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
    DXUTCreateWindow( L"FogPolygonVolumes3" );
    DXUTCreateDevice(  true, BB_WIDTH, BB_HEIGHT );

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
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    bool bCapsAcceptable = true;

	FogTombDemo demo;
	HRESULT hr = S_OK;
	hr = demo.ConfirmDevice( pCaps, 0, AdapterFormat, BackBufferFormat );
	if( FAILED(hr) )
		bCapsAcceptable = false;

	return( bCapsAcceptable );
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
	if( gbCLFullscreen )
	{
		// Use this to start fullscreen.  Change width & height to suitable fullscreen resolution
		pDeviceSettings->d3d9.pp.BackBufferWidth = 1024;
		pDeviceSettings->d3d9.pp.BackBufferHeight = 768;
		pDeviceSettings->d3d9.pp.Windowed = false;
	}

	if( gbCLPerfHUDOptIn )
	{
		// Ref count is NOT incremented
		IDirect3D9 * pD3D9 = DXUTGetD3D9Object();
		if( pD3D9 != NULL )
		{
			OutputDebugString( TEXT("Opting in to nvPerfHUD!\n"));
			pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
			pDeviceSettings->d3d9.AdapterOrdinal = pD3D9->GetAdapterCount() - 1;
		}
		else
		{
			OutputDebugString( TEXT("DXUTGetD3DObject() returned NULL!\n"));
		}
	}
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

    SAFE_DELETE( g_pFogTombDemo );
	g_pFogTombDemo = new FogTombDemo;
	hr = g_pFogTombDemo->Initialize( pd3dDevice );
	V_RETURN(hr);
	if( !gbCLPS3 )
		g_pFogTombDemo->SetTechnique( FogTombDemo::FOGDEMO_PS20 );

	SAFE_DELETE( g_pFogDemoGUI );
	g_pFogDemoGUI = new FogPolygonVolumes3GUI;
	hr = g_pFogDemoGUI->Initialize( &g_pFogTombDemo,
                                     g_DialogResourceManager,
									20, 120, 
									0, 0 );
	int h = g_pFogDemoGUI->m_pDialog->GetHeight();
	int w = g_pFogDemoGUI->m_pDialog->GetWidth();
	g_pFogDemoGUI->m_pDialog->SetLocation( pBackBufferSurfaceDesc->Width - w - 10, 5 );
	V_RETURN(hr);

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

	g_HUD.SetLocation( 0, 0 );
	g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
	g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
	g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
	g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

	ShowGUI( !gbCLNoGUI );
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
	if( g_pFogTombDemo )
		g_pFogTombDemo->FrameMove( fElapsedTime );
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

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		if( g_pFogTombDemo )
		{
			hr = g_pFogTombDemo->Render();
			V(hr);
		}

		IDirect3DDevice9 * pD3DDev;
		g_pFont->GetDevice( &pD3DDev );	
		pD3DDev->SetRenderState( D3DRS_ZENABLE,		false );
		pD3DDev->SetPixelShader( NULL );
		pD3DDev->SetVertexShader( NULL );
		SAFE_RELEASE( pD3DDev );

		if( g_pFogDemoGUI != NULL )
		{
			V( g_pFogDemoGUI->Render( fElapsedTime ) );
		}
		V( g_HUD.OnRender( fElapsedTime ) );
        // Render stats and help text  
        RenderText();
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

		// Display the technique being used
		TCHAR szMsg[2048];
		if( g_pFogTombDemo != NULL )
		{
			switch( g_pFogTombDemo->m_eRenderMode )
			{
			case FogTombDemo::FOGDEMO_PS13 :
				lstrcpy(szMsg, TEXT("Using ps.1.3 A8R8G8B8 path"));
				break;
			case FogTombDemo::FOGDEMO_PS20 :
				lstrcpy(szMsg, TEXT("Using ps.2.0 A8R8G8B8 path"));
				break;
			case FogTombDemo::FOGDEMO_PS30_MRT :
				lstrcpy(szMsg, TEXT("Using ps.3.0 MRT fp blending path"));
				break;
			default:
				lstrcpy(szMsg, TEXT("Unknown FogTombDemo m_eRenderMode"));
				break;
			}
			txtHelper.DrawTextLine( szMsg );
		}

		if( !g_bShowHelp )
		{
			txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
			txtHelper.DrawTextLine( TEXT("F1      - Toggle help text") );
		}
	}

	if( g_bShowHelp )
	{
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
        return( 0 );

	if( g_pFogDemoGUI != NULL )
	{
		g_pFogDemoGUI->MsgProc( hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing );
		if( *pbNoFurtherProcessing )
			return( 0 );
	}

	if( g_pFogTombDemo != NULL )
	{
		if( g_pFogTombDemo->m_pFogTombScene != NULL )
		{
			g_pFogTombDemo->m_pFogTombScene->MsgProc( hWnd, uMsg, wParam, lParam );
		}
	}
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
			ShowGUI( g_bShowUI );
			break;
		}
	}
}

void ShowGUI( bool bShowUI )
{
	g_bShowUI = bShowUI;
	for( int i = 0; i < IDC_LAST; i++ )
		g_HUD.GetControl(i)->SetVisible( g_bShowUI );
	if( g_pFogDemoGUI != NULL )
	{
		g_pFogDemoGUI->SetVisible( g_bShowUI );
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
    }
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

    SAFE_DELETE( g_pFogTombDemo );
	SAFE_DELETE( g_pFogDemoGUI );

    if( g_pFont )
        g_pFont->OnLostDevice();
	SAFE_RELEASE(g_pTextSprite);
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