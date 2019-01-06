#define STRICT
#include "dxstdafx.h"

#include "HLSL_SoftShadows.h"
#include "HLSL_SoftShadowsApp.h"

#include <DXUT/SDKmisc.h>
#include <TCHAR.h>

#define DEPTH_TEXT "Depth Bias:  %.6f"
#define SLOPE_TEXT "Slope Scale Bias:  %.2f"
#define SAMPLES_TEXT "# Shadow Samples:  %d"
#define SOFTNESS_TEXT "Shadow Filter Size:  %.2f"
#define DEPTH_ALG g_pSoftShadows->m_fDepthBias = ((float)g_HUD.GetSlider(IDC_DEPTH)->GetValue())/1000000.0f
#define SLOPE_ALG g_pSoftShadows->m_fBiasSlope = ((float)g_HUD.GetSlider(IDC_SLOPE)->GetValue())/100.0f
#define SAMPLES_ALG g_pSoftShadows->m_NumSamples = g_HUD.GetSlider(IDC_SAMPLES)->GetValue()*16
#define SOFTNESS_ALG g_pSoftShadows->m_softness = ((float)g_HUD.GetSlider(IDC_SOFTNESS)->GetValue())/10.0f

bool g_ShaderCaps[4] = {true, true, true, true};

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
	DXUTSetCallbackD3D9DeviceChanging(ModifyDeviceSettings);

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"SoftShadows 1.0 (DX9) (HLSL)" );
    DXUTCreateDevice(  true, 640, 576 );

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
	g_pSoftShadows = new SoftShadows();
	g_pSoftShadows->m_width = 640, g_pSoftShadows->m_height = 576;

	// Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );

	g_HUD.AddSlider(IDC_SAMPLES,0,0,150,22,1,4,(int)(g_pSoftShadows->m_NumSamples/16.0f));
    g_HUD.AddSlider(IDC_SOFTNESS,0,0,150,22,0,120,(int)(g_pSoftShadows->m_softness*10.0f));
	g_HUD.AddSlider(IDC_DEPTH,0,0,150,22,0,4000,(int)(g_pSoftShadows->m_fDepthBias*1000000.0f));
	g_HUD.AddSlider(IDC_SLOPE,0,0,150,22,0,5000,(int)(g_pSoftShadows->m_fBiasSlope*100.0f));

    CDXUTComboBox *pCombo;
    g_HUD.AddComboBox( IDC_COMBOBOX, 0, 0, 170, 24, 0, false, &pCombo );
    if( pCombo )
    {
        pCombo->SetDropHeight( 50 );
        pCombo->AddItem( L"Hard Shadow", (LPVOID)0 );
        pCombo->AddItem( L"Soft Shadow - PS 2.0", (LPVOID)1);
        pCombo->AddItem( L"Soft Shadow - PS 3.0", (LPVOID)2);
        pCombo->AddItem( L"Show Penumbra", (LPVOID)3);
    }

	TCHAR sz[100];

	_sntprintf( sz, 100, TEXT(DEPTH_TEXT), DEPTH_ALG); sz[99] = 0;
	g_HUD.AddStatic(IDC_DEPTH_STATIC,sz,0,0,140,22);
	_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
	g_HUD.AddStatic(IDC_SLOPE_STATIC,sz,0,0,140,22);
	_sntprintf( sz, 100, TEXT(SAMPLES_TEXT), SAMPLES_ALG); sz[99] = 0;
	g_HUD.AddStatic(IDC_SAMPLES_STATIC,sz,0,0,140,22);
	_sntprintf( sz, 100, TEXT(SOFTNESS_TEXT), SOFTNESS_ALG); sz[99] = 0;
	g_HUD.AddStatic(IDC_SOFTNESS_STATIC,sz,0,0,140,22);

	g_HUD.AddButton(IDC_WIREFRM,TEXT("(W)ireframe"),0,0,125,22,L'W');
	g_HUD.AddButton(IDC_PAUSE, TEXT("(R)otate"),0,0,125,22,L'P');
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support vertex shaders!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support pixel shaders!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support mipmaps!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    
    if(pCaps->MaxSimultaneousTextures < 2)
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support two simultaneous textures!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if(!(pCaps->RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support slope scale depth bias!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? false : true;
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
    TCHAR sz[100];

    // Set the initial shader used based on this device's caps
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(2,0)) {
        g_ShaderCaps[1] = g_ShaderCaps[2] = g_ShaderCaps[3] = false;
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->RemoveItem(1);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->RemoveItem(1);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->RemoveItem(1);
        g_pSoftShadows->SetShader(0);
        ((CDXUTSlider*)g_HUD.GetControl(IDC_SLOPE))->SetValue(200);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->SetSelectedByIndex(0);
        return true;
    } else if(pCaps->PixelShaderVersion < D3DPS_VERSION(3,0)) {
        g_ShaderCaps[2] = g_ShaderCaps[3] = false;
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->RemoveItem(2);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->RemoveItem(2);
        g_pSoftShadows->SetShader(1);
        ((CDXUTSlider*)g_HUD.GetControl(IDC_SLOPE))->SetValue(1600);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->SetSelectedByIndex(1);
        return  true;
    } else {
        g_pSoftShadows->SetShader(2);
        ((CDXUTSlider*)g_HUD.GetControl(IDC_SLOPE))->SetValue(1600);
        ((CDXUTComboBox*)g_HUD.GetControl(IDC_COMBOBOX))->SetSelectedByIndex(2);
    }

	_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
	g_HUD.GetStatic(IDC_SLOPE_STATIC)->SetText(sz);

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
	
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(3.0f, 3.0f, -2.4f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vMin      = D3DXVECTOR3(-1000.0f, 0.0f, -1000.0f);
    D3DXVECTOR3 vMax      = D3DXVECTOR3(1000.0f, 1000.0f, 1000.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
    g_Camera.SetClipToBoundary( true, &vMin, &vMax );
    vFromPt   = D3DXVECTOR3(6.0f, 6.0f, 0.0f);
    vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_pSoftShadows->m_Light.SetViewParams( &vFromPt, &vLookatPt);

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
    float fAspectRatio = pBackBufferSurfaceDesc->Width;
    fAspectRatio /= pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DXToRadian(60.0f), fAspectRatio, 1.0f, 100.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_pSoftShadows->m_Light.SetProjParams( D3DXToRadian(60.0f), fAspectRatio, 1.0f, 15.0f );
    g_pSoftShadows->m_Light.SetWindow( 1024, 1024 );
	g_pSoftShadows->m_width = pBackBufferSurfaceDesc->Width, 
		g_pSoftShadows->m_height = pBackBufferSurfaceDesc->Height;

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 10;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
	g_HUD.GetControl(IDC_WIREFRM)->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24);
	g_HUD.GetControl(IDC_PAUSE)->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24);

	#define C_WIDTH 170
	iY = 30;
	g_HUD.GetControl(IDC_SLOPE)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY));
	g_HUD.GetControl(IDC_SLOPE_STATIC)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
	g_HUD.GetControl(IDC_DEPTH)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
	g_HUD.GetControl(IDC_DEPTH_STATIC)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY
 += 22));
	g_HUD.GetControl(IDC_SAMPLES)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
	g_HUD.GetControl(IDC_SAMPLES_STATIC)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
	g_HUD.GetControl(IDC_SOFTNESS)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
	g_HUD.GetControl(IDC_SOFTNESS_STATIC)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH, pBackBufferSurfaceDesc->Height - (iY += 22));
    g_HUD.GetControl(IDC_COMBOBOX)->SetLocation( pBackBufferSurfaceDesc->Width - C_WIDTH - 5, pBackBufferSurfaceDesc->Height - (iY
 += 22));


    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pSoftShadows->m_Projection = *g_Camera.GetProjMatrix();
    if(FAILED(g_pSoftShadows->ResetDevice( pd3dDevice, pBackBufferSurfaceDesc ))) {
        return E_FAIL;
    }

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
    g_pSoftShadows->m_Light.FrameMove( fElapsedTime );
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
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		g_pSoftShadows->Render(pd3dDevice, fTime, fElapsedTime, 
			g_Camera.GetWorldMatrix(), 
			g_Camera.GetViewMatrix(), 
			g_Camera.GetProjMatrix());

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
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

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
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 to hide help" );
		txtHelper.DrawTextLine( L"Press ESC to quit" );
		txtHelper.DrawTextLine( L"Toggle UI: H" );
		txtHelper.DrawTextLine( L"" );
		txtHelper.DrawTextLine( L"Controls:" );
		txtHelper.DrawTextLine( L"Rotate Light: Left Mouse button\n"
            L"Rotate camera: Right Mouse button\n"
			L"Zoom camera: Mouse wheel scroll\n" );
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

    static bool divert = false;

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    if(divert || (uMsg == WM_LBUTTONDOWN) || (uMsg == WM_LBUTTONUP)) {
        if(uMsg == WM_LBUTTONDOWN) {
            uMsg = WM_RBUTTONDOWN;
            divert = true;
        } else if(uMsg == WM_LBUTTONUP) {
            uMsg = WM_RBUTTONUP;
            divert = false;
        }
        
        g_pSoftShadows->m_Light.HandleMessages( hWnd, uMsg, wParam, lParam );
    } else {
        g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
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
	if(bKeyDown) {
        switch(nChar) {
		case VK_F1:
			{
				g_bShowHelp = !g_bShowHelp;
				break;
			}
		case 'H':
		case 'h':
			{
				if( g_bShowUI = !g_bShowUI )
					for( int i = 0; i < IDC_LAST; i++ )
						g_HUD.GetControl(i)->SetVisible( true );
				else
					for( int i = 0; i < IDC_LAST; i++ )
						g_HUD.GetControl(i)->SetVisible( false );
				break;
			}
        case VK_HOME :
            {
                g_Camera.Reset();
                g_pSoftShadows->m_Light.Reset();

                D3DXVECTOR3 vFromPt   = D3DXVECTOR3(3.0f, 3.0f, -2.4f);
                D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	            g_Camera.SetViewParams( &vFromPt, &vLookatPt);
                vFromPt   = D3DXVECTOR3(6.0f, 6.0f, -2.4f);
                vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	            g_pSoftShadows->m_Light.SetViewParams( &vFromPt, &vLookatPt);

                g_pSoftShadows->m_bWireframe = false;
                g_pSoftShadows->m_bPaused = true;
            }
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
		case IDC_DEPTH:
			{
				TCHAR sz[100];
				_sntprintf( sz, 100, TEXT(DEPTH_TEXT), DEPTH_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_DEPTH_STATIC)->SetText(sz);
				break;
			}
		case IDC_SLOPE:
			{
				TCHAR sz[100];
				_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_SLOPE_STATIC)->SetText(sz);
				break;
			}
		case IDC_SAMPLES:
			{
				TCHAR sz[100];
				_sntprintf( sz, 100, TEXT(SAMPLES_TEXT), SAMPLES_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_SAMPLES_STATIC)->SetText(sz);
                g_pSoftShadows->SetShader(-1);
				break;
			}
		case IDC_SOFTNESS:
			{
				TCHAR sz[100];
				_sntprintf( sz, 100, TEXT(SOFTNESS_TEXT), SOFTNESS_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_SOFTNESS_STATIC)->SetText(sz);
				break;
			}
        case IDC_COMBOBOX:
        {
            DXUTComboBoxItem *pItem = ((CDXUTComboBox*)pControl)->GetSelectedItem();
            if(g_ShaderCaps[(int)pItem->pData]) {
                // Reset the FPS counter
                g_pSoftShadows->m_time = ::timeGetTime()*0.001f;
                g_pSoftShadows->m_startTime = g_pSoftShadows->m_time;
                g_pSoftShadows->m_frame = 0;

                // Set the shader
                g_pSoftShadows->SetShader( (int)pItem->pData );

                // Adjust slope bias based on whether or not we're using soft shadows
                if((int)pItem->pData) {
                    ((CDXUTSlider*)g_HUD.GetControl(IDC_SLOPE))->SetValue(1600);
                } else {
                    ((CDXUTSlider*)g_HUD.GetControl(IDC_SLOPE))->SetValue(200);
                }

                // Change the slope bias display occordingly
                TCHAR sz[100];
				_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_SLOPE_STATIC)->SetText(sz);
            }
            break;
        }
		case IDC_WIREFRM:
			{
				if(g_pSoftShadows->m_bWireframe = !g_pSoftShadows->m_bWireframe)
					g_HUD.GetButton(IDC_WIREFRM)->SetText(TEXT("Un(W)ireframe"));
				else
					g_HUD.GetButton(IDC_WIREFRM)->SetText(TEXT("(W)ireframe"));
				break;
				break;
			}
		case IDC_PAUSE:
			{
				if(g_pSoftShadows->m_bPaused = !g_pSoftShadows->m_bPaused)
					g_HUD.GetButton(IDC_PAUSE)->SetText(TEXT("Stop (R)otation"));
				else
					g_HUD.GetButton(IDC_PAUSE)->SetText(TEXT("(R)otate"));
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
	g_pSoftShadows->LostDevice();
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