#define STRICT
#include "dxstdafx.h"

#include "DeferredShading.h"
#include "DeferredShadingApp.h"

#include <DXUT/SDKmisc.h>
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
    DXUTCreateWindow( L"DeferredShading" );
    DXUTCreateDevice(  true, 512, 512 );

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
	g_pDS = new DeferredShading();
	g_pDS->OneTimeSceneInit(DXUTGetHWND());
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );

	g_HUD.AddButton( IDC_SCISSORING, L"Disable Scissoring (C)", 0, 0, 140, 22, L'C' );
	g_HUD.AddButton( IDC_SCISOROUTLINE, L"Show Scissor Outline (V)", 0, 0, 140, 22, L'V' );
	CDXUTListBox* pList;
	g_HUD.AddListBox( IDC_MRTBUFFERS, 0, 0, 125, 70, 0, &pList );
	
	int i = 0;
	pList->AddItem( L"MRT0 - Diffuse Texture", (void*)(i++));
	pList->AddItem( L"MRT1 - Normals", (void*)(i++));
	pList->AddItem( L"MRT2 - Depth", (void*)(i++));
	pList->AddItem( L"Final Scene", (void*)(i));
	pList->SelectItem(i);

	g_HUD.AddButton( IDC_TONEMAP, L"Disable Tone Mapping (T)", 0, 0, 140, 22, L'T' );

	g_HUD.AddSlider( IDC_DEPTHBIAS_SLIDER, 0, 0, 125, 22, 0, 500, g_pDS->m_fDepthBias*100000.0f );
	TCHAR sz[100];
	_sntprintf( sz, 33, L"Depth Bias: %.4f", g_pDS->m_fDepthBias = ((float)g_HUD.GetSlider(IDC_DEPTHBIAS_SLIDER)->GetValue()/100000.0f) );
	sz[99] = 0;
	g_HUD.AddStatic( IDC_DEPTHBIAS_STATIC, sz, 0, 0, 125, 22 );

	g_HUD.AddSlider( IDC_SLOPEBIAS_SLIDER, 0, 0, 125, 22, 0, 1000, g_pDS->m_fBiasSlope*10.0f );
	_sntprintf( sz, 33, L"Slope Bias: %.1f", g_pDS->m_fBiasSlope = ((float)g_HUD.GetSlider(IDC_DEPTHBIAS_SLIDER)->GetValue()/10.0f) );
	sz[99] = 0;
	g_HUD.AddStatic( IDC_SLOPEBIAS_STATIC, sz, 0, 0, 125, 22 );

	g_HUD.AddButton( IDC_CONTROLLIGHT, L"Control Light (P)", 0, 0, 140, 22, L'P' );
	g_HUD.AddButton( IDC_ANIMATELIGHTS, L"Animate Lights (L)", 0, 0, 140, 22, L'L' );
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	bool bCapsAcceptable = true;

	if(FAILED(DXUTGetD3D9Object()->CheckDeviceFormat(D3DADAPTER_DEFAULT, pCaps->DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
		D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
		bCapsAcceptable = false;
	if(FAILED(g_pDS->ConfirmDevice(pCaps,0,AdapterFormat, BackBufferFormat)))
		bCapsAcceptable = false;

	return bCapsAcceptable;
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

	g_pDS->m_iBBHeight = pBackBufferSurfaceDesc->Height;
	g_pDS->m_iBBWidth = pBackBufferSurfaceDesc->Width;

	D3DCAPS9 deviceCaps;
	pd3dDevice->GetDeviceCaps(&deviceCaps);
	g_pDS->m_bCanTonemap = g_pDS->m_bCanTonemap && SUCCEEDED(DXUTGetD3D9Object()->CheckDeviceFormat(D3DADAPTER_DEFAULT, 
		deviceCaps.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, 
		D3DFMT_A16B16G16R16F));

	V_RETURN(g_pDS->RestoreDeviceObjects(pd3dDevice));

	if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

	D3DXVECTOR3 eyePt( 3.4184132f, -47.082867f, 25.271486f);
	D3DXVECTOR3 vLookatPt( 3.2370236f, -47.331966f, 24.320147f);// = g_pDS->m_Scene->m_Center;
	g_Camera.SetViewParams( &eyePt, &vLookatPt);
	g_pDS->m_View = *g_Camera.GetViewMatrix();

	float viewRadius = 1.5f * g_pDS->m_Scene->m_Radius;
	// Setup the camera's projection parameters
	float zFar = 10.0f * viewRadius / 2.0f;
	float zNear = 0.2f;
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DXToRadian(60.0f), fAspectRatio, zNear, zFar );
	//g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_pDS->m_Projection = *g_Camera.GetProjMatrix();

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

	iY += 24;
	g_HUD.GetControl( IDC_SCISSORING )->SetLocation( pBackBufferSurfaceDesc->Width - 145, iY += 24 );
	g_HUD.GetControl( IDC_SCISOROUTLINE )->SetLocation( pBackBufferSurfaceDesc->Width - 145, iY += 24 );
	g_HUD.GetControl( IDC_TONEMAP )->SetLocation( pBackBufferSurfaceDesc->Width - 145, iY += 24 );
	g_HUD.GetControl( IDC_CONTROLLIGHT )->SetLocation( pBackBufferSurfaceDesc->Width - 145, iY += 24 );
	g_HUD.GetControl( IDC_ANIMATELIGHTS )->SetLocation( pBackBufferSurfaceDesc->Width - 145, iY += 24 );

	g_HUD.GetControl( IDC_MRTBUFFERS )->SetLocation( pBackBufferSurfaceDesc->Width - (iY = 135), pBackBufferSurfaceDesc->Height - 80 );

	g_HUD.GetControl( IDC_DEPTHBIAS_SLIDER )->SetLocation( pBackBufferSurfaceDesc->Width - (iY += 135), pBackBufferSurfaceDesc->Height - 35 );
	g_HUD.GetControl( IDC_DEPTHBIAS_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - (iY), pBackBufferSurfaceDesc->Height - 50 );

	g_HUD.GetControl( IDC_SLOPEBIAS_SLIDER )->SetLocation( pBackBufferSurfaceDesc->Width - (iY += 135), pBackBufferSurfaceDesc->Height - 35 );
	g_HUD.GetControl( IDC_SLOPEBIAS_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - (iY), pBackBufferSurfaceDesc->Height - 50 );

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);


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

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		g_pDS->m_View = *g_Camera.GetViewMatrix();
		g_pDS->m_Projection = *g_Camera.GetProjMatrix();

		g_pDS->Render(pd3dDevice);
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
		txtHelper.DrawTextLine( TEXT("") );
		txtHelper.DrawTextLine( TEXT("W,A,S,D - Move") );
		txtHelper.DrawTextLine( TEXT("Q,E     - Strafe Up/Down") );
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

	if( !(g_bControlLight && (wParam == 'w' || wParam == 'W' || wParam == 'a' || wParam == 'A' || wParam == 's' || wParam == 'S' || wParam == 'd' || wParam == 'D' || wParam == 'q' || wParam == 'Q' || wParam == 'e' || wParam == 'E')))
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
		case 'w':
		case 'W':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.x += 0.5f;
			break;
		case 'a':
		case 'A':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.z += 0.5f;
			break;
		case 's':
		case 'S':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.x -= 0.5f;
			break;
		case 'd':
		case 'D':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.z -= 0.5f;
			break;
		case 'q':
		case 'Q':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.y += 0.5f;
			break;
		case 'e':
		case 'E':
			if(g_bControlLight)
				g_pDS->m_Lights[1].Position.y -= 0.5f;
			break;
		}
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
		case IDC_SCISSORING:
			{
				g_pDS->m_bDoScissor = !g_pDS->m_bDoScissor;
				if( g_pDS->m_bDoScissor )
					g_HUD.GetButton( IDC_SCISSORING )->SetText(L"Enable Scissoring (C)");
				else
					g_HUD.GetButton( IDC_SCISSORING )->SetText(L"Disable Scissoring (C)");
					
				break;
			}
		case IDC_SCISOROUTLINE:
			{
				if( g_pDS->m_bDebugScissor = !g_pDS->m_bDebugScissor )
					g_HUD.GetButton( IDC_SCISOROUTLINE )->SetText( L"Hide Scissor Outline (V)" );
				else
					g_HUD.GetButton( IDC_SCISOROUTLINE )->SetText( L"Show Scissor Outline (V)" );
				break;
			}
		case IDC_MRTBUFFERS:
			{
				switch (g_HUD.GetListBox( IDC_MRTBUFFERS )->GetSelectedIndex())
				{
				case 0:
					g_pDS->m_currRenderMode = RM_CHANNEL0;
					break;
				case 1:
					g_pDS->m_currRenderMode = RM_CHANNEL1;
					break;
				case 2:
					g_pDS->m_currRenderMode = RM_CHANNEL2;
					break;
				case 3:
					g_pDS->m_currRenderMode = RM_NORMAL;
					break;
				}
				break;
			}
		case IDC_TONEMAP:
			{
				g_pDS->m_bTonemap = !g_pDS->m_bTonemap;
				if( g_pDS->m_bTonemap  )
					g_HUD.GetButton( IDC_TONEMAP )->SetText( L"Enable Tone Mapping (T)" );
				else
					g_HUD.GetButton( IDC_TONEMAP )->SetText( L"Disable Tone Mapping (T)" );
				break;
			}
		case IDC_DEPTHBIAS_SLIDER:
			{
				TCHAR sz[100];
				_sntprintf( sz, 33, L"Depth Bias: %.4f", g_pDS->m_fDepthBias = ((float)g_HUD.GetSlider(IDC_DEPTHBIAS_SLIDER)->GetValue()/100000.0f) );
				sz[99] = 0;
				g_HUD.GetStatic(IDC_DEPTHBIAS_STATIC)->SetText(sz);
				break;
			}
		case IDC_SLOPEBIAS_SLIDER:
			{
				TCHAR sz[100];
				_sntprintf( sz, 33, L"Slope Bias: %.1f", g_pDS->m_fBiasSlope = ((float)g_HUD.GetSlider(IDC_SLOPEBIAS_SLIDER)->GetValue()/10.0f));
				sz[99] = 0;
				g_HUD.GetStatic(IDC_SLOPEBIAS_STATIC)->SetText(sz);
				break;
			}
		case IDC_CONTROLLIGHT:
			{
				g_bControlLight = !g_bControlLight;
				if( g_bControlLight )
					g_HUD.GetButton( IDC_CONTROLLIGHT )->SetText( L"Control Light (P)" );
				else
					g_HUD.GetButton( IDC_CONTROLLIGHT )->SetText( L"Control Camera (P)" );
				break;
			}
		case IDC_ANIMATELIGHTS:
			{
				g_pDS->m_bAnimateLights = !g_pDS->m_bAnimateLights;
				break;
			}
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

    if( g_pFont )
        g_pFont->OnLostDevice();

	SAFE_RELEASE(g_pTextSprite);

	g_pDS->InvalidateDeviceObjects();
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